#include <charconv>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <string_view>

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
            argument != "--duration") {
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
    if (options.output.empty() || options.output.ends_with('/') ||
        options.output.size() < 4 ||
        options.output.substr(options.output.size() - 3) != ".ts") {
        return fail("output must be a file path ending in .ts");
    }

    return true;
}

}  // namespace

int main(int argc, char* argv[]) {
    Options options;
    if (!parse_options(argc, argv, options)) {
        print_usage(std::cerr);
        return EXIT_FAILURE;
    }

    std::cerr << "monitor_sender: error: sender pipeline is not implemented yet\n";
    return EXIT_FAILURE;
}
