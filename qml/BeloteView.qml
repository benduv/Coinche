import QtQuick
import QtQuick.Controls
import QtMultimedia

Item {
    id: beloteRoot
    anchors.fill: parent

    // Couche de base : CoincheView complète (avec les guards qui masquent les éléments Coinche)
    Loader {
        id: coincheLoader
        anchors.fill: parent
        source: "qrc:/qml/CoincheView.qml"
    }

    // Panneau d'annonces Belote unifié (tours 1 et 2)
    BeloteAnnoncesPanel {
        width:  beloteRoot.width  * 0.45
        height: beloteRoot.height * 0.5
        anchors.centerIn: parent
        visible: gameModel.isBeloteMode &&
                 gameModel.biddingPhase &&
                 gameModel.distributionPhase === 0 &&
                 gameModel.retourneeSuit >= 0 &&
                 gameModel.retourneeValue >= 0 &&
                 !gameModel.showGoodGameAnimation &&
                 !(coincheLoader.item && coincheLoader.item.ufoNewMancheAnimationVisible) &&
                 !(coincheLoader.item && coincheLoader.item.exitPopupVisible) &&
                 !(coincheLoader.item && coincheLoader.item.showBotReplacementPopupAlias)
    }
}
