#include "GameModel.h"
#include <QTimer>
#include <QRandomGenerator>

GameModel::GameModel(QObject *parent)
    : QObject(parent)
    , m_myPosition(-1)
    , m_currentPlayer(-1)
    , m_biddingPhase(false)
    , m_biddingPlayer(-1)
    , m_firstPlayerIndex(0)
    , m_scoreTeam1(0)
    , m_scoreTeam2(0)
    , m_scoreTotalTeam1(0)
    , m_scoreTotalTeam2(0)
    , m_lastBidAnnonce(Player::ANNONCEINVALIDE)
    , m_lastBidCouleur(Carte::COULEURINVALIDE)
    , m_lastBidderIndex(-1)
    , m_surcoincheAvailable(false)
    , m_surcoincheTimeLeft(0)
    , m_showCoincheAnimation(false)
    , m_showSurcoincheAnimation(false)
    , m_isCoinched(false)
    , m_isSurcoinched(false)
    , m_coinchedByPlayerIndex(-1)
    , m_surcoinchedByPlayerIndex(-1)
    , m_showBeloteAnimation(false)
    , m_showRebeloteAnimation(false)
    , m_showCapotAnimation(false)
    , m_distributionPhase(0)
    , m_playTimeRemaining(15)
    , m_maxPlayTime(15)
    , m_pliBeingCleared(false)
    , m_pliWinnerId(-1)
{
    // Créer les HandModels vides
    m_player0Hand = new HandModel(this);
    m_player1Hand = new HandModel(this);
    m_player2Hand = new HandModel(this);
    m_player3Hand = new HandModel(this);

    // Initialiser les annonces des joueurs (4 joueurs, tous vides au depart)
    for (int i = 0; i < 4; i++) {
        QVariantMap bid;
        bid["bidValue"] = "";
        bid["suitSymbol"] = "";
        bid["isRed"] = false;
        m_playerBids.append(bid);
    }

    // Creer le timer de jeu
    m_playTimer = new QTimer(this);
    m_playTimer->setInterval(1000); // 1 seconde
    connect(m_playTimer, &QTimer::timeout, this, [this]() {
        if (m_playTimeRemaining > 0) {
            m_playTimeRemaining--;
            emit playTimeRemainingChanged();
        } else {
            // Temps ecoule, jouer une carte aleatoire
            m_playTimer->stop();
            playRandomCard();
        }
    });

    qDebug() << "GameModel cree en mode online";
}

GameModel::~GameModel()
{
    // Nettoyer les joueurs
    qDeleteAll(m_onlinePlayers);
    m_onlinePlayers.clear();
    
    qDebug() << "GameModel detruit";
}

HandModel* GameModel::player0Hand() const
{
    return m_player0Hand;
}

HandModel* GameModel::player1Hand() const
{
    return m_player1Hand;
}

HandModel* GameModel::player2Hand() const
{
    return m_player2Hand;
}

HandModel* GameModel::player3Hand() const
{
    return m_player3Hand;
}

int GameModel::myPosition() const
{
    return m_myPosition;
}

int GameModel::playerIndex() const
{
    return m_myPosition;
}

int GameModel::currentPlayer() const
{
    return m_currentPlayer;
}

QString GameModel::currentPlayerName() const
{
    return m_currentPlayerName;
}

QList<QVariant> GameModel::currentPli() const
{
    QList<QVariant> pliList;
    for (const auto &cdp : m_currentPli) {
        QVariantMap map;
        map["playerId"] = cdp.playerId;
        if (cdp.carte) {
            map["value"] = static_cast<int>(cdp.carte->getChiffre());
            map["suit"] = static_cast<int>(cdp.carte->getCouleur());
            // Vérifier si c'est un atout
            map["isAtout"] = (m_lastBidCouleur != Carte::COULEURINVALIDE &&
                             cdp.carte->getCouleur() == m_lastBidCouleur);
        } else {
            map["value"] = -1;
            map["suit"] = -1;
            map["isAtout"] = false;
        }
        pliList.append(map);
    }
    return pliList;
}

bool GameModel::biddingPhase() const
{
    return m_biddingPhase;
}

int GameModel::biddingPlayer() const
{
    return m_biddingPlayer;
}

int GameModel::lastBidValue() const
{
    return static_cast<int>(m_lastBidAnnonce);
}

int GameModel::lastBidderIndex() const
{
    return m_lastBidderIndex;
}

int GameModel::scoreTeam1() const
{
    return m_scoreTeam1;
}

int GameModel::scoreTeam2() const
{
    return m_scoreTeam2;
}

int GameModel::scoreTotalTeam1() const
{
    return m_scoreTotalTeam1;
}

int GameModel::scoreTotalTeam2() const
{
    return m_scoreTotalTeam2;
}

QString GameModel::lastBid() const
{
    if (m_lastBidAnnonce == Player::ANNONCEINVALIDE) return "Aucune annonce";
    if (m_lastBidAnnonce == Player::PASSE) return "Passe";
    
    switch(m_lastBidAnnonce) {
        case Player::QUATREVINGT: return "80";
        case Player::QUATREVINGTDIX: return "90";
        case Player::CENT: return "100";
        case Player::CENTDIX: return "110";
        case Player::CENTVINGT: return "120";
        case Player::CENTTRENTE: return "130";
        case Player::CENTQUARANTE: return "140";
        case Player::CENTCINQUANTE: return "150";
        case Player::CENTSOIXANTE: return "160";
        case Player::CAPOT: return "Capot";
        case Player::GENERALE: return "Générale";
        default: return "?";
    }
}

QString GameModel::lastBidSuit() const
{
    switch(m_lastBidCouleur) {
        case Carte::COEUR: return "♥ Cœur";
        case Carte::CARREAU: return "♦ Carreau";
        case Carte::TREFLE: return "♣ Trèfle";
        case Carte::PIQUE: return "♠ Pique";
        default: return "";
    }
}

int GameModel::lastBidSuitValue() const
{
    return static_cast<int>(m_lastBidCouleur);
}

bool GameModel::surcoincheAvailable() const
{
    return m_surcoincheAvailable;
}

int GameModel::surcoincheTimeLeft() const
{
    return m_surcoincheTimeLeft;
}

bool GameModel::showCoincheAnimation() const
{
    return m_showCoincheAnimation;
}

bool GameModel::showSurcoincheAnimation() const
{
    return m_showSurcoincheAnimation;
}

bool GameModel::isCoinched() const
{
    return m_isCoinched;
}

bool GameModel::isSurcoinched() const
{
    return m_isSurcoinched;
}

int GameModel::coinchedByPlayerIndex() const
{
    return m_coinchedByPlayerIndex;
}

int GameModel::surcoinchedByPlayerIndex() const
{
    return m_surcoinchedByPlayerIndex;
}

bool GameModel::showBeloteAnimation() const
{
    return m_showBeloteAnimation;
}

bool GameModel::showRebeloteAnimation() const
{
    return m_showRebeloteAnimation;
}

bool GameModel::showCapotAnimation() const
{
    return m_showCapotAnimation;
}

QList<QVariant> GameModel::lastPliCards() const
{
    QList<QVariant> result;
    for (const CarteDuPliSauvegardee& cdp : m_lastPliCards) {
        QVariantMap cardInfo;
        cardInfo["playerId"] = cdp.playerId;
        cardInfo["value"] = static_cast<int>(cdp.chiffre);
        cardInfo["suit"] = static_cast<int>(cdp.couleur);
        cardInfo["isWinner"] = cdp.isWinner;
        result.append(cardInfo);
    }
    return result;
}

int GameModel::distributionPhase() const
{
    return m_distributionPhase;
}

QList<QVariant> GameModel::playerBids() const
{
    QList<QVariant> result;
    for (const QVariantMap& bid : m_playerBids) {
        result.append(bid);
    }
    return result;
}

