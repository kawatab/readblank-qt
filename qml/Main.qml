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
        RowLayout {
            anchors.fill: parent

            ToolButton {
                text: "\u2630"
                onClicked: leftDrawer.open()
            }

            Label {
                text: "ReadBlank"
                font.bold: true
                Layout.fillWidth: true
            }

            ToolButton {
                text: "\u2699"
                onClicked: rightDrawer.open()
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
