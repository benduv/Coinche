#include "GameModel.h"
#include <QTimer>

GameModel::GameModel(QObject *parent)
    : QObject(parent)
    , m_myPosition(-1)
    , m_currentPlayer(-1)
    , m_biddingPhase(false)
    , m_biddingPlayer(-1)
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
    , m_showBeloteAnimation(false)
    , m_showRebeloteAnimation(false)
    , m_distributionPhase(0)
{
    // Créer les HandModels vides
    m_player0Hand = new HandModel(this);
    m_player1Hand = new HandModel(this);
    m_player2Hand = new HandModel(this);
    m_player3Hand = new HandModel(this);
    
    qDebug() << "GameModel créé en mode online";
}

GameModel::~GameModel()
{
    // Nettoyer les joueurs
    qDeleteAll(m_onlinePlayers);
    m_onlinePlayers.clear();
    
    qDebug() << "GameModel détruit";
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

bool GameModel::showBeloteAnimation() const
{
    return m_showBeloteAnimation;
}

bool GameModel::showRebeloteAnimation() const
{
    return m_showRebeloteAnimation;
}

QList<QVariant> GameModel::lastPliCards() const
{
    QList<QVariant> result;
    for (const CarteDuPliSauvegardee& cdp : m_lastPliCards) {
        QVariantMap cardInfo;
        cardInfo["playerId"] = cdp.playerId;
        cardInfo["value"] = static_cast<int>(cdp.chiffre);
        cardInfo["suit"] = static_cast<int>(cdp.couleur);
        result.append(cardInfo);
    }
    return result;
}

int GameModel::distributionPhase() const
{
    return m_distributionPhase;
}

void GameModel::initOnlineGame(int myPosition, const QJsonArray& myCards, const QJsonArray& opponents)
{
    qDebug() << "Initialisation partie online - Position:" << myPosition;
    qDebug() << "Nombre de cartes reçues:" << myCards.size();
    qDebug() << "Nombre d'adversaires:" << opponents.size();

    m_myPosition = myPosition;

    // Nettoyer les anciens joueurs si existants
    qDeleteAll(m_onlinePlayers);
    m_onlinePlayers.clear();

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

    // Créer le joueur local avec une main vide initialement
    Player* localPlayer = new Player("Moi", std::vector<Carte*>(), myPosition);
    m_onlinePlayers.append(localPlayer);

    qDebug() << "Joueur local créé (main vide pour animation)";

    // Associer à la bonne HandModel selon la position
    HandModel* myHand = getHandModelByPosition(myPosition);
    if (myHand) {
        myHand->setPlayer(localPlayer, true);  // Face visible
        qDebug() << "Main du joueur associée à HandModel position" << myPosition;
    }

    // Créer des joueurs "fantômes" pour les adversaires (mains vides initialement)
    for (const QJsonValue& val : opponents) {
        QJsonObject oppObj = val.toObject();
        int position = oppObj["position"].toInt();
        QString name = oppObj["name"].toString();

        qDebug() << "Création adversaire:" << name << "position:" << position;

        Player* opponent = new Player(name.toStdString(), std::vector<Carte*>(), position);
        m_onlinePlayers.append(opponent);

        HandModel* oppHand = getHandModelByPosition(position);
        if (oppHand) {
            oppHand->setPlayer(opponent, false);  // Face cachée
        }
    }

    m_biddingPhase = true;
    m_biddingPlayer = 0; // Supposons que le joueur 0 commence  la partie
    m_currentPlayer = m_biddingPlayer;
    emit currentPlayerChanged();
    emit biddingPhaseChanged();
    emit biddingPlayerChanged();

    emit myPositionChanged();

    // Animation de distribution 3-2-3
    qDebug() << "Animation de distribution 3-2-3 démarrée (debut de partie)";

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
                    qDebug() << "Partie initialisée - Distribution terminée";
                    emit gameInitialized();
                });
            });
        });
    });

    qDebug() << "Partie initialisée avec" << m_onlinePlayers.size() << "joueurs";
}