int GameModel::playTimeRemaining() const
{
    return m_playTimeRemaining;
}

int GameModel::maxPlayTime() const
{
    return m_maxPlayTime;
}

int GameModel::dealerPosition() const
{
    // Le dealer est toujours le joueur à droite de celui qui commence (firstPlayerIndex)
    // Si firstPlayerIndex = 0, dealer = 3
    // Si firstPlayerIndex = 1, dealer = 0
    // etc.
    return (m_firstPlayerIndex + 3) % 4;
}

int GameModel::pliWinnerId() const
{
    return m_pliWinnerId;
}

QString GameModel::getPlayerName(int position) const
{
    Player* player = const_cast<GameModel*>(this)->getPlayerByPosition(position);
    if (player) {
        return QString::fromStdString(player->getName());
    }
    return QString("Joueur %1").arg(position + 1);
}

QString GameModel::getPlayerAvatar(int position) const
{
    if (m_playerAvatars.contains(position)) {
        return m_playerAvatars[position];
    }
    return "avataaars1.svg";  // Avatar par défaut
}

void GameModel::setPlayerAvatar(int position, const QString& avatar)
{
    m_playerAvatars[position] = avatar;
    qDebug() << "Avatar défini pour position" << position << ":" << avatar;
}

void GameModel::initOnlineGame(int myPosition, const QJsonArray& myCards, const QJsonArray& opponents, const QString& myPseudo, bool isReconnection)
{
    qDebug() << "Initialisation partie online - Position:" << myPosition << "Pseudo:" << myPseudo << "Reconnexion:" << isReconnection;
    qDebug() << "Nombre de cartes reçues:" << myCards.size();
    qDebug() << "Nombre d'adversaires:" << opponents.size();

    m_myPosition = myPosition;
    m_firstPlayerIndex = 0;  // Le joueur 0 commence la première manche
    emit dealerPositionChanged();  // Initialiser le dealer au démarrage

    // Nettoyer les anciens joueurs si existants
    qDeleteAll(m_onlinePlayers);
    m_onlinePlayers.clear();
    m_playerAvatars.clear();

    // Créer les cartes du joueur local (sans les ajouter encore)
    std::vector<Carte*> myNewCartes;
    for (const QJsonValue& val : myCards) {
        QJsonObject cardObj = val.toObject();
        Carte* carte = new Carte(
            static_cast<Carte::Couleur>(cardObj["suit"].toInt()),
            static_cast<Carte::Chiffre>(cardObj["value"].toInt())
        );
        myNewCartes.push_back(carte);
    }

    // Créer le joueur local avec son pseudo (le serveur a déjà généré un nom pour les invités)
    Player* localPlayer = new Player(myPseudo.toStdString(), std::vector<Carte*>(), myPosition);
    m_onlinePlayers.append(localPlayer);

    // Stocker l'avatar du joueur local (récupéré depuis networkManager)
    // Pour l'instant on utilise un avatar par défaut, il sera mis à jour par le serveur
    m_playerAvatars[myPosition] = "avataaars1.svg";

    qDebug() << "Joueur local cree (main vide pour animation)";

    // Associer à la bonne HandModel selon la position
    HandModel* myHand = getHandModelByPosition(myPosition);
    if (myHand) {
        myHand->setPlayer(localPlayer, true);  // Face visible
        qDebug() << "Main du joueur associee à HandModel position" << myPosition;
    }

    // Créer des joueurs "fantômes" pour les adversaires (mains vides initialement)
    for (const QJsonValue& val : opponents) {
        QJsonObject oppObj = val.toObject();
        int position = oppObj["position"].toInt();
        QString name = oppObj["name"].toString();
        QString avatar = oppObj["avatar"].toString();
        int cardCount = oppObj["cardCount"].toInt(0);  // Nombre de cartes pour cet adversaire (reconnexion)
        if (avatar.isEmpty()) avatar = "avataaars1.svg";

        qDebug() << "Creation adversaire:" << name << "position:" << position << "avatar:" << avatar << "cardCount:" << cardCount;

        Player* opponent = new Player(name.toStdString(), std::vector<Carte*>(), position);
        m_onlinePlayers.append(opponent);

        // Stocker l'avatar de l'adversaire
        m_playerAvatars[position] = avatar;

        HandModel* oppHand = getHandModelByPosition(position);
        if (oppHand) {
            oppHand->setPlayer(opponent, false);  // Face cachée
        }
    }

    // Marquer la distribution comme en cours AVANT de changer biddingPhase
    // pour eviter que l'AnnoncesPanel s'affiche brievement
    m_distributionPhase = 1;
    emit distributionPhaseChanged();

    m_biddingPhase = true;
    m_biddingPlayer = 0; // Supposons que le joueur 0 commence la partie
    m_currentPlayer = m_biddingPlayer;
    emit currentPlayerChanged();
    emit biddingPhaseChanged();
    emit biddingPlayerChanged();

    emit myPositionChanged();

    // Lors d'une reconnexion, ajouter toutes les cartes d'un coup sans animation
    if (isReconnection) {
        qDebug() << "Reconnexion: Ajout immediat de toutes les cartes sans animation";

        // Ajouter toutes les cartes instantanément au joueur local
        Player* localPlayer = getPlayerByPosition(myPosition);
        if (localPlayer) {
            for (Carte* carte : myNewCartes) {
                localPlayer->addCardToHand(carte);
            }
            qDebug() << "Reconnexion: " << myNewCartes.size() << "cartes ajoutees au joueur local";

            // Rafraîchir le HandModel du joueur local
            HandModel* hand = getHandModelByPosition(myPosition);
            if (hand) {
                hand->refresh();
            }
        }

        // Ajouter les cartes fantômes (face cachée) pour les adversaires
        // Utiliser le cardCount envoyé par le serveur pour chaque adversaire
        for (const QJsonValue& val : opponents) {
            QJsonObject oppObj = val.toObject();
            int position = oppObj["position"].toInt();
            int cardCount = oppObj["cardCount"].toInt(0);

            Player* opponent = getPlayerByPosition(position);
            if (opponent && cardCount > 0) {
                // Ajouter le bon nombre de cartes fantômes pour cet adversaire
                for (int i = 0; i < cardCount; i++) {
                    Carte* phantomCard = new Carte(Carte::COEUR, Carte::SEPT);
                    opponent->addCardToHand(phantomCard);
                }

                // Rafraîchir le HandModel de l'adversaire
                HandModel* hand = getHandModelByPosition(position);
                if (hand) {
                    hand->refresh();
                }

                qDebug() << "Reconnexion: " << cardCount << "cartes fantomes ajoutees pour joueur" << position;
            }
        }

        // Pas d'animation, signaler immédiatement que c'est prêt
        m_distributionPhase = 0;
        emit distributionPhaseChanged();
        qDebug() << "Reconnexion: Partie initialisee sans animation";
        emit gameInitialized();
    } else {
        // Animation de distribution 3-2-3 pour une nouvelle partie
        qDebug() << "Animation de distribution 3-2-3 demarree (debut de partie)";

        // Phase 1 : 3 cartes (apres 250ms)
        QTimer::singleShot(250, this, [this, myNewCartes]() {
            distributeCards(0, 3, myNewCartes);

            // Phase 2 : 2 cartes (après 1000ms supplémentaires)
            QTimer::singleShot(1000, this, [this, myNewCartes]() {
                m_distributionPhase = 2;
                emit distributionPhaseChanged();
                distributeCards(3, 5, myNewCartes);

                // Phase 3 : 3 cartes (après 1000ms supplémentaires)
                QTimer::singleShot(1000 , this, [this, myNewCartes]() {
                    m_distributionPhase = 3;
                    emit distributionPhaseChanged();
                    distributeCards(5, 8, myNewCartes);

                    // Fin de distribution (après 1000ms supplémentaires)
                    QTimer::singleShot(1000, this, [this]() {
                        m_distributionPhase = 0;
                        emit distributionPhaseChanged();
                        qDebug() << "Partie initialisee - Distribution terminée";
                        emit gameInitialized();
                    });
                });
            });
        });
    }

    qDebug() << "Partie initialisee avec" << m_onlinePlayers.size() << "joueurs";
}

