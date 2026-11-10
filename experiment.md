# Rapport d'expérimentation GStreamer

## 1. Objectif

L'objectif de cette expérience est d'étudier l'influence du nombre d'images par seconde (FPS) sur la taille d'un enregistrement vidéo.

Nous comparons deux configurations : 5 FPS et 15 FPS.

## 2. Hypothèse

Nous supposons qu'une augmentation du nombre d'images par seconde entraîne une augmentation de la taille du fichier vidéo.

## 3. Méthode

Deux enregistrements sont réalisés avec GStreamer en conservant les mêmes paramètres, à l'exception de la fréquence d'images.

| Configuration | Motif | Résolution | FPS | Codec | Conteneur | Durée |
| Test 1        | ball  | 800 × 450  | 5   | H.264 | MPEG-TS   | 46 s  |
| Test 2        | ball  | 800 × 450  | 15  | H.264 | MPEG-TS   | 46 s  |

Les enregistrements sont générés à l'aide du script `scripts/run-experiment.sh`.

Pour chaque configuration, nous mesurons la durée d'exécution et la taille du fichier obtenu.

## 4. Résultats

Les résultats obtenus sont les suivants :

| Indicateur        | 5 FPS           | 15 FPS         |
| Durée mesurée     | 46 s            | 46 s           |
| Taille du fichier | 308 132 octets  | 574 152 octets |

Le passage de 5 à 15 FPS entraîne une augmentation de 266 020 octets, soit environ **86,3 %**.

La taille du fichier augmente donc avec la fréquence d'images, mais pas de manière proportionnelle.

Les mesures sont enregistrées dans `evidence/measurements.csv`.

## 5. Limites

Cette expérience utilise une vidéo artificielle générée par GStreamer.

Les résultats dépendent également du contenu de la vidéo et des paramètres de compression H.264.

Une seule mesure a été réalisée pour chaque configuration. De plus, nous avons mesuré le temps d'exécution et la taille des fichiers, mais pas la consommation CPU ni la qualité visuelle.

L'expérience utilise un pipeline GStreamer indépendant et non le système complet de vidéosurveillance.

## 6. Conclusion

Les résultats confirment notre hypothèse : une fréquence d'images plus élevée augmente la taille du fichier vidéo.

Pour une même durée, l'enregistrement à 15 FPS occupe environ 86 % d'espace supplémentaire par rapport à celui à 5 FPS.

Dans un système de vidéosurveillance, une fréquence de 5 FPS permet donc d'économiser de l'espace de stockage, mais rend les mouvements moins fluides.

Une fréquence de 15 FPS offre une meilleure fluidité temporelle, au prix d'un stockage plus important.

Le choix dépend donc du compromis recherché entre la fluidité de la vidéo et l'espace de stockage disponible.