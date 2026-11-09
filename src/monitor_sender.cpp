#include <charconv>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <string_view>

#include <gst/gst.h>

namespace {

struct Options {
    std::string pattern;
    int width = 0;
    int height = 0;
    int fps = 0;
    int port = 0;
    std::string output;
    int duration = 0;
    bool headless = false;
    std::string overlay_position = "top-center";
};

void print_usage(std::ostream& output) {
    output << "Usage: monitor_sender"
           << " --pattern <smpte|ball|snow|zone-plate>"
           << " --width <640|800|960>"
           << " --height <360|450|540>"
           << " --fps <5|10|15>"
           << " --port <5000-5999>"
           << " --output <recording.ts>"
           << " --duration <seconds>"
           << " [--overlay-position <top-left|top-center|bottom-right>]"
           << " [--headless]\n"
           << "\n"
           << "Options:\n"
           << "  --pattern   Video test pattern\n"
           << "  --width     Video width in pixels\n"
           << "  --height    Video height in pixels\n"
           << "  --fps       Video frame rate\n"
           << "  --port      RTP/UDP destination port\n"
           << "  --output    MPEG-TS recording path (.ts)\n"
           << "  --duration  Run duration in seconds\n"
           << "  --overlay-position  Position of the team and video information\n"
           << "  --headless  Disable the local video preview\n"
           << "  --help      Show this help message\n";
}

bool parse_integer(std::string_view text, int& value) {
    if (text.empty()) {
        return false;
    }

    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    return result.ec == std::errc{} && result.ptr == text.data() + text.size();
}

bool fail(const std::string& message) {
    std::cerr << "monitor_sender: error: " << message << '\n';
    return false;
}

bool parse_options(int argc, char* argv[], Options& options) {
    std::map<std::string, std::string> values;

    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "--headless") {
            if (options.headless) {
                return fail("option --headless was provided more than once");
            }
            options.headless = true;
            continue;
        }

        if (argument == "--help") {
            print_usage(std::cout);
            std::exit(EXIT_SUCCESS);
        }

        if (argument != "--pattern" && argument != "--width" &&
            argument != "--height" && argument != "--fps" &&
            argument != "--port" && argument != "--output" &&
            argument != "--duration" && argument != "--overlay-position") {
            return fail("unknown option '" + argument + "'");
        }
        if (i + 1 >= argc || std::string_view(argv[i + 1]).starts_with("--")) {
            return fail("missing value for " + argument);
        }
        if (!values.emplace(argument, argv[++i]).second) {
            return fail("option " + argument + " was provided more than once");
        }
    }

    const char* required[] = {
        "--pattern", "--width", "--height", "--fps",
        "--port", "--output", "--duration"
    };
    for (const char* option : required) {
        if (!values.contains(option)) {
            return fail(std::string("missing required option ") + option);
        }
    }

    options.pattern = values.at("--pattern");
    options.output = values.at("--output");

    if (options.pattern != "smpte" && options.pattern != "ball" &&
        options.pattern != "snow" && options.pattern != "zone-plate") {
        return fail("unsupported pattern '" + options.pattern + "'");
    }
    if (!parse_integer(values.at("--width"), options.width) ||
        !parse_integer(values.at("--height"), options.height)) {
        return fail("width and height must be integers");
    }
    if ((options.width != 640 || options.height != 360) &&
        (options.width != 800 || options.height != 450) &&
        (options.width != 960 || options.height != 540)) {
        return fail("unsupported resolution; use 640x360, 800x450, or 960x540");
    }
    if (!parse_integer(values.at("--fps"), options.fps) ||
        (options.fps != 5 && options.fps != 10 && options.fps != 15)) {
        return fail("unsupported frame rate; use 5, 10, or 15");
    }
    if (!parse_integer(values.at("--port"), options.port) ||
        options.port < 5000 || options.port > 5999) {
        return fail("port must be an integer between 5000 and 5999");
    }
    if (!parse_integer(values.at("--duration"), options.duration) ||
        options.duration <= 0) {
        return fail("duration must be a positive integer number of seconds");
    }
    if (options.duration > std::numeric_limits<int>::max() / options.fps) {
        return fail("duration is too large for the selected frame rate");
    }
    if (options.output.empty() || options.output.ends_with('/') ||
        options.output.size() < 4 ||
        options.output.substr(options.output.size() - 3) != ".ts") {
        return fail("output must be a file path ending in .ts");
    }
    if (values.contains("--overlay-position")) {
        options.overlay_position = values.at("--overlay-position");
    }
    if (options.overlay_position != "top-left" &&
        options.overlay_position != "top-center" &&
        options.overlay_position != "bottom-right") {
        return fail("unsupported overlay position '" + options.overlay_position + "'");
    }

    return true;
}

