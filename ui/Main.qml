import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import horos 1.0

ApplicationWindow {
    id: window

    width: 800
    height: 600
    title: qsTr("Horos - Edit environment variables")
    visible: true

    EnvVarModel {
        id: envModel
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        // Toolbar
        RowLayout {
            Layout.fillWidth: true

            ToolButton {
                icon.name: "list-add"
                text: qsTr("New")
                display: AbstractButton.IconOnly
                onClicked: editDialog.openForNew()
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Add a new variable")
            }

            ToolButton {
                icon.name: "view-refresh"
                text: qsTr("Refresh")
                display: AbstractButton.IconOnly
                onClicked: envModel.refresh()
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Refresh from environment")
            }
        }

        // Header row
        Rectangle {
            Layout.fillWidth: true
            height: 32
            color: palette.alternateBase
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                Label { text: qsTr("Name");  font.bold: true; Layout.preferredWidth: 250 }
                Label { text: qsTr("Value"); font.bold: true; Layout.fillWidth: true }
                Label { text: "";             Layout.preferredWidth: 40 }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: palette.mid
        }

        // Rows
        ListView {
            id: rowsView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: envModel

            delegate: Rectangle {
                width: rowsView.width
                height: 40
                color: index % 2 === 0 ? palette.base : palette.alternateBase

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8

                    TextField {
                        id: nameField
                        text: model.name
                        Layout.preferredWidth: 250

                        onEditingFinished: {
                            if (text !== model.name) {
                                if (!envModel.renameVariable(index, text)) {
                                    text = model.name
                                }
                            }
                        }
                    }

                    TextField {
                        id: valueField
                        text: model.value
                        Layout.fillWidth: true

                        onEditingFinished: {
                            if (text !== model.value)
                                envModel.setValue(index, text)
                        }
                    }

                    ToolButton {
                        icon.name: "edit-delete"
                        display: AbstractButton.IconOnly
                        Layout.preferredWidth: 32
                        onClicked: deleteDialog.openFor(index, model.name)
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Delete")
                    }
                }
            }
        }
    }

    Dialog {
        id: editDialog
        title: qsTr("New Environment Variable")
        modal: true
        anchors.centerIn: parent
        standardButtons: Dialog.Save | Dialog.Cancel

        function openForNew() {
            nameField.text = ""
            valueField.text = ""
            open()
        }

        ColumnLayout {
            Label { text: qsTr("Name") }
            TextField {
                id: nameField
                Layout.fillWidth: true
                Layout.minimumWidth: 300
            }
            Label { text: qsTr("Value") }
            TextField {
                id: valueField
                Layout.fillWidth: true
                Layout.minimumWidth: 300
            }
        }

        onAccepted: envModel.addVariable(nameField.text, valueField.text)
    }

    // Delete confirmation
    Dialog {
        id: deleteDialog
        title: qsTr("Delete Variable")
        modal: true
        anchors.centerIn: parent
        standardButtons: Dialog.Yes | Dialog.No

        property int targetRow: -1

        function openFor(row, varName) {
            targetRow = row
            messageLabel.text = qsTr("Delete \"%1\"?").arg(varName)
            open()
        }

        Label { id: messageLabel }

        onAccepted: envModel.removeVariable(deleteDialog.targetRow)
    }
}