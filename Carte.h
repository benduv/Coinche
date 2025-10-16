#ifndef CARTE_H
#define CARTE_H

class Carte
{
    public:
        enum Couleur {
            COEUR = 3,
            CARREAU,
            TREFLE,
            PIQUE
        };

        enum Chiffre {
            SEPT = 7,
            HUIT,
            NEUF,
            DIX,
            VALET,
            DAME,
            ROI,
            AS
        };

        Carte();
        Carte(const Carte &carte);
        Carte(Couleur couleur, Chiffre chiffre);
        virtual ~Carte();

        Couleur getCouleur();
        Chiffre getChiffre();

        void printCarte();

        int getValeurDeLaCarte();

        void setAtout(bool isAtout);

        bool operator<(Carte &other);

    private:
        Couleur m_couleur;
        Chiffre m_chiffre;
        bool m_isAtout = false;
};

#endif