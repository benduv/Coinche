#ifndef SCORECALCULATOR_H
#define SCORECALCULATOR_H

#include "../Player.h"

namespace ScoreCalculator {

struct ScoreResult {
    int scoreTeam1;
    int scoreTeam2;
};

/**
 * Calcule les scores d'une manche selon les règles de la Coinche
 *
 * @param pointsRealisesTeam1 Points réalisés par l'équipe 1 (incluant belote si applicable)
 * @param pointsRealisesTeam2 Points réalisés par l'équipe 2 (incluant belote si applicable)
 * @param valeurContrat Valeur du contrat (80, 90, 100, etc.)
 * @param team1HasBid true si l'équipe 1 a fait l'annonce
 * @param coinched true si coinché
 * @param surcoinched true si surcoinché
 * @param isCapotAnnonce true si c'est une annonce de capot
 * @param capotReussi true si le capot annoncé a été réussi
 * @param isGeneraleAnnonce true si c'est une annonce de générale
 * @param generaleReussie true si la générale annoncée a été réussie
 * @param capotNonAnnonceTeam1 true si l'équipe 1 a fait un capot non annoncé
 * @param capotNonAnnonceTeam2 true si l'équipe 2 a fait un capot non annoncé
 *
 * @return ScoreResult contenant les scores finaux pour chaque équipe
 */
inline ScoreResult calculateMancheScore(
    int pointsRealisesTeam1,
    int pointsRealisesTeam2,
    int valeurContrat,
    bool team1HasBid,
    bool coinched,
    bool surcoinched,
    bool isCapotAnnonce,
    bool capotReussi,
    bool isGeneraleAnnonce,
    bool generaleReussie,
    bool capotNonAnnonceTeam1,
    bool capotNonAnnonceTeam2
) {
    ScoreResult result;
    result.scoreTeam1 = 0;
    result.scoreTeam2 = 0;

    // Multiplicateur COINCHE/SURCOINCHE
    int multiplicateur = 1;
    if (surcoinched) {
        multiplicateur = 4;
    } else if (coinched) {
        multiplicateur = 2;
    }

    // Règles spéciales pour CAPOT et GENERALE
    if (isCapotAnnonce) {
        if (capotReussi) {
            // CAPOT réussi
            int scoreCapot = (multiplicateur > 1) ? (250 + (250 * multiplicateur)) : 500;
            if (team1HasBid) {
                result.scoreTeam1 = scoreCapot;
                result.scoreTeam2 = 0;
            } else {
                result.scoreTeam1 = 0;
                result.scoreTeam2 = scoreCapot;
            }
        } else {
            // CAPOT échoué: équipe adverse marque
            int scoreCapotEchoue = (multiplicateur > 1) ? (160 + (250 * multiplicateur)) : 410;
            if (team1HasBid) {
                result.scoreTeam1 = 0;
                result.scoreTeam2 = scoreCapotEchoue;
            } else {
                result.scoreTeam1 = scoreCapotEchoue;
                result.scoreTeam2 = 0;
            }
        }
    } else if (isGeneraleAnnonce) {
        if (generaleReussie) {
            // GENERALE réussie
            int scoreGenerale = (multiplicateur > 1) ? (500 + (500 * multiplicateur)) : 1000;
            if (team1HasBid) {
                result.scoreTeam1 = scoreGenerale;
                result.scoreTeam2 = 0;
            } else {
                result.scoreTeam1 = 0;
                result.scoreTeam2 = scoreGenerale;
            }
        } else {
            // GENERALE échouée: équipe adverse marque
            int scoreGeneraleEchoue = (multiplicateur > 1) ? (160 + (500 * multiplicateur)) : 660;
            if (team1HasBid) {
                result.scoreTeam1 = 0;
                result.scoreTeam2 = scoreGeneraleEchoue;
            } else {
                result.scoreTeam1 = scoreGeneraleEchoue;
                result.scoreTeam2 = 0;
            }
        }
    } else if (coinched || surcoinched) {
        // COINCHE ou SURCOINCHE: scoring spécial
        if (team1HasBid) {
            // Team1 a annoncé
            bool contractReussi = (pointsRealisesTeam1 >= valeurContrat);

            if (contractReussi) {
                if (capotNonAnnonceTeam1) {
                    // CAPOT non annoncé coinché: 250 + (valeurContrat × multiplicateur)
                    result.scoreTeam1 = 250 + (valeurContrat * multiplicateur);
                    result.scoreTeam2 = 0;
                } else {
                    // Contrat réussi: 160 + (valeurContrat × multiplicateur)
                    result.scoreTeam1 = 160 + (valeurContrat * multiplicateur);
                    result.scoreTeam2 = 0;
                }
            } else {
                // Contrat échoué: équipe adverse marque 160 + (valeurContrat × multiplicateur)
                result.scoreTeam1 = 0;
                result.scoreTeam2 = 160 + (valeurContrat * multiplicateur);
            }
        } else {
            // Team2 a annoncé
            bool contractReussi = (pointsRealisesTeam2 >= valeurContrat);

            if (contractReussi) {
                if (capotNonAnnonceTeam2) {
                    // CAPOT non annoncé coinché: 250 + (valeurContrat × multiplicateur)
                    result.scoreTeam1 = 0;
                    result.scoreTeam2 = 250 + (valeurContrat * multiplicateur);
                } else {
                    // Contrat réussi: 160 + (valeurContrat × multiplicateur)
                    result.scoreTeam1 = 0;
                    result.scoreTeam2 = 160 + (valeurContrat * multiplicateur);
                }
            } else {
                // Contrat échoué: équipe adverse marque 160 + (valeurContrat × multiplicateur)
                result.scoreTeam1 = 160 + (valeurContrat * multiplicateur);
                result.scoreTeam2 = 0;
            }
        }
    } else if (team1HasBid) {
        // L'équipe 1 a annoncé (contrat normal)
        if (pointsRealisesTeam1 >= valeurContrat) {
            if (capotNonAnnonceTeam1) {
                // CAPOT non annoncé: 250 + pointsRéalisés
                result.scoreTeam1 = 250 + pointsRealisesTeam1;
                result.scoreTeam2 = 0;
            } else {
                // Contrat réussi: valeurContrat + pointsRéalisés
                result.scoreTeam1 = valeurContrat + pointsRealisesTeam1;
                result.scoreTeam2 = pointsRealisesTeam2;
            }
        } else {
            // Contrat échoué: équipe 1 marque 0, équipe 2 marque 160 + valeurContrat
            result.scoreTeam1 = 0;
            result.scoreTeam2 = 160 + valeurContrat;
        }
    } else {
        // L'équipe 2 a annoncé (contrat normal)
        if (pointsRealisesTeam2 >= valeurContrat) {
            if (capotNonAnnonceTeam2) {
                // CAPOT non annoncé: 250 + pointsRéalisés
                result.scoreTeam1 = 0;
                result.scoreTeam2 = 250 + pointsRealisesTeam2;
            } else {
                // Contrat réussi: valeurContrat + pointsRéalisés
                result.scoreTeam1 = pointsRealisesTeam1;
                result.scoreTeam2 = valeurContrat + pointsRealisesTeam2;
            }
        } else {
            // Contrat échoué: équipe 2 marque 0, équipe 1 marque 160 + valeurContrat
            result.scoreTeam1 = 160 + valeurContrat;
            result.scoreTeam2 = 0;
        }
    }

    return result;
}

} // namespace ScoreCalculator

#endif // SCORECALCULATOR_H