void GameModel::playCard(int cardIndex)
{
    qDebug() << "Tentative de jouer carte - Position:" << m_myPosition << "Index:" << cardIndex;

    // Vérifier que l'on n'est pas en phase d'annonces
    if (m_biddingPhase) {
        qDebug() << "Impossible de jouer une carte pendant la phase d'annonces";
        return;
    }

    // Vérifier que le pli précédent n'est pas en cours de nettoyage
    if (m_pliBeingCleared) {
        qDebug() << "Le pli precedent est en cours de nettoyage, impossible de jouer";
        return;
    }

    // Vérifier que c'est notre tour
    if (m_currentPlayer != m_myPosition) {
        qDebug() << "Ce n'est pas votre tour! Joueur actuel:" << m_currentPlayer;
        return;
    }

    // Vérifier que l'index est valide
    Player* localPlayer = getPlayerByPosition(m_myPosition);
    if (!localPlayer) {
        qDebug() << "Erreur: joueur local non trouvé";
        return;
    }

    if (cardIndex < 0 || cardIndex >= localPlayer->getMain().size()) {
        qDebug() << "Index de carte invalide:" << cardIndex;
        return;
    }

    // Arreter le timer
    m_playTimer->stop();

    qDebug() << "Carte jouee localement, envoi au serveur";

    // Émettre signal vers NetworkManager pour envoyer au serveur
    emit cardPlayedLocally(cardIndex);
}

void GameModel::playRandomCard()
{
    qDebug() << "Temps ecoule, jeu d'une carte aleatoire";

    // Ne pas jouer de carte si on est en phase d'annonces
    if (m_biddingPhase) {
        qDebug() << "Erreur: tentative de jouer une carte pendant la phase d'annonces";
        return;
    }

    Player* localPlayer = getPlayerByPosition(m_myPosition);
    if (!localPlayer) {
        qDebug() << "Erreur: joueur local non trouve";
        return;
    }

    HandModel* hand = getHandModelByPosition(m_myPosition);
    if (!hand) {
        qDebug() << "Erreur: main du joueur non trouvee";
        return;
    }

    // Trouver toutes les cartes jouables
    QList<int> playableIndices;
    for (int i = 0; i < hand->rowCount(); i++) {
        if (hand->data(hand->index(i), HandModel::IsPlayableRole).toBool()) {
            playableIndices.append(i);
        }
    }

    if (playableIndices.isEmpty()) {
        qDebug() << "Aucune carte jouable trouvee";
        return;
    }

    // Jouer une carte aleatoire parmi les cartes jouables
    int randomIndex = playableIndices[QRandomGenerator::global()->bounded(playableIndices.size())];
    qDebug() << "Carte aleatoire choisie a l'index:" << randomIndex;

    playCard(randomIndex);
}

void GameModel::makeBid(int bidValue, int suitValue)
{
    qDebug() << "GameModel - Enchere locale - Valeur:" << bidValue << "Couleur:" << suitValue;
    
    if (m_biddingPlayer != m_myPosition) {
        qDebug() << "Ce n'est pas votre tour d'annoncer!";
        return;
    }
    
    emit bidMadeLocally(bidValue, suitValue);
}

void GameModel::passBid()
{
    qDebug() << "GameModel - Passe locale";
    emit bidMadeLocally(static_cast<int>(Player::PASSE), 0);
}

void GameModel::coincheBid()
{
    qDebug() << "GameModel - Coinche locale";
    // COINCHE est une annonce spéciale (valeur 12 dans l'enum Player::Annonce)
    emit bidMadeLocally(static_cast<int>(Player::COINCHE), 0);
}

void GameModel::surcoincheBid()
{
    qDebug() << "GameModel - Surcoinche locale";
    // SURCOINCHE est une annonce spéciale (valeur 13 dans l'enum Player::Annonce)
    emit bidMadeLocally(static_cast<int>(Player::SURCOINCHE), 0);

    // Désactiver le bouton surcoinche immédiatement
    m_surcoincheAvailable = false;
    m_surcoincheTimeLeft = 0;
    emit surcoincheAvailableChanged();
    emit surcoincheTimeLeftChanged();
}

void GameModel::forfeit()
{
    qDebug() << "GameModel - Forfeit (abandon de partie)";

    // Arrêter le timer de jeu
    m_playTimer->stop();

    // Émettre signal vers NetworkManager pour envoyer au serveur
    emit forfeitLocally();
}

