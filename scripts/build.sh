
#!/bin/bash
set -euo pipefail

# Se placer à la racine du projet
cd "$(dirname "$0")/.."

echo "Début de la compilation..."

# Créer le dossier des résultats
mkdir -p artifacts

# Configurer et compiler le projet
cmake -S . -B build 2>&1 | tee artifacts/build.log
cmake --build build 2>&1 | tee -a artifacts/build.log

echo "Compilation terminée."
