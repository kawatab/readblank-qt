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

    Connections {
        target: contentController

        function onWordRevealed(word, language) {
            console.log("wordRevealed:", word, language);
            speechController.speak(word, language);
        }
    }

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
            anchors.margins: 12
            spacing: 8

            Label {
                text: "Speech"
                font.bold: true
                Layout.fillWidth: true
            }

            Column {
                spacing: 4

                Label {
                    text: "Speed"
                    Layout.fillWidth: true
                }

                Slider {
                    id: speedSlider
                    Layout.fillWidth: true
                    from: 0.4
                    to: 2.0
                    stepSize: 0.4
                    snapMode: Slider.SnapAlways

                    value: 1.0 / speechController.lengthScale

                    onMoved: speechController.lengthScale = 1.0 / value
                }

                Row {
                    width: speedSlider.availableWidth - speedSlider.handle.width
                    anchors.horizontalCenter: speedSlider.horizontalCenter

                    Rectangle {
                        width: 1
                        height: 5
                        color: palette.text
                    }

                    Repeater {
                        model: 4

                        Item {
                            width: parent.width / 4
                            height: 5

                            Rectangle {
                                width: 1
                                height: 5
                                color: palette.text
                                anchors.horizontalCenter: parent.right
                            }
                        }
                    }
                }

                RowLayout {
                    width: speedSlider.availableWidth

                    Label {
                        text: "Slower"
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Label {
                        text: "Faster"
                    }
                }
            }

            Column {
                spacing: 4

                Label {
                    text: "Volume"
                    Layout.fillWidth: true
                }

                Slider {
		    id: volumeSlider
                    Layout.fillWidth: true
                    from: 0.0
                    to: 1.0
                    stepSize: 0.25
                    snapMode: Slider.SnapAlways

                    value: speechController.volume

                    onMoved: speechController.volume = value
                }

                Row {
                    width: volumeSlider.availableWidth - volumeSlider.handle.width
                    anchors.horizontalCenter: volumeSlider.horizontalCenter

                    Rectangle {
                        width: 1
                        height: 5
                        color: palette.text
                    }

                    Repeater {
                        model: 4

                        Item {
                            width: parent.width / 4
                            height: 5

                            Rectangle {
                                width: 1
                                height: 5
                                color: palette.text
                                anchors.horizontalCenter: parent.right
                            }
                        }
                    }
                }

                RowLayout {
                    width: speedSlider.availableWidth

                    Label {
                        text: "Mute"
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Label {
                        text: "Maximum"
                    }
                }
            }

            Item {
                Layout.fillHeight: true
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
            Layout.preferredHeight: 5

            TextArea {
                width: parent.width
                readOnly: true
                selectByMouse: true
                wrapMode: TextArea.Wrap
                textFormat: TextEdit.RichText

                text: contentController.currentParagraphHtml

                onLinkActivated: link => {
                    if (link.startsWith("word:")) {
                        const wordIndex = Number(link.substring(5));
                        contentController.selectWord(wordIndex);
                    }
                }
            }
        }

        Label {
            visible: speechController.errorString !== ""
            text: speechController.errorString
            color: palette.brightText
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true

            ToolButton {
                icon.name: "go-previous"
                enabled: contentController.canSelectPreviousWord
                ToolTip.text: "Previous selectable word"

                onClicked: contentController.previousWord()
            }

            ToolButton {
                icon.name: "go-next"
                enabled: contentController.canSelectNextWord
                ToolTip.text: "Next selectable word"

                onClicked: contentController.nextWord()
            }

            Item {
                Layout.fillWidth: true
            }

            ToolButton {
                icon.name: speechController.speaking ? "media-playback-stop" : "audio-volume-high"

                enabled: contentController.paragraphCount > 0

                onClicked: {
                    if (speechController.speaking)
                        speechController.stop();
                    else
                        speechController.speak(contentController.currentParagraph, contentController.language);
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
                        text: "Engine: Piper"
                        wrapMode: Text.Wrap
                        Layout.fillWidth: true
                    }

                    Label {
                        text: "Voice: " + speechController.voiceForLanguage(contentController.language)
                        wrapMode: Text.Wrap
                        Layout.fillWidth: true
                    }

                    Label {
                        text: "Engine license: GPL-3.0-or-later"
                        wrapMode: Text.Wrap
                        Layout.fillWidth: true
                    }

                    Label {
                        text: "Voice models have their own licenses."
                        wrapMode: Text.Wrap
                        Layout.fillWidth: true
                    }

                    Label {
                        text: '<a href="https://github.com/OHF-Voice/piper1-gpl">Piper project</a>'
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

        Flow {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 5
            spacing: 6

            Repeater {
                model: contentController.selectableWords

                delegate: Button {
                    required property var modelData

                    enabled: !modelData.revealed

                    text: modelData.text

                    implicitWidth: contentItem.implicitWidth + leftPadding + rightPadding
                    implicitHeight: contentItem.implicitHeight + topPadding + bottomPadding

                    onClicked: {
                        contentController.revealWord(modelData.index);
                    }
                }
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
        }

        Label {
            text: contentController.loading ? "Loading..." : contentController.errorString
        }
    }
}
