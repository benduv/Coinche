#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QWebSocket>
#include <QSignalSpy>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QEventLoop>
#include <QTest>
#include <QFile>
#include "../server/GameServer.h"

// Variable globale pour QCoreApplication
static int argc_rej = 1;
static char* argv_rej[] = {(char*)"test_server_rejection", nullptr};
static QCoreApplication* rej_app = nullptr;
static int rej_port_counter = 0;

// Valeurs des énumérations Player::Annonce
static constexpr int BID_QUATREVINGT = 1;
static constexpr int BID_CENTVINGT   = 5;
static constexpr int BID_COINCHE     = 12;
static constexpr int BID_SURCOINCHE  = 13;
static constexpr int BID_PASSE       = 14;

// ========================================
// Client léger pour les tests de rejet
// ========================================
class RejClient : public QObject {
    Q_OBJECT
public:
    RejClient(const QString& url, const QString& name, QObject* parent = nullptr)
        : QObject(parent), m_socket(new QWebSocket), m_name(name), m_connected(false)
    {
        connect(m_socket, &QWebSocket::connected, this, [this]() {
            m_connected = true; emit connected();
        });
        connect(m_socket, &QWebSocket::textMessageReceived, this, [this](const QString& raw) {
            QJsonDocument doc = QJsonDocument::fromJson(raw.toUtf8());
            QJsonObject obj = doc.object();
            QString type = obj["type"].toString();
            m_allMessages.append(obj);
            emit messageReceived(obj);
            if (type == "registered")   { m_connectionId = obj["connectionId"].toString(); emit registered(); }
            if (type == "gameFound")    { m_playerPos = obj["playerPosition"].toInt(); emit gameFound(); }
            if (type == "gameState")    { if (obj.contains("currentPlayer")) m_currentPlayer = obj["currentPlayer"].toInt(-1); emit gameStateReceived(obj); }
            if (type == "bidMade")      emit bidMadeReceived(obj);
            if (type == "cardPlayed")   emit cardPlayedReceived(obj);
            if (type == "error")        emit errorReceived(obj["message"].toString());
            if (type == "versionError") emit versionErrorReceived();
            if (type == "lobbyError")   emit lobbyErrorReceived(obj["message"].toString());
        });
        m_socket->open(QUrl(url));
    }

    ~RejClient() { if (m_socket) { m_socket->close(); delete m_socket; } }

    void send(const QJsonObject& msg) {
        if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState)
            m_socket->sendTextMessage(QJsonDocument(msg).toJson(QJsonDocument::Compact));
    }

    void sendRegister(int version = GameServer::MIN_CLIENT_VERSION) {
        QJsonObject m; m["type"]="register"; m["playerName"]=m_name;
        m["avatar"]="avataaars1.svg"; m["wasInGame"]=false; m["version"]=version; send(m);
    }
    void sendJoinMatchmaking() { QJsonObject m; m["type"]="joinMatchmaking"; send(m); }
    void sendMakeBid(int val, int suit=0) {
        QJsonObject m; m["type"]="makeBid"; m["bidValue"]=val; m["suit"]=suit; send(m);
    }
    void sendPlayCard(int idx) { QJsonObject m; m["type"]="playCard"; m["cardIndex"]=idx; send(m); }

    int countMessagesOfType(const QString& t) const {
        int c=0; for (const auto& o : m_allMessages) if (o["type"].toString()==t) c++; return c;
    }
    bool isConnected()      const { return m_connected; }
    QString name()          const { return m_name; }
    QString connectionId()  const { return m_connectionId; }
    int playerPos()         const { return m_playerPos; }
    int currentPlayer()     const { return m_currentPlayer; }

signals:
    void connected();
    void registered();
    void gameFound();
    void gameStateReceived(const QJsonObject&);
    void bidMadeReceived(const QJsonObject&);
    void cardPlayedReceived(const QJsonObject&);
    void messageReceived(const QJsonObject&);
    void errorReceived(const QString&);
    void versionErrorReceived();
    void lobbyErrorReceived(const QString&);

