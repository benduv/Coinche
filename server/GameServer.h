#ifndef GAMESERVER_H
#define GAMESERVER_H

#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QSslConfiguration>
#include <QSslCertificate>
#include <QSslKey>
#include <QFile>
#include <QMap>
#include <QQueue>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QRandomGenerator>
#include "Player.h"
#include "Deck.h"
#include "Carte.h"
#include "GameModel.h"
#include "DatabaseManager.h"
#include "SmtpClient.h"
#include "StatsReporter.h"
#include "ScoreCalculator.h"

// Connexion réseau d'un joueur (pas la logique métier)
struct PlayerConnection {
    QWebSocket* socket;
    QString connectionId;      // ID unique WebSocket
    QString playerName;
    QString avatar;            // Avatar du joueur
    int gameRoomId;
    int playerIndex;           // Position dans la partie (0-3)
    QString lobbyPartnerId;    // ID du partenaire de lobby (vide si pas de partenaire)
    bool isAnonymous = false;  // RGPD - droit à l'opposition
};

// Une partie de jeu avec la vraie logique
struct GameRoom {
    int roomId;
    QList<QString> connectionIds;  // IDs des connexions WebSocket actuelles
    QList<QString> originalConnectionIds;  // IDs originaux pour reconnexion
    QList<QString> playerNames;  // Noms des joueurs pour reconnexion
    QList<QString> playerAvatars;  // Avatars des joueurs pour reconnexion
    QString gameState; // "waiting", "bidding", "playing", "finished"

    // Les objets de jeu réels
    std::vector<std::unique_ptr<Player>> players;  // Les 4 joueurs
    std::vector<bool> isBot;  // true si le joueur à cet index est un bot
    Deck deck;

    // État de la partie
    Carte::Couleur couleurAtout = Carte::COULEURINVALIDE;
    bool isToutAtout = false;  // Mode Tout Atout : toutes les cartes sont des atouts
    bool isSansAtout = false;  // Mode Sans Atout : aucune carte n'est atout
    int currentPlayerIndex = 0;
    int biddingPlayer = 0;
    int firstPlayerIndex = 0;  // Joueur qui commence les enchères ET qui jouera en premier

    // Gestion des enchères
    int passedBidsCount = 0;
    Player::Annonce lastBidAnnonce = Player::ANNONCEINVALIDE;
    Carte::Couleur lastBidCouleur = Carte::COULEURINVALIDE;
    int lastBidSuit = 0;  // Couleur originale de l'enchère (3=♥, 4=♣, 5=♦, 6=♠, 7=TA, 8=SA)
    int lastBidderIndex = -1;
    bool coinched = false;  // True si COINCHE a été annoncé
    bool surcoinched = false;  // True si SURCOINCHE a été annoncé
    QTimer* surcoincheTimer = nullptr;  // Timer pour le timeout de surcoinche
    int surcoincheTimeLeft = 0;  // Temps restant en secondes
    int coinchePlayerIndex = -1;  // Index du joueur qui a coinché
    int surcoinchePlayerIndex = -1;  // Index du joueur qui a surcoinché

    // Timer pour détecter les joueurs qui ne répondent pas
    QTimer* turnTimeout = nullptr;  // Timer pour timeout du tour (15 secondes)
    int turnTimeoutGeneration = 0;  // Compteur pour invalider les anciens timeouts

    // Timer pour détecter les joueurs qui ne répondent pas pendant les enchères
    QTimer* bidTimeout = nullptr;  // Timer pour timeout des enchères (15 secondes)
    int bidTimeoutGeneration = 0;  // Compteur pour invalider les anciens timeouts d'enchères

    // Pli en cours
    std::vector<std::pair<int, Carte*>> currentPli;  // pair<playerIndex, carte>
    Carte::Couleur couleurDemandee = Carte::COULEURINVALIDE;
    bool waitingForNextPli = false;  // True pendant l'attente de 1500ms entre les plis

    // Scores
    int scoreTeam1 = 0;  // Équipe 0: Joueurs 0 et 2
    int scoreTeam2 = 0;  // Équipe 1: Joueurs 1 et 3
    int scoreMancheTeam1 = 0;
    int scoreMancheTeam2 = 0;

    // Plis gagnés dans la manche en cours
    std::vector<std::pair<int, Carte*>> plisTeam1;  // Cartes gagnées par équipe 1
    std::vector<std::pair<int, Carte*>> plisTeam2;  // Cartes gagnées par équipe 2

    // Compteur de plis par joueur (pour CAPOT et GENERALE)
    int plisCountPlayer0 = 0;
    int plisCountPlayer1 = 0;
    int plisCountPlayer2 = 0;
    int plisCountPlayer3 = 0;

    // Belote (détectée au début de la phase de jeu)
    bool beloteTeam1 = false;
    bool beloteTeam2 = false;

    // Suivi des cartes de la belote jouées (pour animations)
    bool beloteRoiJoue = false;   // Roi de l'atout joué
    bool beloteDameJouee = false; // Dame de l'atout jouée

    // Tracking des cartes jouées pour l'IA des bots
    // Structure: playedCards[couleur][chiffre] = true si la carte a été jouée
    std::map<Carte::Couleur, std::map<Carte::Chiffre, bool>> playedCards;

    // Initialise le tracking des cartes jouées (à appeler au début de chaque manche)
    void resetPlayedCards() {
        playedCards.clear();
        std::array<Carte::Couleur, 4> couleurs = {Carte::COEUR, Carte::TREFLE, Carte::CARREAU, Carte::PIQUE};
        std::array<Carte::Chiffre, 8> chiffres = {Carte::SEPT, Carte::HUIT, Carte::NEUF, Carte::DIX,
                                                   Carte::VALET, Carte::DAME, Carte::ROI, Carte::AS};
        for (auto couleur : couleurs) {
            for (auto chiffre : chiffres) {
                playedCards[couleur][chiffre] = false;
            }
        }
    }

    // Marque une carte comme jouée
    void markCardAsPlayed(Carte* carte) {
        if (carte) {
            playedCards[carte->getCouleur()][carte->getChiffre()] = true;
        }
    }

    // Vérifie si une carte a été jouée
    bool isCardPlayed(Carte::Couleur couleur, Carte::Chiffre chiffre) const {
        auto itCouleur = playedCards.find(couleur);
        if (itCouleur != playedCards.end()) {
            auto itChiffre = itCouleur->second.find(chiffre);
            if (itChiffre != itCouleur->second.end()) {
                return itChiffre->second;
            }
        }
        return false;
    }

    // Destructeur pour nettoyer les ressources
    ~GameRoom() {
        // Nettoyer les timers pour éviter les fuites mémoire
        if (turnTimeout) {
            turnTimeout->stop();
            delete turnTimeout;
            turnTimeout = nullptr;
        }
        if (bidTimeout) {
            bidTimeout->stop();
            delete bidTimeout;
            bidTimeout = nullptr;
        }
        if (surcoincheTimer) {
            surcoincheTimer->stop();
            delete surcoincheTimer;
            surcoincheTimer = nullptr;
        }
    }

    GameModel* gameModel = nullptr;
};

// Lobby privé pour jouer avec des amis
struct PrivateLobby {
    QString code;  // Code à 4 caractères
    QString hostPlayerName;  // Nom du joueur hôte
    QList<QString> playerNames;  // Noms des joueurs dans le lobby
    QList<QString> playerAvatars;  // Avatars des joueurs
    QList<bool> readyStatus;  // Statut "prêt" de chaque joueur
};

class GameServer : public QObject {
    Q_OBJECT

    // Allow test classes to access private methods
    friend class GameServerUtilsTest;

public:
    // Constante de délai pour l'affichage du panneau d'annonces
    static constexpr int BID_PANEL_DISPLAY_DELAY_MS = 3000;  // Délai pour laisser le temps au client de recevoir les cartes et d'afficher le panneau

    // Constructeur avec support SSL optionnel
    // Si certPath et keyPath sont fournis, le serveur démarre en mode sécurisé (WSS)
    // Sinon, il démarre en mode non sécurisé (WS)
    // smtpPassword: mot de passe pour l'envoi d'emails via SMTP
    explicit GameServer(quint16 port, QObject *parent = nullptr,
                        const QString &certPath = QString(),
                        const QString &keyPath = QString(),
                        const QString &smtpPassword = QString())
        : QObject(parent)
        , m_server(nullptr)
        , m_nextRoomId(1)
        , m_dbManager(new DatabaseManager(this))
        , m_smtpPassword(smtpPassword)
    {
        // Initialiser la base de donnees
        if (!m_dbManager->initialize("coinche.db")) {
            qCritical() << "Echec de l'initialisation de la base de donnees";
        }

        // Déterminer le mode (sécurisé ou non)
        bool useSecureMode = !certPath.isEmpty() && !keyPath.isEmpty();

        if (useSecureMode) {
            // Mode sécurisé WSS
            m_server = new QWebSocketServer("CoinchServer", QWebSocketServer::SecureMode, this);

            // Charger le certificat SSL
            QFile certFile(certPath);
            if (!certFile.open(QIODevice::ReadOnly)) {
                qCritical() << "Impossible d'ouvrir le certificat:" << certPath;
                return;
            }
            QSslCertificate certificate(&certFile, QSsl::Pem);
            certFile.close();

            // Charger la clé privée
            QFile keyFile(keyPath);
            if (!keyFile.open(QIODevice::ReadOnly)) {
                qCritical() << "Impossible d'ouvrir la clé privée:" << keyPath;
                return;
            }
            QSslKey sslKey(&keyFile, QSsl::Rsa, QSsl::Pem);
            keyFile.close();

            // Configurer SSL
            QSslConfiguration sslConfig;
            sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
            sslConfig.setLocalCertificate(certificate);
            sslConfig.setPrivateKey(sslKey);
            sslConfig.setProtocol(QSsl::TlsV1_2OrLater);

            m_server->setSslConfiguration(sslConfig);

            qDebug() << "Mode sécurisé (WSS) activé";
        } else {
            // Mode non sécurisé WS (pour développement local)
            m_server = new QWebSocketServer("CoinchServer", QWebSocketServer::NonSecureMode, this);
            qDebug() << "Mode non sécurisé (WS) - Utilisez SSL en production!";
        }

        if (m_server->listen(QHostAddress::Any, port)) {
            qDebug() << "Serveur demarre sur le port" << port << (useSecureMode ? "(WSS)" : "(WS)");
            connect(m_server, &QWebSocketServer::newConnection,
                    this, &GameServer::onNewConnection);
        } else {
            qDebug() << "Erreur: impossible de demarrer le serveur";
        }

        // Initialiser le timer de matchmaking avec bots (45 secondes)
        // Le timer principal attend 35 secondes, puis le countdown démarre pour 9 secondes
        m_matchmakingTimer = new QTimer(this);
        m_matchmakingTimer->setInterval(2000);  // 35 secondes avant le début du compte à rebours
        m_lastQueueSize = 0;
        connect(m_matchmakingTimer, &QTimer::timeout, this, &GameServer::onMatchmakingStartCountdown);

        // Timer de compte à rebours (9 dernières secondes)
        m_countdownTimer = new QTimer(this);
        m_countdownTimer->setInterval(1000);  // 1 seconde
        m_countdownSeconds = 0;
        connect(m_countdownTimer, &QTimer::timeout, this, &GameServer::onCountdownTick);

        // Initialiser le StatsReporter (rapports quotidiens)
        m_statsReporter = new StatsReporter(m_dbManager, m_smtpPassword, this);
        qInfo() << "StatsReporter initialisé - Rapports quotidiens activés";
    }

    ~GameServer() {
        m_server->close();

        // Libére toutes les GameRooms
        qDeleteAll(m_gameRooms.values());
        m_gameRooms.clear();

        // Libére toutes les connexions
        qDeleteAll(m_connections.values());
        m_connections.clear();

        // Libére tous les lobbies privés
        qDeleteAll(m_privateLobbies.values());
        m_privateLobbies.clear();
    }

    // Accès au StatsReporter (pour tests et monitoring)
    StatsReporter* getStatsReporter() const {
        return m_statsReporter;
    }

private slots:
    void onNewConnection();

    void onTextMessageReceived(const QString &message);

    void onDisconnected();

private:
    // Calcule la valeur d'une carte en mode Tout Atout
    int getCardValueToutAtout(Carte* carte) const {
        if (!carte) return 0;

        Carte::Chiffre chiffre = carte->getChiffre();
        switch (chiffre) {
            case Carte::SEPT:   return 0;
            case Carte::HUIT:   return 0;
            case Carte::DAME:   return 2;  // Dame vaut 2 point en TA
            case Carte::ROI:    return 3;
            case Carte::DIX:    return 4;
            case Carte::AS:     return 6;  // As vaut 6 points en TA
            case Carte::NEUF:   return 9;
            case Carte::VALET:  return 14;
            default:            return 0;
        }
    }

