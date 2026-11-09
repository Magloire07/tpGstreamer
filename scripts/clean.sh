
#!/bin/bash
set -e

# Se placer à la racine du projet
cd "$(dirname "$0")/.."

# Supprimer les fichiers générés
rm -rf build artifacts

echo "Nettoyage terminé."