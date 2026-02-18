// server_main.cpp
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include <QElapsedTimer>
#include <csignal>
#include "GameServer.h"
#include "SmtpClient.h"

// Includes pour stack trace (Unix/Linux)
#ifdef Q_OS_UNIX
#include <execinfo.h>
#include <cstdlib>
#endif

// Fichier de log global
QFile *logFile = nullptr;
QTextStream *logStream = nullptr;
bool verboseLogging = false; // Mode verbeux désactivé par défaut pour de meilleures performances

// Pointeurs globaux pour le crash handler
static QString g_smtpPassword;
static QCoreApplication *g_app = nullptr;

// Obtenir la stack trace (version simplifiée, multi-plateforme)
QString getStackTrace() {
    QString trace;

#ifdef Q_OS_UNIX
    // Sur Unix/Linux, utiliser backtrace si disponible
    void *array[50];
    int size = backtrace(array, 50);
    char **strings = backtrace_symbols(array, size);

    if (strings && size > 0) {
        trace = "Stack trace:\n";
        for (int i = 0; i < size; i++) {
            trace += QString("  [%1] %2\n").arg(i).arg(strings[i]);
        }
        free(strings);
    }
#endif

    if (trace.isEmpty()) {
        trace = "Stack trace non disponible sur cette plateforme (Windows)";
    }

    return trace;
}

