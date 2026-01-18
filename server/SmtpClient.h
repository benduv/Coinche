#ifndef SMTPCLIENT_H
#define SMTPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QSslSocket>
#include <QDebug>
#include <QTimer>

class SmtpClient : public QObject {
    Q_OBJECT

public:
    explicit SmtpClient(QObject *parent = nullptr)
        : QObject(parent)
        , m_socket(nullptr)
        , m_timeout(30000) // 30 secondes timeout
    {}

    // Configure les parametres SMTP
    void setHost(const QString &host, int port = 587) {
        m_host = host;
        m_port = port;
    }

    void setCredentials(const QString &user, const QString &password) {
        m_user = user;
        m_password = password;
    }

    void setFrom(const QString &from, const QString &fromName = QString()) {
        m_from = from;
        m_fromName = fromName.isEmpty() ? from : fromName;
    }

    // Envoie un email de maniere asynchrone
    void sendEmail(const QString &to, const QString &subject, const QString &body) {
        m_to = to;
        m_subject = subject;
        m_body = body;
        m_state = Init;

        // Creer le socket SSL
        if (m_socket) {
            m_socket->deleteLater();
        }
        m_socket = new QSslSocket(this);

        connect(m_socket, &QSslSocket::connected, this, &SmtpClient::onConnected);
        connect(m_socket, &QSslSocket::encrypted, this, &SmtpClient::onEncrypted);
        connect(m_socket, &QSslSocket::readyRead, this, &SmtpClient::onReadyRead);
        connect(m_socket, &QSslSocket::errorOccurred, this, &SmtpClient::onError);
        connect(m_socket, &QSslSocket::disconnected, this, &SmtpClient::onDisconnected);

        qDebug() << "SmtpClient: Connexion a" << m_host << ":" << m_port;
        m_socket->connectToHost(m_host, m_port);

        // Timer de timeout
        QTimer::singleShot(m_timeout, this, [this]() {
            if (m_socket && m_socket->state() != QAbstractSocket::UnconnectedState) {
                qWarning() << "SmtpClient: Timeout";
                m_socket->abort();
                emit emailSent(false, "Timeout de connexion");
            }
        });
    }

signals:
    void emailSent(bool success, const QString &error);

private slots:
    void onConnected() {
        qDebug() << "SmtpClient: Connecte au serveur SMTP";
    }

    void onEncrypted() {
        qDebug() << "SmtpClient: Connexion chiffree etablie";
    }

