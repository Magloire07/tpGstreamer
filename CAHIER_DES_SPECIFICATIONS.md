##  Répartition proposée des tâches

La numérotation reprend l’ordre des noms figurant dans `CONTRIBUTIONS.md`. La répartition est une proposition ; chaque membre reste responsable de comprendre et de relire l’ensemble du système.

### Membre 1 — Kokou KOMBEDE : application native et architecture

- Définir l’interface entre l’application et les paramètres de configuration.
- Réaliser `monitor_sender` : arguments, validation, création du pipeline, source et habillage vidéo, branche d’aperçu, mode sans affichage, boucle GLib, bus, gestion d’erreur, durée, EOS, libération des ressources et changement dynamique.
- Générer le graphe du pipeline à partir de l’implémentation.
- Rédiger `architecture.md`, notamment le diagramme et les explications des éléments, caps, files d’attente et modes.
- Fournir les éléments techniques nécessaires à `sender.sh`.
- **Exigences principales :** REQ-001 à REQ-004, REQ-007 à REQ-009, REQ-013 et une partie de REQ-010/014.
- **Revue croisée :** le membre 2 relit la branche réseau et le membre 3 vérifie les modes de terminaison et l’enregistrement.

### Membre 2 — Ounissa SADAOUI : transport RTP, réception et diagnostic

- Réaliser et intégrer la branche d’émission RTP/UDP utilisant JPEG et le port de configuration.
- Créer `scripts/receiver.sh` avec un mode graphique et un mode test sans affichage.
- Vérifier expérimentalement l’encapsulation, les caps RTP, la dépayloadisation, le décodage et la réception effective.
- Rédiger `debug-report.md` à partir d’un problème rencontré pendant le développement, avec les preuves exigées par l’énoncé.
- Participer à la collecte des mesures réseau si elles sont pertinentes pour l’expérience.
- **Exigences principales :** REQ-006, REQ-014 pour le rapport de diagnostic et les parties réseau de REQ-011/015.
- **Revue croisée :** le membre 1 relit l’intégration de la branche et le membre 3 vérifie que le récepteur est contrôlable par la vérification automatisée.

### Membre 3 — Aissatou CISS : enregistrement, automatisation et reproductibilité

- Réaliser et valider la branche d’enregistrement selon le conteneur attribué, avec finalisation EOS.
- Créer `scripts/play-recording.sh`, `scripts/build.sh`, `scripts/clean.sh` et `scripts/verify.sh`.
- Automatiser les contrôles de réception, de décodage, d’arrêt propre, de délais, de nettoyage des processus et de fidélité du résumé.
- Rédiger `README.md` et tenir à jour `CONTRIBUTIONS.md` et `AI_USAGE.md`.
- Préparer `scripts/run-experiment.sh`, `experiment.md` et `evidence/measurements.csv`, en coordination avec le membre 2 pour les mesures réseau.
- **Exigences principales :** REQ-005, REQ-010 à REQ-012, REQ-015 à REQ-017.
- **Revue croisée :** les membres 1 et 2 exécutent la vérification depuis un environnement propre et relisent les consignes.

# Cahier des spécifications — Système de vidéosurveillance GStreamer

## 1. Objet et périmètre

Le projet consiste à concevoir, réaliser et vérifier un système de vidéosurveillance avec GStreamer. Le système doit produire une vidéo de test, l’afficher localement, l’enregistrer, la transmettre sur le réseau local et permettre sa réception. L’émetteur doit être une application native en C ou C++.

Le périmètre reprend les six volets de l’énoncé :

1. pipeline GStreamer et scripts en ligne de commande ;
2. documentation de l’architecture ;
3. application native d’émission ;
4. rapport de diagnostic d’un problème rencontré ;
5. expérience technique comparative ;
6. vérification automatisée et reproductible.

La solution doit pouvoir être construite, exécutée et évaluée par l’enseignant à partir du dépôt, sans modification manuelle des scripts ni intervention sur plusieurs terminaux. L’aperçu graphique reste disponible, mais la vérification fonctionnelle doit aussi pouvoir s’exécuter sans interface graphique.

## 2. Configuration de l’équipe

Les valeurs relevées dans [`config/team.conf`](config/team.conf) sont :

