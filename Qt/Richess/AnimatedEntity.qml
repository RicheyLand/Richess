import Qt3D.Core 2.12
import Qt3D.Render 2.12
import Qt3D.Input 2.12
import Qt3D.Extras 2.12

import QtQuick 2.0 as QQ2

Entity {
    id: sceneRoot

    Camera {
        id: camera
        projectionType: CameraLens.PerspectiveProjection
        fieldOfView: 45
        nearPlane : 0.1
        farPlane : 1000.0
        position: Qt.vector3d( 0.0, 0.0, 20.0 )
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

    PhongMaterial {
        id: pawnMaterial

        ambient: Qt.rgba(0.1, 0.1, 0.1, 1.0)
        diffuse: Qt.rgba(1.0, 1.0, 1.0, 1.0)
    }

    Mesh {
        id: pawnMesh

        source: "qrc:/Resources/pawn.obj"
    }

    Transform {
        id: pawnTransform

        translation: Qt.vector3d(0.0, -1.0, 0.0)
        scale3D: Qt.vector3d(1.0, 1.0, 1.0)
//        rotation: fromAxisAndAngle(Qt.vector3d(1, 0, 0), 90)
    }

    Entity {
        id: pawnEntity

        components: [ pawnMesh, pawnMaterial, pawnTransform ]
    }
}
