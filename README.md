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