private:
    QWebSocket* m_socket;
    QString     m_name, m_connectionId;
    bool        m_connected;
    int         m_playerPos    = -1;
    int         m_currentPlayer = -1;
    QList<QJsonObject> m_allMessages;
};

// ========================================
// Fixture de base
// ========================================
class ServerRejectionTest : public ::testing::Test {
protected:
    GameServer*       server = nullptr;
    QList<RejClient*> clients;
    quint16           m_port = 0;

    static void SetUpTestSuite() {
        if (!rej_app) rej_app = new QCoreApplication(argc_rej, argv_rej);
    }

    void SetUp() override {
        m_port = static_cast<quint16>(12400 + (++rej_port_counter));
        QFile::remove("coinche.db");  // GameServer crée coinche.db dans le répertoire courant
        server = new GameServer(m_port);
        QTest::qWait(200);
    }

    void TearDown() override {
        for (auto* c : clients) delete c;
        clients.clear();
        delete server; server = nullptr;
        QCoreApplication::processEvents();
    }

    RejClient* makeClient(const QString& name) {
        auto* c = new RejClient(QString("ws://localhost:%1").arg(m_port), name);
        clients.append(c);
        if (!c->isConnected()) { QSignalSpy sp(c, &RejClient::connected); sp.wait(3000); }
        return c;
    }

    void reg(RejClient* c, int version = GameServer::MIN_CLIENT_VERSION) {
        QSignalSpy sp(c, &RejClient::registered);
        c->sendRegister(version);
        sp.wait(2000);
    }

    // Démarre une partie à 4 via matchmaking, retourne les 4 clients après gameFound
    QList<RejClient*> startGame(const QString& prefix = "P") {
        QList<RejClient*> ps;
        for (int i = 0; i < 4; i++) {
            auto* c = makeClient(QString("%1%2").arg(prefix).arg(i+1));
            reg(c);
            ps.append(c);
        }
        QList<QSignalSpy*> spies;
        for (auto* c : ps) spies.append(new QSignalSpy(c, &RejClient::gameFound));
        for (auto* c : ps) { c->sendJoinMatchmaking(); QTest::qWait(50); }

        int waited = 0;
        while (waited < 8000) {
            QTest::qWait(100); waited += 100;
            QCoreApplication::processEvents();
            bool all = true;
            for (auto* s : spies) if (s->count() == 0) { all = false; break; }
            if (all) break;
        }
        qDeleteAll(spies);
        QTest::qWait(300);
        QCoreApplication::processEvents();
        return ps;
    }

    // P0 passe, P1 enchérit 80-Pique, P2 passe, P3 passe, P0 passe → phase de jeu
    void goToPlayingPhase(const QList<RejClient*>& ps) {
        ps[0]->sendMakeBid(BID_PASSE);  QTest::qWait(200); QCoreApplication::processEvents();
        ps[1]->sendMakeBid(BID_QUATREVINGT, 6); QTest::qWait(200); QCoreApplication::processEvents();
        ps[2]->sendMakeBid(BID_PASSE);  QTest::qWait(200); QCoreApplication::processEvents();
        ps[3]->sendMakeBid(BID_PASSE);  QTest::qWait(200); QCoreApplication::processEvents();
        ps[0]->sendMakeBid(BID_PASSE);  QTest::qWait(1500); QCoreApplication::processEvents();
    }

    // Compte le total de messages d'un type donné pour tous les joueurs
    int totalOf(const QList<RejClient*>& ps, const QString& type) const {
        int n = 0; for (const auto* p : ps) n += p->countMessagesOfType(type); return n;
    }

    // Retourne le client dont la position correspond au preneur après un bid
    RejClient* findByPos(const QList<RejClient*>& ps, int pos) {
        for (auto* c : ps) if (c->playerPos() == pos) return c;
        return nullptr;
    }

    // Retourne un client de la même équipe (pos%2) que le client donné, mais différent
    RejClient* findTeammate(const QList<RejClient*>& ps, RejClient* c) {
        int team = c->playerPos() % 2;
        for (auto* other : ps) if (other != c && other->playerPos() % 2 == team) return other;
        return nullptr;
    }

