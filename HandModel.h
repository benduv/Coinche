#ifndef HANDMODEL_H
#define HANDMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include <functional>
#include "Player.h"
#include "Carte.h"
#include <iostream>

// Modèle pour une main de cartes
class HandModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum CardRoles {
        ValueRole = Qt::UserRole + 1,
        SuitRole,
        FaceUpRole,
        IsAtoutRole,
        IsPlayableRole
    };

    explicit HandModel(QObject *parent = nullptr);

    ~HandModel();

    // Définir le joueur source
    void setPlayer(Player* player, bool faceUp = true);

    // Définir quelles cartes sont jouables (indices reçus du serveur)
    void setPlayableCards(const QList<int>& playableIndices);

    // Définir la couleur d'atout
    void setAtoutCouleur(Carte::Couleur atoutCouleur);

    // Rafraîchir les données (reset complet — pour mains adverses et resets)
    void refresh();

    // Tri animé : capture l'ancien ordre, exécute sortFunction, émet des moveRows
    void sortAndAnimate(std::function<void()> sortFunction);

    // Nombre de cartes dans la main
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    // Données pour chaque carte
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Noms des rôles pour QML
    QHash<int, QByteArray> roleNames() const override;

signals:
    void countChanged();

private:
    Player* m_player;
    bool m_faceUp;
    QList<int> m_playableIndices;  // Indices des cartes jouables (depuis le serveur)
    Carte::Couleur m_atoutCouleur;  // Couleur d'atout actuelle
};
#endif