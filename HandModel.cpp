#include <iostream>
#include "HandModel.h"


HandModel::HandModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_player(nullptr)
    , m_faceUp(true)
    , m_couleurDemandee(static_cast<Carte::Couleur>(7))
    , m_couleurAtout(static_cast<Carte::Couleur>(7))
    , m_carteAtout(nullptr)
    , m_idxPlayerWinning(-1)
    , m_isCurrentPlayer(false)
{
}

HandModel::~HandModel()
{
}

// Définir le joueur source
void HandModel::setPlayer(Player* player, bool faceUp) {
    beginResetModel();
    m_player = player;
    m_faceUp = faceUp;
    endResetModel();
}

// Mettre à jour le contexte de jeu pour déterminer les cartes jouables
void HandModel::setGameContext(const Carte::Couleur& couleurDemandee, 
                    const Carte::Couleur& couleurAtout,
                    Carte* carteAtout,
                    int idxPlayerWinning,
                    bool isCurrentPlayer) {
    std::cout << "couleur demandee dans HandModel: " << couleurDemandee << std::endl;
    m_couleurDemandee = couleurDemandee;
    m_couleurAtout = couleurAtout;
    m_carteAtout = carteAtout;
    m_idxPlayerWinning = idxPlayerWinning;
    m_isCurrentPlayer = isCurrentPlayer;
    
    // Notifier QML que les états "playable" ont changé
    if (rowCount() > 0) {
        emit dataChanged(index(0), index(rowCount() - 1), {IsPlayableRole});
    }
}

// Rafraîchir les données
void HandModel::refresh() {
    beginResetModel();
    endResetModel();
}

// Nombre de cartes dans la main
int HandModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid() || !m_player)
        return 0;
    return m_player->getMain().size();
}

// Données pour chaque carte
QVariant HandModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || !m_player)
        return QVariant();

    const auto& main = m_player->getMain();
    if (index.row() >= main.size())
        return QVariant();

    const Carte* carte = main[index.row()];
    if (!carte)
        return QVariant();

    switch (role) {
        case ValueRole:
            return static_cast<int>(carte->getChiffre());
        case SuitRole:
            return static_cast<int>(carte->getCouleur());
        case FaceUpRole:
            return m_faceUp;
        case IsAtoutRole:
            return false; // À adapter selon la logique
        case IsPlayableRole:
            if (!m_isCurrentPlayer || !m_player) {
                return false;
            }
            return m_player->isCartePlayable(index.row(), m_couleurDemandee, 
                                                m_couleurAtout, m_carteAtout, 
                                                m_idxPlayerWinning);
        default:
            return QVariant();
    }
}

// Noms des rôles pour QML
QHash<int, QByteArray> HandModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[ValueRole] = "value";
    roles[SuitRole] = "suit";
    roles[FaceUpRole] = "faceUp";
    roles[IsAtoutRole] = "isAtout";
    roles[IsPlayableRole] = "isPlayable";
    return roles;
}