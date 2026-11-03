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
                text: contentController.content
            }
        }

        Label {
            text: contentController.loading ? "Loading..." : contentController.errorString
        }
    }
}
