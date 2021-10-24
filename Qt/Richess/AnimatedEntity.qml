import Qt3D.Core 2.12
import Qt3D.Render 2.12
import Qt3D.Input 2.12
import Qt3D.Extras 2.12

import QtQml 2.12
import QtQml.Models 2.12
import QtQuick 2.0 as QQ2

Entity {
    id: root

    property int selectedIndex: -1

    Camera {
        id: camera
        projectionType: CameraLens.PerspectiveProjection
        fieldOfView: 45
        nearPlane : 0.1
        farPlane : 1000.0
        position: Qt.vector3d( 0.0, 0.0, 40.0 )
        upVector: Qt.vector3d( 0.0, 1.0, 0.0 )
        viewCenter: Qt.vector3d( 0.0, 0.0, 0.0 )
    }

    OrbitCameraController {
        camera: camera
    }

    components: [
        RenderSettings {
            activeFrameGraph: ForwardRenderer {
                camera: camera
                clearColor: "lightgray"
            }
        },
        InputSettings { }
    ]

    ListModel {
        id: entityModel
    }

    NodeInstantiator {
        model: entityModel

        delegate: Entity {
            property bool selected: root.selectedIndex === index

            PhongMaterial {
                id: entityMaterial

                ambient: Qt.rgba(0.1, 0.1, 0.1, 1.0)
                diffuse: parent.selected ? Qt.rgba(0.0, 1.0, 0.0, 1.0) : Qt.rgba(1.0, 1.0, 1.0, 1.0)
            }

            Mesh {
                id: entityMesh

                source: "qrc:/Resources/pawn.obj"
            }

            Transform {
                id: entityTransform

                translation: Qt.vector3d(x, y, z)
                scale3D: Qt.vector3d(1.0, 1.0, 1.0)
//                rotation: fromAxisAndAngle(Qt.vector3d(1, 0, 0), 90)
            }

            ObjectPicker {
                id: entityPicker

                onPressed: {
                    root.selectedIndex = root.selectedIndex !== index ? root.selectedIndex = index : -1
                }
            }

            components: [ entityMesh, entityMaterial, entityTransform, entityPicker ]
        }

        Component.onCompleted: {
            for (var i = 0; i < 4; i++)
                entityModel.append({"x" : i * 5.0 + 2.5, "y" : 0.0, "z" : 0.0} )

            for (var i = 0; i < 4; i++)
                entityModel.append({"x" : -i * 5.0 - 2.5, "y" : 0.0, "z" : 0.0} )
        }
    }
}