void GameModel::updateGameState(const QJsonObject& state)
{
    qDebug() << "Mise a jour etat du jeu:" << state;

    // Mettre à jour l'état global
    if (state.contains("currentPlayer")) {
        int newCurrentPlayer = state["currentPlayer"].toInt();
        bool playerChanged = (m_currentPlayer != newCurrentPlayer);

        if (playerChanged) {
            m_currentPlayer = newCurrentPlayer;
            emit currentPlayerChanged();
            qDebug() << "Joueur actuel change:" << m_currentPlayer;
        }

        // Demarrer le timer pour tous les joueurs (pas seulement le joueur local)
        // On reinitialise le timer meme si c'est le meme joueur (cas du gagnant d'un pli)
        if (!m_biddingPhase) {
            m_playTimeRemaining = m_maxPlayTime;
            emit playTimeRemainingChanged();
            m_playTimer->start();
            qDebug() << "Timer de jeu demarre (joueur actuel:" << newCurrentPlayer << ")";
        }
    }

    if (state.contains("currentPlayerName")) {
        m_currentPlayerName = state["currentPlayerName"].toString();
    }

    if (state.contains("biddingPhase")) {
        bool newBiddingPhase = state["biddingPhase"].toBool();
        if (m_biddingPhase != newBiddingPhase) {
            m_biddingPhase = newBiddingPhase;
            emit biddingPhaseChanged();
            qDebug() << "Phase d'annonces:" << m_biddingPhase;

            // Si on passe en phase de jeu, vider le pli actuel et masquer les animations
            if (!m_biddingPhase) {
                m_currentPli.clear();
                emit currentPliChanged();
                qDebug() << "Debut de la phase de jeu!";

                // Réinitialiser les scores de manche SEULEMENT si le message ne contient pas déjà les scores
                // (lors d'une reconnexion, les scores sont envoyés dans le même message)
                if (!state.contains("scoreTeam1") && !state.contains("scoreTeam2")) {
                    m_scoreTeam1 = 0;
                    m_scoreTeam2 = 0;
                    emit scoreTeam1Changed();
                    emit scoreTeam2Changed();
                    qDebug() << "Scores de manche réinitialisés (fin des enchères)";
                } else {
                    qDebug() << "Scores de manche conservés (reconnexion)";
                }

                // Demarrer le timer pour le joueur actuel (quel qu'il soit)
                m_playTimeRemaining = m_maxPlayTime;
                emit playTimeRemainingChanged();
                m_playTimer->start();
                qDebug() << "Timer de jeu demarre (fin des annonces)";


                // Re-trier les cartes avec l'atout en premier (ordre croissant)
                if (m_lastBidCouleur != Carte::COULEURINVALIDE) {
                    // Mettre à jour la couleur d'atout pour tous les HandModels
                    m_player0Hand->setAtoutCouleur(m_lastBidCouleur);
                    m_player1Hand->setAtoutCouleur(m_lastBidCouleur);
                    m_player2Hand->setAtoutCouleur(m_lastBidCouleur);
                    m_player3Hand->setAtoutCouleur(m_lastBidCouleur);
                    qDebug() << "Re-tri des cartes avec atout:" << static_cast<int>(m_lastBidCouleur);
                    Player* localPlayer = getPlayerByPosition(m_myPosition);
                    if (localPlayer) {
                        // Marquer les cartes d'atout avant le tri
                        for (Carte* carte : localPlayer->getMain()) {
                            carte->setAtout(carte->getCouleur() == m_lastBidCouleur);
                        }
                        localPlayer->sortHand();
                        HandModel* hand = getHandModelByPosition(m_myPosition);
                        if (hand) hand->refresh();
                    }
                }

                // Masquer toutes les animations Coinche/Surcoinche
                m_showCoincheAnimation = false;
                m_showSurcoincheAnimation = false;
                emit showCoincheAnimationChanged();
                emit showSurcoincheAnimationChanged();
            }
        }
    }

    if (state.contains("biddingPlayer")) {
        int newBiddingPlayer = state["biddingPlayer"].toInt();
        if (m_biddingPlayer != newBiddingPlayer) {
            m_biddingPlayer = newBiddingPlayer;
            emit biddingPlayerChanged();
        }
    }

    // RECONNEXION: Synchroniser le dealer (firstPlayerIndex)
    if (state.contains("firstPlayerIndex")) {
        int newFirstPlayer = state["firstPlayerIndex"].toInt();
        if (m_firstPlayerIndex != newFirstPlayer) {
            m_firstPlayerIndex = newFirstPlayer;
            emit dealerPositionChanged();
            qDebug() << "Reconnexion: Dealer resynchronisé:" << m_firstPlayerIndex;
        }
    }

    if (state.contains("atout")) {
        Carte::Couleur newAtout = static_cast<Carte::Couleur>(state["atout"].toInt());
        bool atoutChanged = (m_lastBidCouleur != newAtout);
        m_lastBidCouleur = newAtout;

        // Vérifier si on est en mode Tout Atout ou Sans Atout
        bool isToutAtout = state.contains("isToutAtout") ? state["isToutAtout"].toBool() : false;
        bool isSansAtout = state.contains("isSansAtout") ? state["isSansAtout"].toBool() : false;
        qDebug() << "Atout defini:" << static_cast<int>(m_lastBidCouleur)
                 << "Mode Tout Atout:" << isToutAtout
                 << "Mode Sans Atout:" << isSansAtout
                 << "atoutChanged:" << atoutChanged;

        // Si l'atout vient d'être défini ou change, trier et highlighter les cartes
        // En mode TA ou SA, forcer le tri même si atoutChanged est false
        if ((atoutChanged && m_lastBidCouleur != Carte::COULEURINVALIDE) || isToutAtout || isSansAtout) {
            // Mettre à jour la couleur d'atout pour tous les HandModels
            // En mode TA/SA, on passe COULEURINVALIDE pour ne pas highlighter de couleur spécifique
            m_player0Hand->setAtoutCouleur(m_lastBidCouleur);
            m_player1Hand->setAtoutCouleur(m_lastBidCouleur);
            m_player2Hand->setAtoutCouleur(m_lastBidCouleur);
            m_player3Hand->setAtoutCouleur(m_lastBidCouleur);
            qDebug() << "Tri et highlight des cartes avec atout:" << static_cast<int>(m_lastBidCouleur);

            Player* localPlayer = getPlayerByPosition(m_myPosition);
            if (localPlayer) {
                // Marquer les cartes d'atout avant le tri
                if (isToutAtout) {
                    // Mode Tout Atout: toutes les cartes sont des atouts
                    qDebug() << "Mode Tout Atout - Marquage de toutes les cartes comme atout";
                    localPlayer->setAllCardsAsAtout();
                    localPlayer->sortHandToutAtout();
                } else if (isSansAtout) {
                    // Mode Sans Atout: aucune carte n'est atout
                    qDebug() << "Mode Sans Atout - Aucune carte n'est atout";
                    for (Carte* carte : localPlayer->getMain()) {
                        carte->setAtout(false);
                    }
                    localPlayer->sortHandSansAtout();
                } else {
                    // Mode normal: seules les cartes de la couleur d'atout
                    for (Carte* carte : localPlayer->getMain()) {
                        carte->setAtout(carte->getCouleur() == m_lastBidCouleur);
                    }
                    localPlayer->sortHand();
                }
                HandModel* hand = getHandModelByPosition(m_myPosition);
                if (hand) hand->refresh();
            }
        }
    }

    if (state.contains("biddingWinnerId")) {
        int winnerId = state["biddingWinnerId"].toInt();
        qDebug() << "Gagnant des encheres: joueur" << winnerId;
    }

    if (state.contains("biddingWinnerAnnonce")) {
        m_lastBidAnnonce = static_cast<Player::Annonce>(state["biddingWinnerAnnonce"].toInt());
        emit lastBidChanged();
    }

    if (state.contains("scoreTeam1")) {
        int newScore = state["scoreTeam1"].toInt();
        if (m_scoreTeam1 != newScore) {
            m_scoreTeam1 = newScore;
            emit scoreTeam1Changed();
        }
    }

    if (state.contains("scoreTeam2")) {
        int newScore = state["scoreTeam2"].toInt();
        if (m_scoreTeam2 != newScore) {
            m_scoreTeam2 = newScore;
            emit scoreTeam2Changed();
        }
    }

    if (state.contains("scoreTotalTeam1")) {
        int newScore = state["scoreTotalTeam1"].toInt();
        if (m_scoreTotalTeam1 != newScore) {
            m_scoreTotalTeam1 = newScore;
            emit scoreTotalTeam1Changed();
        }
    }

    if (state.contains("scoreTotalTeam2")) {
        int newScore = state["scoreTotalTeam2"].toInt();
        if (m_scoreTotalTeam2 != newScore) {
            m_scoreTotalTeam2 = newScore;
            emit scoreTotalTeam2Changed();
        }
    }

    // Note: La resynchronisation des cartes est maintenant gérée par resyncCards()
    // appelée directement depuis NetworkManager lors de la réception de gameFound avec reconnection=true

    // RECONNEXION: Resynchroniser le pli en cours
    if (state.contains("reconnectionPli")) {
        QJsonArray reconnectionPliArray = state["reconnectionPli"].toArray();
        qDebug() << "Reconnexion: Resynchronisation du pli avec" << reconnectionPliArray.size() << "cartes";

        // Vider le pli actuel
        for (auto& cdp : m_currentPli) {
            delete cdp.carte;
        }
        m_currentPli.clear();

        // Recréer le pli avec les cartes du serveur
        for (const QJsonValue& cardVal : reconnectionPliArray) {
            QJsonObject cardObj = cardVal.toObject();
            int playerIdx = cardObj["playerIndex"].toInt();
            int value = cardObj["value"].toInt();
            int suit = cardObj["suit"].toInt();

            Carte* carte = new Carte(static_cast<Carte::Couleur>(suit),
                                     static_cast<Carte::Chiffre>(value));
            CarteDuPli cdp;
            cdp.playerId = playerIdx;
            cdp.carte = carte;
            m_currentPli.append(cdp);
        }

        qDebug() << "Reconnexion: Pli resynchronisé avec" << m_currentPli.size() << "cartes";
        emit currentPliChanged();
    }

    // RECONNEXION: Resynchroniser les informations d'annonce
    if (state.contains("lastBidderIndex")) {
        m_lastBidderIndex = state["lastBidderIndex"].toInt();
        emit lastBidderIndexChanged();
        qDebug() << "Reconnexion: lastBidderIndex resynchronisé:" << m_lastBidderIndex;
    }

    if (state.contains("lastBidAnnonce")) {
        m_lastBidAnnonce = static_cast<Player::Annonce>(state["lastBidAnnonce"].toInt());
        emit lastBidChanged();
        qDebug() << "Reconnexion: lastBidAnnonce resynchronisé:" << static_cast<int>(m_lastBidAnnonce);

        // RECONNEXION: Reconstruire l'entrée playerBids pour afficher l'indicateur d'annonce
        if (m_lastBidderIndex >= 0 && m_lastBidderIndex < m_playerBids.size()) {
            QVariantMap bid;

            // Convertir l'annonce en valeur textuelle
            int bidValue = 0;
            switch (m_lastBidAnnonce) {
                case Player::QUATREVINGT: bidValue = 80; break;
                case Player::QUATREVINGTDIX: bidValue = 90; break;
                case Player::CENT: bidValue = 100; break;
                case Player::CENTDIX: bidValue = 110; break;
                case Player::CENTVINGT: bidValue = 120; break;
                case Player::CENTTRENTE: bidValue = 130; break;
                case Player::CENTQUARANTE: bidValue = 140; break;
                case Player::CENTCINQUANTE: bidValue = 150; break;
                case Player::CENTSOIXANTE: bidValue = 160; break;
                case Player::CAPOT: bidValue = 250; break;
                case Player::GENERALE: bidValue = 500; break;
                default: bidValue = 0; break;
            }

            bid["bidValue"] = QString::number(bidValue);

            // Déterminer le symbole de la couleur d'atout
            int atout = state.contains("atout") ? state["atout"].toInt() : 0;
            bool isToutAtout = state.contains("isToutAtout") ? state["isToutAtout"].toBool() : false;
            bool isSansAtout = state.contains("isSansAtout") ? state["isSansAtout"].toBool() : false;

            QString suitSymbol = "";
            bool isRed = false;

            if (isToutAtout) {
                suitSymbol = "TA";
                isRed = false;
            } else if (isSansAtout) {
                suitSymbol = "SA";
                isRed = false;
            } else {
                // Convertir la couleur en symbole
                switch (atout) {
                    case 3: suitSymbol = "♥"; isRed = true; break;  // COEUR
                    case 4: suitSymbol = "♣"; isRed = false; break; // TREFLE
                    case 5: suitSymbol = "♦"; isRed = true; break;  // CARREAU
                    case 6: suitSymbol = "♠"; isRed = false; break; // PIQUE
                    default: suitSymbol = ""; break;
                }
            }

            bid["suitSymbol"] = suitSymbol;
            bid["isRed"] = isRed;

            m_playerBids[m_lastBidderIndex] = bid;
            emit playerBidsChanged();

            qDebug() << "Reconnexion: playerBids reconstruit pour joueur" << m_lastBidderIndex
                     << "- bidValue:" << bid["bidValue"].toString()
                     << "suit:" << suitSymbol;
        }
    }

    if (state.contains("isCoinched")) {
        m_isCoinched = state["isCoinched"].toBool();
        emit isCoinchedChanged();
        qDebug() << "Reconnexion: isCoinched resynchronisé:" << m_isCoinched;
    }

    if (state.contains("isSurcoinched")) {
        m_isSurcoinched = state["isSurcoinched"].toBool();
        emit isSurcoinchedChanged();
        qDebug() << "Reconnexion: isSurcoinched resynchronisé:" << m_isSurcoinched;
    }

    if (state.contains("coinchedByPlayerIndex")) {
        m_coinchedByPlayerIndex = state["coinchedByPlayerIndex"].toInt();
        emit coinchedByPlayerIndexChanged();
        qDebug() << "Reconnexion: coinchedByPlayerIndex resynchronisé:" << m_coinchedByPlayerIndex;
    }

    if (state.contains("surcoinchedByPlayerIndex")) {
        m_surcoinchedByPlayerIndex = state["surcoinchedByPlayerIndex"].toInt();
        emit surcoinchedByPlayerIndexChanged();
        qDebug() << "Reconnexion: surcoinchedByPlayerIndex resynchronisé:" << m_surcoinchedByPlayerIndex;
    }

    // Mettre à jour les cartes jouables pour le joueur actuel
    if (state.contains("playableCards") && state.contains("currentPlayer")) {
        int currentPlayer = state["currentPlayer"].toInt();
        QJsonArray playableArray = state["playableCards"].toArray();

        // Convertir en QList<int>
        QList<int> playableIndices;
        for (const QJsonValue& val : playableArray) {
            playableIndices.append(val.toInt());
        }

        qDebug() << "Cartes jouables reçues pour joueur" << currentPlayer << ":" << playableIndices;

        // Mettre à jour le HandModel SEULEMENT si c'est le joueur local
        if (currentPlayer == m_myPosition) {
            HandModel* hand = getHandModelByPosition(currentPlayer);
            if (hand) {
                hand->setPlayableCards(playableIndices);
            }
        }

        // Si c'est le dernier pli (une seule carte en main) et c'est notre tour, jouer automatiquement
        if (currentPlayer == m_myPosition && !m_biddingPhase) {
            Player* localPlayer = getPlayerByPosition(m_myPosition);
            int handSize = localPlayer ? localPlayer->getMain().size() : -1;

            if (localPlayer && handSize == 1) {
                qDebug() << "Dernier pli detecte - Jeu automatique de la dernière carte";
                // Jouer automatiquement après un court délai pour que ce soit visible
                QTimer::singleShot(500, this, [this]() {
                    playCard(0);  // La seule carte restante est à l'index 0
                });
            }
        }
    }
}