    // Calcule la valeur d'une carte en mode Sans Atout
    int getCardValueSansAtout(Carte* carte) const {
        if (!carte) return 0;

        Carte::Chiffre chiffre = carte->getChiffre();
        switch (chiffre) {
            case Carte::SEPT:   return 0;
            case Carte::HUIT:   return 0;
            case Carte::NEUF:   return 0;
            case Carte::VALET:  return 2;
            case Carte::DAME:   return 3;
            case Carte::ROI:    return 4;
            case Carte::DIX:    return 10;
            case Carte::AS:     return 19;
            default:            return 0;
        }
    }

    void handleRegister(QWebSocket *socket, const QJsonObject &data);  

    void handleReconnection(const QString& connectionId, int roomId, int playerIndex);
        
    void handleUpdateAvatar(QWebSocket *socket, const QJsonObject &data);

    // Handler pour réhumaniser un joueur après qu'il ait été remplacé par un bot
    void handleRehumanize(QWebSocket *socket);

    void handleSendContactMessage(QWebSocket *socket, const QJsonObject &data);

    void handleRegisterAccount(QWebSocket *socket, const QJsonObject &data);

    void handleLoginAccount(QWebSocket *socket, const QJsonObject &data);

    void handleDeleteAccount(QWebSocket *socket, const QJsonObject &data);

    void handleForgotPassword(QWebSocket *socket, const QJsonObject &data);

    void handleChangePassword(QWebSocket *socket, const QJsonObject &data);

    void handleChangePseudo(QWebSocket *socket, const QJsonObject &data);

    void handleChangeEmail(QWebSocket *socket, const QJsonObject &data);

    void handleSetAnonymous(QWebSocket *socket, const QJsonObject &data);

    void handleReportCrash(QWebSocket *socket, const QJsonObject &data);

    void handleGetStats(QWebSocket *socket, const QJsonObject &data);

    void handleJoinMatchmaking(QWebSocket *socket);

    void handleLeaveMatchmaking(QWebSocket *socket);

    void tryCreateGame();
        

    // Slot appelé après 20 secondes d'inactivité - démarre le compte à rebours de 9 secondes
    void onMatchmakingStartCountdown() {
        qDebug() << "MATCHMAKING - Début du compte à rebours de 9 secondes";
        qDebug() << "Joueurs dans la queue:" << m_matchmakingQueue.size();

        // Arrêter le timer principal
        m_matchmakingTimer->stop();

        // S'il y a des joueurs mais pas assez pour une partie complète
        if (m_matchmakingQueue.size() > 0 && m_matchmakingQueue.size() < 4) {
            // Démarrer le compte à rebours
            m_countdownSeconds = 9;
            sendCountdownToQueue(m_countdownSeconds);
            m_countdownTimer->start();
        }
    }

    // Slot appelé chaque seconde pendant le compte à rebours
    void onCountdownTick() {
        m_countdownSeconds--;
        qDebug() << "MATCHMAKING COUNTDOWN:" << m_countdownSeconds << "secondes";

        if (m_countdownSeconds > 0) {
            // Envoyer le compte à rebours aux joueurs dans la queue
            sendCountdownToQueue(m_countdownSeconds);
        } else {
            // Fin du compte à rebours - créer la partie avec des bots
            m_countdownTimer->stop();
            qDebug() << "MATCHMAKING - Fin du compte à rebours, création de la partie avec bots";

            if (m_matchmakingQueue.size() > 0 && m_matchmakingQueue.size() < 4) {
                createGameWithBots();
            }
        }
    }

    // Envoie le compte à rebours à tous les joueurs dans la queue
    void sendCountdownToQueue(int seconds) {
        QJsonObject msg;
        msg["type"] = "matchmakingCountdown";
        msg["seconds"] = seconds;
        msg["message"] = QString("Les joueurs manquants seront remplacés par des bots dans %1 seconde%2")
                         .arg(seconds)
                         .arg(seconds > 1 ? "s" : "");

        for (const QString& connectionId : m_matchmakingQueue) {
            if (m_connections.contains(connectionId)) {
                PlayerConnection* conn = m_connections[connectionId];
                if (conn && conn->socket) {
                    sendMessage(conn->socket, msg);
                }
            }
        }
    }

    // Génère un avatar aléatoire parmi les 24 disponibles
    QString getRandomBotAvatar() {
        int avatarNumber = 1 + QRandomGenerator::global()->bounded(24);  // 1 à 24
        return QString("avataaars%1.svg").arg(avatarNumber);
    }

    // Crée une partie en complétant avec des bots
    void createGameWithBots();

    void notifyGameStart(int roomId, const QList<QString> &connectionIds);

    void handlePlayCard(QWebSocket *socket, const QJsonObject &data);

    void finishPli(int roomId);

    void finishManche(int roomId);

    void startNewManche(int roomId) {
        GameRoom* room = m_gameRooms.value(roomId);
        if (!room) return;

        qDebug() << "GameServer - Nouvelle manche: envoi de l'animation aux clients";

        // Envoyer le message d'animation "Nouvelle Manche" à tous les joueurs
        QJsonObject newMancheMsg;
        newMancheMsg["type"] = "newMancheAnimation";
        broadcastToRoom(roomId, newMancheMsg);

        // Attendre 3 secondes pour l'animation avant de distribuer les cartes
        QTimer::singleShot(3000, this, [this, roomId]() {
            doStartNewManche(roomId);
        });
    }

    void doStartNewManche(int roomId);

    void notifyNewManche(int roomId) {
        GameRoom* room = m_gameRooms.value(roomId);
        if (!room) return;

        qDebug() << "Envoi des notifications de nouvelle manche a" << room->connectionIds.size() << "joueurs";

        for (int i = 0; i < room->connectionIds.size(); i++) {
            // Envoyer à tous les joueurs connectés, même les bots
            // Car un joueur peut être bot temporairement et se réhumaniser
            QString connId = room->connectionIds[i];
            if (connId.isEmpty()) {
                qDebug() << "Joueur" << i << "déconnecté (connectionId vide), skip";
                continue;
            }
            PlayerConnection *conn = m_connections.value(connId);
            if (!conn || !conn->socket) {
                qDebug() << "Joueur" << i << "pas de connexion valide, skip";
                continue;
            }

            QJsonObject msg;
            msg["type"] = "newManche";
            msg["roomId"] = roomId;
            msg["playerPosition"] = i;
            msg["biddingPlayer"] = room->biddingPlayer;
            msg["currentPlayer"] = room->currentPlayerIndex;

            // Envoi les nouvelles cartes du joueur
            QJsonArray myCards;
            const auto& playerHand = room->players[i]->getMain();
            for (const auto* carte : playerHand) {
                if (carte) {
                    QJsonObject cardObj;
                    cardObj["value"] = static_cast<int>(carte->getChiffre());
                    cardObj["suit"] = static_cast<int>(carte->getCouleur());
                    myCards.append(cardObj);
                }
            }
            msg["myCards"] = myCards;

            sendMessage(conn->socket, msg);
            qDebug() << "Nouvelle manche envoyée au joueur" << i;
        }
    }

    void handleMakeBid(QWebSocket *socket, const QJsonObject &data);

    void handleForfeit(QWebSocket *socket);

    // Évalue la force d'une main pour une couleur d'atout donnée
    // Retourne un score estimé de points que le bot peut espérer faire
    int evaluateHandForSuit(Player* player, Carte::Couleur atoutCouleur) {
        int score = 0;
        int atoutCount = 0;
        bool hasValetAtout = false;
        bool hasNeufAtout = false;
        bool hasRoiAtout = false;
        bool hasDameAtout = false;

        const auto& main = player->getMain();

        for (Carte* carte : main) {
            Carte::Couleur couleur = carte->getCouleur();
            Carte::Chiffre chiffre = carte->getChiffre();

            if (couleur == atoutCouleur) {
                // C'est un atout
                atoutCount++;

                if (chiffre == Carte::VALET) {
                    hasValetAtout = true;
                    score += 20; // Valet d'atout : 20 points
                } else if (chiffre == Carte::NEUF) {
                    hasNeufAtout = true;
                    score += 14; // 9 d'atout : 14 points
                } else if (chiffre == Carte::AS) {
                    score += 11;
                } else if (chiffre == Carte::DIX) {
                    score += 10;
                } else if (chiffre == Carte::ROI) {
                    hasRoiAtout = true;
                    score += 4;
                } else if (chiffre == Carte::DAME) {
                    hasDameAtout = true;
                    score += 3;
                }
            } else {
                // Hors atout
                if (chiffre == Carte::AS) {
                    score += 10; // Bonus pour As hors atout (maître potentiel)
                }
            }
        }

        // Bonus pour la belote (Roi + Dame d'atout)
        if (hasRoiAtout && hasDameAtout) {
            score += 20;
        }

        // Bonus pour nombre d'atouts (plus on en a, mieux c'est)
        if (atoutCount >= 5) {
            score += 30;
        } else if (atoutCount >= 4) {
            score += 20;
        } else if (atoutCount >= 3) {
            score += 10;
        } else if (atoutCount < 2) {
            // Pénalité si trop peu d'atouts
            score -= 20;
        }

        // Bonus si on a le Valet ET le 9 d'atout (très fort)
        if (hasValetAtout && hasNeufAtout) {
            score += 15;
        }

        return score;
    }

    // Évalue la main pour soutenir l'annonce du partenaire
    // Retourne un score bonus à ajouter si le bot veut surenchérir sur l'atout du partenaire
    int evaluateHandForPartnerSuit(Player* player, Carte::Couleur partnerAtout) {
        int score = 0;
        int atoutCount = 0;
        bool hasValetAtout = false;
        bool hasNeufAtout = false;
        bool hasRoiAtout = false;
        bool hasDameAtout = false;

        const auto& main = player->getMain();

        for (Carte* carte : main) {
            Carte::Couleur couleur = carte->getCouleur();
            Carte::Chiffre chiffre = carte->getChiffre();

            if (couleur == partnerAtout) {
                // C'est un atout (la couleur du partenaire)
                atoutCount++;

                if (chiffre == Carte::VALET) {
                    hasValetAtout = true;
                    score += 15; // Valet d'atout pour soutien : +15
                } else if (chiffre == Carte::NEUF) {
                    hasNeufAtout = true;
                    score += 10; // 9 d'atout pour soutien : +10
                } else if (chiffre == Carte::ROI) {
                    hasRoiAtout = true;
                } else if (chiffre == Carte::DAME) {
                    hasDameAtout = true;
                }
            } else {
                // Hors atout - les As sont très utiles pour le partenaire
                if (chiffre == Carte::AS) {
                    score += 10; // As hors atout : +10 (on peut faire des plis)
                }
            }
        }

        // Bonus pour la belote (Roi + Dame de l'atout du partenaire)
        if (hasRoiAtout && hasDameAtout) {
            score += 15; // Belote pour soutien : +15
        }

        // Bonus si on a des atouts pour soutenir
        if (hasNeufAtout && atoutCount >= 2) {
            score += 10; // 9 + autres atouts : bon soutien
        }

        // Bonus si on a le Valet (très fort pour soutenir)
        if (hasValetAtout) {
            score += 5; // Bonus additionnel si on a le Valet
        }

        return score;
    }

    // Convertit un score d'évaluation en annonce
    Player::Annonce scoreToAnnonce(int score, Player::Annonce currentBid) {
        Player::Annonce annonce = Player::PASSE;

        // Définir l'annonce en fonction du score
        if (score >= 140) {
            annonce = Player::CENTSOIXANTE;
        } else if (score >= 130) {
            annonce = Player::CENTCINQUANTE;
        } else if (score >= 120) {
            annonce = Player::CENTQUARANTE;
        } else if (score >= 110) {
            annonce = Player::CENTTRENTE;
        } else if (score >= 100) {
            annonce = Player::CENTVINGT;
        } else if (score >= 90) {
            annonce = Player::CENTDIX;
        } else if (score >= 80) {
            annonce = Player::CENT;
        } else if (score >= 70) {
            annonce = Player::QUATREVINGTDIX;
        } else if (score >= 60) {
            annonce = Player::QUATREVINGT;
        }

        // S'assurer que l'annonce est supérieure à l'annonce actuelle
        if (annonce != Player::PASSE && annonce <= currentBid) {
            // Essayer d'annoncer un cran au-dessus si le score le permet
            if (currentBid < Player::CENTSOIXANTE && score >= 60) {
                annonce = static_cast<Player::Annonce>(static_cast<int>(currentBid) + 1);
                // Vérifier qu'on ne dépasse pas nos moyens
                int requiredScore = 60 + (static_cast<int>(annonce) - 1) * 10;
                if (score < requiredScore) {
                    annonce = Player::PASSE;
                }
            } else {
                annonce = Player::PASSE;
            }
        }

        return annonce;
    }

