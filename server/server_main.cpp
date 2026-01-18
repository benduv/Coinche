// server_main.cpp
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include "GameServer.h"

// Fichier de log global
QFile *logFile = nullptr;
QTextStream *logStream = nullptr;
bool verboseLogging = false; // Mode verbeux désactivé par défaut pour de meilleures performances

// Handler personnalisé pour rediriger qDebug vers un fichier
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // En mode non-verbeux, ignorer les messages DEBUG
    if (!verboseLogging && type == QtDebugMsg) {
        return;
    }
    QString logMessage;
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");

    switch (type) {
    case QtDebugMsg:
        logMessage = QString("[%1] DEBUG: %2").arg(timestamp, msg);
        break;
    case QtInfoMsg:
        logMessage = QString("[%1] INFO: %2").arg(timestamp, msg);
        break;
    case QtWarningMsg:
        logMessage = QString("[%1] WARNING: %2").arg(timestamp, msg);
        break;
    case QtCriticalMsg:
        logMessage = QString("[%1] CRITICAL: %2").arg(timestamp, msg);
        break;
    case QtFatalMsg:
        logMessage = QString("[%1] FATAL: %2").arg(timestamp, msg);
        break;
    }

    // Écrire dans le fichier
    if (logStream) {
        (*logStream) << logMessage << Qt::endl;
        // flush() supprimé pour améliorer les performances réseau
        // Les logs seront bufferisés et écrits périodiquement
    }

    // Aussi afficher dans la console
    fprintf(stderr, "%s\n", logMessage.toLocal8Bit().constData());

    // Si fatal, fermer proprement le fichier avant de quitter
    if (type == QtFatalMsg) {
        if (logStream) {
            delete logStream;
            logStream = nullptr;
        }
        if (logFile) {
            logFile->close();
            delete logFile;
            logFile = nullptr;
        }
        abort();
    }
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    // Vérifier si le mode verbeux est activé
    // Via argument: --verbose ou -v
    // Via variable d'environnement: COINCHE_VERBOSE=1
    QString sslCertPath;
    QString sslKeyPath;
    QString smtpPassword;

    for (int i = 1; i < argc; ++i) {
        QString arg = QString::fromLocal8Bit(argv[i]);
        if (arg == "--verbose" || arg == "-v") {
            verboseLogging = true;
        } else if (arg == "--ssl-cert" && i + 1 < argc) {
            sslCertPath = QString::fromLocal8Bit(argv[++i]);
        } else if (arg == "--ssl-key" && i + 1 < argc) {
            sslKeyPath = QString::fromLocal8Bit(argv[++i]);
        } else if (arg == "--smtp-password" && i + 1 < argc) {
            smtpPassword = QString::fromLocal8Bit(argv[++i]);
        }
    }
    if (!verboseLogging) {
        verboseLogging = qEnvironmentVariableIsSet("COINCHE_VERBOSE");
    }

    // Vérifier aussi les variables d'environnement pour SSL
    if (sslCertPath.isEmpty()) {
        sslCertPath = qEnvironmentVariable("COINCHE_SSL_CERT");
    }
    if (sslKeyPath.isEmpty()) {
        sslKeyPath = qEnvironmentVariable("COINCHE_SSL_KEY");
    }
    // Mot de passe SMTP depuis variable d'environnement si non fourni en argument
    if (smtpPassword.isEmpty()) {
        smtpPassword = qEnvironmentVariable("COINCHE_SMTP_PASSWORD");
    }

    // Ouvrir le fichier de log
    logFile = new QFile("server_log.txt");
    if (logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        logStream = new QTextStream(logFile);

        // Installer le handler de messages personnalisé
        qInstallMessageHandler(messageHandler);

        qInfo() << "========================================";
        qInfo() << "Serveur de jeu démarre...";
        qInfo() << "Mode verbeux:" << (verboseLogging ? "ACTIVE" : "DESACTIVE");
        if (!sslCertPath.isEmpty() && !sslKeyPath.isEmpty()) {
            qInfo() << "Mode SSL: ACTIVE (WSS)";
            qInfo() << "Certificat:" << sslCertPath;
            qInfo() << "Clé privée:" << sslKeyPath;
        } else {
            qInfo() << "Mode SSL: DESACTIVE (WS) - Utilisez --ssl-cert et --ssl-key pour activer";
        }
        qInfo() << "SMTP Contact:" << (smtpPassword.isEmpty() ? "DESACTIVE" : "ACTIVE");
        qInfo() << "Logs écrits dans: server_log.txt";
        qInfo() << "========================================";
    } else {
        qWarning() << "Impossible d'ouvrir le fichier de log!";
    }

    // Créer le serveur avec ou sans SSL, et mot de passe SMTP pour les emails de contact
    GameServer server(1234, nullptr, sslCertPath, sslKeyPath, smtpPassword);

    int result = app.exec();

    // Nettoyer à la fin
    if (logStream) {
        delete logStream;
    }
    if (logFile) {
        logFile->close();
        delete logFile;
    }

    return result;
}