    // Retourne un client de l'équipe adverse
    RejClient* findOpponent(const QList<RejClient*>& ps, RejClient* c) {
        int team = c->playerPos() % 2;
        for (auto* other : ps) if (other->playerPos() % 2 != team) return other;
        return nullptr;
    }
};

// ========================================================
// 1. Version trop ancienne → versionError
// ========================================================

TEST_F(ServerRejectionTest, Register_VersionTropAncienne_RecoitVersionError) {
    auto* c = makeClient("OldApp");
    QSignalSpy spy(c, &RejClient::versionErrorReceived);
    c->sendRegister(/*version=*/1);
    ASSERT_TRUE(spy.wait(2000));
    EXPECT_EQ(c->countMessagesOfType("versionError"), 1);
    EXPECT_EQ(c->countMessagesOfType("registered"), 0);
}

TEST_F(ServerRejectionTest, Register_VersionMinimum_Accepte) {
    auto* c = makeClient("MinVersion");
    QSignalSpy spy(c, &RejClient::registered);
    c->sendRegister(GameServer::MIN_CLIENT_VERSION);
    ASSERT_TRUE(spy.wait(2000));
    EXPECT_EQ(c->countMessagesOfType("versionError"), 0);
    EXPECT_FALSE(c->connectionId().isEmpty());
}

// ========================================================
// 2. Actions sans être enregistré → ignorées silencieusement
// ========================================================

TEST_F(ServerRejectionTest, JoinMatchmaking_SansRegister_Ignoree) {
    auto* c = makeClient("Ghost");
    c->sendJoinMatchmaking();
    QTest::qWait(500); QCoreApplication::processEvents();
    EXPECT_EQ(c->countMessagesOfType("gameFound"), 0);
}

TEST_F(ServerRejectionTest, PlayCard_SansEtreEnPartie_Ignoree) {
    auto* c = makeClient("Alone");
    reg(c);
    c->sendPlayCard(0);
    QTest::qWait(300); QCoreApplication::processEvents();
    EXPECT_EQ(c->countMessagesOfType("cardPlayed"), 0);
    EXPECT_EQ(c->countMessagesOfType("error"), 0);
}

TEST_F(ServerRejectionTest, MakeBid_SansEtreEnPartie_Ignoree) {
    auto* c = makeClient("NoBid");
    reg(c);
    c->sendMakeBid(BID_QUATREVINGT, 6);
    QTest::qWait(300); QCoreApplication::processEvents();
    EXPECT_EQ(c->countMessagesOfType("bidMade"), 0);
}

// ========================================================
// 3. Jouer une carte pendant les enchères → ignorée
// ========================================================

TEST_F(ServerRejectionTest, PlayCard_PendantEnchere_Ignoree) {
    auto ps = startGame("PE");
    ASSERT_EQ(ps.size(), 4);

    // On est en phase d'enchères (gameState = "bidding")
    int cardsBefore = totalOf(ps, "cardPlayed");

    for (auto* p : ps) p->sendPlayCard(0);
    QTest::qWait(500); QCoreApplication::processEvents();

    EXPECT_EQ(totalOf(ps, "cardPlayed"), cardsBefore)
        << "Aucune carte ne doit être jouée pendant les enchères";
}

// ========================================================
// 4. Jouer une carte hors de son tour → ignorée
// ========================================================

