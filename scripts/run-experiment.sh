
#!/bin/bash
set -e

# Se placer à la racine du projet
cd "$(dirname "$0")/.."

# Charger la configuration
source config/team.conf

mkdir -p artifacts evidence

# Fichier des résultats
CSV="evidence/measurements.csv"
echo "fps,duration_s,size_bytes" > "$CSV"

echo "=== Expérimentation GStreamer ==="

# Comparer 5 et 15 FPS
for FPS_TEST in 5 15; do

    FILE="artifacts/experiment-${FPS_TEST}fps.ts"
    FRAMES=$((FPS_TEST * 46))

    echo "Test à $FPS_TEST FPS..."

    START=$(date +%s)

    timeout 65s gst-launch-1.0 -e -q \
        videotestsrc is-live=true pattern="$PATTERN" num-buffers="$FRAMES" ! \
        "video/x-raw,width=$WIDTH,height=$HEIGHT,framerate=$FPS_TEST/1" ! \
        videoconvert ! video/x-raw,format=I420 ! \
        x264enc ! h264parse ! mpegtsmux ! \
        filesink location="$FILE"

    DURATION=$(($(date +%s) - START))

    # Vérifier l'enregistrement
    if [ ! -s "$FILE" ] || [ "$DURATION" -lt 45 ]; then
        echo "Erreur : expérience non valide."
        exit 1
    fi

    # Mesurer la taille du fichier
    SIZE=$(stat -c %s "$FILE")

    # Enregistrer les résultats
    echo "$FPS_TEST,$DURATION,$SIZE" >> "$CSV"

    echo "Terminé : $SIZE octets"
done

echo "=== Expérimentation terminée ==="
