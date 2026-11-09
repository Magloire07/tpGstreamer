
# Deuxième version 

#!/bin/bash
set -euo pipefail

# Se placer à la racine du projet
cd "$(dirname "$0")/.."

mkdir -p artifacts

LOG="artifacts/verification.log"
SUMMARY="artifacts/verification-summary.txt"

# Vider les anciens résultats
> "$LOG"
> "$SUMMARY"

# Fonction pour exécuter un test
check() {
    name="$1"
    shift

    if "$@" >> "$LOG" 2>&1; then
        echo "$name=PASS" | tee -a "$SUMMARY"
    else
        echo "$name=FAIL" | tee -a "$SUMMARY"
        echo "OVERALL=FAIL" | tee -a "$SUMMARY"
        exit 1
    fi
}

echo "=== Vérification du projet ==="

# Vérifier les dépendances
check TOOLS bash -c 'command -v cmake && command -v gst-launch-1.0 && command -v gst-inspect-1.0'

# Vérifier les éléments GStreamer de base
check GST_ELEMENTS bash -c 'for e in videotestsrc tee queue fakesink; do gst-inspect-1.0 "$e" >/dev/null || exit 1; done'

# Vérifier les fichiers
check CONFIG test -r config/team.conf
check SOURCE test -s src/monitor_sender.cpp
check EXECUTABLE test -x build/monitor_sender

# Vérifier l'aide
check HELP_OPTION ./build/monitor_sender --help

# Vérifier le rejet d'arguments invalides
reject_invalid() {
    ! ./build/monitor_sender --invalid-option
}

check INVALID_ARGUMENTS reject_invalid


# Lire la configuration du groupe
source config/team.conf

RECORDING="artifacts/test-recording.ts"
SENDER_LOG="artifacts/sender.log"

# Supprimer l'ancien enregistrement
rm -f "$RECORDING"

# Lancer l'émetteur pendant 10 secondes
check SENDER_RUN timeout 25s ./build/monitor_sender \
    --pattern "$PATTERN" \
    --width "$WIDTH" \
    --height "$HEIGHT" \
    --fps "$FPS" \
    --port "$PORT" \
    --output "$RECORDING" \
    --duration 10 \
    --overlay-position "$OVERLAY_POSITION" \
    --headless

# Vérifier le démarrage et l'arrêt
check HEADLESS_MODE grep -q "PREVIEW_MODE=headless" "$LOG"
check EOS_HANDLING grep -q "EOS_HANDLED=PASS" "$LOG"
check CLEAN_TERMINATION grep -q "SENDER_STATE=NULL" "$LOG"

# Vérifier le changement dynamique
check DYNAMIC_PROPERTY grep -q "DYNAMIC_PROPERTY" "$LOG"

# Vérifier l'enregistrement
check RECORDING_CREATED test -s "$RECORDING"

# Vérifier que la vidéo est décodable
check RECORDING_DECODABLE ./scripts/play-recording.sh "$RECORDING" --test

# La vérification RTP sera ajoutée ensuite
echo "OVERALL=INCOMPLETE" | tee -a "$SUMMARY"
exit 1