// Handler de signaux pour les crashes serveur
void crashSignalHandler(int signal) {
    const char *signalName = "UNKNOWN";

    switch(signal) {
        case SIGSEGV: signalName = "SIGSEGV (Segmentation Fault)"; break;
        case SIGABRT: signalName = "SIGABRT (Abort)"; break;
        case SIGFPE:  signalName = "SIGFPE (Floating Point Exception)"; break;
        case SIGILL:  signalName = "SIGILL (Illegal Instruction)"; break;
#ifdef SIGBUS
        case SIGBUS:  signalName = "SIGBUS (Bus Error)"; break;
#endif
    }

    QString crashMsg = QString("🚨 CRASH SERVEUR DÉTECTÉ - Signal: %1 (%2)")
                          .arg(signalName)
                          .arg(signal);

    fprintf(stderr, "\n%s\n", crashMsg.toLocal8Bit().constData());

    // Logger dans le fichier
    if (logStream) {
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        (*logStream) << QString("[%1] FATAL CRASH: %2").arg(timestamp, crashMsg) << Qt::endl;
        logStream->flush();
    }

    // Obtenir la stack trace
    QString stackTrace = getStackTrace();
    fprintf(stderr, "%s\n", stackTrace.toLocal8Bit().constData());

    if (logStream) {
        (*logStream) << stackTrace << Qt::endl;
        logStream->flush();
    }

    // Envoyer un email d'alerte si SMTP configuré
    if (!g_smtpPassword.isEmpty() && g_app) {
        fprintf(stderr, "Envoi d'email d'alerte de crash serveur...\n");

        SmtpClient *smtp = new SmtpClient();
        smtp->setHost("ssl0.ovh.net", 587);
        smtp->setCredentials("contact@nebuludik.fr", g_smtpPassword);
        smtp->setFrom("contact@nebuludik.fr", "Coinche Server - CRITICAL CRASH");

        QString emailBody = QString(
            "🚨🚨🚨 CRASH CRITIQUE DU SERVEUR COINCHE 🚨🚨🚨\n\n"
            "Le serveur de jeu Coinche a subi un crash fatal et s'est arrêté.\n\n"
            "═══════════════════════════════════════════════\n"
            "📊 INFORMATIONS DU CRASH\n"
            "═══════════════════════════════════════════════\n"
            "Date/Heure: %1\n"
            "Signal: %2\n"
            "Type: Crash Serveur Fatal\n\n"
            "═══════════════════════════════════════════════\n"
            "📋 STACK TRACE\n"
            "═══════════════════════════════════════════════\n"
            "%3\n\n"
            "═══════════════════════════════════════════════\n\n"
            "⚠️ ACTION REQUISE IMMÉDIATEMENT:\n"
            "1. Redémarrer le serveur manuellement\n"
            "2. Analyser les logs: tail -f /var/log/coinche/server.log\n"
            "3. Corriger le bug responsable du crash\n\n"
            "Ce rapport a été généré automatiquement par le crash handler.\n"
        ).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"))
         .arg(signalName)
         .arg(stackTrace);

        QString subject = QString("🚨 CRASH SERVEUR FATAL - Coinche - %1")
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm"));

        // Envoyer de manière synchrone (on va crasher de toute façon)
        bool sent = false;
        QObject::connect(smtp, &SmtpClient::emailSent, [&sent, smtp](bool success, const QString &error) {
            if (success) {
                fprintf(stderr, "✅ Email d'alerte envoyé avec succès\n");
            } else {
                fprintf(stderr, "❌ Échec envoi email: %s\n", error.toLocal8Bit().constData());
            }
            sent = true;
            smtp->deleteLater();
        });

        smtp->sendEmail("contact@nebuludik.fr", subject, emailBody);

        // Attendre un peu pour que l'email parte (max 2 secondes)
        QElapsedTimer timer;
        timer.start();
        while (!sent && timer.elapsed() < 2000 && g_app) {
            g_app->processEvents();
        }
    }

    // Nettoyer les fichiers
    if (logStream) {
        delete logStream;
        logStream = nullptr;
    }
    if (logFile) {
        logFile->close();
        delete logFile;
        logFile = nullptr;
    }

    // Restaurer le handler par défaut et re-raise le signal pour vraiment crash
    std::signal(signal, SIG_DFL);
    std::raise(signal);
}

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

    // Si fatal ou critique grave, envoyer un email d'alerte
    if (type == QtFatalMsg ||
        (type == QtCriticalMsg && (msg.contains("crash", Qt::CaseInsensitive) ||
                                     msg.contains("fatal", Qt::CaseInsensitive) ||
                                     msg.contains("segfault", Qt::CaseInsensitive)))) {

        if (!g_smtpPassword.isEmpty() && g_app) {
            fprintf(stderr, "🚨 Envoi d'email d'alerte pour erreur critique...\n");

            SmtpClient *smtp = new SmtpClient();
            smtp->setHost("ssl0.ovh.net", 587);
            smtp->setCredentials("contact@nebuludik.fr", g_smtpPassword);
            smtp->setFrom("contact@nebuludik.fr", "Coinche Server - CRITICAL ERROR");

            QString stackInfo = QString("File: %1:%2\nFunction: %3")
                                   .arg(context.file ? context.file : "unknown")
                                   .arg(context.line)
                                   .arg(context.function ? context.function : "unknown");

            QString emailBody = QString(
                "🚨 ERREUR CRITIQUE DU SERVEUR COINCHE\n\n"
                "Une erreur critique a été détectée sur le serveur.\n\n"
                "═══════════════════════════════════════════════\n"
                "📊 INFORMATIONS\n"
                "═══════════════════════════════════════════════\n"
                "Date/Heure: %1\n"
                "Type: %2\n\n"
                "═══════════════════════════════════════════════\n"
                "❌ MESSAGE D'ERREUR\n"
                "═══════════════════════════════════════════════\n"
                "%3\n\n"
                "═══════════════════════════════════════════════\n"
                "📋 CONTEXTE\n"
                "═══════════════════════════════════════════════\n"
                "%4\n\n"
                "═══════════════════════════════════════════════\n\n"
                "Consultez les logs pour plus de détails: /var/log/coinche/server.log\n"
            ).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"))
             .arg(type == QtFatalMsg ? "FATAL" : "CRITICAL")
             .arg(msg)
             .arg(stackInfo);

            QString subject = QString("🚨 ERREUR SERVEUR %1 - Coinche - %2")
                .arg(type == QtFatalMsg ? "FATALE" : "CRITIQUE")
                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm"));

            bool sent = false;
            QObject::connect(smtp, &SmtpClient::emailSent, [&sent, smtp](bool success, const QString &error) {
                if (success) {
                    fprintf(stderr, "✅ Email d'alerte envoyé\n");
                } else {
                    fprintf(stderr, "❌ Échec envoi email: %s\n", error.toLocal8Bit().constData());
                }
                sent = true;
                smtp->deleteLater();
            });

            smtp->sendEmail("contact@nebuludik.fr", subject, emailBody);

            // Attendre max 2 secondes pour l'envoi
            QElapsedTimer timer;
            timer.start();
            while (!sent && timer.elapsed() < 2000 && g_app) {
                g_app->processEvents();
            }
        }
    }

    // Si fatal, fermer proprement le fichier avant de quitter
    if (type == QtFatalMsg) {
        if (logStream) {
            logStream->flush();
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
    g_app = &app;  // Sauvegarder pour le crash handler

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

    // Sauvegarder pour le crash handler
    g_smtpPassword = smtpPassword;

    // Installer les handlers de signaux pour détecter les crashes serveur
    std::signal(SIGSEGV, crashSignalHandler);  // Segmentation fault
    std::signal(SIGABRT, crashSignalHandler);  // Abort
    std::signal(SIGFPE, crashSignalHandler);   // Floating point exception
    std::signal(SIGILL, crashSignalHandler);   // Illegal instruction
#ifdef SIGBUS
    std::signal(SIGBUS, crashSignalHandler);   // Bus error (Unix)
#endif

    fprintf(stderr, "✅ Crash handlers installés - Les crashs serveur seront détectés et rapportés\n");

    // Déterminer le chemin du fichier de log (production vs développement)
    QString logFilePath;
    if (QFile::exists("/var/log/coinche")) {
        // Production : utiliser /var/log/coinche
        logFilePath = "/var/log/coinche/server.log";
    } else {
        // Développement : utiliser répertoire local
        logFilePath = "server_log.txt";
    }

    // Ouvrir le fichier de log
    logFile = new QFile(logFilePath);
    if (logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        logStream = new QTextStream(logFile);

        // Installer le handler de messages personnalisé
        qInstallMessageHandler(messageHandler);

        qInfo() << "========================================";
        qInfo() << "Serveur de jeu démarre...";
        qInfo() << "PID:" << QCoreApplication::applicationPid();
        qInfo() << "Mode verbeux:" << (verboseLogging ? "ACTIVE" : "DESACTIVE");
        if (!sslCertPath.isEmpty() && !sslKeyPath.isEmpty()) {
            qInfo() << "Mode SSL: ACTIVE (WSS)";
            qInfo() << "Certificat:" << sslCertPath;
            qInfo() << "Clé privée:" << sslKeyPath;
        } else {
            qInfo() << "Mode SSL: DESACTIVE (WS) - Utilisez --ssl-cert et --ssl-key pour activer";
        }
        qInfo() << "SMTP Contact:" << (smtpPassword.isEmpty() ? "DESACTIVE" : "ACTIVE");
        qInfo() << "Fichier de log:" << logFilePath;
        qInfo() << "========================================";
    } else {
        fprintf(stderr, "ERREUR CRITIQUE: Impossible d'ouvrir le fichier de log: %s\n", qPrintable(logFilePath));
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
