// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 Yasuhiro Yamakawa <kawatab@gmail.com>

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    visible: true
    width: 800
    height: 600
    title: "ReadBlankQt"

    Component.onCompleted: {
        const first = favoriteListModel.firstItem();

        if (first)
            contentController.loadFavorite(first);
    }

    header: ToolBar {
        padding: 0

        RowLayout {
            anchors.fill: parent
            anchors.bottomMargin: 3

            ToolButton {
                icon.name: "open-menu-symbolic"
                onClicked: leftDrawer.open()
            }
            Label {
                text: contentController.title
                font.bold: true
            }

            Item {
                Layout.fillWidth: true
            }

            ToolButton {
                icon.name: contentController.loading ? "process-stop" : "edit-copy"

                onClicked: {
                    if (contentController.loading) {
                        contentController.cancelLoading();
                    } else {
                        rightDrawer.open();
                    }
                }
            }
        }

        // NOTE: Qt Quick Controls' ProgressBar causes EGL warnings
        // when hardware-accelerated rendering is used on Wayland.
        // Use a custom Rectangle-based indicator instead.
        // ProgressBar {
        //     anchors.left: parent.left
        //     anchors.right: parent.right
        //     anchors.bottom: parent.bottom
        //     visible: contentController.loading
        //     indeterminate: true
        //     height: 3
        // }

        Rectangle {
            id: progressIndicator

            width: parent.width * 0.3
            height: 3
            y: parent.height - height
            color: palette.highlight
            visible: contentController.loading

            SequentialAnimation on x {
                running: progressIndicator.visible
                loops: Animation.Infinite

                NumberAnimation {
                    from: -progressIndicator.width
                    to: progressIndicator.parent.width
                    duration: 1200
                    easing.type: Easing.Linear
                }
            }
        }
    }

    Drawer {
        id: leftDrawer

        edge: Qt.LeftEdge
        width: 250
        height: parent.height

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            ItemDelegate {
                text: "Open"
                Layout.fillWidth: true
                onClicked: leftDrawer.close()
            }

            ItemDelegate {
                text: "Recent"
                Layout.fillWidth: true
                onClicked: leftDrawer.close()
            }
        }
    }

    Drawer {
        id: rightDrawer

        edge: Qt.RightEdge
        width: 600
        height: parent.height

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            Button {
                Layout.alignment: Qt.AlignLeft
                text: "Add from clipboard"
                icon.name: "edit-paste"

                onClicked: {
                    favoriteListModel.addUrlFromClipboard();
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Rectangle {
                    anchors.fill: parent
                    color: palette.alternateBase
                    radius: 4
                    border.color: palette.mid
                }

                ListView {
                    id: favoriteListView

                    anchors.fill: parent
                    anchors.margins: 2

                    model: favoriteListModel
                    clip: true
                    spacing: 2

                    delegate: RowLayout {
                        width: ListView.view.width
                        spacing: 0

                        ItemDelegate {
                            Layout.fillWidth: true

                            highlighted: contentModel === contentController.currentModel

                            contentItem: ColumnLayout {
                                spacing: 2

                                Label {
                                    Layout.fillWidth: true
                                    text: title !== "" ? title : url
                                    font.bold: true
                                    elide: Text.ElideRight
                                }

                                Label {
                                    Layout.fillWidth: true
                                    text: url
                                    font.pixelSize: 11
                                    color: palette.placeholderText
                                    elide: Text.ElideMiddle
                                }
                                Label {
                                    Layout.fillWidth: true
                                    text: lastAccessed ? Qt.formatDateTime(lastAccessed, "yyyy-MM-dd HH:mm") : ""
                                    font.pixelSize: 10
                                    color: palette.placeholderText
                                }
                            }

                            onClicked: {
                                contentController.loadFavorite(contentModel);
                                favoriteListModel.moveToTop(index);

                                rightDrawer.close();
                            }
                        }

                        Label {
                            text: language
                            font.pixelSize: 11
                            color: palette.placeholderText
                            Layout.rightMargin: 8
                        }

                        ToolButton {
                            icon.name: "user-trash"

                            onClicked: {
                                favoriteListModel.removeUrl(index);
                            }
                        }
                    }
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            TextArea {
                width: parent.width
                readOnly: true
                wrapMode: TextArea.Wrap
                text: contentController.currentParagraph
            }
        }

        RowLayout {
            Layout.fillWidth: true

            ToolButton {
                icon.name: "go-previous"
                enabled: contentController.canGoPrevious

                onClicked: contentController.previousParagraph()
            }

            ToolButton {
                icon.name: "go-next"
                enabled: contentController.canGoNext

                onClicked: contentController.nextParagraph()
            }

            Item {
                Layout.fillWidth: true
            }

            ToolButton {
                icon.name: contentController.speechActive ? "media-playback-stop" : "audio-volume-high"

                enabled: contentController.paragraphCount > 0

                onClicked: {
                    if (contentController.speechActive)
                        contentController.stopSpeaking();
                    else
                        contentController.speakCurrentParagraph();
                }
            }

            ToolButton {
                icon.name: "dialog-information"
                onClicked: speechInfoPopup.open()
            }

            Popup {
                id: speechInfoPopup

                anchors.centerIn: Overlay.overlay
                width: Math.min(360, parent.width - 40)
                padding: 16

                ColumnLayout {
                    width: parent.width
                    spacing: 8

                    Label {
                        text: "Speech synthesis"
                        font.bold: true
                        font.pixelSize: 16
                    }

                    Label {
                        text: "Engine: " + contentController.speechEngineName
                        wrapMode: Text.Wrap
                        Layout.fillWidth: true
                    }

                    Label {
                        text: "Voice: " + contentController.speechVoiceName
                        wrapMode: Text.Wrap
                        Layout.fillWidth: true
                    }

                    Label {
                        visible: contentController.speechVoiceFallback
                        text: "The requested language is unavailable. Using English instead."
                        color: palette.placeholderText
                        wrapMode: Text.Wrap
                        Layout.fillWidth: true
                    }

                    Label {
                        text: "License: " + contentController.speechLicense
                        wrapMode: Text.Wrap
                        Layout.fillWidth: true
                    }

                    Label {
                        text: "Copyright: Jonathan Duddington and other contributors"
                        wrapMode: Text.Wrap
                        Layout.fillWidth: true
                    }

                    Label {
                        text: '<a href="' + contentController.speechProjectUrl + '">eSpeak NG project</a>'
                        textFormat: Text.RichText
                        onLinkActivated: link => Qt.openUrlExternally(link)
                        Layout.fillWidth: true
                    }

                    Button {
                        text: "Close"
                        Layout.alignment: Qt.AlignRight
                        onClicked: speechInfoPopup.close()
                    }
                }
            }
        }

        Label {
            text: contentController.loading ? "Loading..." : contentController.errorString
        }
    }
}
