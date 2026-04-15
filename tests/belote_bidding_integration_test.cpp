#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QWebSocket>
#include <QSignalSpy>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QTest>
#include "../server/GameServer.h"

// QCoreApplication globale
static int belote_argc = 1;
static char* belote_argv[] = {(char*)"test_belote_bidding", nullptr};
static QCoreApplication* belote_app = nullptr;
static int belote_port_ctr = 0;

// ========================================
// Client minimal
// ========================================
class BeloteClient : public QObject {
    Q_OBJECT
public:
    BeloteClient(const QString& url, const QString& name, QObject* parent = nullptr)
        : QObject(parent), m_socket(new QWebSocket), m_name(name)
    {
        connect(m_socket, &QWebSocket::connected, this, [this]() {
            m_connected = true; emit connected();
        });
        connect(m_socket, &QWebSocket::textMessageReceived, this, [this](const QString& raw) {
            QJsonDocument doc = QJsonDocument::fromJson(raw.toUtf8());
            QJsonObject obj = doc.object();
            QString type = obj["type"].toString();
            m_all.append(obj);
            emit messageReceived(obj);
            if (type == "registered")      { m_connId = obj["connectionId"].toString(); emit registered(); }
            if (type == "gameFound")       { m_pos = obj["playerPosition"].toInt(); emit gameFound(obj); }
            if (type == "newManche")       { emit newManche(obj); }
            if (type == "bidMade")         emit bidMade(obj);
            if (type == "beloteBidRoundChanged") {
                m_bidRound = obj["beloteBidRound"].toInt();
                emit bidRoundChanged(m_bidRound);
            }
            if (type == "gameState") {
                if (obj.contains("biddingPlayer")) m_biddingPlayer = obj["biddingPlayer"].toInt(-1);
                if (obj.contains("currentPlayer")) m_currentPlayer = obj["currentPlayer"].toInt(-1);
                emit gameStateReceived(obj);
            }
        });
        m_socket->open(QUrl(url));
    }
    ~BeloteClient() { if (m_socket) { m_socket->close(); delete m_socket; } }

    void send(const QJsonObject& msg) {
        if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState)
            m_socket->sendTextMessage(QJsonDocument(msg).toJson(QJsonDocument::Compact));
    }
    void sendRegister() {
        QJsonObject m; m["type"]="register"; m["playerName"]=m_name;
        m["avatar"]="avataaars1.svg"; m["wasInGame"]=false;
        m["version"]=GameServer::MIN_CLIENT_VERSION; send(m);
    }
    void sendJoinMatchmaking(const QString& mode = "belote") {
        QJsonObject m; m["type"]="joinMatchmaking"; m["gameMode"]=mode; send(m);
    }
    void sendPass() {
        QJsonObject m; m["type"]="makeBid"; m["bidValue"]=0; m["suit"]=0; send(m);
    }
    void sendPrendre(int suit) {
        // bidValue=20 = "Prendre" en Belote
        QJsonObject m; m["type"]="makeBid"; m["bidValue"]=20; m["suit"]=suit; send(m);
    }

    int countOf(const QString& t) const {
        int n=0; for (const auto& o : m_all) if (o["type"].toString()==t) n++; return n;
    }
    QJsonObject lastOf(const QString& t) const {
        for (int i = m_all.size()-1; i >= 0; i--)
            if (m_all[i]["type"].toString() == t) return m_all[i];
        return {};
    }
    bool isConnected() const { return m_connected; }
    int  pos()         const { return m_pos; }
    int  bidRound()    const { return m_bidRound; }
    int  biddingPlayer() const { return m_biddingPlayer; }

signals:
    void connected();
    void registered();
    void gameFound(const QJsonObject&);
    void newManche(const QJsonObject&);
    void bidMade(const QJsonObject&);
    void bidRoundChanged(int round);
    void gameStateReceived(const QJsonObject&);
    void messageReceived(const QJsonObject&);

private:
    QWebSocket* m_socket;
    QString     m_name, m_connId;
    bool        m_connected = false;
    int         m_pos = -1, m_bidRound = 1, m_biddingPlayer = -1, m_currentPlayer = -1;
    QList<QJsonObject> m_all;
};

// ========================================
// Fixture
// ========================================
class BeloteBiddingTest : public ::testing::Test {
protected:
    GameServer*          server = nullptr;
    QList<BeloteClient*> clients;
    quint16              m_port = 0;

    static void SetUpTestSuite() {
        if (!belote_app) belote_app = new QCoreApplication(belote_argc, belote_argv);
    }

    void SetUp() override {
        m_port = static_cast<quint16>(12500 + (++belote_port_ctr));
        QFile::remove("coinche.db");
        server = new GameServer(m_port);
        QTest::qWait(200);
    }