    void playBotBid(int roomId, int playerIndex) {
        GameRoom* room = m_gameRooms.value(roomId);
        if (!room || room->currentPlayerIndex != playerIndex) return;

        // IMPORTANT: Vérifier que le joueur est toujours un bot
        // Il peut avoir été réhumanisé entre-temps
        if (!room->isBot[playerIndex]) {
            qDebug() << "playBotBid - ANNULÉ: Joueur" << playerIndex << "n'est plus un bot (réhumanisé)";
            return;
        }

        // Arrêter le timer de timeout et invalider les anciens callbacks
        if (room->bidTimeout) {
            room->bidTimeout->stop();
            room->bidTimeoutGeneration++;
            qDebug() << "playBotBid - Timer de timeout enchères arrêté, génération:" << room->bidTimeoutGeneration;
        }

        // IMPORTANT: Vérifier si une COINCHE est en cours
        if (room->coinched) {
            // Après une coinche, biddingPlayer est mis à -1 et le timer de surcoinche prend le relais
            // Les bots ne doivent RIEN faire pendant ce temps (pas de passe, pas d'annonce)
            // Le timer de surcoinche gérera automatiquement la fin des enchères
            qDebug() << "GameServer - Bot" << playerIndex << "ignore son tour (coinche en cours, timer actif)";
            return;
        }

        Player* player = room->players[playerIndex].get();

        // Évaluer la main pour chaque couleur d'atout possible (annonce propre)
        int bestOwnScore = 0;
        Carte::Couleur bestOwnCouleur = Carte::COULEURINVALIDE;

        std::array<Carte::Couleur, 4> couleurs = {Carte::COEUR, Carte::TREFLE, Carte::CARREAU, Carte::PIQUE};

        for (Carte::Couleur couleur : couleurs) {
            int score = evaluateHandForSuit(player, couleur);
            qDebug() << "Bot" << playerIndex << "- Evaluation couleur" << static_cast<int>(couleur) << ":" << score;
            if (score > bestOwnScore) {
                bestOwnScore = score;
                bestOwnCouleur = couleur;
            }
        }

        // Vérifier si le partenaire a déjà annoncé
        int partnerIndex = (playerIndex + 2) % 4;
        bool partnerHasBid = (room->lastBidderIndex == partnerIndex &&
                              room->lastBidAnnonce != Player::ANNONCEINVALIDE);

        int bestScore = bestOwnScore;
        Carte::Couleur bestCouleur = bestOwnCouleur;

        if (partnerHasBid) {
            // Le partenaire a annoncé - évaluer si on peut soutenir
            Carte::Couleur partnerCouleur = room->lastBidCouleur;
            int supportScore = evaluateHandForPartnerSuit(player, partnerCouleur);

            qDebug() << "Bot" << playerIndex << "- Partenaire a annoncé en"
                     << static_cast<int>(partnerCouleur) << ", score soutien:" << supportScore;

            // Si on a un bon soutien, calculer le score équivalent pour comparer
            if (supportScore >= 25) {
                int supportTotalScore = supportScore + 60; // Score équivalent pour surenchérir

                // Comparer : soutenir le partenaire vs annoncer soi-même
                if (supportTotalScore > bestOwnScore) {
                    qDebug() << "Bot" << playerIndex << "- Préfère soutenir le partenaire ("
                             << supportTotalScore << ") vs propre annonce (" << bestOwnScore << ")";
                    bestScore = supportTotalScore;
                    bestCouleur = partnerCouleur;
                } else {
                    qDebug() << "Bot" << playerIndex << "- Préfère sa propre annonce ("
                             << bestOwnScore << ") vs soutien (" << supportTotalScore << ")";
                }
            }
        }

        // Déterminer l'annonce en fonction du score
        Player::Annonce annonce = scoreToAnnonce(bestScore, room->lastBidAnnonce);

        qDebug() << "GameServer - Bot joueur" << playerIndex << "meilleur score:" << bestScore
                 << "couleur:" << static_cast<int>(bestCouleur) << "annonce:" << static_cast<int>(annonce);

        if (annonce == Player::PASSE) {
            // Le bot passe
            qDebug() << "GameServer - Bot joueur" << playerIndex << "passe";
            room->passedBidsCount++;

            // Broadcast l'enchère à tous
            QJsonObject msg;
            msg["type"] = "bidMade";
            msg["playerIndex"] = playerIndex;
            msg["bidValue"] = static_cast<int>(Player::PASSE);
            msg["suit"] = 0;
            broadcastToRoom(roomId, msg);

            // Vérifier si phase d'enchères terminée
            if (room->passedBidsCount >= 3 && room->lastBidAnnonce != Player::ANNONCEINVALIDE) {
                qDebug() << "GameServer - Fin des encheres! Lancement phase de jeu";
                for (int i = 0; i < 4; i++) {
                    // IMPORTANT: Réinitialiser toutes les cartes avant de définir le nouvel atout
                    const auto& main = room->players[i]->getMain();
                    for (Carte* carte : main) {
                        carte->setAtout(false);
                    }

                    if (room->isToutAtout) {
                        room->players[i]->setAllCardsAsAtout();
                    } else if (room->isSansAtout) {
                        room->players[i]->setNoAtout();
                    } else {
                        room->players[i]->setAtout(room->lastBidCouleur);
                    }
                }
                startPlayingPhase(roomId);
            } else if (room->passedBidsCount >= 4 && room->lastBidAnnonce == Player::ANNONCEINVALIDE) {
                // Tous les joueurs ont passé sans annonce -> nouvelle manche
                qDebug() << "GameServer - Tous les joueurs ont passe! Nouvelle manche";
                startNewManche(roomId);
            } else {
                // Passer au joueur suivant
                advanceToNextBidder(roomId);
            }
        } else {
            // Le bot fait une annonce
            qDebug() << "GameServer - Bot joueur" << playerIndex << "annonce"
                     << static_cast<int>(annonce) << "en" << static_cast<int>(bestCouleur);

            room->passedBidsCount = 0;  // Réinitialiser le compteur de passes
            room->lastBidAnnonce = annonce;
            room->lastBidCouleur = bestCouleur;
            room->lastBidderIndex = playerIndex;
            room->couleurAtout = bestCouleur;

            // IMPORTANT: Quand un joueur annonce TA (7) ou SA (8), lastBidCouleur est COULEURINVALIDE
            // Donc si le bot soutient son partenaire TA/SA, bestCouleur sera COULEURINVALIDE
            if (bestCouleur == Carte::COULEURINVALIDE) {
                // Le bot soutient une enchère TA ou SA du partenaire
                // Conserver les flags isToutAtout/isSansAtout existants (ne rien changer)
                // Ne pas toucher à lastBidSuit non plus (conserve 7 ou 8)
                qDebug() << "GameServer - Bot soutient enchère TA/SA du partenaire (isToutAtout="
                         << room->isToutAtout << ", isSansAtout=" << room->isSansAtout << ")";
            } else {
                // Le bot annonce une couleur normale (COEUR, TREFLE, CARREAU, PIQUE)
                // Désactiver les modes TA/SA et mettre à jour lastBidSuit
                room->isToutAtout = false;
                room->isSansAtout = false;
                room->lastBidSuit = static_cast<int>(bestCouleur);
                qDebug() << "GameServer - Bot annonce couleur normale, désactivation TA/SA";
            }

            // Broadcast l'enchère à tous
            QJsonObject msg;
            msg["type"] = "bidMade";
            msg["playerIndex"] = playerIndex;
            msg["bidValue"] = static_cast<int>(annonce);
            msg["suit"] = static_cast<int>(bestCouleur);
            broadcastToRoom(roomId, msg);

            // Passer au joueur suivant
            advanceToNextBidder(roomId);
        }
    }

    // Fonction utilitaire pour passer au prochain enchérisseur
    void advanceToNextBidder(int roomId) {
        GameRoom* room = m_gameRooms.value(roomId);
        if (!room) return;

        // IMPORTANT: Ne pas avancer si une coinche est en cours
        // Le timer de surcoinche gère la fin des enchères
        if (room->coinched) {
            qDebug() << "advanceToNextBidder - Ignoré (coinche en cours, timer actif)";
            return;
        }

        room->currentPlayerIndex = (room->currentPlayerIndex + 1) % 4;
        room->biddingPlayer = room->currentPlayerIndex;

        QJsonObject stateMsg;
        stateMsg["type"] = "gameState";
        stateMsg["currentPlayer"] = room->currentPlayerIndex;
        stateMsg["biddingPlayer"] = room->biddingPlayer;
        stateMsg["biddingPhase"] = true;
        broadcastToRoom(roomId, stateMsg);

        // Si le prochain joueur est aussi un bot, le faire jouer
        if (room->isBot[room->currentPlayerIndex]) {
            QTimer::singleShot(3000, this, [this, roomId]() {
                GameRoom* room = m_gameRooms.value(roomId);
                if (room && room->gameState == "bidding") {
                    playBotBid(roomId, room->currentPlayerIndex);
                }
            });
        } else {
            // Joueur humain : démarrer le timer de timeout pour les enchères
            startBidTimeout(roomId, room->currentPlayerIndex);
        }
    }

    // Démarre le timer de timeout pour la phase d'enchères (20 secondes)
    void startBidTimeout(int roomId, int currentBidder) {
        GameRoom* room = m_gameRooms.value(roomId);
        if (!room || room->gameState != "bidding") return;

        // Créer le timer si nécessaire
        if (!room->bidTimeout) {
            room->bidTimeout = new QTimer(this);
            room->bidTimeout->setSingleShot(true);
        }

        // Arrêter et déconnecter l'ancien timer
        room->bidTimeout->stop();
        disconnect(room->bidTimeout, nullptr, this, nullptr);

        // Incrémenter la génération pour invalider les anciens callbacks en queue
        room->bidTimeoutGeneration++;
        int currentGeneration = room->bidTimeoutGeneration;

        qDebug() << "startBidTimeout - Timer de 20s démarré pour joueur" << currentBidder << "(génération:" << currentGeneration << ")";

        // Démarrer le nouveau timer
        connect(room->bidTimeout, &QTimer::timeout, this, [this, roomId, currentBidder, currentGeneration]() {
            GameRoom* room = m_gameRooms.value(roomId);
            if (!room || room->gameState != "bidding") return;

            // Vérifier que ce timeout est toujours valide (pas un ancien signal en queue)
            if (room->bidTimeoutGeneration != currentGeneration) {
                qDebug() << "BID TIMEOUT - Ignoré (ancienne génération:" << currentGeneration
                         << "actuelle:" << room->bidTimeoutGeneration << ")";
                return;
            }

            // Vérifier que c'est toujours le tour de ce joueur
            if (room->currentPlayerIndex != currentBidder) {
                qDebug() << "BID TIMEOUT - Ignoré (joueur actuel:" << room->currentPlayerIndex
                         << ", timeout pour:" << currentBidder << ")";
                return;
            }

            qDebug() << "BID TIMEOUT - Joueur" << currentBidder << "n'a pas annoncé dans les temps!";

            // Marquer le joueur comme bot
            if (!room->isBot[currentBidder]) {
                room->isBot[currentBidder] = true;
                qInfo() << "Bot replacement (timeout enchères) - Joueur index" << currentBidder << "dans room" << roomId;

                // Envoyer une notification au joueur
                if (currentBidder < room->connectionIds.size()) {
                    QString connId = room->connectionIds[currentBidder];
                    PlayerConnection* conn = connId.isEmpty() ? nullptr : m_connections.value(connId);
                    if (conn && conn->socket) {
                        QJsonObject botMsg;
                        botMsg["type"] = "botReplacement";
                        botMsg["message"] = "Un bot a pris le relai car vous n'avez pas annoncé à temps.";
                        sendMessage(conn->socket, botMsg);
                    }
                }
            }

            // Faire jouer le bot à la place du joueur
            playBotBid(roomId, currentBidder);
        });

        room->bidTimeout->start(20250);  // Un peu apres 20 secondes (20 sec dans le front end mais avec 250ms de delais réseau et de traitement pour laisser une petite marge)
    }

    // Vérifie si le partenaire est le joueur qui gagne actuellement le pli
    bool isPartnerWinning(int playerIndex, int winningPlayerIndex) {
        // Les partenaires sont aux positions 0-2 et 1-3
        return (playerIndex % 2) == (winningPlayerIndex % 2);
    }

