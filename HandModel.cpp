#include <QDebug>
#include "HandModel.h"


HandModel::HandModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_player(nullptr)
    , m_faceUp(true)
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

// Définir quelles cartes sont jouables (indices reçus du serveur)
void HandModel::setPlayableCards(const QList<int>& playableIndices) {
    m_playableIndices = playableIndices;

    qDebug() << "HandModel - Cartes jouables mises à jour:" << m_playableIndices;

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
            // Vérifier si l'index est dans la liste des cartes jouables (reçue du serveur)
            return m_playableIndices.contains(index.row());
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