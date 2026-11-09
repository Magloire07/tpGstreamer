# Architecture du pipeline GStreamer

Ce document décrit le pipeline actuellement construit par `monitor_sender`, avec la configuration de l’équipe. Le DOT complet est produit par l’application ; [`evidence/pipeline.png`](evidence/pipeline.png) en donne une représentation simplifiée dont les éléments et les connexions correspondent au graphe observé.

## Diagramme du pipeline actuellement implémenté

<!-- mermaid-checked: no \n, no em-dash/en-dash, no {} in labels, subgraphs are id["label"], arrows are -->|"label"|, all subgraphs closed by end, ids unique -->
~~~mermaid
flowchart TD
    Source["videotestsrc pattern ball"]
    Convert["videoconvert"]
    Caps["capsfilter 800x450 10 fps"]
    Team["textoverlay team names and video settings"]
    Clock["timeoverlay elapsed time"]
    Tee["tee"]

    subgraph Preview["Local preview"]
        PreviewQueue["queue preview-queue"]
        PreviewSink["autovideosink or fakesink"]
    end
    subgraph Recording["Recording branch current placeholder"]
        RecordingQueue["queue recording-queue"]
        RecordingSink["fakesink placeholder"]
    end
    subgraph Network["RTP branch current placeholder"]
        NetworkQueue["queue network-queue"]
        NetworkSink["fakesink placeholder"]
    end

    Source -->|"live test video"| Convert
    Convert -->|"converted raw video"| Caps
    Caps -->|"configured raw video"| Team
    Team -->|"labeled raw video"| Clock
    Clock -->|"timestamped raw video"| Tee
    Tee -->|"preview"| PreviewQueue
    PreviewQueue -->|"preview frames"| PreviewSink
    Tee -->|"recording placeholder"| RecordingQueue
    RecordingQueue -->|"raw frames"| RecordingSink
    Tee -->|"network placeholder"| NetworkQueue
    NetworkQueue -->|"raw frames"| NetworkSink
~~~

## Éléments et caps

| Élément | Rôle dans le pipeline courant |
|---|---|
| `videotestsrc` | Produit le motif attribué (`ball`) comme source vidéo en direct. |
| `videoconvert` | Convertit le format vidéo pour faciliter la négociation entre les éléments. |
| `capsfilter` | Impose les caps `video/x-raw, width=800, height=450, framerate=10/1`. |
| `textoverlay` | Superpose les noms des membres, la résolution et la fréquence. L’alignement est configuré selon `OVERLAY_POSITION`. |
| `timeoverlay` | Superpose le temps écoulé du flux. |
| `tee` | Répartit le flux brut commun vers l’aperçu, l’enregistrement et le réseau. |
| `queue` | Fournit à chaque branche un tampon et un thread distinct afin qu’un sink lent ne bloque pas directement les autres branches. |

Le format de pixel brut est négocié par GStreamer selon les capacités des éléments connectés. La configuration impose la largeur, la hauteur et la fréquence, mais ne fixe pas explicitement un format de pixel.

## Branches et état d’intégration

- **Aperçu local :** `tee` → `preview-queue` → `autovideosink`. Avec `--headless`, le sink devient `fakesink`.
- **Enregistrement :** `tee` → `recording-queue` → `fakesink` temporaire dans la version actuelle. L’encodage, le muxer MPEG-TS, l’écriture du fichier et sa finalisation EOS restent à intégrer ; le graphe présent ne prouve donc pas encore un enregistrement.
- **Réseau :** `tee` → `network-queue` → `fakesink` temporaire dans la version actuelle. L’encodage JPEG, le payloader RTP, l’envoi UDP au port 5102 et le récepteur restent à intégrer ; le graphe présent ne prouve donc pas une transmission.

Quand les branches seront intégrées, la vidéo partagée par le `tee` sera convertie en format comprimé indépendamment dans chacune des branches concernées. L’encodeur compresse l’image ; le payloader RTP fragmente et encapsule le flux comprimé en paquets RTP, sans effectuer lui-même l’encodage. Le récepteur devra retirer l’encapsulation RTP avant de décoder la vidéo.

## Exécution et vérification du graphe

Le programme appelle l’outil de diagnostic DOT de GStreamer quand il atteint l’état `PLAYING` et que `GST_DEBUG_DUMP_DOT_DIR` désigne un répertoire existant. Le graphe est ainsi extrait du pipeline réellement créé, et non reconstruit manuellement.

```bash
mkdir -p evidence
GST_DEBUG_DUMP_DOT_DIR=evidence ./build/monitor_sender \
  --pattern ball --width 800 --height 450 --fps 10 \
  --port 5102 --output build/test.ts --duration 5 --headless
```

Le DOT généré est conservé dans `evidence/` et peut contenir un horodatage ajouté par GStreamer. Avec Graphviz installé, le graphe DOT complet peut être converti ainsi :

```bash
dot -Tpng "$(ls -t evidence/*-monitor-sender.dot | head -n 1)" \
  -o evidence/pipeline.png
```

Le PNG fourni simplifie les pads détaillés du DOT pour rendre les douze éléments et les trois branches lisibles. Le graphe a été capturé en mode headless : le sink de prévisualisation est donc lui aussi un `fakesink`.
