import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: privacyPolicyRoot
    anchors.fill: parent
    color: "#0a0a2e"

    signal closeRequested()

    // Ratio responsive
    property real minRatio: Math.min(width / 1024, height / 768)

    // Étoiles scintillantes en arrière-plan
    StarryBackground {
        minRatio: privacyPolicyRoot.minRatio
    }

    // Bouton fermer en haut à droite
    Rectangle {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 20 * privacyPolicyRoot.minRatio
        width: 80 * privacyPolicyRoot.minRatio
        height: 80 * privacyPolicyRoot.minRatio
        color: "#cc4444"
        radius: 10 * privacyPolicyRoot.minRatio
        z: 100

        Text {
            anchors.centerIn: parent
            text: "✕"
            font.pixelSize: 48 * privacyPolicyRoot.minRatio
            font.bold: true
            color: "white"
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                privacyPolicyRoot.closeRequested()
            }
            onEntered: {
                parent.color = "#dd5555"
            }
            onExited: {
                parent.color = "#cc4444"
            }
        }
    }

    // ScrollView pour le contenu
    ScrollView {
        anchors.fill: parent
        anchors.margins: 40 * privacyPolicyRoot.minRatio
        anchors.topMargin: 100 * privacyPolicyRoot.minRatio
        clip: true

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
            width: 12 * privacyPolicyRoot.minRatio
            contentItem: Rectangle {
                implicitWidth: 12 * privacyPolicyRoot.minRatio
                radius: 6 * privacyPolicyRoot.minRatio
                color: "#FFD700"
                opacity: parent.pressed ? 1.0 : (parent.hovered ? 0.8 : 0.6)
            }
        }

        ColumnLayout {
            width: Math.min(parent.parent.width - 80 * privacyPolicyRoot.minRatio, 1000 * privacyPolicyRoot.minRatio)
            spacing: 20 * privacyPolicyRoot.minRatio

            // Titre principal
            Text {
                text: "Politique de Confidentialité"
                font.pixelSize: 52 * privacyPolicyRoot.minRatio
                font.bold: true
                color: "#FFD700"
                Layout.alignment: Qt.AlignHCenter
                Layout.bottomMargin: 10 * privacyPolicyRoot.minRatio
            }

            Text {
                text: "Coinche de l'Espace"
                font.pixelSize: 36 * privacyPolicyRoot.minRatio
                font.bold: true
                color: "#FFD700"
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: "Dernière mise à jour : 18 février 2026"
                font.pixelSize: 24 * privacyPolicyRoot.minRatio
                color: "#aaaaaa"
                Layout.alignment: Qt.AlignHCenter
                Layout.bottomMargin: 20 * privacyPolicyRoot.minRatio
            }

            // Section 1 - Responsable du traitement
            Text {
                text: "1. Responsable du traitement"
                font.pixelSize: 32 * privacyPolicyRoot.minRatio
                font.bold: true
                color: "#FFD700"
                Layout.fillWidth: true
            }

            Text {
                text: "Le responsable du traitement des données est :\n\nNebuludik\nEmail : contact@nebuludik.fr"
                font.pixelSize: 26 * privacyPolicyRoot.minRatio
                color: "white"
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }

            // Section 2 - Introduction
            Text {
                text: "2. Introduction"
                font.pixelSize: 32 * privacyPolicyRoot.minRatio
                font.bold: true
                color: "#FFD700"
                Layout.fillWidth: true
                Layout.topMargin: 10 * privacyPolicyRoot.minRatio
            }

            Text {
                text: "La protection de votre vie privée est importante pour nous. Cette politique explique quelles données sont collectées lorsque vous utilisez Coinche de l'Espace, pourquoi elles sont collectées et comment elles sont protégées.\n\nEn créant un compte et en utilisant l'application, vous acceptez les pratiques décrites ci-dessous."
                font.pixelSize: 26 * privacyPolicyRoot.minRatio
                color: "white"
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }

            // Section 3 - Données collectées
            Text {
                text: "3. Données collectées"
                font.pixelSize: 32 * privacyPolicyRoot.minRatio
                font.bold: true
                color: "#FFD700"
                Layout.fillWidth: true
                Layout.topMargin: 10 * privacyPolicyRoot.minRatio
            }

            Text {
                text: "Nous collectons uniquement les données strictement nécessaires au fonctionnement du jeu :\n\n• Adresse e-mail — création et gestion du compte\n• Pseudonyme — identification en jeu\n• Mot de passe chiffré — sécurité du compte\n• Statistiques de jeu — scores, parties, classements\n• Date de consentement RGPD — traçabilité de votre accord\n\nNous ne collectons aucune donnée sensible, ni localisation, ni contacts, ni fichiers personnels."
                font.pixelSize: 26 * privacyPolicyRoot.minRatio
                color: "white"
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }

            // Section 4 - Finalités du traitement
            Text {
                text: "4. Finalités du traitement"
                font.pixelSize: 32 * privacyPolicyRoot.minRatio
                font.bold: true
                color: "#FFD700"
                Layout.fillWidth: true
                Layout.topMargin: 10 * privacyPolicyRoot.minRatio
            }

            Text {
                text: "Les données sont utilisées uniquement pour :\n\n• Créer et gérer votre compte utilisateur\n• Permettre le fonctionnement du jeu en ligne\n• Afficher les scores et classements\n• Répondre aux demandes d'assistance\n• Sécuriser les comptes\n\nAucune donnée n'est utilisée à des fins publicitaires ou commerciales."
                font.pixelSize: 26 * privacyPolicyRoot.minRatio
                color: "white"
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }

            // Section 5 - Base légale
            Text {
                text: "5. Base légale"
                font.pixelSize: 32 * privacyPolicyRoot.minRatio
                font.bold: true
                color: "#FFD700"
                Layout.fillWidth: true
                Layout.topMargin: 10 * privacyPolicyRoot.minRatio
            }

            Text {
                text: "Le traitement repose sur :\n\n• L'exécution du service (fonctionnement du jeu)\n• Votre consentement explicite lors de la création du compte"
                font.pixelSize: 26 * privacyPolicyRoot.minRatio
                color: "white"
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }

            // Section 6 - Hébergement et sécurité
            Text {
                text: "6. Hébergement et sécurité"
                font.pixelSize: 32 * privacyPolicyRoot.minRatio
                font.bold: true
                color: "#FFD700"
                Layout.fillWidth: true
                Layout.topMargin: 10 * privacyPolicyRoot.minRatio
            }

            Text {
                text: "• Les données sont hébergées chez OVHcloud sur des serveurs situés en France\n• Les communications sont chiffrées via protocole SSL/TLS\n• Les mots de passe sont stockés sous forme hachée et sécurisée (SHA-256 + salt)\n• Des mesures techniques sont mises en œuvre pour empêcher tout accès non autorisé"
                font.pixelSize: 26 * privacyPolicyRoot.minRatio
                color: "white"
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }

            // Section 7 - Partage des données
            Text {
                text: "7. Partage des données"
                font.pixelSize: 32 * privacyPolicyRoot.minRatio
                font.bold: true
                color: "#FFD700"
                Layout.fillWidth: true
                Layout.topMargin: 10 * privacyPolicyRoot.minRatio
            }

            Text {
                text: "Nous ne vendons, louons ni partageons vos données personnelles avec des tiers.\n\nLes données sont utilisées exclusivement pour le fonctionnement de l'application."
                font.pixelSize: 26 * privacyPolicyRoot.minRatio
                color: "white"
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }

            // Section 8 - Durée de conservation
            Text {
                text: "8. Durée de conservation"
                font.pixelSize: 32 * privacyPolicyRoot.minRatio
                font.bold: true
                color: "#FFD700"
                Layout.fillWidth: true
                Layout.topMargin: 10 * privacyPolicyRoot.minRatio
            }

            Text {
                text: "• Les données de compte sont conservées tant que votre compte est actif\n• En cas de suppression du compte, les données sont supprimées sous 30 jours maximum\n• Les logs de sécurité sont conservés 30 jours pour la détection de brute force"
                font.pixelSize: 26 * privacyPolicyRoot.minRatio
                color: "white"
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }

            // Section 9 - Droits des utilisateurs (RGPD)
            Text {
                text: "9. Droits des utilisateurs (RGPD)"
                font.pixelSize: 32 * privacyPolicyRoot.minRatio
                font.bold: true
                color: "#FFD700"
                Layout.fillWidth: true
                Layout.topMargin: 10 * privacyPolicyRoot.minRatio
            }

            Text {
                text: "Conformément au Règlement Général sur la Protection des Données, vous disposez des droits suivants :\n\n• Droit d'accès à vos données\n• Droit de rectification (modifier votre email, pseudo, mot de passe)\n• Droit de suppression (supprimer votre compte)\n• Droit à la limitation du traitement\n• Droit d'opposition\n• Droit à la portabilité des données\n\nPour exercer ces droits, contactez-nous à :\n📧 contact@nebuludik.fr"
                font.pixelSize: 26 * privacyPolicyRoot.minRatio
                color: "white"
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }

            // Section 10 - Suppression de compte
            Text {
                text: "10. Suppression de compte"
                font.pixelSize: 32 * privacyPolicyRoot.minRatio
                font.bold: true
                color: "#FFD700"
                Layout.fillWidth: true
                Layout.topMargin: 10 * privacyPolicyRoot.minRatio
            }

            Text {
                text: "Vous pouvez supprimer votre compte directement depuis l'application dans les paramètres.\n\nToutes vos données personnelles (email, pseudo, mot de passe, statistiques) seront supprimées définitivement sous 30 jours."
                font.pixelSize: 26 * privacyPolicyRoot.minRatio
                color: "white"
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }

            // Section 11 - Enfants
            Text {
                text: "11. Enfants"
                font.pixelSize: 32 * privacyPolicyRoot.minRatio
                font.bold: true
                color: "#FFD700"
                Layout.fillWidth: true
                Layout.topMargin: 10 * privacyPolicyRoot.minRatio
            }

            Text {
                text: "L'application ne cible pas spécifiquement les enfants de moins de 13 ans et ne collecte pas sciemment de données les concernant."
                font.pixelSize: 26 * privacyPolicyRoot.minRatio
                color: "white"
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }

            // Section 12 - Modifications
            Text {
                text: "12. Modifications"
                font.pixelSize: 32 * privacyPolicyRoot.minRatio
                font.bold: true
                color: "#FFD700"
                Layout.fillWidth: true
                Layout.topMargin: 10 * privacyPolicyRoot.minRatio
            }

            Text {
                text: "Cette politique peut être modifiée à tout moment. La version la plus récente est toujours disponible dans l'application."
                font.pixelSize: 26 * privacyPolicyRoot.minRatio
                color: "white"
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }

            // Section 13 - Contact
            Text {
                text: "13. Contact"
                font.pixelSize: 32 * privacyPolicyRoot.minRatio
                font.bold: true
                color: "#FFD700"
                Layout.fillWidth: true
                Layout.topMargin: 10 * privacyPolicyRoot.minRatio
            }

            Text {
                text: "Pour toute question concernant la confidentialité :\n\n📧 contact@nebuludik.fr"
                font.pixelSize: 26 * privacyPolicyRoot.minRatio
                color: "white"
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                Layout.bottomMargin: 40 * privacyPolicyRoot.minRatio
            }
        }
    }
}
