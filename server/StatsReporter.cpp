#include "StatsReporter.h"
#include <QDateTime>
#include <QDebug>
#include <cmath>

StatsReporter::StatsReporter(DatabaseManager *dbManager, const QString &smtpPassword, QObject *parent)
    : QObject(parent)
    , m_dbManager(dbManager)
    , m_smtpPassword(smtpPassword)
    , m_dailyTimer(new QTimer(this))
    , m_reportHour(0)  // Minuit par défaut
    , m_reportMinute(0)
{
    // Connecter le timer
    connect(m_dailyTimer, &QTimer::timeout, this, &StatsReporter::checkAndSendReport);

    // Démarrer le timer (vérification toutes les heures)
    m_dailyTimer->start(3600000); // 1 heure en millisecondes

    // Planifier le premier envoi
    scheduleNextReport();

    qInfo() << "StatsReporter initialisé - Rapport quotidien à" << m_reportHour << "h" << m_reportMinute;
}

StatsReporter::~StatsReporter()
{
    if (m_dailyTimer) {
        m_dailyTimer->stop();
    }
}

void StatsReporter::setReportTime(int hour, int minute)
{
    m_reportHour = hour;
    m_reportMinute = minute;
    scheduleNextReport();
    qInfo() << "Heure de rapport modifiée:" << hour << "h" << minute;
}

void StatsReporter::scheduleNextReport()
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime nextReport = now;
    nextReport.setTime(QTime(m_reportHour, m_reportMinute, 0));

    // Si l'heure est déjà passée aujourd'hui, planifier pour demain
    if (nextReport <= now) {
        nextReport = nextReport.addDays(1);
    }

    qInfo() << "Prochain rapport planifié pour:" << nextReport.toString("yyyy-MM-dd hh:mm:ss");
}

void StatsReporter::checkAndSendReport()
{
    QDateTime now = QDateTime::currentDateTime();

    // Vérifier si c'est l'heure du rapport (avec une marge de 1 heure)
    if (now.time().hour() == m_reportHour) {
        qInfo() << "Il est l'heure d'envoyer le rapport quotidien!";
        sendDailyReport();
    }
}

void StatsReporter::sendDailyReport()
{
    if (m_smtpPassword.isEmpty()) {
        qWarning() << "StatsReporter - Mot de passe SMTP non configuré, envoi impossible";
        emit reportSent(false);
        return;
    }

    qInfo() << "Génération du rapport quotidien...";

    // Récupérer les stats d'aujourd'hui et d'hier
    DatabaseManager::DailyStats today = m_dbManager->getDailyStats();
    DatabaseManager::DailyStats yesterday = m_dbManager->getYesterdayStats();

    // Générer le contenu HTML
    QString htmlContent = generateReportHtml(today, yesterday);

    // Créer et configurer le client SMTP (même config que Contact)
    SmtpClient *smtp = new SmtpClient(this);
    smtp->setHost("ssl0.ovh.net", 587);  // Port 587 = STARTTLS (comme Contact)
    smtp->setCredentials("contact@nebuludik.fr", m_smtpPassword);
    smtp->setFrom("contact@nebuludik.fr", "Coinche Server");

    QString subject = QString("📊 Rapport Quotidien Coinche - %1").arg(today.date);
    QString to = "contact@nebuludik.fr";

    qInfo() << "Envoi du rapport par email à:" << to;

    // Connecter le signal de réussite/échec
    connect(smtp, &SmtpClient::emailSent, this, [this, smtp](bool success, const QString &error) {
        if (success) {
            qInfo() << "✅ Rapport quotidien envoyé avec succès!";
        } else {
            qWarning() << "❌ Échec de l'envoi du rapport quotidien:" << error;
        }
        emit reportSent(success);
        smtp->deleteLater();
    });

    // Envoyer l'email de manière asynchrone (avec HTML activé)
    smtp->sendEmail(to, subject, htmlContent, true);
}