    // Calcule la valeur totale des points dans le pli actuel
    int calculatePliPoints(const std::vector<std::pair<int, Carte*>>& pli) {
        int points = 0;
        for (const auto& pair : pli) {
            points += pair.second->getValeurDeLaCarte();
        }
        return points;
    }

    // Trouve l'index de la carte avec la plus petite valeur
    int findLowestValueCard(Player* player, const std::vector<int>& playableIndices) {
        const auto& main = player->getMain();
        int lowestIdx = playableIndices[0];
        int lowestValue = main[lowestIdx]->getValeurDeLaCarte() * 100 + main[lowestIdx]->getOrdreCarteForte();
        qInfo() << "Carte initiale: " << main[lowestIdx]->getChiffre() << " " << main[lowestIdx]->getCouleur();
        qInfo() << "lowestValue initiale: " << lowestValue;

        for (int idx : playableIndices) {
            int value = main[idx]->getValeurDeLaCarte() * 100 + main[idx]->getOrdreCarteForte();
            qInfo() << "Carte: " << main[idx]->getChiffre() << " " << main[idx]->getCouleur();
            qInfo() << "Value: " << value;
            if (value < lowestValue) {
                qInfo() << "Cette carte est moins forte que la carte la moins forte";
                lowestValue = value;
                lowestIdx = idx;
                qInfo() << "Carte la moins forte actuelle: " << main[lowestIdx]->getChiffre() << " " << main[lowestIdx]->getCouleur();
            }
        }
        return lowestIdx;
    }

    // Trouve l'index de la carte avec la plus petite valeur en SANS ATOUT
    // Évite de défausser les As et les 10 (cartes maîtres) si possible
    int findLowestValueCardSansAtout(Player* player, const std::vector<int>& playableIndices) {
        const auto& main = player->getMain();

        // D'abord, chercher les cartes qui ne sont NI des As NI des 10
        std::vector<int> nonMasterIndices;
        for (int idx : playableIndices) {
            if (main[idx]->getChiffre() != Carte::AS && main[idx]->getChiffre() != Carte::DIX) {
                nonMasterIndices.push_back(idx);
            }
        }

        // Si on a des cartes qui ne sont ni As ni 10, trouver la plus faible parmi elles
        if (!nonMasterIndices.empty()) {
            return findLowestValueCard(player, nonMasterIndices);
        }

        // Sinon, on doit défausser un As ou un 10 (on n'a que ça)
        return findLowestValueCard(player, playableIndices);
    }

    // Trouve l'index de la carte avec la plus petite valeur en TOUT ATOUT
    // Évite de défausser les Valets et les 9 (cartes maîtres) si possible
    int findLowestValueCardToutAtout(Player* player, const std::vector<int>& playableIndices) {
        const auto& main = player->getMain();

        // D'abord, chercher les cartes qui ne sont NI des Valets NI des 9
        std::vector<int> nonMasterIndices;
        for (int idx : playableIndices) {
            if (main[idx]->getChiffre() != Carte::VALET && main[idx]->getChiffre() != Carte::NEUF) {
                nonMasterIndices.push_back(idx);
            }
        }

        // Si on a des cartes qui ne sont ni Valets ni 9, trouver la plus faible parmi elles
        if (!nonMasterIndices.empty()) {
            return findLowestValueCard(player, nonMasterIndices);
        }

        // Sinon, on doit défausser un Valet ou un 9 (on n'a que ça)
        return findLowestValueCard(player, playableIndices);
    }

    // Trouve l'index de la carte avec la plus petite valeur en évitant les cartes maîtres et les atouts
    // Utilisé quand le bot ne peut pas gagner le pli et veut conserver ses cartes fortes
    int findLowestValueCardAvoidMasters(GameRoom* room, Player* player, const std::vector<int>& playableIndices) {
        const auto& main = player->getMain();

        // Chercher les cartes qui sont:
        // - Hors atout (on garde tous les atouts)
        // - ET non maîtres (on garde les As, 10, Roi, etc. qui peuvent gagner plus tard)
        std::vector<int> safeDiscardIndices;
        for (int idx : playableIndices) {
            Carte* carte = main[idx];

            // Ne jamais défausser un atout
            if (carte->getCouleur() == room->couleurAtout) {
                continue;
            }

            // Ne défausser que les cartes hors atout non maîtres
            if (!isMasterCard(room, carte)) {
                safeDiscardIndices.push_back(idx);
            }
        }

        // Si on a des cartes non maîtres hors atout, trouver la plus faible
        if (!safeDiscardIndices.empty()) {
            return findLowestValueCard(player, safeDiscardIndices);
        }

        // Sinon, on doit défausser une carte maître ou un atout (on n'a que ça)
        // Dans ce cas, on privilégie la carte la plus faible globalement
        return findLowestValueCard(player, playableIndices);
    }

    // Trouve l'index de la carte avec la plus petite valeur en évitant l'atout
    // Si toutes les cartes jouables sont des atouts, retourne le plus petit atout
    int findLowestValueCardAvoidTrump(Player* player, const std::vector<int>& playableIndices,
                                       Carte::Couleur couleurAtout) {
        const auto& main = player->getMain();

        // D'abord, chercher les cartes hors atout
        std::vector<int> nonTrumpIndices;
        for (int idx : playableIndices) {
            if (main[idx]->getCouleur() != couleurAtout) {
                nonTrumpIndices.push_back(idx);
            }
        }

        // Si on a des cartes hors atout, trouver la plus faible parmi elles
        if (!nonTrumpIndices.empty()) {
            int lowestIdx = nonTrumpIndices[0];
            int lowestValue = main[lowestIdx]->getValeurDeLaCarte() * 100 + main[lowestIdx]->getOrdreCarteForte();

            for (int idx : nonTrumpIndices) {
                int value = main[idx]->getValeurDeLaCarte() * 100 + main[idx]->getOrdreCarteForte();
                if (value < lowestValue) {
                    lowestValue = value;
                    lowestIdx = idx;
                }
            }
            return lowestIdx;
        }

        // Sinon, retourner la plus faible carte (qui sera un atout)
        return findLowestValueCard(player, playableIndices);
    }

    // Trouve l'index de la carte avec la plus grande valeur
    int findHighestValueCard(Player* player, const std::vector<int>& playableIndices) {
        const auto& main = player->getMain();
        int highestIdx = playableIndices[0];
        int highestValue = main[highestIdx]->getValeurDeLaCarte() * 100 + main[highestIdx]->getOrdreCarteForte();

        for (int idx : playableIndices) {
            int value = main[idx]->getValeurDeLaCarte() * 100 + main[idx]->getOrdreCarteForte();
            if (value > highestValue) {
                highestValue = value;
                highestIdx = idx;
            }
        }
        return highestIdx;
    }

    // Trouve la carte la plus faible qui bat la carte gagnante actuelle
    int findLowestWinningCard(Player* player, const std::vector<int>& playableIndices,
                              Carte* carteGagnante, Carte::Couleur couleurAtout) {
        const auto& main = player->getMain();
        int bestIdx = -1;
        int bestValue = 9999;

        for (int idx : playableIndices) {
            Carte* carte = main[idx];
            // Vérifier si cette carte bat la carte gagnante
            if (*carteGagnante < *carte) {
                int value = carte->getValeurDeLaCarte() * 100 + carte->getOrdreCarteForte();
                if (value < bestValue) {
                    bestValue = value;
                    bestIdx = idx;
                }
            }
        }
        return bestIdx;
    }

    // Choisit la meilleure carte à jouer selon la stratégie
    // Compte le nombre d'atouts restants chez les autres joueurs
    int countRemainingTrumps(GameRoom* room, Player* player) {
        int remaining = 0;
        Carte::Couleur atout = room->couleurAtout;

        std::array<Carte::Chiffre, 8> chiffres = {Carte::SEPT, Carte::HUIT, Carte::NEUF, Carte::DIX,
                                                   Carte::VALET, Carte::DAME, Carte::ROI, Carte::AS};

        for (auto chiffre : chiffres) {
            // Si la carte n'a pas été jouée et n'est pas dans ma main, elle est chez un autre
            if (!room->isCardPlayed(atout, chiffre)) {
                bool inMyHand = false;
                for (Carte* c : player->getMain()) {
                    if (c->getCouleur() == atout && c->getChiffre() == chiffre) {
                        inMyHand = true;
                        break;
                    }
                }
                if (!inMyHand) {
                    remaining++;
                }
            }
        }
        return remaining;
    }

    // Vérifie si l'As d'une couleur a été joué
    bool isAcePlayed(GameRoom* room, Carte::Couleur couleur) {
        return room->isCardPlayed(couleur, Carte::AS);
    }

    // Vérifie si le 10 d'une couleur est maître (l'As est tombé)
    bool isTenMaster(GameRoom* room, Carte::Couleur couleur, Player* player) {
        // Le 10 est maître si l'As de cette couleur a été joué
        // et si le 10 n'est pas lui-même dans ma main (sinon je le sais déjà)
        return isAcePlayed(room, couleur);
    }

    // Vérifie si une carte hors atout est maître (toutes les cartes plus fortes sont tombées)
    // Ordre hors atout: As > 10 > Roi > Dame > Valet > 9 > 8 > 7
    // Vérifie si une carte a été jouée dans les plis PRÉCÉDENTS (pas dans le pli actuel)
    // Utile pour déterminer si une carte est vraiment "maître" ou si une carte supérieure est dans le pli actuel
    bool isCardPlayedInPreviousTricks(GameRoom* room, Carte::Couleur couleur, Carte::Chiffre chiffre) {
        // Vérifier si la carte a été jouée globalement
        if (!room->isCardPlayed(couleur, chiffre)) {
            return false; // Pas jouée du tout
        }

        // Vérifier si la carte est dans le pli actuel
        for (const auto& pair : room->currentPli) {
            Carte* carte = pair.second;
            if (carte->getCouleur() == couleur && carte->getChiffre() == chiffre) {
                return false; // La carte est dans le pli actuel, donc pas "morte"
            }
        }

        return true; // La carte a été jouée dans un pli précédent
    }

    bool isMasterCard(GameRoom* room, Carte* carte) {
        Carte::Couleur couleur = carte->getCouleur();
        Carte::Chiffre chiffre = carte->getChiffre();

        // L'As est toujours maître s'il est encore en jeu
        if (chiffre == Carte::AS) {
            return true;
        }

        // Le 10 est maître si l'As est tombé DANS UN PLI PRÉCÉDENT (pas dans le pli actuel)
        if (chiffre == Carte::DIX) {
            return isCardPlayedInPreviousTricks(room, couleur, Carte::AS);
        }

        // Le Roi est maître si l'As et le 10 sont tombés DANS DES PLIS PRÉCÉDENTS
        if (chiffre == Carte::ROI) {
            return isCardPlayedInPreviousTricks(room, couleur, Carte::AS) &&
                   isCardPlayedInPreviousTricks(room, couleur, Carte::DIX);
        }

        // La Dame est maître si As, 10 et Roi sont tombés DANS DES PLIS PRÉCÉDENTS
        if (chiffre == Carte::DAME) {
            return isCardPlayedInPreviousTricks(room, couleur, Carte::AS) &&
                   isCardPlayedInPreviousTricks(room, couleur, Carte::DIX) &&
                   isCardPlayedInPreviousTricks(room, couleur, Carte::ROI);
        }

        // Le Valet est maître si As, 10, Roi et Dame sont tombés DANS DES PLIS PRÉCÉDENTS
        if (chiffre == Carte::VALET) {
            return isCardPlayedInPreviousTricks(room, couleur, Carte::AS) &&
                   isCardPlayedInPreviousTricks(room, couleur, Carte::DIX) &&
                   isCardPlayedInPreviousTricks(room, couleur, Carte::ROI) &&
                   isCardPlayedInPreviousTricks(room, couleur, Carte::DAME);
        }

        // Les autres cartes (9, 8, 7) ne sont généralement pas considérées comme maîtres
        return false;
    }

    // Vérifie si le Valet d'atout est tombé
    bool isTrumpJackPlayed(GameRoom* room) {
        return room->isCardPlayed(room->couleurAtout, Carte::VALET);
    }

    // Vérifie si le 9 d'atout est tombé
    bool isTrumpNinePlayed(GameRoom* room) {
        return room->isCardPlayed(room->couleurAtout, Carte::NEUF);
    }

