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
    property var evenList: []

    Camera {
        id: camera
        projectionType: CameraLens.PerspectiveProjection
        fieldOfView: 45
        nearPlane: 0.1
        farPlane: 1000.0
        position: Qt.vector3d(0.0, 0.0, 40.0)
        upVector: Qt.vector3d(0.0, 1.0, 0.0)
        viewCenter: Qt.vector3d(0.0, 0.0, 0.0)
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
            pickingSettings.pickMethod: PickingSettings.TrianglePicking
            pickingSettings.pickResultMode: PickingSettings.NearestPick
        },
        InputSettings { }
    ]

    ListModel {
        id: cubeModel
    }

    NodeInstantiator {
        model: cubeModel

        delegate: Entity {
            PhongMaterial {
                id: cubeMaterial

                ambient: Qt.rgba(0.2, 0.2, 0.2, 1.0)
                diffuse: index < root.evenList.length && root.evenList[index] ? Qt.rgba(1.0, 1.0, 1.0, 1.0) : Qt.rgba(0.0, 0.0, 0.0, 1.0)
            }

            CuboidMesh {
                id: cubeMesh

                xExtent: 5.0
                yExtent: 2.5
                zExtent: 5.0
            }

            Transform {
                id: cubeTransform

                translation: Qt.vector3d(x, y, z)
                scale3D: Qt.vector3d(1.0, 1.0, 1.0)
//                rotation: fromAxisAndAngle(Qt.vector3d(1, 0, 0), 90)
            }

            ObjectPicker {
                id: cubePicker

                onPressed: {
                }
            }

            components: [ cubeMesh, cubeMaterial, cubeTransform, cubePicker ]
        }

        Component.onCompleted: {
            for (var i = 0; i < 8; i++)
            {
                for (var j = 0; j < 8; j++)
                {
                    if (i % 2)
                    {
                        if (j % 2)
                            root.evenList.push(true)
                        else
                            root.evenList.push(false)
                    }
                    else
                    {
                        if (j % 2)
                            root.evenList.push(false)
                        else
                            root.evenList.push(true)
                    }

                    cubeModel.append({"x" : i * 5.0 - 17.5, "y" : -3.32, "z" : j * 5.0 - 35.0})
                }
            }
        }
    }

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
            for (var i = 0; i < 8; i++)
                entityModel.append({"x" : i * 5.0 - 17.5, "y" : 0.0, "z" : 0.0})
        }
    }
}