void GameModel::receivePlayerAction(int playerIndex, const QString& action, const QVariant& data)
{
    qDebug() << "GameModel::receivePlayerAction - Action recue - Joueur:" << playerIndex << "Action:" << action;
    
    if (action == "playCard") {
        QJsonObject cardData = data.toJsonObject();
        int cardIndex = cardData["index"].toInt();
        int cardValue = cardData["value"].toInt();
        int cardSuit = cardData["suit"].toInt();

        qDebug() << "GameModel::receivePlayerAction - Joueur" << playerIndex
                 << "joue carte - Index:" << cardIndex
                 << "Valeur:" << cardValue << "Couleur:" << cardSuit;

        // Créer la carte à partir des infos reçues du serveur
        Carte* cartePlayed = new Carte(
            static_cast<Carte::Couleur>(cardSuit),
            static_cast<Carte::Chiffre>(cardValue)
        );

        // Ajouter au pli courant
        CarteDuPli cdp;
        cdp.playerId = playerIndex;
        cdp.carte = cartePlayed;
        m_currentPli.append(cdp);

        // Retirer la carte de la main du joueur
        Player* player = getPlayerByPosition(playerIndex);
        if (player && cardIndex >= 0 && cardIndex < player->getMain().size()) {
            player->removeCard(cardIndex);

            // Rafraîchir l'affichage
            HandModel* hand = getHandModelByPosition(playerIndex);
            if (hand) {
                hand->refresh();
            }
        }

        emit currentPliChanged();

        qDebug() << "Carte ajoutee au pli - Total:" << m_currentPli.size() << "cartes";
    } else if (action == "makeBid") {
        QJsonObject bidData = data.toJsonObject();
        Player::Annonce annonce = static_cast<Player::Annonce>(bidData["value"].toInt());
        int suit = bidData["suit"].toInt();

        // Stocker l'annonce du joueur pour l'affichage (sauf Coinche/Surcoinche)
        if (playerIndex >= 0 && playerIndex < 4) {
            QVariantMap bid;
            bid["value"] = static_cast<int>(annonce);
            bid["suit"] = suit;

            // Construire le texte de l'annonce (separe en valeur et symbole)
            QString bidValue = "";
            QString suitSymbol = "";
            if (annonce == Player::PASSE) {
                bidValue = "Passe";
            } else if (annonce != Player::ANNONCEINVALIDE && annonce != Player::COINCHE && annonce != Player::SURCOINCHE) {
                // Convertir la valeur de l'annonce en texte
                switch (annonce) {
                    case Player::QUATREVINGT: bidValue = "80"; break;
                    case Player::QUATREVINGTDIX: bidValue = "90"; break;
                    case Player::CENT: bidValue = "100"; break;
                    case Player::CENTDIX: bidValue = "110"; break;
                    case Player::CENTVINGT: bidValue = "120"; break;
                    case Player::CENTTRENTE: bidValue = "130"; break;
                    case Player::CENTQUARANTE: bidValue = "140"; break;
                    case Player::CENTCINQUANTE: bidValue = "150"; break;
                    case Player::CENTSOIXANTE: bidValue = "160"; break;
                    case Player::CAPOT: bidValue = "Capot"; break;
                    case Player::GENERALE: bidValue = "Generale"; break;
                    default: bidValue = "?"; break;
                }
                // Symbole de la couleur
                switch (suit) {
                    case 3: suitSymbol = QString::fromUtf8("\u2665"); break; // Coeur
                    case 4: suitSymbol = QString::fromUtf8("\u2663"); break; // Trefle
                    case 5: suitSymbol = QString::fromUtf8("\u2666"); break; // Carreau
                    case 6: suitSymbol = QString::fromUtf8("\u2660"); break; // Pique
                    case 7: suitSymbol = "TA"; break; // Tout Atout
                    case 8: suitSymbol = "SA"; break; // Sans Atout
                }
            }
            // Ne pas afficher Coinche/Surcoinche dans l'indicateur
            if (annonce != Player::COINCHE && annonce != Player::SURCOINCHE) {
                bid["bidValue"] = bidValue;
                bid["suitSymbol"] = suitSymbol;
                // Coeur (3) et Carreau (5) sont rouges
                // Trefle (4) et Pique (6) sont noirs
                // TA (7) et SA (8) sont blancs (isRed = false)
                bid["isRed"] = (suit == 3 || suit == 5);
                m_playerBids[playerIndex] = bid;
                emit playerBidsChanged();
            }
        }

        // Mettre a jour l'affichage de la derniere enchere pour l'UI
        if (annonce != Player::PASSE && annonce != Player::COINCHE && annonce != Player::SURCOINCHE) {
            m_lastBidAnnonce = annonce;
            m_lastBidCouleur = static_cast<Carte::Couleur>(suit);
            m_lastBidderIndex = playerIndex;
            emit lastBidChanged();
            emit lastBidderIndexChanged();
            qDebug() << "GameModel::receivePlayerAction - Enchere:" << lastBid() << lastBidSuit() << "par joueur" << playerIndex;
        } else if (annonce == Player::COINCHE) {
            qDebug() << "GameModel::receivePlayerAction - Joueur" << playerIndex << "COINCHE";

            // Marquer que l'annonce actuelle a été coinchée
            m_isCoinched = true;
            m_coinchedByPlayerIndex = playerIndex;
            qDebug() << "GameModel - Coinche detectee! isCoinched=" << m_isCoinched << "coinchedByPlayerIndex=" << m_coinchedByPlayerIndex;
            emit isCoinchedChanged();
            emit coinchedByPlayerIndexChanged();

            // Afficher l'animation Coinche pour tous les joueurs
            m_showCoincheAnimation = true;
            emit showCoincheAnimationChanged();
        } else if (annonce == Player::SURCOINCHE) {
            qDebug() << "GameModel::receivePlayerAction - Joueur" << playerIndex << "SURCOINCHE";

            // Marquer que l'annonce actuelle a été surcoinchée
            m_isSurcoinched = true;
            m_surcoinchedByPlayerIndex = playerIndex;
            emit isSurcoinchedChanged();
            emit surcoinchedByPlayerIndexChanged();

            // Afficher l'animation Surcoinche pour tous les joueurs
            m_showSurcoincheAnimation = true;
            emit showSurcoincheAnimationChanged();
        } else {
            qDebug() << "GameModel::receivePlayerAction - Joueur" << playerIndex << "passe";
        }
    } else if (action == "surcoincheOffer") {
        QJsonObject surcoincheData = data.toJsonObject();
        int timeLeft = surcoincheData["timeLeft"].toInt();

        qDebug() << "GameModel::receivePlayerAction - Offre de Surcoinche! Temps restant:" << timeLeft;

        // Masquer l'animation Coinche
        m_showCoincheAnimation = false;
        emit showCoincheAnimationChanged();

        // Vérifier si je suis dans l'équipe qui a fait l'annonce (et donc peut surcoincher)
        int myTeam = m_myPosition % 2;
        int bidderTeam = m_lastBidderIndex % 2;

        if (myTeam == bidderTeam) {
            // Activer le bouton surcoinche
            m_surcoincheAvailable = true;
            m_surcoincheTimeLeft = timeLeft;
            emit surcoincheAvailableChanged();
            emit surcoincheTimeLeftChanged();
        }
    } else if (action == "surcoincheTimeout") {
        qDebug() << "GameModel::receivePlayerAction - Timeout Surcoinche";

        // Masquer toutes les animations
        m_showCoincheAnimation = false;
        m_showSurcoincheAnimation = false;
        emit showCoincheAnimationChanged();
        emit showSurcoincheAnimationChanged();

        // Désactiver le bouton surcoinche
        m_surcoincheAvailable = false;
        m_surcoincheTimeLeft = 0;
        emit surcoincheAvailableChanged();
        emit surcoincheTimeLeftChanged();
    } else if (action == "surcoincheTimeUpdate") {
        QJsonObject timeData = data.toJsonObject();
        int timeLeft = timeData["timeLeft"].toInt();

        m_surcoincheTimeLeft = timeLeft;
        emit surcoincheTimeLeftChanged();
    } else if (action == "belote") {
        qDebug() << "GameModel::receivePlayerAction - BELOTE annoncée par joueur" << playerIndex;

        // Afficher l'animation Belote pour tous les joueurs
        m_showBeloteAnimation = true;
        emit showBeloteAnimationChanged();

        // Masquer l'animation après 2 secondes
        QTimer::singleShot(2000, this, [this]() {
            m_showBeloteAnimation = false;
            emit showBeloteAnimationChanged();
        });
    } else if (action == "rebelote") {
        qDebug() << "GameModel::receivePlayerAction - REBELOTE annoncée par joueur" << playerIndex;

        // Afficher l'animation Rebelote pour tous les joueurs
        m_showRebeloteAnimation = true;
        emit showRebeloteAnimationChanged();

        // Masquer l'animation après 2 secondes
        QTimer::singleShot(2000, this, [this]() {
            m_showRebeloteAnimation = false;
            emit showRebeloteAnimationChanged();
        });
    } else if (action == "pliFinished") {
        QJsonObject pliData = data.toJsonObject();
        int winnerId = pliData["winnerId"].toInt();
        int scoreMancheTeam1 = pliData["scoreMancheTeam1"].toInt();
        int scoreMancheTeam2 = pliData["scoreMancheTeam2"].toInt();

        qDebug() << "GameModel::receivePlayerAction - Pli terminé, gagnant: joueur" << winnerId;
        qDebug() << "  Scores de manche: Team1 =" << scoreMancheTeam1 << ", Team2 =" << scoreMancheTeam2;

        // Mettre à jour les scores de manche pour affichage dans l'UI
        m_scoreTeam1 = scoreMancheTeam1;
        m_scoreTeam2 = scoreMancheTeam2;
        emit scoreTeam1Changed();
        emit scoreTeam2Changed();

        // Marquer que le pli est en cours de nettoyage pour bloquer les nouvelles cartes
        m_pliBeingCleared = true;

        // Attendre 700ms que la dernière carte arrive avant de lancer l'animation de sortie
        QTimer::singleShot(700, this, [this, winnerId]() {
            m_pliWinnerId = winnerId;
            emit pliWinnerIdChanged();
        });

        // Attendre un peu pour que l'utilisateur voie le pli, puis le nettoyer
        QTimer::singleShot(1500, this, [this, winnerId]() {
            qDebug() << "Nettoyage du pli";

            // Sauvegarder le pli courant comme dernier pli avant de le nettoyer
            // On copie les VALEURS des cartes, pas les pointeurs
            m_lastPliCards.clear();
            for (const CarteDuPli& cdp : m_currentPli) {
                CarteDuPliSauvegardee saved;
                saved.playerId = cdp.playerId;
                saved.chiffre = cdp.carte->getChiffre();
                saved.couleur = cdp.carte->getCouleur();
                saved.isWinner = (cdp.playerId == winnerId);  // Marquer la carte gagnante
                m_lastPliCards.append(saved);
            }
            emit lastPliCardsChanged();

            // Libérer la mémoire des cartes créées pour le pli
            for (const CarteDuPli& cdp : m_currentPli) {
                delete cdp.carte;
            }
            m_currentPli.clear();
            emit currentPliChanged();

            // Réinitialiser le flag pour autoriser de nouveau les cartes à être jouées
            m_pliBeingCleared = false;

            // Réinitialiser le gagnant après l'animation
            m_pliWinnerId = -1;
            emit pliWinnerIdChanged();
        });
    } else if (action == "mancheFinished") {
        QJsonObject scoreData = data.toJsonObject();
        int scoreTotalTeam1 = scoreData["scoreTotalTeam1"].toInt();
        int scoreTotalTeam2 = scoreData["scoreTotalTeam2"].toInt();
        int scoreMancheTeam1 = scoreData["scoreMancheTeam1"].toInt();
        int scoreMancheTeam2 = scoreData["scoreMancheTeam2"].toInt();
        int capotTeam = scoreData["capotTeam"].toInt(0);

        qDebug() << "GameModel::receivePlayerAction - Manche terminee";
        qDebug() << "  Scores de manche finaux: Team1 =" << scoreMancheTeam1 << ", Team2 =" << scoreMancheTeam2;
        qDebug() << "  Scores totaux: Team1 =" << scoreTotalTeam1 << ", Team2 =" << scoreTotalTeam2;

        m_lastPliCards.clear();
        emit lastPliCardsChanged();

        // Mettre à jour les scores de manche avec les points finaux attribués
        m_scoreTeam1 = scoreMancheTeam1;
        m_scoreTeam2 = scoreMancheTeam2;
        emit scoreTeam1Changed();
        emit scoreTeam2Changed();

        // Mettre à jour les scores totaux
        m_scoreTotalTeam1 = scoreTotalTeam1;
        m_scoreTotalTeam2 = scoreTotalTeam2;
        emit scoreTotalTeam1Changed();
        emit scoreTotalTeam2Changed();

        // Afficher l'animation CAPOT si une équipe a fait un capot
        if (capotTeam > 0) {
            qDebug() << "GameModel::receivePlayerAction - CAPOT réalisé par équipe" << capotTeam;
            m_showCapotAnimation = true;
            emit showCapotAnimationChanged();

            // Masquer l'animation après 3 secondes
            QTimer::singleShot(3000, this, [this]() {
                m_showCapotAnimation = false;
                emit showCapotAnimationChanged();
            });
        }

        // Le dealer change automatiquement car il est calculé à partir de biddingPlayer

        // TODO: Afficher une popup ou notification pour les scores de la manche
    } else if (action == "gameOver") {
        QJsonObject gameOverData = data.toJsonObject();
        int winner = gameOverData["winner"].toInt();
        int scoreTeam1 = gameOverData["scoreTeam1"].toInt();
        int scoreTeam2 = gameOverData["scoreTeam2"].toInt();

        qDebug() << "GameModel::receivePlayerAction - Partie terminee!";
        qDebug() << "  Gagnant: Equipe" << winner;
        qDebug() << "  Scores finaux: Team1 =" << scoreTeam1 << ", Team2 =" << scoreTeam2;

        m_scoreTeam1 = scoreTeam1;
        m_scoreTeam2 = scoreTeam2;
        emit scoreTeam1Changed();
        emit scoreTeam2Changed();

        // Emettre le signal de fin de partie
        emit gameOver(winner, scoreTeam1, scoreTeam2);
    } else if (action == "newManche") {
        QJsonObject newMancheData = data.toJsonObject();
        int biddingPlayer = newMancheData["biddingPlayer"].toInt();
        int currentPlayer = newMancheData["currentPlayer"].toInt();
        QJsonArray myCards = newMancheData["myCards"].toArray();

        qDebug() << "GameModel::receivePlayerAction - Nouvelle manche";
        qDebug() << "  Joueur qui commence les encheres:" << biddingPlayer;
        qDebug() << "  Nouvelles cartes:" << myCards.size();

        // Prévenir la distribution en double si une distribution est déjà en cours
        if (m_distributionPhase > 0) {
            qDebug() << "GameModel::receivePlayerAction - Distribution déjà en cours (phase" << m_distributionPhase << "), newManche ignoré";
            return;
        }

        // Le biddingPlayer au début de la manche est le firstPlayerIndex
        m_firstPlayerIndex = biddingPlayer;
        emit dealerPositionChanged();  // Le dealer change avec le nouveau firstPlayerIndex

        // Nettoyer le pli en cours
        for (const CarteDuPli& cdp : m_currentPli) {
            delete cdp.carte;
        }
        m_currentPli.clear();
        emit currentPliChanged();

        // Marquer la distribution comme en cours AVANT de changer biddingPhase
        // pour eviter que l'AnnoncesPanel s'affiche brievement
        m_distributionPhase = 1;
        emit distributionPhaseChanged();

        // Arrêter le timer de jeu (on passe en phase d'annonces)
        m_playTimer->stop();

        // Reinitialiser les encheres
        m_biddingPhase = true;
        m_biddingPlayer = biddingPlayer;
        emit dealerPositionChanged();  // Le dealer change avec le nouveau biddingPlayer
        m_currentPlayer = currentPlayer;
        m_lastBidAnnonce = Player::ANNONCEINVALIDE;
        m_lastBidCouleur = Carte::COULEURINVALIDE;
        m_lastBidderIndex = -1;
        m_isCoinched = false;
        m_isSurcoinched = false;
        m_coinchedByPlayerIndex = -1;
        m_surcoinchedByPlayerIndex = -1;
        emit isCoinchedChanged();
        emit isSurcoinchedChanged();
        emit coinchedByPlayerIndexChanged();
        emit surcoinchedByPlayerIndexChanged();

        // Reinitialiser les annonces de chaque joueur
        for (int i = 0; i < 4; i++) {
            m_playerBids[i]["bidValue"] = "";
            m_playerBids[i]["suitSymbol"] = "";
            m_playerBids[i]["isRed"] = false;
        }
        emit playerBidsChanged();

        emit biddingPhaseChanged();
        emit biddingPlayerChanged();
        emit currentPlayerChanged();
        emit lastBidChanged();
        emit lastBidderIndexChanged();

        // Vider les mains d'abord
        for (int i = 0; i < 4; i++) {
            Player* player = getPlayerByPosition(i);
            if (player) {
                player->clearHand();
                HandModel* hand = getHandModelByPosition(i);
                if (hand) {
                    // Réinitialiser la couleur d'atout pour la nouvelle manche
                    hand->setAtoutCouleur(Carte::COULEURINVALIDE);
                    hand->refresh();
                }
            }
        }

        // Préparer les cartes pour la distribution progressive
        std::vector<Carte*> myNewCartes;
        for (const QJsonValue& val : myCards) {
            QJsonObject cardObj = val.toObject();
            Carte* carte = new Carte(
                static_cast<Carte::Couleur>(cardObj["suit"].toInt()),
                static_cast<Carte::Chiffre>(cardObj["value"].toInt())
            );
            myNewCartes.push_back(carte);
        }

        // Animation de distribution 3-2-3
        qDebug() << "Animation de distribution 3-2-3 démarrée";

        // Phase 1 : 3 cartes (apres 250ms)
    QTimer::singleShot(250, this, [this, myNewCartes]() {
        m_distributionPhase = 1;
        emit distributionPhaseChanged();
        distributeCards(0, 3, myNewCartes);

        // Phase 2 : 2 cartes (après 1000ms supplémentaires)
        QTimer::singleShot(1000, this, [this, myNewCartes]() {
            m_distributionPhase = 2;
            emit distributionPhaseChanged();
            distributeCards(3, 5, myNewCartes);

            // Phase 3 : 3 cartes (après 1000ms supplémentaires)
            QTimer::singleShot(1000 , this, [this, myNewCartes]() {
                m_distributionPhase = 3;
                emit distributionPhaseChanged();
                distributeCards(5, 8, myNewCartes);

                // Fin de distribution (après 1000ms supplémentaires)
                QTimer::singleShot(1000, this, [this]() {
                    m_distributionPhase = 0;
                    emit distributionPhaseChanged();
                    qDebug() << "Partie initialisee - Distribution terminee";
                    emit gameInitialized();
                });
            });
        });
    });

    }
}