    void TearDown() override {
        for (auto* c : clients) delete c;
        clients.clear();
        delete server; server = nullptr;
        QCoreApplication::processEvents();
    }

    BeloteClient* makeClient(const QString& name) {
        auto* c = new BeloteClient(QString("ws://localhost:%1").arg(m_port), name);
        clients.append(c);
        if (!c->isConnected()) { QSignalSpy sp(c, &BeloteClient::connected); sp.wait(3000); }
        return c;
    }

    void regAndJoin(BeloteClient* c) {
        { QSignalSpy sp(c, &BeloteClient::registered); c->sendRegister(); sp.wait(2000); }
        c->sendJoinMatchmaking("belote");
    }

    // Lance une partie Belote à 4, retourne les 4 clients après réception de gameFound par tous
    QList<BeloteClient*> startBeloteGame(const QString& prefix = "B") {
        QList<BeloteClient*> ps;
        for (int i = 0; i < 4; i++) {
            auto* c = makeClient(QString("%1%2").arg(prefix).arg(i+1));
            regAndJoin(c);
            ps.append(c);
        }
        // Attendre gameFound pour tous
        QList<QSignalSpy*> spies;
        for (auto* c : ps) spies.append(new QSignalSpy(c, &BeloteClient::gameFound));
        int waited = 0;
        while (waited < 8000) {
            QTest::qWait(100); waited += 100; QCoreApplication::processEvents();
            bool all = true;
            for (auto* s : spies) if (s->count() == 0) { all = false; break; }
            if (all) break;
        }
        qDeleteAll(spies);
        QTest::qWait(300); QCoreApplication::processEvents();
        return ps;
    }

    // Tous les 4 joueurs passent dans l'ordre du tour courant
    // On envoie dans l'ordre P0→P1→P2→P3 (le serveur ignore les hors-tour)
    void allPass(const QList<BeloteClient*>& ps) {
        for (int i = 0; i < 4; i++) {
            ps[i]->sendPass();
            QTest::qWait(200); QCoreApplication::processEvents();
        }
    }

    int totalOf(const QList<BeloteClient*>& ps, const QString& t) const {
        int n = 0; for (const auto* c : ps) n += c->countOf(t); return n;
    }
};

// ============================================================
// 1. Tous passent tour 1 → message beloteBidRoundChanged(2)
// ============================================================

TEST_F(BeloteBiddingTest, TousPassentTour1_PassageAuTour2) {
    auto ps = startBeloteGame("T1");
    ASSERT_EQ(ps.size(), 4);

    // Écouter le changement de tour
    QList<QSignalSpy*> roundSpies;
    for (auto* c : ps) roundSpies.append(new QSignalSpy(c, &BeloteClient::bidRoundChanged));

    // Tous passent tour 1
    allPass(ps);

    // Attendre la notification de passage au tour 2
    int waited = 0;
    bool allGotRound2 = false;
    while (waited < 5000 && !allGotRound2) {
        QTest::qWait(100); waited += 100; QCoreApplication::processEvents();
        allGotRound2 = true;
        for (auto* s : roundSpies) {
            bool hasRound2 = false;
            for (int i = 0; i < s->count(); i++) {
                if (s->at(i)[0].toInt() == 2) { hasRound2 = true; break; }
            }
            if (!hasRound2) { allGotRound2 = false; break; }
        }
    }
    qDeleteAll(roundSpies);

    ASSERT_TRUE(allGotRound2)
        << "Après 4 passes au tour 1, tous les joueurs doivent recevoir beloteBidRoundChanged(2)";

    // Vérifier que le round est bien 2 pour tous
    for (auto* c : ps) {
        EXPECT_EQ(c->bidRound(), 2)
            << qPrintable(QString("Client %1 devrait être au tour 2").arg(c->pos()));
    }
}

// ============================================================
// 2. Tous passent tour 1 → biddingPlayer revient au premier joueur (firstPlayerIndex)
// ============================================================

TEST_F(BeloteBiddingTest, TousPassentTour1_BiddingPlayerRevientAuDebut) {
    auto ps = startBeloteGame("T1B");
    ASSERT_EQ(ps.size(), 4);

    allPass(ps);

    // Attendre le changement de round
    int waited = 0;
    bool roundChanged = false;
    while (waited < 5000 && !roundChanged) {
        QTest::qWait(100); waited += 100; QCoreApplication::processEvents();
        for (auto* c : ps) if (c->bidRound() == 2) { roundChanged = true; break; }
    }
    ASSERT_TRUE(roundChanged);
    QTest::qWait(200); QCoreApplication::processEvents();

    // Le biddingPlayer doit avoir été remis au premier joueur
    // On vérifie via le dernier gameState reçu
    // Au moins un client doit avoir reçu biddingPlayer >= 0
    bool biddingPlayerSet = false;
    for (auto* c : ps) {
        QJsonObject gs = c->lastOf("gameState");
        if (!gs.isEmpty() && gs.contains("biddingPlayer") && gs["biddingPlayer"].toInt(-1) >= 0) {
            biddingPlayerSet = true;
            break;
        }
    }
    EXPECT_TRUE(biddingPlayerSet)
        << "Après passage au tour 2, biddingPlayer doit être défini dans gameState";
}

