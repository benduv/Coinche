# Plan de test UI — Coinche de l'Espace v1

## Comment utiliser ce plan

Execute dans l'ordre. Coche chaque case `[ ]` en remplaçant par `[x]`.
Les tests marqués **[CRITIQUE]** bloquent le go-live si échoués.

---

## 0. Pré-conditions

- [ ] Serveur de test démarré (`bash MAJAndBuildServer.sh test`)
- [ ] Config pointée sur `test-remote` dans `qml/Config.qml`
- [ ] 4 comptes de test disponibles : `aaa@aaa.fr`, `bbb@bbb.fr`, `ccc@ccc.fr`, `ddd@ddd.fr`
- [ ] Client lancé en résolution standard 1024×768

---

## 1. SplashScreen & démarrage

| # | Test | Attendu | OK |
|---|---|---|---|
| 1.1 | Lancement à froid (pas de credentials sauvegardés) | SplashScreen s'affiche → transition vers LoginView | [OK] |
| 1.2 | Lancement avec credentials valides sauvegardés | SplashScreen → auto-login → MainMenu directement | [OK] |
| 1.3 | Lancement avec credentials expirés/invalides | SplashScreen → LoginView (pas de crash) | [OK] |
| 1.4 | Lancement sans réseau | Indicateur rouge clignotant visible sur LoginView, boutons désactivés | [OK] |
| 1.5 **[CRITIQUE]** | Reconnexion réseau après démarrage offline | Indicateur passe au vert, boutons s'activent sans redémarrage | [OK] |

---

## 2. Écran d'accueil (LoginView — Welcome)

| # | Test | Attendu | OK |
|---|---|---|---|
| 2.1 | Affichage initial | Titre "COINCHE DE L'ESPACE", 3 boutons visibles (Créer, Se connecter, Invité), indicateur connexion | [OK] |
| 2.2 | Clic "Créer un compte" | Navigation vers écran d'inscription | [OK] |
| 2.3 | Clic "Se connecter" | Navigation vers écran de login | [OK] |
| 2.4 | Clic "Jouer en tant qu'invité" | Navigation vers écran invité | [OK] |
| 2.5 | Boutons désactivés si déconnecté | Opacité réduite, clics sans effet | [OK] |

---

## 3. Inscription **[CRITIQUE]**

| # | Test | Attendu | OK |
|---|---|---|---|
| 3.1 | Affichage formulaire | Champs Pseudo, Email, Mot de passe visibles + sélecteur avatar + bouton retour | [OK] |
| 3.2 | Bouton œil mot de passe | Toggle afficher/masquer le mot de passe | [OK] |
| 3.3 | Saisie avec espaces dans pseudo | Espaces supprimés automatiquement | [OK] |
| 3.4 | Saisie avec espaces dans mot de passe | Espaces supprimés + message d'erreur affiché | [OK] |
| 3.5 | Sélection avatar | Carousel/grille d'avatars, sélection visible | [OK] |
| 3.6 | Soumission formulaire vide | Message d'erreur approprié, pas de crash | [OK] |
| 3.7 | Email invalide | Message d'erreur format email | [OK] |
| 3.8 | Pseudo trop long (>12 chars) | Troncature automatique à 12 | [OK] |
| 3.9 **[CRITIQUE]** | Inscription valide | Transition vers étape vérification email | [OK] |
| 3.10 | Étape vérification email | Affichage de l'email cible, bouton "Renvoyer" visible | [OK] |
| 3.11 | Cooldown renvoi email | Bouton désactivé pendant cooldown, réactivé après | [OK] |
| 3.12 | Email déjà utilisé | Message d'erreur spécifique | [OK] |
| 3.13 | Pseudo déjà utilisé | Message d'erreur spécifique | [OK] |

---

## 4. Connexion **[CRITIQUE]**

| # | Test | Attendu | OK |
|---|---|---|---|
| 4.1 | Affichage formulaire | Champs Email, Mot de passe, bouton retour visibles | [OK] |
| 4.2 **[CRITIQUE]** | Connexion valide | Transition vers MainMenu, nom joueur affiché | [OK] |
| 4.3 | Mauvais mot de passe | Message d'erreur, pas de crash | [ ] |
| 4.4 | Email inconnu | Message d'erreur, pas de crash | [ ] |
| 4.5 | Email non vérifié | Message d'erreur spécifique | [ ] |
| 4.6 | Bouton retour | Retour à l'écran d'accueil | [ ] |
| 4.7 | Mot de passe oublié (si présent) | Flow de réinitialisation s'ouvre | [ ] |

---

## 5. Connexion invité

