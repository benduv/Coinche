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

// Handler personnalisé pour rediriger qDebug vers un fichier
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
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
        logStream->flush(); // Forcer l'écriture immédiate
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

    // Ouvrir le fichier de log
    logFile = new QFile("server_log.txt");
    if (logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        logStream = new QTextStream(logFile);

        // Installer le handler de messages personnalisé
        qInstallMessageHandler(messageHandler);

        qDebug() << "========================================";
        qDebug() << "Serveur de jeu démarre...";
        qDebug() << "Logs écrits dans: server_log.txt";
        qDebug() << "========================================";
    } else {
        qWarning() << "Impossible d'ouvrir le fichier de log!";
    }

    GameServer server(1234); // port du serveur

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