QString StatsReporter::generateReportHtml(const DatabaseManager::DailyStats &today, const DatabaseManager::DailyStats &yesterday)
{
    QString html = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #0a0a2e 0%, #16213e 100%);
            color: #ffffff;
            padding: 20px;
            margin: 0;
        }
        .container {
            max-width: 600px;
            margin: 0 auto;
            background: rgba(26, 26, 62, 0.8);
            border-radius: 15px;
            padding: 30px;
            box-shadow: 0 8px 32px rgba(0, 0, 0, 0.4);
        }
        .header {
            text-align: center;
            margin-bottom: 30px;
            padding-bottom: 20px;
            border-bottom: 2px solid #FFD700;
        }
        .header h1 {
            margin: 0;
            color: #FFD700;
            font-size: 28px;
        }
        .header .date {
            color: #cccccc;
            font-size: 16px;
            margin-top: 5px;
        }
        .stats-grid {
            display: grid;
            grid-template-columns: 1fr;
            gap: 20px;
            margin: 20px 0;
        }
        .stat-card {
            background: rgba(255, 255, 255, 0.05);
            border-radius: 10px;
            padding: 20px;
            border-left: 4px solid;
        }
        .stat-card.logins { border-left-color: #4CAF50; }
        .stat-card.games { border-left-color: #2196F3; }
        .stat-card.accounts { border-left-color: #FF9800; }
        .stat-card.quits { border-left-color: #F44336; }
        .stat-icon {
            font-size: 32px;
            margin-bottom: 10px;
        }
        .stat-label {
            color: #cccccc;
            font-size: 14px;
            margin-bottom: 5px;
        }
        .stat-value {
            font-size: 36px;
            font-weight: bold;
            margin-bottom: 5px;
        }
        .stat-trend {
            font-size: 14px;
            padding: 5px 10px;
            border-radius: 5px;
            display: inline-block;
        }
        .trend-up {
            background: rgba(76, 175, 80, 0.2);
            color: #4CAF50;
        }
        .trend-down {
            background: rgba(244, 67, 54, 0.2);
            color: #F44336;
        }
        .trend-stable {
            background: rgba(158, 158, 158, 0.2);
            color: #9E9E9E;
        }
        .footer {
            text-align: center;
            margin-top: 30px;
            padding-top: 20px;
            border-top: 1px solid rgba(255, 255, 255, 0.1);
            color: #888888;
            font-size: 12px;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🚀 Rapport Quotidien Coinche</h1>
            <div class="date">)" + today.date + R"(</div>
        </div>

        <div class="stats-grid">
            <div class="stat-card logins">
                <div class="stat-icon">🔐</div>
                <div class="stat-label">Connexions</div>
                <div class="stat-value">)" + QString::number(today.logins) + R"(</div>
                <div class="stat-trend )" + (today.logins >= yesterday.logins ? "trend-up" : "trend-down") + R"(">
                    )" + calculateTrend(today.logins, yesterday.logins) + R"(
                </div>
            </div>

            <div class="stat-card games">
                <div class="stat-icon">🎮</div>
                <div class="stat-label">Parties créées</div>
                <div class="stat-value">)" + QString::number(today.gameRoomsCreated) + R"(</div>
                <div class="stat-trend )" + (today.gameRoomsCreated >= yesterday.gameRoomsCreated ? "trend-up" : "trend-down") + R"(">
                    )" + calculateTrend(today.gameRoomsCreated, yesterday.gameRoomsCreated) + R"(
                </div>
            </div>

            <div class="stat-card accounts">
                <div class="stat-icon">✨</div>
                <div class="stat-label">Nouveaux joueurs</div>
                <div class="stat-value">)" + QString::number(today.newAccounts) + R"(</div>
                <div class="stat-trend )" + (today.newAccounts >= yesterday.newAccounts ? "trend-up" : "trend-down") + R"(">
                    )" + calculateTrend(today.newAccounts, yesterday.newAccounts) + R"(
                </div>
            </div>

            <div class="stat-card quits">
                <div class="stat-icon">⚠️</div>
                <div class="stat-label">Abandons de partie</div>
                <div class="stat-value">)" + QString::number(today.playerQuits) + R"(</div>
                <div class="stat-trend )" + (today.playerQuits >= yesterday.playerQuits ? "trend-down" : "trend-up") + R"(">
                    )" + calculateTrend(today.playerQuits, yesterday.playerQuits) + R"(
                </div>
            </div>
        </div>

        <div class="footer">
            © 2026 NEBULUDIK - Coinche Beta Statistics<br>
            Généré automatiquement par le serveur de jeu
        </div>
    </div>
</body>
</html>
)";

    return html;
}

QString StatsReporter::calculateTrend(int today, int yesterday)
{
    if (yesterday == 0) {
        if (today > 0) {
            return "🚀 Nouveau!";
        }
        return "→ Stable";
    }

    double change = ((double)(today - yesterday) / yesterday) * 100;

    if (std::abs(change) < 5.0) {
        return "→ Stable";
    } else if (change > 0) {
        return QString("📈 +%1%").arg(QString::number(change, 'f', 1));
    } else {
        return QString("📉 %1%").arg(QString::number(change, 'f', 1));
    }
}
