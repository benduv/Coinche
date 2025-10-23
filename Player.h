#ifndef PLAYER_H
#define PLAYER_H

#include "Carte.h"
#include <string>
#include <vector>
#include <array>

class Player
{
    public:
        enum Annonce {
            ANNONCEINVALIDE = 0,
            QUATREVINGT,
            QUATREVINGTDIX,
            CENT,
            CENTDIX,
            CENTVINGT,
            CENTTRENTRE,
            CENTQUARANTE,
            CENTCINQUANTE,
            CENTSOIXANTE,
            CAPOT,
            GENERALE, 
            PASSE
        };

        Player(std::string name, std::vector<Carte*> main, int index);

        ~Player();

        Carte* playCarte();
        Carte* playCarte(const Carte::Couleur &couleurDemandee, const Carte::Couleur &couleurAtout, Carte* carteAtout);

        void annonce(Annonce &annoncePrec, Carte::Couleur &couleurAnnonce);

        void getAnnonce(Annonce &annonce);

        void getCouleurAnnonce(Carte::Couleur &couleur);

        int getIndex();
        void setIndex(int index);

        void printAnnonce();
        void printMain() const;

        std::string getName() const;
        std::vector<Carte*> getMain() const;

        void addPli(std::array<Carte*, 4> &pli);
        const std::vector<std::array<Carte*, 4>>& getPlis() const { return m_plis; }

    private:
        std::string m_name;
        std::vector<Carte*> m_main;
        int m_index;
        Annonce m_annonce;
        Carte::Couleur m_couleur;

        bool hasCouleur(const Carte::Couleur &couleur);
        bool hasHigher(Carte *carte);
        std::vector<std::array<Carte*, 4> > m_plis;
};

#endif