| # | Test | Attendu | OK |
|---|---|---|---|
| 5.1 | Affichage écran invité | Champ pseudo visible, indication mode limité | [ ] |
| 5.2 **[CRITIQUE]** | Connexion invité valide | Transition vers MainMenu | [ ] |
| 5.3 | Fonctionnalités restreintes en invité | Amis/Stats inaccessibles ou message explicatif | [ ] |

---

## 6. Menu principal (MainMenu)

| # | Test | Attendu | OK |
|---|---|---|---|
| 6.1 | Affichage général | Nom joueur visible, sélecteur mode Coinche/Belote, boutons Jouer/Amis/Stats/Règles/Paramètres | [ ] |
| 6.2 | Sélecteur Coinche | Mode Coinche sélectionné, persisté après redémarrage | [ ] |
| 6.3 | Sélecteur Belote | Mode Belote sélectionné, persisté après redémarrage | [ ] |
| 6.4 | Animation fond (symboles de cartes) | Symboles ♥♣♦♠ animés en arrière-plan | [ ] |
| 6.5 | Toast "matchmaking interrompu" | Visible si retour depuis matchmaking annulé, disparaît après 4s | [ ] |
| 6.6 | Dialog version obsolète | Si serveur refuse → dialog bloquant avec bouton "Mettre à jour" | [ ] |

---

## 7. Matchmaking — mode Coinche **[CRITIQUE]**

| # | Test | Attendu | OK |
|---|---|---|---|
| 7.1 | Clic "Jouer" en mode Coinche | Navigation vers MatchMakingView, cercles animés visibles | [ ] |
| 7.2 | Affichage recherche | Animation 3 cercles, étoiles filantes, bouton Annuler | [ ] |
| 7.3 | Annulation matchmaking | Retour au MainMenu | [ ] |
| 7.4 **[CRITIQUE]** | 4 joueurs connectés → partie trouvée | Transition vers CoincheView, cartes distribuées | [ ] |
| 7.5 | Déconnexion pendant recherche | Reconnexion auto ou message d'erreur clair | [ ] |

---

## 8. Matchmaking — mode Belote **[CRITIQUE]**

| # | Test | Attendu | OK |
|---|---|---|---|
| 8.1 | Clic "Jouer" en mode Belote | Navigation vers MatchMakingView | [ ] |
| 8.2 **[CRITIQUE]** | 4 joueurs → partie trouvée | Transition vers BeloteView (pas CoincheView) | [ ] |

---

## 9. Lobby privé **[CRITIQUE]**

| # | Test | Attendu | OK |
|---|---|---|---|
| 9.1 | Création lobby (hôte) | Code lobby affiché, liste joueurs visible (1/4) | [ ] |
| 9.2 | Rejoindre via code (invité) | Lobby rejoint, liste mise à jour chez tous | [ ] |
| 9.3 | Code invalide | Message d'erreur | [ ] |
| 9.4 | Invitation ami depuis lobby | Toast invitation reçu chez l'ami | [ ] |
| 9.5 | Sélection mode Coinche/Belote dans lobby | Mode visible pour tous | [ ] |
| 9.6 **[CRITIQUE]** | Démarrage partie (4 joueurs) | Transition vers vue correcte selon mode | [ ] |
| 9.7 | Départ d'un joueur avant démarrage | Liste mise à jour, hôte peut toujours démarrer avec bots | [ ] |
| 9.8 | Retour au menu depuis lobby | MainMenu proprement restauré | [ ] |
| 9.9 | Restauration lobby après annulation matchmaking | LobbyRoomView correctement restauré | [ ] |

---

## 10. Partie Coinche — flow complet **[CRITIQUE]**

| # | Test | Attendu | OK |
|---|---|---|---|
| 10.1 | Affichage initial | 8 cartes en main, 4 zones joueurs, scores visibles | [ ] |
| 10.2 | Animation distribution | Cartes distribuées une à une avec animation | [ ] |
| 10.3 | Phase d'enchères — affichage | Panel annonces visible (80 à Générale, Coinche), couleurs disponibles | [ ] |
| 10.4 | Enchère valide | Annonce confirmée, tour passe au suivant | [ ] |
| 10.5 | Coinche / Surcoinche | Boutons disponibles au bon moment, multiplicateur affiché | [ ] |
| 10.6 | Passer à tout le monde | Redémarrage enchères ou fin de manche | [ ] |
| 10.7 **[CRITIQUE]** | Jouer une carte légale | Carte jouée, pli mis à jour | [ ] |
| 10.8 | Jouer une carte illégale | Carte refusée (impossible de la sélectionner ou erreur) | [ ] |
| 10.9 | Fin de pli | Animation ramasse-pli, compteur mis à jour | [ ] |
| 10.10 **[CRITIQUE]** | Fin de manche | Popup GameOver avec scores, bouton continuer | [ ] |
| 10.11 | Capot réussi | Score affiché correctement | [ ] |
| 10.12 | Annonce belote/rebelote | Panel annonces visible et fonctionnel | [ ] |
| 10.13 | Bouton forfait | Popup confirmation, partie abandonnée | [ ] |
| 10.14 | Popup remplacement bot | Si joueur déconnecté, popup visible | [ ] |
| 10.15 | Retour au menu en fin de partie | MainMenu proprement chargé | [ ] |