void GameModel::distributeCards(int startIdx, int endIdx, const std::vector<Carte*>& myCards, bool includePhantomCards)
{
    qDebug() << "Distribution cartes" << startIdx << "a" << (endIdx - 1) << "- includePhantomCards:" << includePhantomCards;

    Player* localPlayer = getPlayerByPosition(m_myPosition);
    if (!localPlayer) return;

    // Distribuer les cartes une par une avec un délai
    int numCards = endIdx - startIdx;
    for (int cardOffset = 0; cardOffset < numCards; cardOffset++) {
        int cardIndex = startIdx + cardOffset;
        int delay = cardOffset * 250;  // 250ms entre chaque carte

        // Distribuer au joueur local (avec délai)
        QTimer::singleShot(delay, this, [this, localPlayer, cardIndex, myCards]() {
            if (cardIndex < static_cast<int>(myCards.size())) {
                localPlayer->addCardToHand(myCards[cardIndex]);

                HandModel* hand = getHandModelByPosition(m_myPosition);
                if (hand) hand->refresh();
            }
        });

        // Distribuer cartes fantômes aux autres joueurs (avec même délai) - seulement si demandé
        if (includePhantomCards) {
            for (int playerPos = 0; playerPos < 4; playerPos++) {
                if (playerPos == m_myPosition) continue;

                QTimer::singleShot(delay, this, [this, playerPos]() {
                    Player* player = getPlayerByPosition(playerPos);
                    if (player) {
                        Carte* phantomCard = new Carte(Carte::COEUR, Carte::SEPT);
                        player->addCardToHand(phantomCard);

                        HandModel* hand = getHandModelByPosition(playerPos);
                        if (hand) hand->refresh();
                    }
                });
            }
        }
    }

    // Trier les cartes APRÈS la dernière carte de cette phase
    int sortDelay = numCards * 250;  // Après toutes les cartes de cette phase
    QTimer::singleShot(sortDelay, this, [this, localPlayer]() {
        localPlayer->sortHand();
        HandModel* hand = getHandModelByPosition(m_myPosition);
        if (hand) hand->refresh();
    });
}