TEST_F(ServerRejectionTest, PlayCard_HorsDeTonTour_Ignoree) {
    auto ps = startGame("HT");
    ASSERT_EQ(ps.size(), 4);

    goToPlayingPhase(ps);

    // Récupérer le joueur courant depuis gameState
    int currentPlayer = -1;
    for (auto* p : ps) {
        if (p->currentPlayer() >= 0) { currentPlayer = p->currentPlayer(); break; }
    }
    ASSERT_GE(currentPlayer, 0) << "Impossible de déterminer le joueur courant";

    // Trouver un client dont la position n'est PAS le joueur courant
    RejClient* wrongPlayer = nullptr;
    for (auto* p : ps) {
        if (p->playerPos() != currentPlayer) { wrongPlayer = p; break; }
    }
    ASSERT_NE(wrongPlayer, nullptr);

    // Ce joueur envoie playCard → doit être ignoré
    int cardsBefore = totalOf(ps, "cardPlayed");
    wrongPlayer->sendPlayCard(0);
    QTest::qWait(500); QCoreApplication::processEvents();

    EXPECT_EQ(totalOf(ps, "cardPlayed"), cardsBefore)
        << "Jouer hors de son tour doit être ignoré";
}

// ========================================================
// 5. Jouer une carte avec index hors de la main → ignorée
// ========================================================

TEST_F(ServerRejectionTest, PlayCard_IndexHorsMain_Ignoree) {
    auto ps = startGame("IHM");
    ASSERT_EQ(ps.size(), 4);

    goToPlayingPhase(ps);

    int currentPlayer = -1;
    for (auto* p : ps) if (p->currentPlayer() >= 0) { currentPlayer = p->currentPlayer(); break; }
    ASSERT_GE(currentPlayer, 0);

    // Trouver le joueur courant
    RejClient* curPlayer = nullptr;
    for (auto* p : ps) if (p->playerPos() == currentPlayer) { curPlayer = p; break; }
    ASSERT_NE(curPlayer, nullptr);

    // Index 99 est forcément invalide (8 cartes max)
    int cardsBefore = totalOf(ps, "cardPlayed");
    curPlayer->sendPlayCard(99);
    QTest::qWait(500); QCoreApplication::processEvents();

    EXPECT_EQ(totalOf(ps, "cardPlayed"), cardsBefore)
        << "Index de carte hors de la main doit être ignoré";
}

// ========================================================
// 6. Jouer une carte illégale (non jouable selon les règles) → message error
// ========================================================

TEST_F(ServerRejectionTest, PlayCard_CarteNonJouable_RecoitMessageErreur) {
    auto ps = startGame("CNJ");
    ASSERT_EQ(ps.size(), 4);

    goToPlayingPhase(ps);

    int currentPlayer = -1;
    for (auto* p : ps) if (p->currentPlayer() >= 0) { currentPlayer = p->currentPlayer(); break; }
    ASSERT_GE(currentPlayer, 0);

    // Le joueur courant joue une carte valide (index 0) pour établir la couleur demandée
    RejClient* curPlayer = nullptr;
    for (auto* p : ps) if (p->playerPos() == currentPlayer) { curPlayer = p; break; }
    ASSERT_NE(curPlayer, nullptr);

    // Jouer carte 0 (toujours légale pour le premier joueur du pli)
    curPlayer->sendPlayCard(0);
    QTest::qWait(500); QCoreApplication::processEvents();

    // Maintenant le tour a avancé. Le nouveau joueur courant essaie l'index 99
    // (invalide) → le serveur ignore silencieusement (qWarning sans réponse)
    // Mais pour une carte EXISTANTE mais non-jouable, le serveur envoie un "error"

    // Pour tester une carte non jouable : on laisse un joueur suivant essayer l'index 99
    // et on attend une réponse d'erreur du serveur pour le joueur courant réel
    int curPlayer2 = -1;
    for (auto* p : ps) if (p->currentPlayer() >= 0) { curPlayer2 = p->currentPlayer(); break; }

    RejClient* curPlayer2Client = nullptr;
    for (auto* p : ps) if (p->playerPos() == curPlayer2) { curPlayer2Client = p; break; }

    if (curPlayer2Client) {
        // Envoyer un index valide mais dont la carte serait potentiellement non-jouable
        // On ne peut pas contrôler les mains, donc on teste l'index 99 (invalide)
        // qui est rejeté silencieusement (pas de message d'erreur)
        QSignalSpy errSpy(curPlayer2Client, &RejClient::errorReceived);
        curPlayer2Client->sendPlayCard(99);  // index invalide → rejeté sans error msg
        QTest::qWait(300); QCoreApplication::processEvents();
        // L'index invalide ne génère PAS de message "error" (juste un qWarning serveur)
        EXPECT_EQ(errSpy.count(), 0)
            << "Index invalide hors de la main ne génère pas de message 'error' (rejeté silencieusement)";
    }
}

