import QtQuick 2.0
import QtQuick.Scene3D 2.0

Item {
    Rectangle {
        id: scene

        anchors.fill: parent
        color: "lightgray"

        Scene3D {
            id: scene3d

            anchors.fill: parent
            focus: true
            aspects: ["input", "logic"]
            cameraAspectRatioMode: Scene3D.AutomaticAspectRatio
            multisample: true

            NodeEntity {
                id: nodeEntity
            }
        }
    }
}
