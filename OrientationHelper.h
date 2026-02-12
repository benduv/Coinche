#ifndef ORIENTATIONHELPER_H
#define ORIENTATIONHELPER_H

#include <QObject>
#include <QDebug>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QCoreApplication>
#endif

class OrientationHelper : public QObject
{
    Q_OBJECT

public:
    explicit OrientationHelper(QObject *parent = nullptr) : QObject(parent) {}

    // Force le mode paysage (sensor landscape)
    Q_INVOKABLE void setLandscape()
    {
#ifdef Q_OS_ANDROID
        // ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE = 6
        QJniObject activity = QJniObject::callStaticObjectMethod(
            "org/qtproject/qt/android/QtNative",
            "activity",
            "()Landroid/app/Activity;");
        if (activity.isValid()) {
            activity.callMethod<void>("setRequestedOrientation", "(I)V", 6);
            qDebug() << "OrientationHelper: Mode paysage forcé";
        } else {
            qWarning() << "OrientationHelper: Impossible d'obtenir l'activité Android";
        }
#else
        qDebug() << "OrientationHelper: setLandscape() ignoré (non-Android)";
#endif
    }

    // Autorise toutes les orientations (full sensor)
    Q_INVOKABLE void setAllOrientations()
    {
#ifdef Q_OS_ANDROID
        // ActivityInfo.SCREEN_ORIENTATION_FULL_SENSOR = 10
        QJniObject activity = QJniObject::callStaticObjectMethod(
            "org/qtproject/qt/android/QtNative",
            "activity",
            "()Landroid/app/Activity;");
        if (activity.isValid()) {
            activity.callMethod<void>("setRequestedOrientation", "(I)V", 10);
            qDebug() << "OrientationHelper: Toutes orientations autorisées";
        } else {
            qWarning() << "OrientationHelper: Impossible d'obtenir l'activité Android";
        }
#else
        qDebug() << "OrientationHelper: setAllOrientations() ignoré (non-Android)";
#endif
    }

    // Force le mode portrait
    Q_INVOKABLE void setPortrait()
    {
#ifdef Q_OS_ANDROID
        // ActivityInfo.SCREEN_ORIENTATION_SENSOR_PORTRAIT = 7
        QJniObject activity = QJniObject::callStaticObjectMethod(
            "org/qtproject/qt/android/QtNative",
            "activity",
            "()Landroid/app/Activity;");
        if (activity.isValid()) {
            activity.callMethod<void>("setRequestedOrientation", "(I)V", 7);
            qDebug() << "OrientationHelper: Mode portrait forcé";
        } else {
            qWarning() << "OrientationHelper: Impossible d'obtenir l'activité Android";
        }
#else
        qDebug() << "OrientationHelper: setPortrait() ignoré (non-Android)";
#endif
    }
};

#endif // ORIENTATIONHELPER_H