    // Compte le nombre d'atouts déjà joués (tombés)
    int countPlayedTrumps(GameRoom* room) {
        int count = 0;
        Carte::Couleur atout = room->couleurAtout;

        std::array<Carte::Chiffre, 8> chiffres = {Carte::SEPT, Carte::HUIT, Carte::NEUF, Carte::DIX,
                                                   Carte::VALET, Carte::DAME, Carte::ROI, Carte::AS};

        for (auto chiffre : chiffres) {
            if (room->isCardPlayed(atout, chiffre)) {
                count++;
            }
        }
        return count;
    }

    // Stratégie pour SANS ATOUT (SA): pas d'atout, les cartes maîtres sont très importantes
    // L'ordre de force est: As > 10 > Roi > Dame > Valet > 9 > 8 > 7
    int chooseBestCardSansAtout(GameRoom* room, Player* player, int playerIndex,
                                const std::vector<int>& playableIndices,
                                Carte* carteGagnante, int idxPlayerWinning) {
        const auto& main = player->getMain();
        bool isAttackingTeam = (playerIndex % 2) == (room->lastBidderIndex % 2);

        // Cas 1: Le bot commence le pli
        if (room->currentPli.empty()) {
            qDebug() << "Bot" << playerIndex << "commence le pli en SANS ATOUT";

            // Stratégie: Jouer les As en priorité (cartes maîtres)
            for (int idx : playableIndices) {
                Carte* carte = main[idx];
                if (carte->getChiffre() == Carte::AS) {
                    qDebug() << "Bot" << playerIndex << "- [SA] Joue un As";
                    return idx;
                }
            }

            // Jouer un 10 si l'As de cette couleur est déjà tombé (le 10 devient maître)
            for (int idx : playableIndices) {
                Carte* carte = main[idx];
                if (carte->getChiffre() == Carte::DIX && isAcePlayed(room, carte->getCouleur())) {
                    qDebug() << "Bot" << playerIndex << "- [SA] Joue un 10 maître";
                    return idx;
                }
            }

            // Si on attaque, jouer les cartes fortes (Roi, 10)
            // Mais ne jouer le 10 QUE si l'As de cette couleur est déjà tombé
            if (isAttackingTeam) {
                for (int idx : playableIndices) {
                    Carte* carte = main[idx];
                    if (carte->getChiffre() == Carte::ROI) {
                        qDebug() << "Bot" << playerIndex << "- [SA] Joue un Roi";
                        return idx;
                    }
                    if (carte->getChiffre() == Carte::DIX && room->isCardPlayed(carte->getCouleur(), Carte::AS)) {
                        qDebug() << "Bot" << playerIndex << "- [SA] Joue un 10 (As de cette couleur déjà tombé)";
                        return idx;
                    }
                }
            }

            // Sinon, jouer la carte la plus faible pour économiser les fortes
            return findLowestValueCardSansAtout(player, playableIndices);
        }

        // Cas 2: Le partenaire gagne le pli
        if (isPartnerWinning(playerIndex, idxPlayerWinning)) {
            qDebug() << "Bot" << playerIndex << "- [SA] Partenaire gagne, défausse (évite As et 10)";
            return findLowestValueCardSansAtout(player, playableIndices);
        }

        // Cas 3: L'adversaire gagne le pli
        // Essayer de reprendre avec une carte plus forte
        int bestCardIdx = -1;
        int bestOrder = -1;

        for (int idx : playableIndices) {
            Carte* carte = main[idx];
            // Vérifier si cette carte peut battre la carte gagnante
            if (carte->getCouleur() == carteGagnante->getCouleur()) {
                int order = carte->getOrdreCarteForte();
                if (order > carteGagnante->getOrdreCarteForte() && order > bestOrder) {
                    bestOrder = order;
                    bestCardIdx = idx;
                }
            }
        }

        // Si on peut gagner, jouer la plus petite carte gagnante
        if (bestCardIdx != -1) {
            qDebug() << "Bot" << playerIndex << "- [SA] Peut gagner, joue carte gagnante";
            return bestCardIdx;
        }

        // Sinon, défausser la plus petite carte (évite les As et les 10)
        qDebug() << "Bot" << playerIndex << "- [SA] Ne peut pas gagner, défausse (évite As et 10)";
        return findLowestValueCardSansAtout(player, playableIndices);
    }

    // Stratégie pour TOUT ATOUT (TA): toutes les cartes sont atouts
    // L'ordre de force est: Valet > 9 > As > 10 > Roi > Dame > 8 > 7
    // IMPORTANT: On est obligé de monter si on peut (règle du jeu)
    int chooseBestCardToutAtout(GameRoom* room, Player* player, int playerIndex,
                                const std::vector<int>& playableIndices,
                                Carte* carteGagnante, int idxPlayerWinning) {
        const auto& main = player->getMain();
        bool isAttackingTeam = (playerIndex % 2) == (room->lastBidderIndex % 2);

        // Cas 1: Le bot commence le pli
        if (room->currentPli.empty()) {
            qInfo() << "Bot" << playerIndex << "commence le pli en TOUT ATOUT";

            // Stratégie: Jouer les cartes maîtres (Valet, 9, As)
            // Jouer le Valet si on l'a
            for (int idx : playableIndices) {
                Carte* carte = main[idx];
                if (carte->getChiffre() == Carte::VALET) {
                    qInfo() << "Bot" << playerIndex << "- [TA] Joue un Valet";
                    return idx;
                }
            }

            // Jouer un 9 SEULEMENT si le Valet de la même couleur est déjà tombé
            for (int idx : playableIndices) {
                Carte* carte = main[idx];
                if (carte->getChiffre() == Carte::NEUF) {
                    // Vérifier si le Valet de cette couleur est tombé
                    if (room->isCardPlayed(carte->getCouleur(), Carte::VALET)) {
                        qInfo() << "Bot" << playerIndex << "- [TA] Joue un 9 (Valet de cette couleur déjà tombé)";
                        return idx;
                    } else {
                        qInfo() << "Bot" << playerIndex << "- [TA] Ne joue PAS le 9 (Valet de cette couleur encore en jeu)";
                    }
                }
            }

            // Jouer un As SEULEMENT si le Valet ET le 9 de la même couleur sont déjà tombés
            for (int idx : playableIndices) {
                Carte* carte = main[idx];
                if (carte->getChiffre() == Carte::AS) {
                    // Vérifier si le Valet ET le 9 de cette couleur sont tombés
                    if (room->isCardPlayed(carte->getCouleur(), Carte::VALET) &&
                        room->isCardPlayed(carte->getCouleur(), Carte::NEUF)) {
                        qDebug() << "Bot" << playerIndex << "- [TA] Joue un As (Valet et 9 de cette couleur déjà tombés)";
                        return idx;
                    } else {
                        qDebug() << "Bot" << playerIndex << "- [TA] Ne joue PAS l'As (Valet ou 9 de cette couleur encore en jeu)";
                    }
                }
            }

            // Sinon, jouer la carte la plus faible pour économiser les fortes
            return findLowestValueCardToutAtout(player, playableIndices);
        }

        // Cas 2: Le partenaire gagne le pli
        if (isPartnerWinning(playerIndex, idxPlayerWinning)) {
            qDebug() << "Bot" << playerIndex << "- [TA] Partenaire gagne";

            // En TA, on doit MONTER si on peut, même si le partenaire gagne
            // IMPORTANT: On monte uniquement sur la carte de la COULEUR DEMANDÉE

            // Trouver la carte la plus forte de la couleur demandée dans le pli
            Carte* carteGagnanteCouleurDemandee = nullptr;
            for (const auto& pair : room->currentPli) {
                if (pair.second->getCouleur() == room->couleurDemandee) {
                    if (!carteGagnanteCouleurDemandee ||
                        pair.second->getOrdreCarteForte() > carteGagnanteCouleurDemandee->getOrdreCarteForte()) {
                        carteGagnanteCouleurDemandee = pair.second;
                    }
                }
            }

            // Séparer les cartes jouables : celles de la couleur demandée et les autres
            std::vector<int> cartesCouleurDemandee;
            std::vector<int> autresCouleurs;

            for (int idx : playableIndices) {
                if (main[idx]->getCouleur() == room->couleurDemandee) {
                    cartesCouleurDemandee.push_back(idx);
                } else {
                    autresCouleurs.push_back(idx);
                }
            }

            // Si on a la couleur demandée, chercher si on peut monter sur cette couleur
            if (!cartesCouleurDemandee.empty() && carteGagnanteCouleurDemandee) {
                int lowestWinningIdx = -1;
                int lowestWinningOrder = 999;

                for (int idx : cartesCouleurDemandee) {
                    Carte* carte = main[idx];
                    int order = carte->getOrdreCarteForte();
                    if (order > carteGagnanteCouleurDemandee->getOrdreCarteForte() && order < lowestWinningOrder) {
                        lowestWinningOrder = order;
                        lowestWinningIdx = idx;
                    }
                }

                // Si on peut monter dans la couleur demandée, jouer la plus petite carte qui monte
                if (lowestWinningIdx != -1) {
                    qDebug() << "Bot" << playerIndex << "- [TA] Monte sur le partenaire (couleur demandée)";
                    return lowestWinningIdx;
                }

                // Si on ne peut pas monter, jouer la plus petite carte de la couleur demandée
                qDebug() << "Bot" << playerIndex << "- [TA] Ne peut pas monter, joue plus petite de la couleur demandée";
                return findLowestValueCardToutAtout(player, cartesCouleurDemandee);
            }

            // Si on n'a pas la couleur demandée, défausser la plus petite carte
            qDebug() << "Bot" << playerIndex << "- [TA] Pas la couleur demandée, défausse la plus petite";
            return findLowestValueCardToutAtout(player, autresCouleurs);
        }

        // Cas 3: L'adversaire gagne le pli
        // On DOIT monter si on peut, mais uniquement sur la carte de la COULEUR DEMANDÉE

        // Trouver la carte la plus forte de la couleur demandée dans le pli
        Carte* carteGagnanteCouleurDemandee = nullptr;
        for (const auto& pair : room->currentPli) {
            if (pair.second->getCouleur() == room->couleurDemandee) {
                if (!carteGagnanteCouleurDemandee ||
                    pair.second->getOrdreCarteForte() > carteGagnanteCouleurDemandee->getOrdreCarteForte()) {
                    carteGagnanteCouleurDemandee = pair.second;
                }
            }
        }

        // Séparer les cartes jouables : celles de la couleur demandée et les autres
        std::vector<int> cartesCouleurDemandee;
        std::vector<int> autresCouleurs;

        for (int idx : playableIndices) {
            if (main[idx]->getCouleur() == room->couleurDemandee) {
                cartesCouleurDemandee.push_back(idx);
            } else {
                autresCouleurs.push_back(idx);
            }
        }

        // Si on a la couleur demandée, chercher si on peut monter sur cette couleur
        if (!cartesCouleurDemandee.empty() && carteGagnanteCouleurDemandee) {
            int lowestWinningIdx = -1;
            int lowestWinningOrder = 999;

            for (int idx : cartesCouleurDemandee) {
                Carte* carte = main[idx];
                int order = carte->getOrdreCarteForte();
                if (order > carteGagnanteCouleurDemandee->getOrdreCarteForte() && order < lowestWinningOrder) {
                    lowestWinningOrder = order;
                    lowestWinningIdx = idx;
                }
            }

            // Si on peut monter dans la couleur demandée, jouer la plus petite carte qui monte
            if (lowestWinningIdx != -1) {
                qDebug() << "Bot" << playerIndex << "- [TA] Monte sur l'adversaire (couleur demandée)";
                return lowestWinningIdx;
            }

            // Si on ne peut pas monter, jouer la plus petite carte de la couleur demandée
            qDebug() << "Bot" << playerIndex << "- [TA] Ne peut pas monter, joue plus petite de la couleur demandée";
            return findLowestValueCardToutAtout(player, cartesCouleurDemandee);
        }

        // Si on n'a pas la couleur demandée, défausser la plus petite carte
        qDebug() << "Bot" << playerIndex << "- [TA] Pas la couleur demandée, défausse la plus petite";
        return findLowestValueCardToutAtout(player, autresCouleurs);
    }