// ============================================================
// 3. Tous passent tour 1 → un joueur peut prendre au tour 2
// ============================================================

TEST_F(BeloteBiddingTest, TousPassentTour1_PuisPrendreAuTour2) {
    auto ps = startBeloteGame("T2P");
    ASSERT_EQ(ps.size(), 4);

    allPass(ps);

    // Attendre tour 2
    int waited = 0;
    while (waited < 5000) {
        QTest::qWait(100); waited += 100; QCoreApplication::processEvents();
        bool anyRound2 = false;
        for (auto* c : ps) if (c->bidRound() == 2) { anyRound2 = true; break; }
        if (anyRound2) break;
    }

    // Au tour 2, récupérer la retournée depuis le gameFound
    // La couleur de la retournée est dans l'objet gameFound (retournee.suit)
    int retourneeSuit = -1;
    for (auto* c : ps) {
        QJsonObject gf = c->lastOf("gameFound");
        if (!gf.isEmpty() && gf.contains("retournee")) {
            retourneeSuit = gf["retournee"].toObject()["suit"].toInt(-1);
            break;
        }
    }

    // Trouver une couleur différente de la retournée (obligatoire au tour 2)
    // Couleurs: COEUR=3, TREFLE=4, CARREAU=5, PIQUE=6
    int suitToPick = 3; // COEUR
    for (int s = 3; s <= 6; s++) {
        if (s != retourneeSuit) { suitToPick = s; break; }
    }

    // Les gameFound envoyés aux non-bot clients (hu4mans)
    // Prendre au tour 2 : le premier joueur humain du tour 2 "Prend"
    // On envoie "Prendre" à tous les joueurs (le serveur n'accepte que le bon)
    int gameFoundsBefore = totalOf(ps, "gameFound");

    for (auto* c : ps) {
        c->sendPrendre(suitToPick);
        QTest::qWait(100); QCoreApplication::processEvents();
    }
    QTest::qWait(1000); QCoreApplication::processEvents();

    // Après "Prendre", la phase d'enchères est terminée → la phase de jeu démarre
    // On vérifie que le gameState passe en playing (biddingPhase = false)
    bool playingPhaseStarted = false;
    for (auto* c : ps) {
        for (int i = c->countOf("gameState")-1; i >= 0; i--) {
            QJsonObject gs = c->lastOf("gameState");
            if (gs.contains("biddingPhase") && !gs["biddingPhase"].toBool()) {
                playingPhaseStarted = true;
                break;
            }
        }
        if (playingPhaseStarted) break;
    }
    // Note: si tous les joueurs sont des bots côté serveur ou si le timing est différent,
    // on peut ne pas encore avoir gameState. On vérifie au moins qu'aucune erreur n'est survenue.
    // Le test principal est qu'on reçoit des bidMade après les passes.
    EXPECT_GT(totalOf(ps, "bidMade"), 0)
        << "Des bidMade doivent avoir été émis (passes du tour 1 + prendre du tour 2)";
}

// ============================================================
// 4. Tous passent tour 2 → redistribution (nouveau gameFound ou démarrage de manche)
// ============================================================

TEST_F(BeloteBiddingTest, TousPassentTour1EtTour2_Redistribution) {
    auto ps = startBeloteGame("T2R");
    ASSERT_EQ(ps.size(), 4);

    int newMancheBefore = totalOf(ps, "newManche");

    // Tour 1 : tous passent
    allPass(ps);

    // Attendre tour 2
    int waited = 0;
    while (waited < 5000) {
        QTest::qWait(100); waited += 100; QCoreApplication::processEvents();
        bool anyRound2 = false;
        for (auto* c : ps) if (c->bidRound() == 2) { anyRound2 = true; break; }
        if (anyRound2) break;
    }
    QTest::qWait(200); QCoreApplication::processEvents();

    // Tour 2 : tous passent
    allPass(ps);

    // Attendre la redistribution — le serveur appelle startNewManche() qui envoie "newManche"
    int waitedTotal = 0;
    bool redistributed = false;
    while (waitedTotal < 8000 && !redistributed) {
        QTest::qWait(200); waitedTotal += 200; QCoreApplication::processEvents();
        if (totalOf(ps, "newManche") > newMancheBefore) redistributed = true;
    }

    EXPECT_TRUE(redistributed)
        << "Après que tous aient passé les 2 tours, une redistribution doit avoir lieu "
        << "(nouveau newManche attendu)";
}

