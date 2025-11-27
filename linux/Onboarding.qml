import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    id: onboardingRoot
    objectName: "onboardingPageObject"
    anchors.fill: parent

    signal continueRequested()

    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 16

        Label {
            text: "Willkommen bei LibrePods"
            font.pixelSize: 22
            font.bold: true
        }

        Label {
            text: "Bevor es losgeht, überprüfe bitte die Windows-Treiber und das Pairing deiner AirPods."
            wrapMode: Text.Wrap
            opacity: 0.8
        }

        Frame {
            width: parent.width

            Column {
                id: driverSection
                spacing: 8
                width: parent.width

                Label {
                    text: "Treiberstatus"
                    font.pixelSize: 16
                    font.weight: Font.DemiBold
                }

                Row {
                    spacing: 8
                    anchors.verticalCenter: parent.verticalCenter

                    Rectangle {
                        width: 12
                        height: 12
                        radius: 6
                        color: airPodsTrayApp.driverInstalled && airPodsTrayApp.testModeEnabled ? "#34C759" : "#FF3B30"
                        border.color: "#2c2c2c"
                    }

                    Label {
                        text: airPodsTrayApp.driverStatus
                        wrapMode: Text.Wrap
                        width: onboardingRoot.width - 80
                    }
                }

                Label {
                    visible: !airPodsTrayApp.runningAsAdmin
                    text: "Starte den Installer als Administrator, damit der KMDF-Treiber installiert werden kann."
                    wrapMode: Text.Wrap
                    color: "#e67e22"
                }

                Label {
                    visible: !airPodsTrayApp.testModeEnabled
                    text: "Für unsignierte Builds muss der Testmodus (bcdedit /set TESTSIGNING ON) aktiv sein."
                    wrapMode: Text.Wrap
                    color: "#e67e22"
                }
            }
        }

        Frame {
            width: parent.width

            Column {
                spacing: 8
                width: parent.width

                Label {
                    text: "Pairing"
                    font.pixelSize: 16
                    font.weight: Font.DemiBold
                }

                Row {
                    spacing: 8
                    anchors.verticalCenter: parent.verticalCenter

                    Rectangle {
                        width: 12
                        height: 12
                        radius: 6
                        color: airPodsTrayApp.pairingReady ? "#34C759" : "#FF3B30"
                        border.color: "#2c2c2c"
                    }

                    Label {
                        text: airPodsTrayApp.pairingStatus
                        wrapMode: Text.Wrap
                        width: onboardingRoot.width - 80
                    }
                }

                Label {
                    visible: !airPodsTrayApp.pairingReady
                    text: "Kopple deine AirPods in den Windows-Bluetooth-Einstellungen, damit LibrePods sie automatisch verbinden kann."
                    wrapMode: Text.Wrap
                    color: "#e67e22"
                }
            }
        }

        Button {
            text: airPodsTrayApp.onboardingRequired ? "Weiter (trotzdem)" : "Los geht's"
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: continueRequested()
        }
    }
}