---

## 11. Partie Belote — flow complet **[CRITIQUE]**

| # | Test | Attendu | OK |
|---|---|---|---|
| 11.1 | Affichage initial BeloteView | Carte retournée visible, 8 cartes en main | [ ] |
| 11.2 | Phase d'enchères round 1 | Boutons Prendre/Passer visibles | [ ] |
| 11.3 | Prendre (round 1) | Carte retournée attribuée, partie démarre | [ ] |
| 11.4 | Tout le monde passe (round 1) | Round 2 démarre, nouvelles options d'enchères | [ ] |
| 11.5 | Prendre (round 2) | Partie démarre avec couleur choisie | [ ] |
| 11.6 | Tout le monde passe (round 2) | Nouvelle donne | [ ] |
| 11.7 **[CRITIQUE]** | Fin de manche Belote | Scores selon règles Belote (82/162 pts), popup correct | [ ] |
| 11.8 | Panel annonces Belote | BeloteAnnoncesPanel distinct du panel Coinche | [ ] |

---

## 12. Amis

| # | Test | Attendu | OK |
|---|---|---|---|
| 12.1 | Affichage FriendsView | Liste amis, champ recherche, demandes en attente | [ ] |
| 12.2 | Recherche joueur | Résultats affichés | [ ] |
| 12.3 | Envoi demande d'ami | Demande envoyée, feedback visuel | [ ] |
| 12.4 | Réception demande (toast) | FriendRequestToast visible | [ ] |
| 12.5 | Accepter/refuser demande | Liste mise à jour | [ ] |
| 12.6 | Invitation à rejoindre un lobby | LobbyInviteToast visible chez le destinataire | [ ] |
| 12.7 | Clic sur profil ami | PlayerStatsPopup s'ouvre | [ ] |

---

## 13. Stats

| # | Test | Attendu | OK |
|---|---|---|---|
| 13.1 | Affichage StatsView | Stats Coinche et Belote séparées, valeurs cohérentes | [ ] |
| 13.2 | Stats invité | Vue bloquée ou message "fonctionnalité réservée aux comptes" | [ ] |

---

## 14. Paramètres

| # | Test | Attendu | OK |
|---|---|---|---|
| 14.1 | Affichage Settings | Sections audio, affichage, compte visibles | [ ] |
| 14.2 | Toggle musique | Musique s'arrête/reprend immédiatement | [ ] |
| 14.3 | Toggle effets sonores | Effets activés/désactivés | [ ] |
| 14.4 | AvatarSelector | Sélection avatar sauvegardée | [ ] |
| 14.5 | DisplaySettings | Changements appliqués | [ ] |
| 14.6 | Changer mot de passe | Flow complet (email vérification) | [ ] |
| 14.7 | Déconnexion | Retour LoginView, credentials effacés | [ ] |

---

## 15. Règles & Politique

| # | Test | Attendu | OK |
|---|---|---|---|
| 15.1 | Affichage Rules | Contenu lisible, scrollable | [ ] |
| 15.2 | Affichage PrivacyPolicy | Contenu lisible, scrollable | [ ] |
| 15.3 | Contact | Formulaire ou lien email visible | [ ] |

---

## 16. Cas limites réseau **[CRITIQUE]**

| # | Test | Attendu | OK |
|---|---|---|---|
| 16.1 **[CRITIQUE]** | Coupure réseau en cours de partie | Reconnexion auto, reprise de la partie en cours | [ ] |
| 16.2 | Déconnexion puis reconnexion au lobby | Lobby restauré (voir 9.9) | [ ] |
| 16.3 | Serveur indisponible au démarrage | Message clair, pas de crash | [ ] |

---

## Résumé go/no-go

| Statut | Critère |
|---|---|
| **GO** | Tous les tests `[CRITIQUE]` sont `[x]` |
| **NO-GO** | Au moins un test `[CRITIQUE]` est `[ ]` |

Les tests non-critiques échoués peuvent être reportés en bug v1.1.

---

*Dernière mise à jour : 2026-04-15*
