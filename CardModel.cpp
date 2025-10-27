#include "CardModel.h"

CardModel::CardModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int CardModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_cards.size();
}

QVariant CardModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_cards.size())
        return QVariant();

    const Carte &card = m_cards[index.row()];

    switch (role) {
        case ValueRole:
            return QVariant::fromValue(static_cast<int>(card.getChiffre()));
        case SuitRole:
            return QVariant::fromValue(static_cast<int>(card.getCouleur()));
        case ImageRole:
            return getCardImagePath(card);
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> CardModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ValueRole] = "value";
    roles[SuitRole] = "suit";
    roles[ImageRole] = "imagePath";
    return roles;
}

void CardModel::setCards(const std::vector<Carte> &cards)
{
    beginResetModel();
    m_cards = cards;
    endResetModel();
}

void CardModel::removeCard(int index)
{
    if (index < 0 || index >= m_cards.size())
        return;

    beginRemoveRows(QModelIndex(), index, index);
    m_cards.erase(m_cards.begin() + index);
    endRemoveRows();
}

void CardModel::addCard(const Carte &card)
{
    beginInsertRows(QModelIndex(), m_cards.size(), m_cards.size());
    m_cards.push_back(card);
    endInsertRows();
}

QString CardModel::getCardImagePath(const Carte &card) const
{
    // Format the path to the card image based on suit and value
    // Assuming images are named like: "resources/cards/hearts_7.png"
    QString suit;
    switch (card.getCouleur()) {
        case Carte::Couleur::COEUR:
            suit = "hearts";
            break;
        case Carte::Couleur::CARREAU:
            suit = "diamonds";
            break;
        case Carte::Couleur::PIQUE:
            suit = "spades";
            break;
        case Carte::Couleur::TREFLE:
            suit = "clubs";
            break;
    }

    QString value;
    switch (card.getChiffre()) {
        case Carte::Chiffre::SEPT:
            value = "7";
            break;
        case Carte::Chiffre::HUIT:
            value = "8";
            break;
        case Carte::Chiffre::NEUF:
            value = "9";
            break;
        case Carte::Chiffre::DIX:
            value = "10";
            break;
        case Carte::Chiffre::VALET:
            value = "jack";
            break;
        case Carte::Chiffre::DAME:
            value = "queen";
            break;
        case Carte::Chiffre::ROI:
            value = "king";
            break;
        case Carte::Chiffre::AS:
            value = "ace";
            break;
    }

    return QString("qrc:/resources/cards/%1_%2.png").arg(suit).arg(value);
}