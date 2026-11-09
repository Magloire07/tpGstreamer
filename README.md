# tp Gstreamer 

# TP GStreamer — Système de vidéosurveillance

## Quick evaluation

Pour compiler et tester le projet, exécuter les commandes suivantes depuis la racine du dépôt :

```bash
./scripts/clean.sh
./scripts/build.sh
./scripts/verify.sh
```

Le script `verify.sh` effectue les contrôles automatiques et enregistre les résultats dans `artifacts/verification-summary.txt`.

**État actuel :** la compilation fonctionne, mais la vérification complète est encore en développement. Les branches d'enregistrement et de transmission RTP doivent être intégrées à l'application principale.

## 1. Présentation du projet

Ce projet consiste à développer un système de vidéosurveillance simulé avec GStreamer et C++.

Le système doit permettre de :

- Générer une vidéo de test en direct.
- Afficher la vidéo localement.
- Enregistrer la vidéo dans un fichier MPEG-TS.
- Transmettre la vidéo sur le réseau avec RTP/UDP.
- Recevoir et afficher le flux réseau.
- Vérifier automatiquement le fonctionnement du système.

Le projet est développé dans un environnement Linux avec GitHub Codespaces.

## 2. Configuration

Les paramètres de notre équipe sont définis dans `config/team.conf`.

| Paramètre         | Valeur     |

| Motif vidéo       | ball       |
| Résolution        | 800 × 450  |
| Fréquence         | 10 FPS     |
| Port UDP          | 5102       |
| Position du texte | top-center |
| Conteneur         | MPEG-TS    |
| Codec réseau      | JPEG       |
| Durée normale     | 60 secondes|

## 3. Prérequis

Le projet utilise :

- C++ et CMake pour la compilation.
- GStreamer pour le traitement vidéo.
- GitHub Codespaces comme environnement de développement.
- Bash pour les scripts.

Sur Ubuntu, les dépendances peuvent être installées avec :

```bash
sudo apt-get update
sudo apt-get install -y \
  build-essential cmake pkg-config \
  libgstreamer1.0-dev \
  libgstreamer-plugins-base1.0-dev \
  gstreamer1.0-tools \
  gstreamer1.0-plugins-base \
  gstreamer1.0-plugins-good \
  gstreamer1.0-plugins-bad \
  gstreamer1.0-plugins-ugly \
  gstreamer1.0-libav \
  gstreamer1.0-x
```

Le projet a été développé avec GStreamer 1.24.2 dans le Codespace utilisé pour les premiers tests.

Temps indicatifs à confirmer lors de la validation finale : moins d'une minute pour la compilation et moins d'une minute pour la vérification automatique.

## 4. Compilation

Depuis la racine du projet :

```bash
./scripts/build.sh
```

Le script utilise CMake pour compiler l'application C++.

L'exécutable est généré dans :

`build/monitor_sender`

Le journal de compilation est enregistré dans :

`artifacts/build.log`

Pour supprimer les fichiers générés :

```bash
./scripts/clean.sh
```

## 5. Exécution de l'application

### Lancement de l'émetteur sans affichage

```bash
./build/monitor_sender \
  --pattern ball \
  --width 800 \
  --height 450 \
  --fps 10 \
  --port 5102 \
  --output artifacts/test-recording.ts \
  --duration 60 \
  --overlay-position top-center \
  --headless
```

L'option `--headless` permet d'exécuter le programme sans ouvrir de fenêtre vidéo.

Pour demander un aperçu graphique, utiliser la même commande sans l'option `--headless`, dans un environnement disposant d'un affichage graphique.

### Réception du flux réseau

Le script `scripts/receiver.sh` doit permettre de recevoir le flux RTP/UDP en mode graphique ou en mode test.

Cette fonctionnalité est en cours de développement. Les commandes définitives seront ajoutées après son intégration.

### Lecture d'un enregistrement

Pour lire un fichier MPEG-TS :

```bash
./scripts/play-recording.sh artifacts/test-recording.ts
```

Pour vérifier le décodage sans affichage :

```bash
./scripts/play-recording.sh artifacts/test-recording.ts --test
```

Ces commandes nécessitent qu'un enregistrement valide soit déjà présent.

## 6. Vérification automatique

Le script suivant permet de lancer les contrôles :

```bash
./scripts/verify.sh
```

Les tests portent notamment sur :

- La disponibilité des dépendances.
- La présence des fichiers nécessaires.
- Le fonctionnement de l'option `--help`.
- Le rejet des arguments invalides.
- Le lancement de l'émetteur en mode sans affichage.
- La création et le décodage de l'enregistrement.
- La réception RTP/UDP.
- L'arrêt propre du programme.

Les derniers contrôles seront finalisés après l'intégration du système complet.

Les résultats et les journaux sont stockés dans `artifacts/`.

## 7. Expérimentation

Une expérience comparative a été réalisée pour mesurer l'influence de la fréquence d'images sur la taille des fichiers vidéo.

Deux configurations ont été comparées : 5 FPS et 15 FPS.

Pour reproduire l'expérience :

```bash
./scripts/run-experiment.sh
```

Les résultats sont disponibles dans :

- `experiment.md` : méthode, résultats et conclusion.
- `evidence/measurements.csv` : mesures enregistrées.

## 8. Documentation

Les documents du projet sont :

- `architecture.md` : architecture du pipeline GStreamer.
- `debug-report.md` : analyse d'un problème technique.
- `experiment.md` : expérience comparative.
- `CONTRIBUTIONS.md` : répartition du travail et contributions.
- `AI_USAGE.md` : utilisation des outils d'intelligence artificielle.

## 9. Limites actuelles

#A compléter

## 10. Équipe

- **Kokou KOMBEDE** : application C++ et architecture.
- **Ounissa SADAOUI** : transmission RTP/UDP, réception et diagnostic.
- **Aissatou CISS** : enregistrement, automatisation, expérimentation et documentation.

Le détail des contributions est disponible dans `CONTRIBUTIONS.md`.