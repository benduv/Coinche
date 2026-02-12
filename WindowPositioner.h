#ifndef WINDOWPOSITIONER_H
#define WINDOWPOSITIONER_H

#include <QObject>
#include <QQuickWindow>
#include <QScreen>
#include <QGuiApplication>
#include <QSettings>
#include <QDebug>

class WindowPositioner : public QObject {
    Q_OBJECT

public:
    explicit WindowPositioner(QObject *parent = nullptr) : QObject(parent) {}

    // Méthode invokable depuis QML pour positionner la fenêtre
    Q_INVOKABLE void positionWindow(QQuickWindow *window) {
        if (!window) {
            qDebug() << "WindowPositioner - Fenêtre nulle";
            return;
        }

        // Obtenir l'écran principal
        QScreen *screen = QGuiApplication::primaryScreen();
        if (!screen) {
            qDebug() << "WindowPositioner - Écran introuvable";
            return;
        }

        QRect screenGeometry = screen->availableGeometry();
        int screenWidth = screenGeometry.width();
        int screenHeight = screenGeometry.height();

        // Calculer les dimensions de la fenêtre (moitié de l'écran)
        int windowWidth = screenWidth / 1.75;
        //int windowWidth = screenWidth / 2;
        int windowHeight = screenHeight / 2;

        // ========== Mode production : Fenêtre centrée ==========
        // Centrer la fenêtre sur l'écran
        int x = screenGeometry.x() + (screenWidth - windowWidth) / 2;
        int y = screenGeometry.y() + (screenHeight - windowHeight) / 2;

        qDebug() << "WindowPositioner - Fenêtre centrée à (" << x << "," << y << ")"
                 << "taille:" << windowWidth << "x" << windowHeight;

        // ========== Mode test : Positionnement en grille (commenté) ==========
        /*
        // Lire le compteur de fenêtres depuis les settings
        QSettings settings("Coinche", "WindowCounter");
        int windowCount = settings.value("windowCount", 0).toInt();

        // Définir les positions selon le compteur
        int x = 0, y = 0;
        switch (windowCount % 4) {
            case 0: // Haut gauche
                x = screenGeometry.x();
                y = screenGeometry.y();
                break;
            case 1: // Haut droite
                x = screenGeometry.x() + windowWidth;
                y = screenGeometry.y();
                break;
            case 2: // Bas gauche
                x = screenGeometry.x();
                y = screenGeometry.y() + windowHeight;
                break;
            case 3: // Bas droite
                x = screenGeometry.x() + windowWidth;
                y = screenGeometry.y() + windowHeight;
                break;
        }

        qDebug() << "WindowPositioner - Fenêtre" << windowCount
                 << "positionnée à (" << x << "," << y << ")"
                 << "taille:" << windowWidth << "x" << windowHeight;

        // Incrémenter le compteur
        settings.setValue("windowCount", windowCount + 1);
        */

        // Positionner et redimensionner la fenêtre
        window->setX(x);
        window->setY(y);
        window->setWidth(windowWidth);
        window->setHeight(windowHeight);
    }

    // Méthode pour réinitialiser le compteur (utile pour les tests)
    Q_INVOKABLE void resetCounter() {
        QSettings settings("Coinche", "WindowCounter");
        settings.setValue("windowCount", 0);
        qDebug() << "WindowPositioner - Compteur réinitialisé";
    }
};

#endif // WINDOWPOSITIONER_H