| Paramètre | Valeur |
|---|---|
| Motif vidéo | `ball` |
| Résolution | `800 × 450` |
| Fréquence | `10 fps` |
| Port RTP/UDP | `5102` |
| Position du texte superposé | `top-center` |
| Conteneur d’enregistrement | MPEG-TS |
| Codec réseau | JPEG |
| Durée normale | `60 s` |

Ces paramètres doivent être lus depuis le fichier de configuration et ne doivent pas être codés en dur dans l’application. Le fichier doit contenir les valeurs attribuées à l’équipe ; celles ci-dessus sont celles actuellement présentes dans le dépôt et restent à confirmer par rapport à l’affectation officielle.


## 3. Acteurs et scénarios d’utilisation

### Scénario 1 — Évaluer le projet sans interface graphique (priorité P1)

**Étant donné** un environnement disposant des dépendances nécessaires,  
**quand** l’enseignant lance successivement `clean.sh`, `build.sh` et `verify.sh` depuis la racine,  
**alors** le projet est construit, le flux réseau et l’enregistrement sont testés sans fenêtre vidéo, un résumé fidèle est produit et aucun processus lancé par le test ne reste actif.

### Scénario 2 — Observer et enregistrer le flux (priorité P1)

**Étant donné** la configuration de l’équipe,  
**quand** un membre lance l’émetteur en mode normal,  
**alors** il voit le motif configuré avec les informations superposées, peut enregistrer le flux et obtient un fichier lisible après l’arrêt normal de l’application.

### Scénario 3 — Recevoir le flux réseau (priorité P1)

**Étant donné** l’émetteur en fonctionnement et le récepteur lancé sur le port configuré,  
**quand** le flux RTP/UDP est envoyé,  
**alors** le récepteur peut afficher la vidéo ou la décoder en mode test sans affichage.

### Scénario 4 — Diagnostiquer et comparer (priorité P2)

**Étant donné** un problème technique ou deux configurations expérimentales,  
**quand** l’équipe suit les commandes et méthodes documentées,  
**alors** elle peut justifier une cause racine à l’aide de preuves pertinentes et tirer une conclusion comparative reproductible.

## 4. Exigences fonctionnelles