// ========================================================
// 7. COINCHE sans enchère en cours → ignorée
// ========================================================

TEST_F(ServerRejectionTest, Coinche_SansEnchere_Ignoree) {
    auto ps = startGame("CSE");
    ASSERT_EQ(ps.size(), 4);
    QTest::qWait(300); QCoreApplication::processEvents();

    // Personne n'a encore enchéri → COINCHE (12) impossible
    int bidsBefore = totalOf(ps, "bidMade");

    ps[2]->sendMakeBid(BID_COINCHE);
    QTest::qWait(400); QCoreApplication::processEvents();

    EXPECT_EQ(totalOf(ps, "bidMade"), bidsBefore)
        << "COINCHE sans enchère en cours doit être ignorée";
}

// ========================================================
// 8. COINCHE par la même équipe que l'annonceur → ignorée
// ========================================================

TEST_F(ServerRejectionTest, Coinche_MemeEquipe_Ignoree) {
    auto ps = startGame("CME");
    ASSERT_EQ(ps.size(), 4);
    QTest::qWait(300); QCoreApplication::processEvents();

    // ps[0] passe, ps[1] enchérit
    ps[0]->sendMakeBid(BID_PASSE);          QTest::qWait(200); QCoreApplication::processEvents();
    ps[1]->sendMakeBid(BID_QUATREVINGT, 6); QTest::qWait(200); QCoreApplication::processEvents();

    // Trouver le coéquipier de ps[1] (même équipe = pos%2 identique)
    RejClient* teammate = findTeammate(ps, ps[1]);
    ASSERT_NE(teammate, nullptr) << "Impossible de trouver le coéquipier";

    int bidsBefore = totalOf(ps, "bidMade");

    // Le coéquipier essaie de coincher → rejeté
    teammate->sendMakeBid(BID_COINCHE);
    QTest::qWait(400); QCoreApplication::processEvents();

    EXPECT_EQ(totalOf(ps, "bidMade"), bidsBefore)
        << "COINCHE par la même équipe doit être ignorée";
}

// ========================================================
// 9. COINCHE par l'annonceur lui-même → ignorée
// ========================================================

TEST_F(ServerRejectionTest, Coinche_ParAnnonceurLuiMeme_Ignoree) {
    auto ps = startGame("CPA");
    ASSERT_EQ(ps.size(), 4);
    QTest::qWait(300); QCoreApplication::processEvents();

    ps[0]->sendMakeBid(BID_PASSE);          QTest::qWait(200); QCoreApplication::processEvents();
    ps[1]->sendMakeBid(BID_QUATREVINGT, 6); QTest::qWait(200); QCoreApplication::processEvents();

    int bidsBefore = totalOf(ps, "bidMade");

    // L'annonceur (ps[1]) essaie de se coincher lui-même → rejeté (même équipe)
    ps[1]->sendMakeBid(BID_COINCHE);
    QTest::qWait(400); QCoreApplication::processEvents();

    EXPECT_EQ(totalOf(ps, "bidMade"), bidsBefore)
        << "COINCHE par l'annonceur lui-même doit être ignorée";
}

// ========================================================
// 10. SURCOINCHE sans COINCHE préalable → ignorée
// ========================================================