    int chooseBestCard(GameRoom* room, Player* player, int playerIndex,
                       const std::vector<int>& playableIndices,
                       Carte* carteGagnante, int idxPlayerWinning) {
        const auto& main = player->getMain();

        // Détection des modes spéciaux: TA (7) et SA (8)
        if (static_cast<int>(room->couleurAtout) == 8) {
            // Mode SANS ATOUT
            qDebug() << "Bot" << playerIndex << "utilise la stratégie SANS ATOUT";
            return chooseBestCardSansAtout(room, player, playerIndex, playableIndices, carteGagnante, idxPlayerWinning);
        }

        if (static_cast<int>(room->couleurAtout) == 7) {
            // Mode TOUT ATOUT
            qDebug() << "Bot" << playerIndex << "utilise la stratégie TOUT ATOUT";
            return chooseBestCardToutAtout(room, player, playerIndex, playableIndices, carteGagnante, idxPlayerWinning);
        }

        // Stratégie classique pour les couleurs normales (Coeur, Trèfle, Carreau, Pique)
        // Cas 1: Le bot commence le pli (premier à jouer)
        if (room->currentPli.empty()) {
            qDebug() << "Bot" << playerIndex << "commence le pli - analyse de la main";

            // Analyser les atouts en main
            int valetAtoutIdx = -1;
            int neufAtoutIdx = -1;
            int otherAtoutCount = 0;
            bool hasAsHorsAtout = false;

            for (size_t i = 0; i < playableIndices.size(); i++) {
                int idx = playableIndices[i];
                Carte* carte = main[idx];
                if (carte->getCouleur() == room->couleurAtout) {
                    if (carte->getChiffre() == Carte::VALET) {
                        valetAtoutIdx = idx;
                    } else if (carte->getChiffre() == Carte::NEUF) {
                        neufAtoutIdx = idx;
                    } else {
                        otherAtoutCount++;
                    }
                } else if (carte->getChiffre() == Carte::AS) {
                    hasAsHorsAtout = true;
                }
            }

            // Compter les atouts restants chez les autres
            int remainingTrumps = countRemainingTrumps(room, player);

            // Trouver le plus petit atout en main (hors Valet et 9)
            int smallestAtoutIdx = -1;
            int smallestAtoutOrder = 999;
            for (int idx : playableIndices) {
                Carte* carte = main[idx];
                if (carte->getCouleur() == room->couleurAtout &&
                    carte->getChiffre() != Carte::VALET && carte->getChiffre() != Carte::NEUF) {
                    int order = carte->getOrdreCarteForte();
                    if (order < smallestAtoutOrder) {
                        smallestAtoutOrder = order;
                        smallestAtoutIdx = idx;
                    }
                }
            }

            int totalAtouts = (valetAtoutIdx >= 0 ? 1 : 0) + (neufAtoutIdx >= 0 ? 1 : 0) + otherAtoutCount;

            // Vérifier si le bot est dans l'équipe qui a pris (fait l'annonce)
            // L'équipe qui prend est celle du lastBidderIndex
            bool isAttackingTeam = (playerIndex % 2) == (room->lastBidderIndex % 2);

            // Compter les atouts déjà tombés
            int playedTrumps = countPlayedTrumps(room);

            // Vérifier si on doit continuer à chasser les atouts
            // Arrêter dès que 5+ atouts sont tombés
            bool shouldStopChasing = (playedTrumps >= 5);

            qDebug() << "Bot" << playerIndex << "- Valet:" << (valetAtoutIdx >= 0)
                     << "9:" << (neufAtoutIdx >= 0) << "autres atouts:" << otherAtoutCount
                     << "total atouts:" << totalAtouts << "equipe attaque:" << isAttackingTeam
                     << "As hors atout:" << hasAsHorsAtout << "atouts restants adversaires:" << remainingTrumps
                     << "atouts tombes:" << playedTrumps << "arreter chasse:" << shouldStopChasing;

            // === STRATÉGIE ÉQUIPE QUI ATTAQUE ===
            // Si 5+ atouts sont tombés, arrêter la chasse et jouer les cartes maîtres hors atout
            if (isAttackingTeam && shouldStopChasing) {
                qDebug() << "Bot" << playerIndex << "- [ATTAQUE] 5+ atouts tombés, arrêt chasse, recherche cartes maîtres";

                // Jouer les cartes maîtres hors atout en priorité
                for (int idx : playableIndices) {
                    Carte* carte = main[idx];
                    if (carte->getCouleur() != room->couleurAtout && isMasterCard(room, carte)) {
                        qDebug() << "Bot" << playerIndex << "- [ATTAQUE] Joue carte maître hors atout (chasse arrêtée)";
                        return idx;
                    }
                }
                // Si pas de carte maître, continuer avec la stratégie normale (en bas)
            }

            // Que ce soit le joueur qui a parlé ou son partenaire, on doit faire tomber les atouts
            // Mais on arrête si 5+ atouts sont tombés
            if (isAttackingTeam && remainingTrumps > 0 && totalAtouts >= 1 && !shouldStopChasing) {
                qDebug() << "Bot" << playerIndex << "- [ATTAQUE] Entre dans strategie chasse aux atouts";

                // Si j'ai le Valet, je le joue directement (carte maîtresse)
                if (valetAtoutIdx >= 0) {
                    qDebug() << "Bot" << playerIndex << "- [ATTAQUE] J'ai le Valet, je le joue";
                    return valetAtoutIdx;
                }

                // Si le Valet est tombé et j'ai le 9 (qui devient maître), je le joue
                if (neufAtoutIdx >= 0 && isTrumpJackPlayed(room)) {
                    qDebug() << "Bot" << playerIndex << "- [ATTAQUE] Valet tombé, je joue le 9 (maître)";
                    return neufAtoutIdx;
                }

                // Si j'ai le 9 + autres atouts (et le Valet pas encore tombé),
                // je joue un autre atout (pas le 9) pour garder le 9 maître pour plus tard
                if (neufAtoutIdx >= 0 && otherAtoutCount > 0 && smallestAtoutIdx >= 0 && !isTrumpJackPlayed(room)) {
                    qDebug() << "Bot" << playerIndex << "- [ATTAQUE] J'ai le 9 + autres, je joue un petit atout";
                    return smallestAtoutIdx;
                }

                // Si j'ai seulement le 9 (sans autre atout) et le Valet pas tombé,
                // je le joue quand même pour aider (mon partenaire a probablement le Valet)
                if (neufAtoutIdx >= 0 && otherAtoutCount == 0 && !isTrumpJackPlayed(room)) {
                    qDebug() << "Bot" << playerIndex << "- [ATTAQUE] J'ai seulement le 9, je le joue";
                    return neufAtoutIdx;
                }

                // Si j'ai d'autres atouts (sans Valet ni 9), je joue le plus petit
                // pour faire tomber les atouts adverses
                if (smallestAtoutIdx >= 0) {
                    qDebug() << "Bot" << playerIndex << "- [ATTAQUE] Je joue un atout pour faire tomber";
                    return smallestAtoutIdx;
                }
            }

            // Si le Valet est tombé mais on a le 9 et on attaque, jouer le 9 SEULEMENT si chasse pas arrêtée
            if (isAttackingTeam && valetAtoutIdx < 0 && neufAtoutIdx >= 0 &&
                isTrumpJackPlayed(room) && remainingTrumps > 0 && !shouldStopChasing) {
                qDebug() << "Bot" << playerIndex << "- [ATTAQUE] Continue avec le 9 (Valet tombé)";
                return neufAtoutIdx;
            }

            // === STRATÉGIE COMMUNE : Jouer les cartes maîtres hors atout ===
            // Si les atouts adverses sont tombés ou qu'on défend (économiser atouts), jouer les As
            if (remainingTrumps == 0 || !isAttackingTeam || (valetAtoutIdx < 0 && neufAtoutIdx < 0)) {
                for (int idx : playableIndices) {
                    Carte* carte = main[idx];
                    if (carte->getCouleur() != room->couleurAtout && carte->getChiffre() == Carte::AS) {
                        qDebug() << "Bot" << playerIndex << "- Joue un As hors atout";
                        return idx;
                    }
                }
            }

            // Jouer un 10 si l'As de cette couleur est tombé (le 10 est maître)
            for (int idx : playableIndices) {
                Carte* carte = main[idx];
                if (carte->getCouleur() != room->couleurAtout && carte->getChiffre() == Carte::DIX) {
                    if (isAcePlayed(room, carte->getCouleur())) {
                        qDebug() << "Bot" << playerIndex << "- Joue un 10 maître (As tombé)";
                        return idx;
                    }
                }
            }

            // === STRATÉGIE ÉQUIPE QUI DÉFEND ===
            if (!isAttackingTeam) {
                // Jouer une carte maître hors atout si on en a (le pli est vide donc pas d'atout dedans)
                // Chercher d'abord les cartes avec le plus de valeur (As, 10, Roi...)
                for (int idx : playableIndices) {
                    Carte* carte = main[idx];
                    if (carte->getCouleur() != room->couleurAtout && isMasterCard(room, carte)) {
                        qDebug() << "Bot" << playerIndex << "- [DEFENSE] Joue une carte maître hors atout en partant";
                        return idx;
                    }
                }

                // Sinon, jouer la carte la plus faible en évitant l'atout
                qDebug() << "Bot" << playerIndex << "- [DEFENSE] Évite de jouer atout en partant";
                return findLowestValueCardAvoidTrump(player, playableIndices, room->couleurAtout);
            }

            // Sinon, jouer la carte la plus faible pour économiser les fortes
            // Éviter de défausser les cartes maîtres et les atouts
            return findLowestValueCardAvoidMasters(room, player, playableIndices);
        }

        // Cas 2: Le partenaire est en train de gagner le pli
        if (isPartnerWinning(playerIndex, idxPlayerWinning)) {
            qDebug() << "Bot" << playerIndex << "- partenaire gagne";

            // Vérifier si le partenaire a joué à l'atout (première carte du pli)
            bool partnerPlayedTrump = false;
            if (!room->currentPli.empty()) {
                Carte* firstCard = room->currentPli[0].second;
                partnerPlayedTrump = (firstCard->getCouleur() == room->couleurAtout);
            }

            // Compter les atouts tombés pour savoir si on doit encore jouer atout
            int playedTrumps = countPlayedTrumps(room);

            // Si le partenaire joue à l'atout et qu'on est dans l'équipe qui attaque,
            // jouer le Valet si on l'a pour être sûr de gagner le pli
            // MAIS seulement si moins de 5 atouts sont tombés (sinon inutile)
            bool isAttackingTeam = (playerIndex % 2) == (room->lastBidderIndex % 2);

            if (partnerPlayedTrump && isAttackingTeam && playedTrumps < 5) {
                // Chercher le Valet d'atout dans ma main
                for (int idx : playableIndices) {
                    Carte* carte = main[idx];
                    if (carte->getCouleur() == room->couleurAtout && carte->getChiffre() == Carte::VALET) {
                        qDebug() << "Bot" << playerIndex << "- Partenaire joue atout, je joue le Valet pour gagner";
                        return idx;
                    }
                }

                // Vérifier si le Valet d'atout est dans le pli actuel (joué par n'importe qui)
                bool jackInCurrentPli = false;
                for (const auto& pair : room->currentPli) {
                    Carte* carte = pair.second;
                    if (carte->getCouleur() == room->couleurAtout && carte->getChiffre() == Carte::VALET) {
                        jackInCurrentPli = true;
                        break;
                    }
                }

                // Analyser mes atouts en main
                int neufAtoutIdx = -1;
                int otherAtoutIdx = -1;  // Plus petit atout hors 9
                int otherAtoutOrder = 999;

                for (int idx : playableIndices) {
                    Carte* carte = main[idx];
                    if (carte->getCouleur() == room->couleurAtout) {
                        if (carte->getChiffre() == Carte::NEUF) {
                            neufAtoutIdx = idx;
                        } else {
                            int order = carte->getOrdreCarteForte();
                            if (order < otherAtoutOrder) {
                                otherAtoutOrder = order;
                                otherAtoutIdx = idx;
                            }
                        }
                    }
                }

                // Si le Valet est dans le pli actuel et que j'ai le 9 + d'autres atouts,
                // je joue un autre atout pour garder le 9 (qui sera maître après ce pli)
                if (jackInCurrentPli && neufAtoutIdx >= 0 && otherAtoutIdx >= 0) {
                    qDebug() << "Bot" << playerIndex << "- Valet dans le pli, je garde le 9 et joue un autre atout";
                    return otherAtoutIdx;
                }

                // Si on a le 9 et le Valet est tombé (dans un pli précédent), jouer le 9 pour gagner
                if (isTrumpJackPlayed(room) && !jackInCurrentPli) {
                    for (int idx : playableIndices) {
                        Carte* carte = main[idx];
                        if (carte->getCouleur() == room->couleurAtout && carte->getChiffre() == Carte::NEUF) {
                            qDebug() << "Bot" << playerIndex << "- Partenaire joue atout, je joue le 9 (Valet tombé avant)";
                            return idx;
                        }
                    }
                }
            }

            // Si c'est le dernier joueur et le pli a beaucoup de points, charger
            if (room->currentPli.size() == 3) {
                int pliPoints = calculatePliPoints(room->currentPli);
                if (pliPoints >= 15) {
                    // Charger avec une carte à points (10 ou As)
                    for (int idx : playableIndices) {
                        Carte* carte = main[idx];
                        if (carte->getCouleur() != room->couleurAtout &&
                            (carte->getChiffre() == Carte::DIX || carte->getChiffre() == Carte::AS)) {
                            qDebug() << "Bot" << playerIndex << "charge le pli avec points";
                            return idx;
                        }
                    }
                }
            }

            // Pour l'équipe qui défend: ne JAMAIS jouer atout si possible
            if (!isAttackingTeam) {
                qDebug() << "Bot" << playerIndex << "- [DEFENSE] Partenaire gagne, évite atout";
                return findLowestValueCardAvoidTrump(player, playableIndices, room->couleurAtout);
            }

            // Pour l'équipe qui attaque avec 5+ atouts tombés: éviter de jouer atout inutilement
            if (isAttackingTeam && playedTrumps >= 5) {
                qDebug() << "Bot" << playerIndex << "- [ATTAQUE] Partenaire gagne, 5+ atouts tombés, évite atout";
                return findLowestValueCardAvoidTrump(player, playableIndices, room->couleurAtout);
            }

            // Jouer la carte la plus faible, mais éviter de défausser les cartes maîtres
            return findLowestValueCardAvoidMasters(room, player, playableIndices);
        }

        // Cas 3: Un adversaire gagne le pli - essayer de prendre
        qDebug() << "Bot" << playerIndex << "- adversaire gagne, essaie de prendre";

        // Vérifier si un atout a été joué dans le pli
        bool trumpInPli = false;
        for (const auto& pair : room->currentPli) {
            if (pair.second->getCouleur() == room->couleurAtout) {
                trumpInPli = true;
                break;
            }
        }

        // Déterminer si on est en défense
        bool isAttackingTeam = (playerIndex % 2) == (room->lastBidderIndex % 2);

        // Pour l'équipe qui défend: jouer une carte maître hors atout si:
        // - Aucun atout dans le pli
        // - La carte maître est de la couleur demandée
        // L'équipe qui attaque garde ses cartes maîtres pour faire le plus de plis possible
        if (!isAttackingTeam && !trumpInPli) {
            for (int idx : playableIndices) {
                Carte* carte = main[idx];
                if (carte->getCouleur() != room->couleurAtout &&
                    carte->getCouleur() == room->couleurDemandee &&
                    isMasterCard(room, carte)) {
                    qDebug() << "Bot" << playerIndex << "- [DEFENSE] Joue une carte maître à la couleur demandée (pas d'atout dans le pli)";
                    return idx;
                }
            }
        }

        int pliPoints = calculatePliPoints(room->currentPli);

        // Si le pli contient des points significatifs (> 10), essayer de prendre
        if (pliPoints >= 10 || room->currentPli.size() == 3) {
            int winningCardIdx = findLowestWinningCard(player, playableIndices,
                                                        carteGagnante, room->couleurAtout);
            if (winningCardIdx >= 0) {
                qDebug() << "Bot" << playerIndex << "prend le pli avec carte gagnante";
                return winningCardIdx;
            }
        }

        // Si on ne peut pas gagner ou le pli ne vaut pas le coup, jouer petit
        // MAIS éviter de défausser les cartes maîtres (As, 10, etc.) et les atouts
        return findLowestValueCardAvoidMasters(room, player, playableIndices);
    }