    void onReadyRead() {
        QString response = QString::fromUtf8(m_socket->readAll());
        qDebug() << "SmtpClient: Reponse:" << response.trimmed();

        // Extraire le code de reponse
        int responseCode = response.left(3).toInt();

        switch (m_state) {
            case Init:
                if (responseCode == 220) {
                    // Envoyer EHLO
                    sendCommand("EHLO coinche-game.local");
                    m_state = Ehlo;
                } else {
                    handleError("Reponse initiale invalide");
                }
                break;

            case Ehlo:
                if (responseCode == 250) {
                    // Demarrer STARTTLS
                    sendCommand("STARTTLS");
                    m_state = StartTls;
                } else {
                    handleError("EHLO echoue");
                }
                break;

            case StartTls:
                if (responseCode == 220) {
                    // Passer en mode SSL
                    m_socket->startClientEncryption();
                    m_state = TlsReady;
                    // Attendre l'encryption puis renvoyer EHLO
                    connect(m_socket, &QSslSocket::encrypted, this, [this]() {
                        sendCommand("EHLO coinche-game.local");
                        m_state = EhloAfterTls;
                    }, Qt::SingleShotConnection);
                } else {
                    handleError("STARTTLS echoue");
                }
                break;

            case EhloAfterTls:
                if (responseCode == 250) {
                    // Authentification
                    sendCommand("AUTH LOGIN");
                    m_state = Auth;
                } else {
                    handleError("EHLO apres TLS echoue");
                }
                break;

            case Auth:
                if (responseCode == 334) {
                    // Envoyer username en base64
                    sendCommand(m_user.toUtf8().toBase64());
                    m_state = AuthUser;
                } else {
                    handleError("AUTH LOGIN echoue");
                }
                break;

            case AuthUser:
                if (responseCode == 334) {
                    // Envoyer password en base64
                    sendCommand(m_password.toUtf8().toBase64());
                    m_state = AuthPass;
                } else {
                    handleError("Authentification utilisateur echoue");
                }
                break;

            case AuthPass:
                if (responseCode == 235) {
                    // Authentification reussie, envoyer MAIL FROM
                    sendCommand(QString("MAIL FROM:<%1>").arg(m_from));
                    m_state = MailFrom;
                } else {
                    handleError("Mot de passe incorrect");
                }
                break;

            case MailFrom:
                if (responseCode == 250) {
                    // Envoyer RCPT TO
                    sendCommand(QString("RCPT TO:<%1>").arg(m_to));
                    m_state = RcptTo;
                } else {
                    handleError("MAIL FROM echoue");
                }
                break;

            case RcptTo:
                if (responseCode == 250) {
                    // Envoyer DATA
                    sendCommand("DATA");
                    m_state = Data;
                } else {
                    handleError("RCPT TO echoue");
                }
                break;

            case Data:
                if (responseCode == 354) {
                    // Envoyer le contenu de l'email
                    QString email = buildEmail();
                    m_socket->write(email.toUtf8());
                    m_socket->write("\r\n.\r\n");
                    m_socket->flush();
                    m_state = Body;
                } else {
                    handleError("DATA echoue");
                }
                break;

            case Body:
                if (responseCode == 250) {
                    // Email envoye avec succes
                    sendCommand("QUIT");
                    m_state = Quit;
                    emit emailSent(true, QString());
                } else {
                    handleError("Envoi du corps echoue");
                }
                break;

            case Quit:
                // Fermer proprement
                m_socket->disconnectFromHost();
                break;

            default:
                break;
        }
    }

    void onError(QAbstractSocket::SocketError error) {
        qWarning() << "SmtpClient: Erreur socket:" << error << m_socket->errorString();
        emit emailSent(false, m_socket->errorString());
    }

    void onDisconnected() {
        qDebug() << "SmtpClient: Deconnecte";
    }

private:
    void sendCommand(const QString &cmd) {
        qDebug() << "SmtpClient: Envoi:" << (cmd.contains("AUTH") || m_state == AuthUser || m_state == AuthPass ? "***" : cmd);
        m_socket->write((cmd + "\r\n").toUtf8());
        m_socket->flush();
    }

    void handleError(const QString &error) {
        qWarning() << "SmtpClient: Erreur:" << error;
        emit emailSent(false, error);
        if (m_socket) {
            m_socket->abort();
        }
    }

    QString buildEmail() {
        QString email;
        email += QString("From: %1 <%2>\r\n").arg(m_fromName, m_from);
        email += QString("To: <%1>\r\n").arg(m_to);
        email += QString("Subject: %1\r\n").arg(m_subject);
        email += "MIME-Version: 1.0\r\n";
        email += "Content-Type: text/plain; charset=utf-8\r\n";
        email += "Content-Transfer-Encoding: 8bit\r\n";
        email += "\r\n";
        email += m_body;
        return email;
    }

    enum State {
        Init,
        Ehlo,
        StartTls,
        TlsReady,
        EhloAfterTls,
        Auth,
        AuthUser,
        AuthPass,
        MailFrom,
        RcptTo,
        Data,
        Body,
        Quit
    };

    QSslSocket *m_socket;
    QString m_host;
    int m_port;
    QString m_user;
    QString m_password;
    QString m_from;
    QString m_fromName;
    QString m_to;
    QString m_subject;
    QString m_body;
    State m_state;
    int m_timeout;
};

#endif // SMTPCLIENT_H
