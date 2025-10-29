#ifndef CARTE_H
#define CARTE_H

class Carte
{
    public:
        enum Couleur {
            COEUR = 3,
            CARREAU,
            TREFLE,
            PIQUE,
            COULEURINVALIDE
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

        Couleur getCouleur() const;
        Chiffre getChiffre() const;

        void printCarte() const;

        int getValeurDeLaCarte() const;
        int getOrdreCarteForte() const;

        void setAtout(bool isAtout);

        bool operator<(const Carte &other) const;

    private:
        Couleur m_couleur;
        Chiffre m_chiffre;
        bool m_isAtout = false;
};

#endif