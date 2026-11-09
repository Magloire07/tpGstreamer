#permet de lire une vidéo enregistrée par notre application au format MPEG-TS (.ts).

#!/bin/bash
set -e

# Se placer à la racine du projet
cd "$(dirname "$0")/.."

# Fichier à lire (par défaut, celui des tests)
FILE="${1:-artifacts/test-recording.ts}"

# Vérifier que le fichier existe
if [ ! -f "$FILE" ]; then
    echo "Erreur : fichier introuvable : $FILE"
    exit 1
fi

# Lire la vidéo avec GStreamer
gst-launch-1.0 filesrc location="$FILE" ! \
    tsdemux ! decodebin ! videoconvert ! autovideosink