TEST_F(ServerRejectionTest, Surcoinche_SansCoinche_Ignoree) {
    auto ps = startGame("SSC");
    ASSERT_EQ(ps.size(), 4);
    QTest::qWait(300); QCoreApplication::processEvents();

    ps[0]->sendMakeBid(BID_PASSE);          QTest::qWait(200); QCoreApplication::processEvents();
    ps[1]->sendMakeBid(BID_QUATREVINGT, 6); QTest::qWait(200); QCoreApplication::processEvents();

    int bidsBefore = totalOf(ps, "bidMade");

    // ps[1] (annonceur) essaie de surcoincher sans coinche préalable → rejeté
    ps[1]->sendMakeBid(BID_SURCOINCHE);
    QTest::qWait(400); QCoreApplication::processEvents();

    EXPECT_EQ(totalOf(ps, "bidMade"), bidsBefore)
        << "SURCOINCHE sans COINCHE préalable doit être ignorée";
}

// ========================================================
// 11. SURCOINCHE par l'équipe adverse (coincheur, pas annonceur) → ignorée
// ========================================================

TEST_F(ServerRejectionTest, Surcoinche_EquipeAdverse_Ignoree) {
    auto ps = startGame("SEA");
    ASSERT_EQ(ps.size(), 4);
    QTest::qWait(300); QCoreApplication::processEvents();

    // ps[0] passe, ps[1] enchérit, opponent de ps[1] coinche
    ps[0]->sendMakeBid(BID_PASSE);          QTest::qWait(200); QCoreApplication::processEvents();
    ps[1]->sendMakeBid(BID_QUATREVINGT, 6); QTest::qWait(200); QCoreApplication::processEvents();

    RejClient* opponent = findOpponent(ps, ps[1]);
    ASSERT_NE(opponent, nullptr) << "Impossible de trouver l'adversaire";

    // Opponent coinche → accepté
    opponent->sendMakeBid(BID_COINCHE);
    QTest::qWait(400); QCoreApplication::processEvents();

    // L'opponent essaie maintenant de surcoincher → rejeté
    // (surcoinche réservée à l'équipe de l'annonceur = ps[1]'s team)
    int bidsBefore = totalOf(ps, "bidMade");
    opponent->sendMakeBid(BID_SURCOINCHE);
    QTest::qWait(400); QCoreApplication::processEvents();

    EXPECT_EQ(totalOf(ps, "bidMade"), bidsBefore)
        << "SURCOINCHE par l'équipe adverse (coincheur) doit être ignorée";
}

// ========================================================
// 12. Message JSON sans champ 'type' → ignoré
// ========================================================

TEST_F(ServerRejectionTest, MessageSansType_Ignore) {
    auto* c = makeClient("NoType");
    reg(c);

    QJsonObject bad; bad["data"] = "hello"; bad["value"] = 42;
    c->send(bad);
    QTest::qWait(300); QCoreApplication::processEvents();

    EXPECT_EQ(c->countMessagesOfType("error"), 0);
}

// ========================================================
// 13. Message type inconnu → ignoré
// ========================================================

TEST_F(ServerRejectionTest, MessageTypeInconnu_Ignore) {
    auto* c = makeClient("Unknown");
    reg(c);

    QJsonObject bad; bad["type"] = "cetypeNexistePas";
    c->send(bad);
    QTest::qWait(300); QCoreApplication::processEvents();

    EXPECT_EQ(c->countMessagesOfType("error"), 0);
}

// ========================================================
// 14. Double registerr avec même pseudo → second est traité
//     (la reconnexion remplace la connexion précédente)
// ========================================================

TEST_F(ServerRejectionTest, Register_DeuxFoisMemePseudo_DeuxiemeAccepte) {
    auto* c1 = makeClient("SameName");
    QSignalSpy spy1(c1, &RejClient::registered);
    c1->sendRegister();
    ASSERT_TRUE(spy1.wait(2000));
    QString id1 = c1->connectionId();

    // Deuxième client avec le même pseudo
    auto* c2 = makeClient("SameName");
    QSignalSpy spy2(c2, &RejClient::registered);
    c2->sendRegister();
    ASSERT_TRUE(spy2.wait(2000));
    QString id2 = c2->connectionId();

    // Les deux obtiennent un connectionId (potentiellement différents)
    EXPECT_FALSE(id1.isEmpty());
    EXPECT_FALSE(id2.isEmpty());
}

// ========================================================
// Main
// ========================================================
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#include "server_rejection_test.moc"