GstElement* make_element(const char* factory, const char* name) {
    GstElement* element = gst_element_factory_make(factory, name);
    if (element == nullptr) {
        std::cerr << "monitor_sender: error: required GStreamer element '"
                  << factory << "' is unavailable\n";
    }
    return element;
}

bool set_enum_property(GstElement* element, const char* property, const char* nick) {
    GParamSpec* spec = g_object_class_find_property(
        G_OBJECT_GET_CLASS(element), property);
    if (spec == nullptr || !G_IS_PARAM_SPEC_ENUM(spec)) {
        std::cerr << "monitor_sender: error: GStreamer property '"
                  << property << "' is unavailable on '"
                  << GST_ELEMENT_NAME(element) << "'\n";
        return false;
    }

    auto* enum_spec = G_PARAM_SPEC_ENUM(spec);
    const GEnumValue* value = g_enum_get_value_by_nick(enum_spec->enum_class, nick);
    if (value == nullptr) {
        std::cerr << "monitor_sender: error: unsupported value '" << nick
                  << "' for GStreamer property '" << property << "'\n";
        return false;
    }

    g_object_set(element, property, value->value, nullptr);
    return true;
}

bool add_tee_branch(GstElement* pipeline, GstElement* tee, const char* queue_name,
                    GstElement* sink, bool silent) {
    GstElement* queue = make_element("queue", queue_name);
    if (queue == nullptr) {
        return false;
    }

    gst_bin_add_many(GST_BIN(pipeline), queue, sink, nullptr);
    if (!gst_element_link(queue, sink)) {
        std::cerr << "monitor_sender: error: could not link queue '"
                  << queue_name << "' to its sink\n";
        return false;
    }

    GstPad* tee_pad = gst_element_request_pad_simple(tee, "src_%u");
    GstPad* queue_pad = gst_element_get_static_pad(queue, "sink");
    if (tee_pad == nullptr || queue_pad == nullptr ||
        gst_pad_link(tee_pad, queue_pad) != GST_PAD_LINK_OK) {
        std::cerr << "monitor_sender: error: could not connect tee branch '"
                  << queue_name << "'\n";
        if (tee_pad != nullptr) {
            gst_object_unref(tee_pad);
        }
        if (queue_pad != nullptr) {
            gst_object_unref(queue_pad);
        }
        return false;
    }

    gst_object_unref(tee_pad);
    gst_object_unref(queue_pad);
    if (silent) {
        g_object_set(sink, "sync", FALSE, nullptr);
    }
    return true;
}

