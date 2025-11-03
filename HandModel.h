#ifndef HANDMODEL_H
#define HANDMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include "Player.h"
#include "Carte.h"
// #include "Deck.h"
#include <iostream>

// Modèle pour une main de cartes
class HandModel : public QAbstractListModel {
    Q_OBJECT

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

    // Mettre à jour le contexte de jeu pour déterminer les cartes jouables
    void setGameContext(const Carte::Couleur& couleurDemandee, 
                       const Carte::Couleur& couleurAtout,
                       Carte* carteAtout,
                       int idxPlayerWinning,
                       bool isCurrentPlayer);

    // Rafraîchir les données
    void refresh();

    // Nombre de cartes dans la main
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    // Données pour chaque carte
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Noms des rôles pour QML
    QHash<int, QByteArray> roleNames() const override;

private:
    Player* m_player;
    bool m_faceUp;
    Carte::Couleur m_couleurDemandee;
    Carte::Couleur m_couleurAtout;
    Carte* m_carteAtout;
    int m_idxPlayerWinning;
    bool m_isCurrentPlayer;
};
#endif