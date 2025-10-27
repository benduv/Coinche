#ifndef CARDMODEL_H
#define CARDMODEL_H

#include <QAbstractListModel>
#include <vector>
#include "Carte.h"

class CardModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum CardRoles {
        ValueRole = Qt::UserRole + 1,
        SuitRole,
        ImageRole
    };

    explicit CardModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setCards(const std::vector<Carte> &cards);
    void removeCard(int index);
    void addCard(const Carte &card);

private:
    std::vector<Carte> m_cards;
    QString getCardImagePath(const Carte &card) const;
};

#endif