- **REQ-001 — Configuration** : le système doit charger les paramètres attribués depuis `config/team.conf`. Il doit vérifier que les valeurs nécessaires sont présentes et valides et signaler clairement une configuration absente ou invalide.
- **REQ-002 — Production et habillage vidéo** : l’émetteur doit produire le motif, la résolution et la fréquence configurés. Il doit superposer l’identifiant ou le nom des trois membres, une horloge ou durée écoulée ainsi que la résolution et la fréquence.
- **REQ-003 — Répartition du flux** : après le traitement vidéo commun, le flux doit être réparti en trois branches indépendantes au moyen d’un `tee`. Les branches doivent comporter les files d’attente nécessaires afin qu’une branche ne bloque pas inutilement les autres.
- **REQ-004 — Aperçu local et mode sans affichage** : la première branche doit afficher la vidéo dans l’environnement graphique. En mode `--headless`, elle doit utiliser une sortie sans affichage tandis que l’enregistrement et l’émission réseau continuent de fonctionner.
- **REQ-005 — Enregistrement** : la deuxième branche doit écrire un fichier dans le conteneur configuré (MPEG-TS pour la configuration actuelle). À la fin normale, l’émetteur doit envoyer EOS, finaliser le conteneur et produire un fichier décodable. Le script de lecture doit fournir une commande de lecture distincte. L’extension doit correspondre au conteneur, soit `.ts` pour la configuration actuelle.
- **REQ-006 — Émission et réception réseau** : la troisième branche doit encoder la vidéo en JPEG, l’encapsuler au moyen du payloader RTP approprié et l’envoyer en UDP sur `localhost:5102`. Le récepteur doit décapsuler et décoder le flux et proposer un mode d’affichage ainsi qu’un mode test sans affichage.
- **REQ-007 — Application native** : un exécutable `monitor_sender` en C ou C++ doit accepter au minimum les paramètres du motif, de la largeur, de la hauteur, de la fréquence, du port et du fichier de sortie, ainsi que `--duration` et `--headless`. Il doit fournir `--help`, refuser les paramètres obligatoires absents, les nombres invalides et les formats ou motifs non pris en charge, avec un message explicite et un code de sortie non nul en cas d’erreur fatale.
- **REQ-008 — Cycle de vie du pipeline** : l’application doit initialiser GStreamer, démarrer et arrêter le pipeline proprement, traiter la boucle GLib et les messages du bus, signaler les erreurs avec l’élément concerné et les informations de diagnostic disponibles, traiter EOS et libérer ses ressources. À l’expiration de `--duration`, elle doit terminer sans intervention manuelle et retourner zéro si tout s’est bien passé.
- **REQ-009 — Changement dynamique** : pendant l’exécution, au moins une propriété visuelle doit changer toutes les cinq secondes sans reconstruire le pipeline. En mode sans affichage, chaque changement doit être journalisé et vérifiable. La propriété choisie et l’élément concerné doivent être documentés.
- **REQ-010 — Scripts d’utilisation et de maintenance** : le dépôt doit fournir les scripts exécutables `scripts/sender.sh`, `scripts/receiver.sh`, `scripts/play-recording.sh`, `scripts/build.sh`, `scripts/verify.sh` et `scripts/clean.sh`. Lancés depuis la racine, ils doivent afficher des messages utiles et retourner zéro en cas de succès, un code non nul en cas d’échec.
- **REQ-011 — Vérification automatisée** : `scripts/verify.sh` doit vérifier les outils, éléments GStreamer, configuration et fichiers requis ; tester `--help` et le rejet d’arguments invalides ; lancer un récepteur en mode test et l’émetteur sans affichage pour une durée finie ; vérifier qu’un trafic RTP a été reçu, que l’enregistrement existe et est décodable, que EOS a été traité et que l’émetteur s’est arrêté proprement. Il doit conserver les journaux utiles et produire un résumé dont chaque statut correspond à un contrôle réellement effectué.
- **REQ-012 — Fin des vérifications et reproductibilité** : aucune étape automatisée ne doit attendre indéfiniment. En cas de délai dépassé, le script doit arrêter les processus qu’il a lancés, afficher une erreur explicite et échouer. Une seconde exécution de la séquence de nettoyage, construction et vérification doit réussir sans nettoyage manuel.
- **REQ-013 — Documentation de l’architecture** : `architecture.md` doit présenter un diagramme lisible de l’émetteur et de ses trois branches, les principaux éléments GStreamer, les caps vidéo, le codec et la transmission réseau, le codec et le conteneur d’enregistrement, le rôle de chaque file d’attente, la reconstruction du flux reçu, la finalisation par EOS et les différences entre modes normal et test. Le choix de construction du pipeline doit y être justifié.
- **REQ-014 — Preuves techniques** : le dépôt doit contenir un graphe DOT issu du pipeline réalisé, converti en `evidence/pipeline.png`. `debug-report.md` doit décrire un problème réellement rencontré, les symptômes, hypothèses, commandes de diagnostic (au moins deux mécanismes), un extrait de journal pertinent de 20 lignes maximum, la cause racine, sa correction et une hypothèse rejetée étayée.
- **REQ-015 — Expérience comparative** : `experiment.md`, `evidence/measurements.csv` et `scripts/run-experiment.sh` doivent permettre de comparer deux configurations pendant au moins 45 secondes chacune, en ne changeant qu’une variable principale. Le rapport doit expliciter la question, l’hypothèse, les variables contrôlées, les méthodes, les résultats, leurs limites et une conclusion liée au scénario de vidéosurveillance.
- **REQ-016 — Guide et transparence** : `README.md` doit commencer par une section `Quick evaluation` donnant les commandes exactes d’évaluation, les dépendances, les temps prévus, la configuration, les emplacements des artefacts, les commandes d’aperçu, de réception graphique et de lecture, ainsi que les limites connues. `CONTRIBUTIONS.md` doit décrire les contributions, les travaux collaboratifs, les tests et les revues. `AI_USAGE.md` doit déclarer l’usage des outils d’IA, des exemples représentatifs et la validation humaine des résultats.
- **REQ-017 — Organisation des sorties** : les fichiers générés par la construction et les tests doivent rester dans `build/` ou `artifacts/`, sans être mélangés aux sources. La vérification doit enregistrer ses journaux et son résumé sous `artifacts/`.