void GameModel::receiveCardsDealt(const QJsonArray& cards)
{
    qDebug() << "GameModel::receiveCardsDealt - Réception de" << cards.size() << "cartes";

    // Prévenir la distribution en double si une distribution est déjà en cours
    if (m_distributionPhase > 0) {
        qDebug() << "GameModel::receiveCardsDealt - Distribution déjà en cours (phase" << m_distributionPhase << "), ignoré";
        return;
    }

    // Convertir les cartes JSON en vecteur de Carte*
    std::vector<Carte*> myNewCartes;
    for (const QJsonValue& val : cards) {
        QJsonObject cardObj = val.toObject();
        Carte* carte = new Carte(
            static_cast<Carte::Couleur>(cardObj["suit"].toInt()),
            static_cast<Carte::Chiffre>(cardObj["value"].toInt())
        );
        myNewCartes.push_back(carte);
    }

    // Lancer l'animation de distribution 3-2-3
    qDebug() << "Animation de distribution 3-2-3 démarrée pour les cartes reçues";

    // Phase 1 : 3 cartes (apres 250ms)
    QTimer::singleShot(250, this, [this, myNewCartes]() {
        m_distributionPhase = 1;
        emit distributionPhaseChanged();
        distributeCards(0, 3, myNewCartes, false);  // false = pas de cartes fantômes (lobby mode)

        // Phase 2 : 2 cartes (après 1000ms supplémentaires)
        QTimer::singleShot(1000, this, [this, myNewCartes]() {
            m_distributionPhase = 2;
            emit distributionPhaseChanged();
            distributeCards(3, 5, myNewCartes, false);  // false = pas de cartes fantômes (lobby mode)

            // Phase 3 : 3 cartes (après 1000ms supplémentaires)
            QTimer::singleShot(1000 , this, [this, myNewCartes]() {
                m_distributionPhase = 3;
                emit distributionPhaseChanged();
                distributeCards(5, 8, myNewCartes, false);  // false = pas de cartes fantômes (lobby mode)

                // Fin de distribution (après 1000ms supplémentaires)
                QTimer::singleShot(1000, this, [this]() {
                    m_distributionPhase = 0;
                    emit distributionPhaseChanged();
                    qDebug() << "Partie initialisee - Distribution terminée (lobby mode)";
                });
            });
        });
    });
}

