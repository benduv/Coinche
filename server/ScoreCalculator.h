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
 * @param pointsRealisesTeam1 Points réalisés par l'équipe 1 (sans belote)
 * @param pointsRealisesTeam2 Points réalisés par l'équipe 2 (sans belote)
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
 * @param beloteTeam1 Points de belote de l'équipe 1 (20 si belote, 0 sinon) — toujours comptés
 * @param beloteTeam2 Points de belote de l'équipe 2 (20 si belote, 0 sinon) — toujours comptés
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
    bool capotNonAnnonceTeam2,
    int beloteTeam1 = 0,
    int beloteTeam2 = 0
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
            int scoreCapot = (multiplicateur > 1) ? (160 + (250 * multiplicateur)) : 500;
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
            int scoreGenerale = (multiplicateur > 1) ? (160 + (500 * multiplicateur)) : 1000;
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
            // Team1 a annoncé (belote compte pour atteindre le contrat)
            bool contractReussi = ((pointsRealisesTeam1 + beloteTeam1) >= valeurContrat);

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
            // Team2 a annoncé (belote compte pour atteindre le contrat)
            bool contractReussi = ((pointsRealisesTeam2 + beloteTeam2) >= valeurContrat);

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
        // L'équipe 1 a annoncé (contrat normal, belote compte pour atteindre le contrat)
        if ((pointsRealisesTeam1 + beloteTeam1) >= valeurContrat) {
            if (capotNonAnnonceTeam1) {
                // CAPOT non annoncé: 250 (bonus capot) + valeurContrat
                result.scoreTeam1 = 250 + valeurContrat;
                result.scoreTeam2 = 0;
            } else {
                // Contrat réussi: valeurContrat + pointsRéalisés (belote déjà incluse)
                result.scoreTeam1 = valeurContrat + pointsRealisesTeam1;
                result.scoreTeam2 = pointsRealisesTeam2;
            }
        } else {
            // Contrat échoué: équipe adverse marque tout
            result.scoreTeam1 = 0;
            result.scoreTeam2 = 160 + valeurContrat;
        }
    } else {
        // L'équipe 2 a annoncé (contrat normal, belote compte pour atteindre le contrat)
        if ((pointsRealisesTeam2 + beloteTeam2) >= valeurContrat) {
            if (capotNonAnnonceTeam2) {
                // CAPOT non annoncé: 250 (bonus capot) + valeurContrat
                result.scoreTeam1 = 0;
                result.scoreTeam2 = 250 + valeurContrat;
            } else {
                // Contrat réussi: valeurContrat + pointsRéalisés
                result.scoreTeam1 = pointsRealisesTeam1;
                result.scoreTeam2 = valeurContrat + pointsRealisesTeam2;
            }
        } else {
            // Contrat échoué: équipe adverse marque tout
            result.scoreTeam1 = 160 + valeurContrat;
            result.scoreTeam2 = 0;
        }
    }

    // La belote (20 pts) est TOUJOURS ajoutée à l'équipe qui la possède, quel que soit le résultat
    result.scoreTeam1 += beloteTeam1;
    result.scoreTeam2 += beloteTeam2;

    return result;
}

/**
 * Calcule les scores d'une manche selon les règles de la Belote
 *
 * @param pointsRealisesTeam1 Points réalisés par l'équipe 1 (sans belote)
 * @param pointsRealisesTeam2 Points réalisés par l'équipe 2 (sans belote)
 * @param team1HasBid true si l'équipe 1 a pris (est le preneur)
 * @param capotByTeam1 true si l'équipe 1 a fait tous les plis
 * @param capotByTeam2 true si l'équipe 2 a fait tous les plis
 * @param beloteTeam1 Points de belote de l'équipe 1 (20 si belote, 0 sinon) — toujours comptés
 * @param beloteTeam2 Points de belote de l'équipe 2 (20 si belote, 0 sinon) — toujours comptés
 *
 * Règles Belote :
 * - Le preneur doit faire >= 81 pts (belote comprise) pour réussir
 * - Succès : chaque équipe marque ses points réels
 * - Chute : preneur = 0, défenseur = 160 pts
 * - Capot par le preneur : 250 pts pour le preneur
 * - Capot par le défenseur : 250 pts pour le défenseur (contre-capot)
 * - Belote (Roi+Dame atout) : toujours +20 pts, quel que soit le résultat
 */
inline ScoreResult calculateBeloteMancheScore(
    int pointsRealisesTeam1,
    int pointsRealisesTeam2,
    bool team1HasBid,
    bool capotByTeam1 = false,
    bool capotByTeam2 = false,
    int beloteTeam1 = 0,
    int beloteTeam2 = 0
) {
    ScoreResult result;
    result.scoreTeam1 = 0;
    result.scoreTeam2 = 0;

    if (team1HasBid) {
        // Équipe 1 est le preneur
        if (capotByTeam1) {
            // Capot par le preneur
            result.scoreTeam1 = 250;
            result.scoreTeam2 = 0;
        } else if (capotByTeam2) {
            // Contre-capot : le défenseur fait tous les plis
            result.scoreTeam1 = 0;
            result.scoreTeam2 = 250;
        } else if ((pointsRealisesTeam1 + beloteTeam1) >= 81) {
            // Contrat réussi : chaque équipe marque ses points réels
            result.scoreTeam1 = pointsRealisesTeam1;
            result.scoreTeam2 = pointsRealisesTeam2;
        } else {
            // Chute : preneur = 0, défenseur = 160
            result.scoreTeam1 = 0;
            result.scoreTeam2 = 160;
        }
    } else {
        // Équipe 2 est le preneur
        if (capotByTeam2) {
            // Capot par le preneur
            result.scoreTeam1 = 0;
            result.scoreTeam2 = 250;
        } else if (capotByTeam1) {
            // Contre-capot : le défenseur fait tous les plis
            result.scoreTeam1 = 250;
            result.scoreTeam2 = 0;
        } else if ((pointsRealisesTeam2 + beloteTeam2) >= 81) {
            // Contrat réussi : chaque équipe marque ses points réels
            result.scoreTeam1 = pointsRealisesTeam1;
            result.scoreTeam2 = pointsRealisesTeam2;
        } else {
            // Chute : preneur = 0, défenseur = 160
            result.scoreTeam1 = 160;
            result.scoreTeam2 = 0;
        }
    }

    // La belote (20 pts) est TOUJOURS ajoutée à l'équipe qui la possède, quel que soit le résultat
    result.scoreTeam1 += beloteTeam1;
    result.scoreTeam2 += beloteTeam2;

    return result;
}

} // namespace ScoreCalculator

#endif // SCORECALCULATOR_H
