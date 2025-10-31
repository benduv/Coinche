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
            CENTTRENTE,
            CENTQUARANTE,
            CENTCINQUANTE,
            CENTSOIXANTE,
            CAPOT,
            GENERALE, 
            PASSE
        };

        Player(std::string name, std::vector<Carte*> main, int index);

        ~Player();

        // Return a vector of player's cards
        std::vector<Carte> getCartes() const;

        void removeCard(int cardIndex);

        void addCardToHand(Carte* carte);

        bool isCartePlayable(int carteIdx, const Carte::Couleur &couleurDemandee, 
                     const Carte::Couleur &couleurAtout, Carte* carteAtout, 
                     int idxPlayerWinning) const;

        // Carte* playCarte(int carteIdx);
        // Carte* playCarte(const Carte::Couleur &couleurDemandee, const Carte::Couleur &couleurAtout, Carte* carteAtout, int idxPlayerWinning);

        //void annonce(Annonce &annoncePrec, Carte::Couleur &couleurAnnonce);

        //void getAnnonce(Annonce &annonce) const;

        //void getCouleurAnnonce(Carte::Couleur &couleur) const;

        int getIndex() const;
        void setIndex(int index);

        void setAtout(const Carte::Couleur &couleurAtout);

        void printAnnonce() const;
        void printMain() const;

        std::string getName() const;
        std::vector<Carte*> getMain() const;

        static int convertAnnonceEnPoint(const Annonce &annonce);

        void addPli(std::array<Carte*, 4> &pli);
        const std::vector<std::array<Carte*, 4>>& getPlis() const { return m_plis; }

    private:
        std::string m_name;
        std::vector<Carte*> m_main;
        int m_index;
        Annonce m_annonce;
        Carte::Couleur m_couleur;

        bool hasCouleur(const Carte::Couleur &couleur) const;
        bool hasHigher(Carte *carte) const;
        std::vector<std::array<Carte*, 4> > m_plis;
};

#endif