void GameModel::resyncCards(const QJsonArray& cards)
{
    qDebug() << "GameModel::resyncCards - Resynchronisation de" << cards.size() << "cartes";

    Player* localPlayer = getPlayerByPosition(m_myPosition);
    if (!localPlayer) {
        qDebug() << "resyncCards - Joueur local non trouvé";
        return;
    }

    // Vider complètement la main actuelle
    localPlayer->clearHand();

    // Recréer la main avec les cartes du serveur
    for (const QJsonValue& val : cards) {
        QJsonObject cardObj = val.toObject();
        Carte* carte = new Carte(
            static_cast<Carte::Couleur>(cardObj["suit"].toInt()),
            static_cast<Carte::Chiffre>(cardObj["value"].toInt())
        );
        localPlayer->addCardToHand(carte);
    }

    qDebug() << "resyncCards - Main resynchronisée avec" << localPlayer->getMain().size() << "cartes";

    // Rafraîchir le HandModel pour que QML affiche les bonnes cartes
    HandModel* hand = getHandModelByPosition(m_myPosition);
    if (hand) {
        hand->refresh();
        qDebug() << "resyncCards - HandModel rafraîchi";
    }
}

void GameModel::refreshAllHands()
{
    qDebug() << "Rafraichissement de toutes les mains";
    m_player0Hand->refresh();
    m_player1Hand->refresh();
    m_player2Hand->refresh();
    m_player3Hand->refresh();
}

void GameModel::pauseTimers()
{
    qDebug() << "GameModel::pauseTimers - Arrêt du timer de jeu";
    if (m_playTimer) {
        m_playTimer->stop();
    }
}

void GameModel::resumeTimers()
{
    qDebug() << "GameModel::resumeTimers - Reprise du timer de jeu si nécessaire";
    // Le timer sera redémarré automatiquement par updateGameState si c'est le tour du joueur
}

HandModel* GameModel::getHandModelByPosition(int position)
{
    switch(position) {
        case 0: return m_player0Hand;
        case 1: return m_player1Hand;
        case 2: return m_player2Hand;
        case 3: return m_player3Hand;
        default: return nullptr;
    }
}

Player* GameModel::getPlayerByPosition(int position)
{
    for (Player* player : m_onlinePlayers) {
        if (player->getIndex() == position) {
            return player;
        }
    }
    return nullptr;
}

void GameModel::refreshHand(int playerIndex)
{
    HandModel* hand = getHandModelByPosition(playerIndex);
    if (hand) {
        hand->refresh();
    }
}