    void playBotCard(int roomId, int playerIndex) {
        qDebug() << "===== playBotCard appele pour joueur" << playerIndex << "isBot:" << (m_gameRooms.value(roomId) ? m_gameRooms.value(roomId)->isBot[playerIndex] : false);

        GameRoom* room = m_gameRooms.value(roomId);
        if (!room || room->currentPlayerIndex != playerIndex || room->gameState != "playing") {
            qDebug() << "playBotCard - Verification echouee: room=" << (room != nullptr)
                     << "currentPlayer=" << (room ? room->currentPlayerIndex : -1)
                     << "expected=" << playerIndex;
            return;
        }

        // IMPORTANT: Vérifier que le joueur est toujours un bot
        // Il peut avoir été réhumanisé entre-temps (reconnexion + clic OK)
        if (!room->isBot[playerIndex]) {
            qDebug() << "playBotCard - ANNULÉ: Joueur" << playerIndex << "n'est plus un bot (réhumanisé)";
            return;
        }

        // Arrêter le timer de timeout et invalider les anciens callbacks
        if (room->turnTimeout) {
            room->turnTimeout->stop();
            room->turnTimeoutGeneration++;  // Invalider les anciens callbacks en queue
            qDebug() << "playBotCard - Timer de timeout arrêté, génération:" << room->turnTimeoutGeneration;
        }

        Player* player = room->players[playerIndex].get();
        if (!player || player->getMain().empty()) return;

        // Calculer les cartes jouables
        Carte* carteGagnante = nullptr;
        int idxPlayerWinning = -1;
        if (!room->currentPli.empty()) {
            carteGagnante = room->currentPli[0].second;
            idxPlayerWinning = room->currentPli[0].first;

            for (size_t i = 1; i < room->currentPli.size(); i++) {
                Carte* c = room->currentPli[i].second;
                if (*carteGagnante < *c) {
                    carteGagnante = c;
                    idxPlayerWinning = room->currentPli[i].first;
                }
            }
        }

        // Trouver toutes les cartes jouables
        std::vector<int> playableIndices;
        const auto& main = player->getMain();
        for (size_t i = 0; i < main.size(); i++) {
            bool isPlayable = player->isCartePlayable(
                static_cast<int>(i),
                room->couleurDemandee,
                room->couleurAtout,
                carteGagnante,
                idxPlayerWinning
            );

            if (isPlayable) {
                playableIndices.push_back(static_cast<int>(i));
            }
        }

        if (playableIndices.empty()) {
            qDebug() << "GameServer - Bot joueur" << playerIndex << "n'a aucune carte jouable!";
            return;
        }

        // Stratégie de jeu intelligente
        int cardIndex = chooseBestCard(room, player, playerIndex, playableIndices,
                                        carteGagnante, idxPlayerWinning);

        qDebug() << "GameServer - Bot joueur" << playerIndex << "joue la carte a l'index" << cardIndex;

        Carte* cartePlayed = player->getMain()[cardIndex];

        // Si c'est la première carte du pli, définir la couleur demandée
        if (room->currentPli.empty()) {
            room->couleurDemandee = cartePlayed->getCouleur();
        }

        // Ajouter au pli courant
        room->currentPli.push_back(std::make_pair(playerIndex, cartePlayed));

        // Marquer la carte comme jouée pour le tracking IA
        room->markCardAsPlayed(cartePlayed);

        // Retirer la carte de la main
        player->removeCard(cardIndex);

        qDebug() << "GameServer - Bot a joué la carte, main contient maintenant" << player->getMain().size() << "cartes";

        // Vérifier si c'est une carte de la belote (Roi ou Dame de l'atout)
        bool isRoi = (cartePlayed->getChiffre() == Carte::ROI);
        bool isDame = (cartePlayed->getChiffre() == Carte::DAME);
        bool isAtout = (cartePlayed->getCouleur() == room->couleurAtout);

        if (isAtout && (isRoi || isDame)) {
            int teamIndex = playerIndex % 2;
            bool hasBelote = (teamIndex == 0) ? room->beloteTeam1 : room->beloteTeam2;

            if (hasBelote) {
                if (!room->beloteRoiJoue && !room->beloteDameJouee) {
                    // Première carte de la belote
                    if (isRoi) {
                        room->beloteRoiJoue = true;
                    } else {
                        room->beloteDameJouee = true;
                    }

                    QJsonObject beloteMsg;
                    beloteMsg["type"] = "belote";
                    beloteMsg["playerIndex"] = playerIndex;
                    broadcastToRoom(roomId, beloteMsg);

                } else if ((isRoi && room->beloteDameJouee) || (isDame && room->beloteRoiJoue)) {
                    // Deuxième carte de la belote
                    if (isRoi) {
                        room->beloteRoiJoue = true;
                    } else {
                        room->beloteDameJouee = true;
                    }

                    QJsonObject rebeloteMsg;
                    rebeloteMsg["type"] = "rebelote";
                    rebeloteMsg["playerIndex"] = playerIndex;
                    broadcastToRoom(roomId, rebeloteMsg);
                }
            }
        }

        // Broadcast l'action
        QJsonObject msg;
        msg["type"] = "cardPlayed";
        msg["playerIndex"] = playerIndex;
        msg["cardIndex"] = cardIndex;
        msg["cardValue"] = static_cast<int>(cartePlayed->getChiffre());
        msg["cardSuit"] = static_cast<int>(cartePlayed->getCouleur());
        broadcastToRoom(roomId, msg);

        // Si le pli est complet (4 cartes)
        if (room->currentPli.size() == 4) {
            finishPli(roomId);
        } else {
            // Passer au joueur suivant
            room->currentPlayerIndex = (room->currentPlayerIndex + 1) % 4;
            qDebug() << "playBotCard - Prochain joueur:" << room->currentPlayerIndex
                     << "isBot:" << room->isBot[room->currentPlayerIndex];

            // notifyPlayersWithPlayableCards gère déjà le scheduling des bots
            notifyPlayersWithPlayableCards(roomId);
        }
    }

    void startSurcoincheTimer(int roomId) {
        GameRoom* room = m_gameRooms.value(roomId);
        if (!room) return;

        qDebug() << "GameServer - Attente de 7 secondes avant d'afficher le bouton Surcoinche (animation fusée + Coinche)";

        // Attendre 7 secondes et demie pour permettre l'animation fusée en spirale (5s) + explosion "Coinche !" (2s)
        QTimer::singleShot(7650, this, [this, roomId]() {
            GameRoom* room = m_gameRooms.value(roomId);
            if (!room) return;

            // Initialiser le temps restant à 10 secondes
            room->surcoincheTimeLeft = 10;

            // Créer le timer s'il n'existe pas
            if (!room->surcoincheTimer) {
                room->surcoincheTimer = new QTimer(this);
                connect(room->surcoincheTimer, &QTimer::timeout, this, [this, roomId]() {
                    onSurcoincheTimerTick(roomId);
                });
            }

            // Démarrer le timer (tick chaque seconde)
            room->surcoincheTimer->start(1000);

            qDebug() << "GameServer - Timer de surcoinche démarré pour la room" << roomId;

            // Envoyer des messages différents selon l'équipe
            int biddingTeam = room->lastBidderIndex % 2; // Équipe qui a fait l'annonce

            for (int i = 0; i < 4; i++) {
                QString connId = room->connectionIds[i];
                if (connId.isEmpty() || !m_connections.contains(connId)) continue;

                PlayerConnection* conn = m_connections[connId];
                if (!conn || !conn->socket) continue;

                int playerTeam = i % 2;
                QJsonObject msg;

                if (playerTeam == biddingTeam) {
                    // Équipe qui a fait l'annonce coichée : peut surcoincher
                    msg["type"] = "surcoincheOffer";
                    msg["timeLeft"] = room->surcoincheTimeLeft;
                } else {
                    // Équipe adverse (a coinché) : message d'attente
                    msg["type"] = "surcoincheWaiting";
                    msg["timeLeft"] = room->surcoincheTimeLeft;
                }

                sendMessage(conn->socket, msg);
            }
        });
    }

    void stopSurcoincheTimer(int roomId) {
        GameRoom* room = m_gameRooms.value(roomId);
        if (!room) return;

        if (room->surcoincheTimer) {
            room->surcoincheTimer->stop();
            qDebug() << "GameServer - Timer de surcoinche arrêté pour la room" << roomId;
        }

        room->surcoincheTimeLeft = 0;
    }

