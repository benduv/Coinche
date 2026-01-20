#ifndef STATSREPORTER_H
#define STATSREPORTER_H

#include <QObject>
#include <QTimer>
#include <QString>
#include "DatabaseManager.h"
#include "SmtpClient.h"

class StatsReporter : public QObject
{
    Q_OBJECT

public:
    explicit StatsReporter(DatabaseManager *dbManager, const QString &smtpPassword, QObject *parent = nullptr);
    ~StatsReporter();

    // Envoyer le rapport quotidien manuellement (pour test)
    void sendDailyReport();

    // Configurer l'heure d'envoi (par défaut: minuit)
    void setReportTime(int hour, int minute);

signals:
    void reportSent(bool success);

private slots:
    void checkAndSendReport();

private:
    DatabaseManager *m_dbManager;
    QString m_smtpPassword;
    QTimer *m_dailyTimer;
    int m_reportHour;
    int m_reportMinute;

    // Générer le contenu HTML de l'email
    QString generateReportHtml(const DatabaseManager::DailyStats &today, const DatabaseManager::DailyStats &yesterday);

    // Calculer le pourcentage de changement
    QString calculateTrend(int today, int yesterday);

    // Planifier le prochain envoi
    void scheduleNextReport();
};

#endif // STATSREPORTER_H
