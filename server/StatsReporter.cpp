#include "StatsReporter.h"
#include <QDateTime>
#include <QDebug>
#include <cmath>
#include <numeric>

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

    // Récupérer les stats de la journée qui vient de se terminer (hier) et de l'avant-veille
    // Le rapport est envoyé à minuit, donc "aujourd'hui" est vide
    DatabaseManager::DailyStats today = m_dbManager->getYesterdayStats();  // Journée terminée
    DatabaseManager::DailyStats yesterday = m_dbManager->getDailyStats(
        QDate::currentDate().addDays(-2).toString("yyyy-MM-dd")  // Avant-veille
    );

    // Récupérer les taux de rétention
    DatabaseManager::RetentionStats retention = m_dbManager->getRetentionStats();

    // Récupérer les tendances (7 jours et 30 jours)
    QList<DatabaseManager::DailyStats> trends7d = m_dbManager->getTrendStats(7);
    QList<DatabaseManager::DailyStats> trends30d = m_dbManager->getTrendStats(30);

    // Générer le contenu HTML
    QString htmlContent = generateReportHtml(today, yesterday, retention, trends7d, trends30d);

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

QString StatsReporter::generateReportHtml(
    const DatabaseManager::DailyStats &today,
    const DatabaseManager::DailyStats &yesterday,
    const DatabaseManager::RetentionStats &retention,
    const QList<DatabaseManager::DailyStats> &trends7d,
    const QList<DatabaseManager::DailyStats> &trends30d)
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

        <!-- Nouvelles statistiques avancées -->
        <div class="stats-grid" style="margin-top: 20px;">
            <div class="stat-card" style="border-left-color: #9C27B0;">
                <div class="stat-icon">⏱️</div>
                <div class="stat-label">Temps de session moyen</div>
                <div class="stat-value">)" + QString::number(today.sessionCount > 0 ? today.totalSessionTime / today.sessionCount / 60 : 0) + R"( min</div>
                <div class="stat-trend trend-stable">
                    )" + QString::number(today.sessionCount) + R"( sessions
                </div>
            </div>

            <div class="stat-card" style="border-left-color: #E91E63;">
                <div class="stat-icon">💥</div>
                <div class="stat-label">Crashes détectés</div>
                <div class="stat-value">)" + QString::number(today.crashes) + R"(</div>
                <div class="stat-trend )" + (today.crashes >= yesterday.crashes ? "trend-down" : "trend-up") + R"(">
                    )" + calculateTrend(today.crashes, yesterday.crashes) + R"(
                </div>
            </div>
        </div>

        <!-- Taux de rétention -->
        <div style="margin-top: 30px; padding: 20px; background: rgba(255, 255, 255, 0.05); border-radius: 10px;">
            <h2 style="color: #FFD700; margin: 0 0 15px 0; font-size: 20px;">📊 Taux de Rétention</h2>
            <div style="display: flex; gap: 15px; flex-wrap: wrap;">
                <div style="flex: 1; min-width: 150px; text-align: center; padding: 15px; background: rgba(76, 175, 80, 0.1); border-radius: 8px;">
                    <div style="font-size: 14px; color: #cccccc; margin-bottom: 5px;">Jour 1</div>
                    <div style="font-size: 32px; font-weight: bold; color: #4CAF50;">)" + QString::number(retention.d1Retention, 'f', 1) + R"(%</div>
                </div>
                <div style="flex: 1; min-width: 150px; text-align: center; padding: 15px; background: rgba(33, 150, 243, 0.1); border-radius: 8px;">
                    <div style="font-size: 14px; color: #cccccc; margin-bottom: 5px;">Jour 7</div>
                    <div style="font-size: 32px; font-weight: bold; color: #2196F3;">)" + QString::number(retention.d7Retention, 'f', 1) + R"(%</div>
                </div>
                <div style="flex: 1; min-width: 150px; text-align: center; padding: 15px; background: rgba(255, 152, 0, 0.1); border-radius: 8px;">
                    <div style="font-size: 14px; color: #cccccc; margin-bottom: 5px;">Jour 30</div>
                    <div style="font-size: 32px; font-weight: bold; color: #FF9800;">)" + QString::number(retention.d30Retention, 'f', 1) + R"(%</div>
                </div>
            </div>
        </div>

        <!-- Graphiques de tendance -->
        <div style="margin-top: 30px;">
            <h2 style="color: #FFD700; margin: 0 0 15px 0; font-size: 20px;">📈 Tendances sur 7 jours</h2>
            <div style="background: rgba(255, 255, 255, 0.05); border-radius: 10px; padding: 20px;">
                )" + generateTrendChart(trends7d, "logins") + R"(
            </div>
        </div>

        <div style="margin-top: 20px;">
            <h2 style="color: #FFD700; margin: 0 0 15px 0; font-size: 20px;">📈 Tendances sur 30 jours</h2>
            <div style="background: rgba(255, 255, 255, 0.05); border-radius: 10px; padding: 20px;">
                )" + generateTrendChart(trends30d, "logins") + R"(
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

QString StatsReporter::generateTrendChart(const QList<DatabaseManager::DailyStats> &trends, const QString &metricName)
{
    if (trends.isEmpty()) {
        return "<p style='color: #888;'>Pas assez de données pour générer un graphique</p>";
    }

    // Extraire les valeurs selon la métrique demandée
    QList<int> values;
    QStringList labels;
    int maxValue = 1;

    for (const auto &stat : trends) {
        int value = 0;
        if (metricName == "logins") value = stat.logins;
        else if (metricName == "games") value = stat.gameRoomsCreated;
        else if (metricName == "newAccounts") value = stat.newAccounts;
        else if (metricName == "quits") value = stat.playerQuits;

        values.append(value);
        labels.append(stat.date.mid(5)); // Garder seulement MM-DD
        if (value > maxValue) maxValue = value;
    }

    // Générer un graphique SVG simple (graphique en barres)
    const int width = 600;
    const int height = 200;
    const int barWidth = width / values.size();
    const int padding = 10;

    QString svg = QString("<svg width='%1' height='%2' style='background: rgba(0,0,0,0.2); border-radius: 8px;'>").arg(width).arg(height);

    // Dessiner les barres
    for (int i = 0; i < values.size(); ++i) {
        int barHeight = (values[i] * (height - 40)) / (maxValue > 0 ? maxValue : 1);
        int x = i * barWidth + padding;
        int y = height - barHeight - 20;

        // Couleur de la barre (gradient de bleu)
        QString color = "#2196F3";

        svg += QString("<rect x='%1' y='%2' width='%3' height='%4' fill='%5' opacity='0.8'/>")
                .arg(x).arg(y).arg(barWidth - 4).arg(barHeight).arg(color);

        // Valeur au-dessus de la barre
        svg += QString("<text x='%1' y='%2' fill='white' font-size='10' text-anchor='middle'>%3</text>")
                .arg(x + (barWidth - 4) / 2).arg(y - 5).arg(values[i]);

        // Label de date (tous les 2 jours pour éviter le chevauchement)
        if (i % 2 == 0 || values.size() <= 7) {
            svg += QString("<text x='%1' y='%2' fill='#cccccc' font-size='9' text-anchor='middle' transform='rotate(-45 %1 %2)'>%3</text>")
                    .arg(x + (barWidth - 4) / 2).arg(height - 5).arg(labels[i]);
        }
    }

    svg += "</svg>";

    // Ajouter une légende
    QString legend = QString("<div style='margin-top: 10px; color: #cccccc; font-size: 12px; text-align: center;'>"
                            "Métrique: <strong>%1</strong> | Max: <strong>%2</strong> | Moy: <strong>%3</strong>"
                            "</div>")
                        .arg(metricName == "logins" ? "Connexions" :
                             metricName == "games" ? "Parties créées" :
                             metricName == "newAccounts" ? "Nouveaux comptes" : "Abandons")
                        .arg(maxValue)
                        .arg(values.isEmpty() ? 0 : std::accumulate(values.begin(), values.end(), 0) / values.size());

    return svg + legend;
}