bool build_pipeline(const Options& options, GstElement*& pipeline) {
    pipeline = gst_pipeline_new("monitor-sender");
    if (pipeline == nullptr) {
        std::cerr << "monitor_sender: error: could not create GStreamer pipeline\n";
        return false;
    }

    GstElement* source = make_element("videotestsrc", "video-source");
    GstElement* convert = make_element("videoconvert", "video-convert");
    GstElement* caps_filter = make_element("capsfilter", "video-caps");
    GstElement* labels = make_element("textoverlay", "team-overlay");
    GstElement* clock = make_element("timeoverlay", "elapsed-time-overlay");
    GstElement* tee = make_element("tee", "video-tee");
    if (source == nullptr || convert == nullptr || caps_filter == nullptr ||
        labels == nullptr || clock == nullptr || tee == nullptr) {
        GstElement* elements[] = {source, convert, caps_filter, labels, clock, tee};
        for (GstElement* element : elements) {
            if (element != nullptr) {
                gst_object_unref(element);
            }
        }
        return false;
    }

    gst_bin_add_many(GST_BIN(pipeline), source, convert, caps_filter,
                     labels, clock, tee, nullptr);

    if (!set_enum_property(source, "pattern", options.pattern.c_str()) ||
        !set_enum_property(labels, "halignment",
                           options.overlay_position == "top-left" ? "left" :
                           options.overlay_position == "bottom-right" ? "right" : "center") ||
        !set_enum_property(labels, "valignment",
                           options.overlay_position == "bottom-right" ? "bottom" : "top") ||
        !set_enum_property(clock, "time-mode", "elapsed-running-time") ||
        !set_enum_property(clock, "halignment", "right") ||
        !set_enum_property(clock, "valignment", "bottom")) {
        return false;
    }

    const int frame_count = options.duration * options.fps;
    g_object_set(source, "is-live", TRUE, "num-buffers", frame_count, nullptr);
    const std::string label_text =
        "Kokou KOMBEDE | Ounissa SADAOUI | Aissatou CISS\n" +
        std::to_string(options.width) + "x" + std::to_string(options.height) +
        " @ " + std::to_string(options.fps) + " fps";
    g_object_set(labels, "text", label_text.c_str(),
                 "font-desc", "Sans, 14", "shaded-background", TRUE, nullptr);
    g_object_set(clock, "font-desc", "Sans, 12",
                 "shaded-background", TRUE, nullptr);

    GstCaps* caps = gst_caps_new_simple(
        "video/x-raw",
        "width", G_TYPE_INT, options.width,
        "height", G_TYPE_INT, options.height,
        "framerate", GST_TYPE_FRACTION, options.fps, 1,
        nullptr);
    g_object_set(caps_filter, "caps", caps, nullptr);
    gst_caps_unref(caps);

    if (!gst_element_link_many(source, convert, caps_filter,
                               labels, clock, tee, nullptr)) {
        std::cerr << "monitor_sender: error: could not link the common video pipeline\n";
        return false;
    }

    GstElement* preview = make_element(
        options.headless ? "fakesink" : "autovideosink", "preview-sink");
    GstElement* recording_placeholder =
        make_element("fakesink", "recording-branch-pending-integration");
    GstElement* network_placeholder =
        make_element("fakesink", "network-branch-pending-integration");
    if (preview == nullptr || recording_placeholder == nullptr ||
        network_placeholder == nullptr) {
        GstElement* sinks[] = {preview, recording_placeholder, network_placeholder};
        for (GstElement* sink : sinks) {
            if (sink != nullptr) {
                gst_object_unref(sink);
            }
        }
        return false;
    }

    if (!add_tee_branch(pipeline, tee, "preview-queue", preview, options.headless) ||
        !add_tee_branch(pipeline, tee, "recording-queue",
                        recording_placeholder, true) ||
        !add_tee_branch(pipeline, tee, "network-queue",
                        network_placeholder, true)) {
        return false;
    }

    if (options.headless) {
        std::cout << "PREVIEW_MODE=headless\n";
    }
    std::cerr << "monitor_sender: note: recording and RTP branches await integration\n";
    return true;
}

int run_pipeline(GstElement* pipeline) {
    GstBus* bus = gst_element_get_bus(pipeline);
    if (bus == nullptr) {
        std::cerr << "monitor_sender: error: could not access the GStreamer bus\n";
        return EXIT_FAILURE;
    }

    const GstStateChangeReturn state = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (state == GST_STATE_CHANGE_FAILURE) {
        std::cerr << "monitor_sender: error: pipeline failed to enter PLAYING state\n";
        gst_object_unref(bus);
        return EXIT_FAILURE;
    }

    GstMessage* message = gst_bus_timed_pop_filtered(
        bus, GST_CLOCK_TIME_NONE,
        static_cast<GstMessageType>(GST_MESSAGE_EOS | GST_MESSAGE_ERROR));
    int result = EXIT_SUCCESS;
    if (message == nullptr) {
        std::cerr << "monitor_sender: error: pipeline stopped without EOS or error\n";
        result = EXIT_FAILURE;
    } else if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_ERROR) {
        GError* error = nullptr;
        gchar* debug = nullptr;
        gst_message_parse_error(message, &error, &debug);
        std::cerr << "monitor_sender: error: "
                  << GST_OBJECT_NAME(GST_MESSAGE_SRC(message)) << ": "
                  << (error != nullptr ? error->message : "unknown GStreamer error")
                  << '\n';
        if (debug != nullptr && *debug != '\0') {
            std::cerr << "GStreamer debug: " << debug << '\n';
        }
        g_clear_error(&error);
        g_free(debug);
        result = EXIT_FAILURE;
    }

    if (message != nullptr) {
        gst_message_unref(message);
    }
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(bus);
    return result;
}

}  // namespace

int main(int argc, char* argv[]) {
    Options options;
    if (!parse_options(argc, argv, options)) {
        print_usage(std::cerr);
        return EXIT_FAILURE;
    }

    gst_init(nullptr, nullptr);
    GstElement* pipeline = nullptr;
    if (!build_pipeline(options, pipeline)) {
        if (pipeline != nullptr) {
            gst_object_unref(pipeline);
        }
        return EXIT_FAILURE;
    }

    const int result = run_pipeline(pipeline);
    gst_object_unref(pipeline);
    return result;
}
