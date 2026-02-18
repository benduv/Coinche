#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>
#include <QDebug>

class LogManager {
public:
    static LogManager& instance() {
        static LogManager instance;
        return instance;
    }

    bool initialize(const QString &logFilePath) {
        QMutexLocker locker(&m_mutex);

        m_logFile = new QFile(logFilePath);
        if (!m_logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            fprintf(stderr, "ERREUR: Impossible d'ouvrir %s\n", qPrintable(logFilePath));
            return false;
        }

        m_logFilePath = logFilePath;
        qInstallMessageHandler(messageHandler);

        qInfo() << "========================================";
        qInfo() << "Démarrage du serveur Coinche";
        qInfo() << "PID:" << QCoreApplication::applicationPid();
        qInfo() << "Fichier de log:" << logFilePath;
        qInfo() << "========================================";

        return true;
    }

    void shutdown() {
        QMutexLocker locker(&m_mutex);

        qInfo() << "========================================";
        qInfo() << "Arrêt du serveur";
        qInfo() << "========================================";

        if (m_logFile) {
            m_logFile->close();
            delete m_logFile;
            m_logFile = nullptr;
        }
    }

private:
    LogManager() : m_logFile(nullptr) {}
    ~LogManager() {
        shutdown();
    }

    // Empêcher la copie
    LogManager(const LogManager&) = delete;
    LogManager& operator=(const LogManager&) = delete;

    static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
        QMutexLocker locker(&instance().m_mutex);

        QString level;
        switch (type) {
            case QtDebugMsg:    level = "DEBUG";    break;
            case QtInfoMsg:     level = "INFO";     break;
            case QtWarningMsg:  level = "WARNING";  break;
            case QtCriticalMsg: level = "CRITICAL"; break;
            case QtFatalMsg:    level = "FATAL";    break;
        }

        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
        QString logLine = QString("[%1] [%2] %3\n").arg(timestamp, level, msg);

        // Écrire dans le fichier
        if (instance().m_logFile && instance().m_logFile->isOpen()) {
            QTextStream stream(instance().m_logFile);
            stream << logLine;
            stream.flush();
        }

        // Aussi afficher sur stderr (pour systemd journal)
        fprintf(stderr, "%s", logLine.toUtf8().constData());
    }

    QFile *m_logFile;
    QString m_logFilePath;
    static QMutex m_mutex;
};

// Définition du mutex statique
QMutex LogManager::m_mutex;

#endif // LOGMANAGER_H