void GameModel::playCard(int cardIndex)
{
    qDebug() << "Tentative de jouer carte - Position:" << m_myPosition << "Index:" << cardIndex;
    
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
    
    qDebug() << "Carte jouée localement, envoi au serveur";
    
    // Émettre signal vers NetworkManager pour envoyer au serveur
    emit cardPlayedLocally(cardIndex);
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

void GameModel::updateGameState(const QJsonObject& state)
{
    qDebug() << "Mise à jour état du jeu:" << state;

    // Mettre à jour l'état global
    if (state.contains("currentPlayer")) {
        int newCurrentPlayer = state["currentPlayer"].toInt();
        if (m_currentPlayer != newCurrentPlayer) {
            m_currentPlayer = newCurrentPlayer;
            emit currentPlayerChanged();
            qDebug() << "Joueur actuel changé:" << m_currentPlayer;
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
                qDebug() << "Début de la phase de jeu!";

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
                        //localPlayer->sortHandWithAtout(m_lastBidCouleur);
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

    if (state.contains("atout")) {
        m_lastBidCouleur = static_cast<Carte::Couleur>(state["atout"].toInt());
        qDebug() << "Atout défini:" << static_cast<int>(m_lastBidCouleur);
    }

    if (state.contains("biddingWinnerId")) {
        int winnerId = state["biddingWinnerId"].toInt();
        qDebug() << "Gagnant des enchères: joueur" << winnerId;
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

        // Mettre à jour le HandModel du joueur actuel
        HandModel* hand = getHandModelByPosition(currentPlayer);
        if (hand) {
            hand->setPlayableCards(playableIndices);
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

        // Mettre à jour l'affichage de la dernière enchère pour l'UI
        if (annonce != Player::PASSE && annonce != Player::COINCHE && annonce != Player::SURCOINCHE) {
            m_lastBidAnnonce = annonce;
            m_lastBidCouleur = static_cast<Carte::Couleur>(bidData["suit"].toInt());
            m_lastBidderIndex = playerIndex;
            emit lastBidChanged();
            emit lastBidderIndexChanged();
            qDebug() << "GameModel::receivePlayerAction - Enchere:" << lastBid() << lastBidSuit() << "par joueur" << playerIndex;
        } else if (annonce == Player::COINCHE) {
            qDebug() << "GameModel::receivePlayerAction - Joueur" << playerIndex << "COINCHE";

            // Afficher l'animation Coinche pour tous les joueurs
            m_showCoincheAnimation = true;
            emit showCoincheAnimationChanged();
        } else if (annonce == Player::SURCOINCHE) {
            qDebug() << "GameModel::receivePlayerAction - Joueur" << playerIndex << "SURCOINCHE";

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

        // Attendre un peu pour que l'utilisateur voie le pli, puis le nettoyer
        QTimer::singleShot(2000, this, [this]() {
            qDebug() << "Nettoyage du pli";

            // Sauvegarder le pli courant comme dernier pli avant de le nettoyer
            // On copie les VALEURS des cartes, pas les pointeurs
            m_lastPliCards.clear();
            for (const CarteDuPli& cdp : m_currentPli) {
                CarteDuPliSauvegardee saved;
                saved.playerId = cdp.playerId;
                saved.chiffre = cdp.carte->getChiffre();
                saved.couleur = cdp.carte->getCouleur();
                m_lastPliCards.append(saved);
            }
            emit lastPliCardsChanged();

            // Libérer la mémoire des cartes créées pour le pli
            for (const CarteDuPli& cdp : m_currentPli) {
                delete cdp.carte;
            }
            m_currentPli.clear();
            emit currentPliChanged();
        });
    } else if (action == "mancheFinished") {
        QJsonObject scoreData = data.toJsonObject();
        int scoreTotalTeam1 = scoreData["scoreTotalTeam1"].toInt();
        int scoreTotalTeam2 = scoreData["scoreTotalTeam2"].toInt();

        qDebug() << "GameModel::receivePlayerAction - Manche terminee";
        qDebug() << "  Scores totaux: Team1 =" << scoreTotalTeam1 << ", Team2 =" << scoreTotalTeam2;

        // Mettre à jour les scores totaux
        m_scoreTotalTeam1 = scoreTotalTeam1;
        m_scoreTotalTeam2 = scoreTotalTeam2;
        emit scoreTotalTeam1Changed();
        emit scoreTotalTeam2Changed();

        // Réinitialiser les scores de manche (ils seront recalculés au prochain pli)
        m_scoreTeam1 = 0;
        m_scoreTeam2 = 0;
        emit scoreTeam1Changed();
        emit scoreTeam2Changed();

        // TODO: Afficher une popup ou notification pour les scores de la manche
    } else if (action == "gameOver") {
        QJsonObject gameOverData = data.toJsonObject();
        int winner = gameOverData["winner"].toInt();
        int scoreTeam1 = gameOverData["scoreTeam1"].toInt();
        int scoreTeam2 = gameOverData["scoreTeam2"].toInt();

        qDebug() << "GameModel::receivePlayerAction - Partie terminée!";
        qDebug() << "  Gagnant: Équipe" << winner;
        qDebug() << "  Scores finaux: Team1 =" << scoreTeam1 << ", Team2 =" << scoreTeam2;

        m_scoreTeam1 = scoreTeam1;
        m_scoreTeam2 = scoreTeam2;
        emit scoreTeam1Changed();
        emit scoreTeam2Changed();

        // TODO: Afficher un écran de victoire
    } else if (action == "newManche") {
        QJsonObject newMancheData = data.toJsonObject();
        int biddingPlayer = newMancheData["biddingPlayer"].toInt();
        int currentPlayer = newMancheData["currentPlayer"].toInt();
        QJsonArray myCards = newMancheData["myCards"].toArray();

        qDebug() << "GameModel::receivePlayerAction - Nouvelle manche";
        qDebug() << "  Joueur qui commence les enchères:" << biddingPlayer;
        qDebug() << "  Nouvelles cartes:" << myCards.size();

        // Nettoyer le pli en cours
        for (const CarteDuPli& cdp : m_currentPli) {
            delete cdp.carte;
        }
        m_currentPli.clear();
        emit currentPliChanged();

        // Réinitialiser les enchères
        m_biddingPhase = true;
        m_biddingPlayer = biddingPlayer;
        m_currentPlayer = currentPlayer;
        m_lastBidAnnonce = Player::ANNONCEINVALIDE;
        m_lastBidCouleur = Carte::COULEURINVALIDE;
        m_lastBidderIndex = -1;
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
                if (hand) hand->refresh();
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
                    qDebug() << "Partie initialisée - Distribution terminée";
                    emit gameInitialized();
                });
            });
        });
    });

    }
}

