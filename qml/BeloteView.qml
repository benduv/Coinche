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
        //width:  beloteRoot.width  * 0.45
        //height: beloteRoot.height * 0.5
        //anchors.centerIn: parent
        anchors.fill: parent
        anchors.leftMargin: parent.width * 0.2
        anchors.rightMargin: parent.width * 0.2
        anchors.topMargin: parent.height * 0.23
        anchors.bottomMargin: parent.height * 0.26
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