## 5. Exigences non fonctionnelles

- L’implémentation de l’émetteur doit être en C ou C++.
- Les scripts doivent être exécutables, fonctionner depuis la racine du dépôt et ne demander aucune édition préalable.
- Les erreurs de dépendance, configuration, arguments, construction, transport ou pipeline doivent être signalées explicitement.
- Les instructions et les résultats affichés par les scripts doivent permettre à un tiers de reproduire l’évaluation.
- Les journaux doivent être ciblés et utiles ; le rapport de diagnostic ne doit pas contenir un dump massif de logs.
- Le dépôt ne doit contenir ni identifiants, ni jetons, ni clés privées.

## 6. Entités et artefacts

- **Configuration d’équipe** : paramètres de génération, d’affichage, de transport, d’enregistrement et de durée.
- **Flux vidéo** : vidéo de test avec caps, informations superposées et données réparties entre aperçu, enregistrement et réseau.
- **Enregistrement** : fichier MPEG-TS finalisé et décodable.
- **Vérification** : exécution reproductible produisant des journaux, un enregistrement de test et un résumé de résultats.
- **Preuves** : graphe du pipeline, mesures de l’expérience et éléments du rapport de diagnostic.

## 7. Critères d’acceptation et de réussite

- **SC-001** : depuis un environnement propre, `clean.sh`, `build.sh` et `verify.sh` s’exécutent dans cet ordre sans édition manuelle et réussissent.
- **SC-002** : le motif, la taille, la fréquence, le port, la position d’overlay, le conteneur et la durée utilisés correspondent aux valeurs de `config/team.conf`.
- **SC-003** : un lancement normal affiche la vidéo et génère un enregistrement décodable ; l’arrêt normal traite EOS et finalise le fichier.
- **SC-004** : un lancement en mode sans affichage fonctionne pour une durée limitée ; le test constate une réception RTP effective et un enregistrement valide.
- **SC-005** : `--help` réussit ; les arguments invalides échouent avec un code non nul et un message explicite.
- **SC-006** : au moins un changement dynamique est observable toutes les cinq secondes et journalisé en mode sans affichage.
- **SC-007** : la vérification termine ou échoue explicitement en cas de délai dépassé, ne laisse aucun processus enfant actif et fournit un résumé fidèle.
- **SC-008** : la séquence complète de nettoyage, construction et vérification peut être répétée deux fois sans intervention manuelle.
- **SC-009** : les documents, le graphe, les mesures et les rapports requis existent et décrivent le comportement effectivement livré.



### Travail collectif et dépendances

1. **Démarrage commun :** confirmer les valeurs de `config/team.conf`, la forme de la commande de l’émetteur, les noms de fichiers et le format du résumé de vérification.
2. **Intégration :** le membre 1 fournit l’interface et le pipeline de base ; les membres 2 et 3 intègrent leurs branches réseau et enregistrement sans coder de paramètres divergents.
3. **Vérification croisée :** chaque membre construit et exécute l’ensemble du projet ; les revues des composants majeurs sont notées dans `CONTRIBUTIONS.md`.
4. **Livraison commune :** contrôler que la documentation correspond au dépôt, que les tests fonctionnent deux fois de suite et que les sorties ne contiennent aucun secret.


## 10. Vérification finale avant remise

- [ ] `config/team.conf` correspond aux paramètres attribués.
- [ ] Le projet se construit selon les commandes du README.
- [ ] `monitor_sender --help` fonctionne et les arguments invalides sont rejetés.
- [ ] L’aperçu, le mode sans affichage, la réception réseau et le mode test du récepteur fonctionnent.
- [ ] L’enregistrement est décodable et finalisé par EOS.
- [ ] La durée et les changements dynamiques sont contrôlés et vérifiables.
- [ ] `verify.sh` finit avec un résumé fidèle et sans processus résiduel.
- [ ] Deux exécutions successives de la séquence complète réussissent.
- [ ] Les rapports, mesures, graphe d’architecture et déclarations de contributions/IA sont présents et exacts.
