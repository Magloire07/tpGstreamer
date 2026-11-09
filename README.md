# tp Gstreamer

## Comportement de l’émetteur

L’émetteur s’arrête automatiquement après la durée demandée en envoyant EOS au
pipeline, attend le message EOS sur le bus, puis arrête le pipeline et libère
ses ressources. En mode `--headless`, il journalise les transitions d’état,
les erreurs et avertissements GStreamer.

Toutes les cinq secondes, il met à jour dynamiquement la propriété `text` de
l’élément GStreamer `team-overlay` sans reconstruire le pipeline. Le texte
conserve les noms des membres, la résolution et la fréquence, et ajoute le
temps écoulé. Les lignes `DYNAMIC_PROPERTY` permettent de vérifier les mises à
jour en mode sans affichage.

## Graphe du pipeline

Pour enregistrer le graphe DOT du pipeline en fonctionnement, lancer
l’émetteur avec un répertoire existant :

```bash
mkdir -p evidence
GST_DEBUG_DUMP_DOT_DIR=evidence ./build/monitor_sender \
  --pattern ball --width 800 --height 450 --fps 10 \
  --port 5102 --output build/test.ts --duration 5 --headless
```

Le programme écrit le graphe DOT au passage à l’état `PLAYING`. Son rendu
Graphviz, avec les éléments, les connexions et les détails de pads, se trouve
dans `evidence/pipeline.png`. Dans l’état courant, les sorties enregistrement
et RTP du graphe sont des sinks temporaires ; elles ne représentent pas encore
une transmission ni un enregistrement fonctionnels.

Avec Graphviz installé, le DOT complet peut être converti en PNG avec :

```bash
dot -Tpng "$(ls -t evidence/*-monitor-sender.dot | head -n 1)" \
  -o evidence/pipeline.png
```