    void onSurcoincheTimerTick(int roomId) {
        GameRoom* room = m_gameRooms.value(roomId);
        if (!room) return;

        room->surcoincheTimeLeft--;

        qDebug() << "GameServer - Surcoinche timer tick, temps restant:" << room->surcoincheTimeLeft;

        if (room->surcoincheTimeLeft <= 0) {
            // Timeout atteint, arrêter le timer et lancer la phase de jeu
            stopSurcoincheTimer(roomId);

            qDebug() << "GameServer - Timeout surcoinche! Fin des enchères, lancement phase de jeu";

            // Notifier les joueurs du timeout
            QJsonObject timeoutMsg;
            timeoutMsg["type"] = "surcoincheTimeout";
            broadcastToRoom(roomId, timeoutMsg);

            // Lancer la phase de jeu
            for (int i = 0; i < 4; i++) {
                // IMPORTANT: Réinitialiser toutes les cartes avant de définir le nouvel atout
                const auto& main = room->players[i]->getMain();
                for (Carte* carte : main) {
                    carte->setAtout(false);
                }

                if (room->isToutAtout) {
                    room->players[i]->setAllCardsAsAtout();
                } else if (room->isSansAtout) {
                    room->players[i]->setNoAtout();
                } else {
                    room->players[i]->setAtout(room->lastBidCouleur);
                }
            }
            startPlayingPhase(roomId);
        } else {
            // Envoyer la mise à jour du temps restant (différente selon l'équipe)
            int biddingTeam = room->lastBidderIndex % 2;

            for (int i = 0; i < 4; i++) {
                QString connId = room->connectionIds[i];
                if (connId.isEmpty() || !m_connections.contains(connId)) continue;

                PlayerConnection* conn = m_connections[connId];
                if (!conn || !conn->socket) continue;

                int playerTeam = i % 2;
                QJsonObject msg;
                msg["timeLeft"] = room->surcoincheTimeLeft;

                if (playerTeam == biddingTeam) {
                    msg["type"] = "surcoincheTimeUpdate";
                } else {
                    msg["type"] = "surcoincheWaitingUpdate";
                }

                sendMessage(conn->socket, msg);
            }
        }
    }

    void startPlayingPhase(int roomId);

    void notifyPlayersWithPlayableCards(int roomId);

    QJsonArray calculatePlayableCards(GameRoom* room, int playerIndex);

    void handlePlayerDisconnect(const QString &connectionId);

    QString getConnectionIdBySocket(QWebSocket *socket) {
        // Vérifier que le socket est valide
        if (!socket) {
            return QString();
        }

        for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
            // Vérifier que la connexion existe et n'a pas été supprimée
            if (it.value() && it.value()->socket == socket) {
                return it.key();
            }
        }
        return QString();
    }

    // ========================================
    // Gestion des lobbies privés
    // ========================================

    QString generateLobbyCode() {
        const QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        QString code;
        for (int i = 0; i < 4; i++) {
            code += chars[QRandomGenerator::global()->bounded(chars.length())];
        }
        // Vérifier que le code n'existe pas déjà
        if (m_privateLobbies.contains(code)) {
            return generateLobbyCode();  // Régénérer si collision
        }
        return code;
    }

    void handleCreatePrivateLobby(QWebSocket *socket);

    void handleJoinPrivateLobby(QWebSocket *socket, const QJsonObject &obj);

    void handleLobbyReady(QWebSocket *socket, const QJsonObject &obj);

    void handleStartLobbyGame(QWebSocket *socket);

    void handleLeaveLobby(QWebSocket *socket);

    void sendLobbyUpdate(const QString &lobbyCode) {
        if (!m_privateLobbies.contains(lobbyCode)) return;

        PrivateLobby* lobby = m_privateLobbies[lobbyCode];

        QJsonArray playersArray;
        for (int i = 0; i < lobby->playerNames.size(); i++) {
            QJsonObject player;
            player["name"] = lobby->playerNames[i];
            player["avatar"] = lobby->playerAvatars[i];
            player["ready"] = lobby->readyStatus[i];
            player["isHost"] = (lobby->playerNames[i] == lobby->hostPlayerName);
            playersArray.append(player);
        }

        QJsonObject update;
        update["type"] = "lobbyUpdate";
        update["players"] = playersArray;

        sendLobbyMessage(lobbyCode, update);
    }

    void sendLobbyMessage(const QString &lobbyCode, const QJsonObject &message) {
        if (!m_privateLobbies.contains(lobbyCode)) return;

        PrivateLobby* lobby = m_privateLobbies[lobbyCode];

        for (const QString &playerName : lobby->playerNames) {
            // Trouver la connexion du joueur
            for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
                if (it.value() && it.value()->socket && it.value()->playerName == playerName) {
                    sendMessage(it.value()->socket, message);
                    break;
                }
            }
        }
    }

    void startLobbyGameWith4Players(PrivateLobby* lobby) {
        // Créer une partie avec les 4 joueurs du lobby
        QList<QString> connectionIds;

        for (const QString &playerName : lobby->playerNames) {
            for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
                if (it.value() && it.value()->playerName == playerName) {
                    connectionIds.append(it.key());
                    break;
                }
            }
        }

        if (connectionIds.size() != 4) {
            qDebug() << "Erreur: impossible de trouver les 4 connexions";
            return;
        }

        // Créer la partie (similaire à tryCreateGame)
        int roomId = m_nextRoomId++;
        GameRoom* room = new GameRoom();
        room->roomId = roomId;
        room->connectionIds = connectionIds;
        room->originalConnectionIds = connectionIds;
        room->gameState = "waiting";

        // Crée les joueurs du jeu
        for (int i = 0; i < 4; i++) {
            PlayerConnection* conn = m_connections.value(connectionIds[i]);
            if (!conn) {
                qDebug() << "ERREUR: Connection non trouvée pour lobby player" << i;
                continue;
            }
            conn->gameRoomId = roomId;
            conn->playerIndex = i;

            // Sauvegarder le nom et avatar du joueur pour reconnexion
            room->playerNames.append(conn->playerName);
            room->playerAvatars.append(conn->avatar);
            m_playerNameToRoomId[conn->playerName] = roomId;

            std::vector<Carte*> emptyHand;
            auto player = std::make_unique<Player>(
                conn->playerName.toStdString(),
                emptyHand,
                i
            );
            room->players.push_back(std::move(player));
            room->isBot.push_back(false);
        }

        // Distribue les cartes
        room->deck.shuffleDeck();
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 8; j++) {
                Carte* carte = room->deck.drawCard();
                if (carte) {
                    room->players[i]->addCardToHand(carte);
                }
            }
        }

        m_gameRooms[roomId] = room;

        // Init le premier joueur (celui qui commence les enchères)
        room->firstPlayerIndex = 0;
        room->currentPlayerIndex = 0;

        qInfo() << "Partie lobby créée - Room" << roomId << "- Joueurs:"
                << room->playerNames[0] << "," << room->playerNames[1] << ","
                << room->playerNames[2] << "," << room->playerNames[3];

        // Notifier tous les joueurs
        for (int i = 0; i < 4; i++) {
            QJsonArray opponentsArray;
            for (int j = 0; j < 4; j++) {
                if (j != i) {
                    QJsonObject opponent;
                    opponent["name"] = room->playerNames[j];
                    opponent["avatar"] = room->playerAvatars[j];
                    opponent["position"] = j;
                    opponentsArray.append(opponent);
                }
            }

            // Préparer les cartes du joueur
            QJsonArray myCards;
            for (Carte* carte : room->players[i]->getMain()) {
                QJsonObject cardObj;
                cardObj["suit"] = static_cast<int>(carte->getCouleur());
                cardObj["value"] = static_cast<int>(carte->getChiffre());
                myCards.append(cardObj);
            }

            QJsonObject gameFoundMsg;
            gameFoundMsg["type"] = "gameFound";
            gameFoundMsg["playerPosition"] = i;
            gameFoundMsg["opponents"] = opponentsArray;
            gameFoundMsg["myCards"] = myCards;  // Ajouter les cartes ici

            sendMessage(m_connections[connectionIds[i]]->socket, gameFoundMsg);
        }

        // Les cartes sont maintenant incluses dans gameFound, pas besoin de message séparé

        // Phase d'enchères
        room->gameState = "bidding";
        QJsonObject biddingMsg;
        biddingMsg["type"] = "biddingPhase";
        biddingMsg["currentPlayer"] = room->currentPlayerIndex;
        broadcastToRoom(roomId, biddingMsg);

        // Démarrer le timer pour le premier joueur à annoncer (toujours humain dans un lobby)
        startBidTimeout(roomId, room->currentPlayerIndex);
    }

    void startLobbyGameWith2Players(PrivateLobby* lobby) {
        // Ajouter les 2 joueurs au matchmaking en tant que partenaires
        // Ils seront placés aux positions 0 et 2 (partenaires)
        qDebug() << "Matchmaking avec 2 joueurs partenaires - ajout à la queue";

        QList<QString> lobbyConnectionIds;

        // Récupérer les IDs de connexion des 2 joueurs du lobby
        for (const QString &playerName : lobby->playerNames) {
            for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
                if (it.value() && it.value()->playerName == playerName) {
                    lobbyConnectionIds.append(it.key());
                    break;
                }
            }
        }

        if (lobbyConnectionIds.size() != 2) {
            qDebug() << "Erreur: impossible de trouver les 2 connexions du lobby";
            return;
        }

        // Marquer les 2 joueurs comme partenaires
        PlayerConnection* player1 = m_connections[lobbyConnectionIds[0]];
        PlayerConnection* player2 = m_connections[lobbyConnectionIds[1]];
        player1->lobbyPartnerId = lobbyConnectionIds[1];
        player2->lobbyPartnerId = lobbyConnectionIds[0];

        qDebug() << "Joueurs marqués comme partenaires:" << player1->playerName << "et" << player2->playerName;

        // Ajouter à la queue de matchmaking
        for (const QString &connId : lobbyConnectionIds) {
            if (!m_matchmakingQueue.contains(connId)) {
                m_matchmakingQueue.enqueue(connId);
                qDebug() << "Joueur du lobby ajouté à la queue:" << m_connections[connId]->playerName;
            }
        }

        // Notifier tous les joueurs en queue
        QJsonObject response;
        response["type"] = "matchmakingStatus";
        response["status"] = "searching";
        response["playersInQueue"] = m_matchmakingQueue.size();

        for (const QString &queuedConnectionId : m_matchmakingQueue) {
            if (m_connections.contains(queuedConnectionId)) {
                sendMessage(m_connections[queuedConnectionId]->socket, response);
            }
        }

        // Vérifier si on peut créer une partie
        tryCreateGame();
    }

    void sendMessage(QWebSocket *socket, const QJsonObject &message) {
        // Protection contre les sockets null ou déconnectés
        if (!socket) {
            qCritical() << "SEGFAULT évité - Tentative d'envoi à socket null";
            return;
        }

        if (socket->state() != QAbstractSocket::ConnectedState) {
            qWarning() << "Socket déconnecté - Message non envoyé";
            return;
        }

        QJsonDocument doc(message);
        socket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
    }

    void broadcastToRoom(int roomId, const QJsonObject &message,
                        const QString &excludeConnectionId = QString()) {
        if (!m_gameRooms.contains(roomId)) return;

        GameRoom* room = m_gameRooms[roomId];
        QString msgType = message["type"].toString();

        // Log détaillé pour les messages de jeu importants
        bool isImportantMsg = (msgType == "gameState" || msgType == "cardPlayed" || msgType == "pliFinished");
        if (isImportantMsg) {
            qDebug() << "broadcastToRoom -" << msgType << "à room" << roomId;
        }

        for (int i = 0; i < room->connectionIds.size(); i++) {
            const QString &connId = room->connectionIds[i];

            // Ignorer les connectionIds vides (joueurs déconnectés)
            if (connId.isEmpty()) {
                if (isImportantMsg) {
                    qDebug() << "  Joueur" << i << ": connectionId VIDE - message non envoyé (isBot:" << room->isBot[i] << ")";
                }
                continue;
            }

            if (connId == excludeConnectionId) {
                continue;
            }

            PlayerConnection *conn = m_connections.value(connId);
            if (conn && conn->socket) {
                sendMessage(conn->socket, message);
                if (isImportantMsg) {
                    qDebug() << "  Joueur" << i << ": message envoyé OK";
                }
            } else {
                if (isImportantMsg) {
                    qDebug() << "  Joueur" << i << ": connexion invalide - message non envoyé (conn:" << (conn != nullptr) << ")";
                }
            }
        }
    }

    QWebSocketServer *m_server;
    QMap<QString, PlayerConnection*> m_connections; // connectionId → PlayerConnection
    QQueue<QString> m_matchmakingQueue;
    QMap<int, GameRoom*> m_gameRooms;
    QMap<QString, int> m_playerNameToRoomId;  // playerName → roomId pour reconnexion
    QMap<QString, PrivateLobby*> m_privateLobbies;  // code → PrivateLobby
    int m_nextRoomId;
    DatabaseManager *m_dbManager;
    QString m_smtpPassword;  // Mot de passe SMTP pour l'envoi d'emails
    StatsReporter *m_statsReporter;  // Rapports quotidiens de statistiques

    // Timer pour démarrer une partie avec des bots après 30 secondes d'inactivité
    QTimer *m_matchmakingTimer;
    int m_lastQueueSize;  // Pour détecter si de nouveaux joueurs arrivent

    // Timer pour le compte à rebours (10 dernières secondes)
    QTimer *m_countdownTimer;
    int m_countdownSeconds;
};

#endif // GAMESERVER_H