void GameModel::distributeCards(int startIdx, int endIdx, const std::vector<Carte*>& myCards)
{
    qDebug() << "Distribution cartes" << startIdx << "à" << (endIdx - 1);

    Player* localPlayer = getPlayerByPosition(m_myPosition);
    if (!localPlayer) return;

    // Distribuer les cartes une par une avec un délai
    int numCards = endIdx - startIdx;
    for (int cardOffset = 0; cardOffset < numCards; cardOffset++) {
        int cardIndex = startIdx + cardOffset;
        int delay = cardOffset * 250;  // 150ms entre chaque carte

        // Distribuer au joueur local (avec délai)
        QTimer::singleShot(delay, this, [this, localPlayer, cardIndex, myCards]() {
            if (cardIndex < static_cast<int>(myCards.size())) {
                localPlayer->addCardToHand(myCards[cardIndex]);
                localPlayer->sortHand();

                HandModel* hand = getHandModelByPosition(m_myPosition);
                if (hand) hand->refresh();
            }
        });

        // Distribuer cartes fantômes aux autres joueurs (avec même délai)
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

void GameModel::refreshAllHands()
{
    qDebug() << "Rafraichissement de toutes les mains";
    m_player0Hand->refresh();
    m_player1Hand->refresh();
    m_player2Hand->refresh();
    m_player3Hand->refresh();
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