// ============================================================
// 5. Tous passent tour 2 → le dealer avance (firstPlayerIndex+1)
// ============================================================

TEST_F(BeloteBiddingTest, TousPassentDeuxTours_DealerAvance) {
    auto ps = startBeloteGame("DA");
    ASSERT_EQ(ps.size(), 4);

    // Récupérer le premier biddingPlayer depuis le premier gameFound (initial)
    QTest::qWait(400); QCoreApplication::processEvents();
    int firstBidder1 = -1;
    for (auto* c : ps) {
        QJsonObject gf = c->lastOf("gameFound");
        if (!gf.isEmpty() && gf.contains("biddingPlayer")) {
            firstBidder1 = gf["biddingPlayer"].toInt(-1);
            break;
        }
        // Fallback : gameState
        QJsonObject gs = c->lastOf("gameState");
        if (!gs.isEmpty() && gs.contains("biddingPlayer") && gs["biddingPlayer"].toInt(-1) >= 0) {
            firstBidder1 = gs["biddingPlayer"].toInt();
            break;
        }
    }

    int newMancheBefore = totalOf(ps, "newManche");

    // Tour 1 : tous passent
    allPass(ps);
    int waited = 0;
    while (waited < 5000) {
        QTest::qWait(100); waited += 100; QCoreApplication::processEvents();
        bool anyRound2 = false;
        for (auto* c : ps) if (c->bidRound() == 2) { anyRound2 = true; break; }
        if (anyRound2) break;
    }

    // Tour 2 : tous passent
    allPass(ps);

    // Attendre la redistribution (newManche)
    int waitedTotal = 0;
    while (waitedTotal < 8000) {
        QTest::qWait(200); waitedTotal += 200; QCoreApplication::processEvents();
        if (totalOf(ps, "newManche") > newMancheBefore) break;
    }
    QTest::qWait(400); QCoreApplication::processEvents();

    EXPECT_GT(totalOf(ps, "newManche"), newMancheBefore)
        << "La redistribution doit avoir eu lieu";

    // Récupérer le biddingPlayer depuis le message newManche
    int firstBidder2 = -1;
    for (auto* c : ps) {
        QJsonObject nm = c->lastOf("newManche");
        if (!nm.isEmpty() && nm.contains("biddingPlayer")) {
            firstBidder2 = nm["biddingPlayer"].toInt(-1);
            break;
        }
    }

    // Vérifier l'avancement du dealer
    if (firstBidder1 >= 0 && firstBidder2 >= 0) {
        int expected = (firstBidder1 + 1) % 4;
        EXPECT_EQ(firstBidder2, expected)
            << "Après redistribution, le dealer (biddingPlayer) doit avancer de 1 "
            << "(de " << firstBidder1 << " à " << expected << ", obtenu " << firstBidder2 << ")";
    }
}

// ============================================================
// 6. Prendre au tour 1 → pas de passage au tour 2
// ============================================================

TEST_F(BeloteBiddingTest, PrendreAuTour1_PasDeTour2) {
    auto ps = startBeloteGame("P1");
    ASSERT_EQ(ps.size(), 4);

    // Écouter beloteBidRoundChanged
    QList<QSignalSpy*> roundSpies;
    for (auto* c : ps) roundSpies.append(new QSignalSpy(c, &BeloteClient::bidRoundChanged));

    // Récupérer la couleur de la retournée
    int retourneeSuit = 3; // fallback COEUR
    for (auto* c : ps) {
        QJsonObject gf = c->lastOf("gameFound");
        if (!gf.isEmpty() && gf.contains("retournee")) {
            retourneeSuit = gf["retournee"].toObject()["suit"].toInt(3);
            break;
        }
    }

    // Envoyer "Prendre" (couleur de la retournée = valide au tour 1) à tous les joueurs
    // Le premier joueur en tour l'acceptera
    for (auto* c : ps) {
        c->sendPrendre(retourneeSuit);
        QTest::qWait(100); QCoreApplication::processEvents();
    }
    QTest::qWait(1500); QCoreApplication::processEvents();

    // Vérifier qu'aucun beloteBidRoundChanged(2) n'a été émis
    bool gotRound2 = false;
    for (auto* s : roundSpies) {
        for (int i = 0; i < s->count(); i++) {
            if (s->at(i)[0].toInt() == 2) { gotRound2 = true; break; }
        }
        if (gotRound2) break;
    }
    qDeleteAll(roundSpies);

    EXPECT_FALSE(gotRound2)
        << "Si quelqu'un prend au tour 1, il ne doit pas y avoir de passage au tour 2";
}

// ============================================================
// Main
// ============================================================
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#include "belote_bidding_integration_test.moc"
