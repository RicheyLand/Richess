#ifndef CAMERA_H
#define CAMERA_H

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/closest_point.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iterator>
#include <cstdlib>
#include <cctype>
#include <map>
#include <vector>
#include <cmath>
#include <exception>
#include <stdexcept>
using namespace std;

class Camera                                            //  represents orbiting camera around center of coordinate system
{
public:
    glm::vec3 Position;                                 //  current position of camera

    int rotateAngle;                                    //  horizontal angle of camera rotation
    int tiltAngle;                                      //  vertical angle of camera rotation
    float distance;                                     //  distance of camera from center of orbit

    int rotateAngleOne;                                 //  restore properties for first orbital state of camera
    int tiltAngleOne;
    float distanceOne;

    int rotateAngleTwo;                                 //  restore properties for second orbital state of camera
    int tiltAngleTwo;
    float distanceTwo;

    bool firstPlayer = true;                            // toggle between two orbital states of camera(toggle between two players)
    bool animation = false;                             //  holds if camera rotation needs to be animated

    Camera()                                            //  one and only class constructor
    {
        setProperties();                                //  set default values of all properties

        rotateAngleOne = rotateAngle;                   //  save both orbital states for future restore
        tiltAngleOne = tiltAngle;
        distanceOne = distance;

        rotateAngleTwo = rotateAngle;
        tiltAngleTwo = tiltAngle;
        distanceTwo = distance;
    }

    void setProperties(int _rotateAngle = 90, int _tiltAngle = 220, int _distance = 5.0f)   //  camera properties setter
    {
        rotateAngle = _rotateAngle;                     //  pass values from paramaters
        tiltAngle = _tiltAngle;
        distance = _distance;

        refreshPosition();                              //  update actual camera position
    }

    void toggle()                                       //  toggle between two orbital states(toggle between two players)
    {
        firstPlayer = !firstPlayer;                     //  toggle orbital state
        animation = true;                               //  enable animation

        if (firstPlayer)
        {
            rotateAngleTwo = rotateAngle;               //  restore camera properties from previous orbital state
            tiltAngleTwo = tiltAngle;
            distanceTwo = distance;
        }
        else
        {
            rotateAngleOne = rotateAngle;               //  restore camera properties from previous orbital state
            tiltAngleOne = tiltAngle;
            distanceOne = distance;
        }
    }

    bool animateToggle()
    {
        if (firstPlayer)
        {                                               //  synchronize actual camera position with orbital state
            if (rotateAngle < rotateAngleOne)
                rotateAngle++;
            else if (rotateAngle > rotateAngleOne)
                rotateAngle--;
            else if (tiltAngle < tiltAngleOne)
                tiltAngle++;
            else if (tiltAngle > tiltAngleOne)
                tiltAngle--;
            else if (distance - distanceOne <= -0.05f)
                distance += 0.05f;
            else if (distance - distanceOne >= 0.05f)
                distance -= 0.05f;
            else
            {                                           //  rotation is complete
                animation = false;                      //  stop animation
                return false;
            }
        }
        else
        {                                               //  synchronize actual camera position with orbital state
            if (rotateAngle < rotateAngleTwo)
                rotateAngle++;
            else if (rotateAngle > rotateAngleTwo)
                rotateAngle--;
            else if (tiltAngle < tiltAngleTwo)
                tiltAngle++;
            else if (tiltAngle > tiltAngleTwo)
                tiltAngle--;
            else if (distance - distanceTwo <= -0.05f)
                distance += 0.05f;
            else if (distance - distanceTwo >= 0.05f)
                distance -= 0.05f;
            else
            {                                           //  rotation is complete
                animation = false;                      //  stop animation
                return false;
            }
        }

        refreshPosition();                              //  update actual position of the camera
        return true;
    }

    void rotateLeft()                                   //  rotate camera left
    {
        if (animation)                                  //  do nothing when animation is running
            return;

        rotateAngle--;

        if (rotateAngle < 0)
            rotateAngle += 360;                         //  update value of camera property

        refreshPosition();                              //  update actual position of the camera
    }

    void rotateRight()
    {
        if (animation)                                  //  do nothing when animation is running
            return;

        rotateAngle++;

        if (rotateAngle > 360)
            rotateAngle -= 360;                         //  update value of camera property

        refreshPosition();                              //  update actual position of the camera
    }

    void tiltUp()
    {
        if (animation)                                  //  do nothing when animation is running
            return;

        if (tiltAngle < 269)
            tiltAngle++;                                //  update value of camera property

        refreshPosition();                              //  update actual position of the camera
    }

    void tiltDown()
    {
        if (animation)                                  //  do nothing when animation is running
            return;

        if (tiltAngle > 180)
            tiltAngle--;                                //  update value of camera property

        refreshPosition();                              //  update actual position of the camera
    }

    void zoomIn()
    {
        if (animation)                                  //  do nothing when animation is running
            return;

        if (distance <= 8.0f)
            distance += 0.05f;                          //  update value of camera property

        refreshPosition();                              //  update actual position of the camera
    }

    void zoomOut()
    {
        if (animation)                                  //  do nothing when animation is running
            return;

        if (distance >= 4.0f)
            distance -= 0.05f;                          //  update value of camera property

        refreshPosition();                              //  update actual position of the camera
    }

    void refreshPosition()
    {                                                   //  calculate actual position of the camera
        Position.x = distance * -sinf(rotateAngle * (M_PI / 180)) * cosf((tiltAngle) * (M_PI / 180));
        Position.y = distance * -sinf((tiltAngle) * (M_PI / 180));
        Position.z = -distance * cosf((rotateAngle) * (M_PI / 180)) * cosf((tiltAngle) * (M_PI / 180));
    }

    glm::mat4 loadViewMatrix()
    {                                                   //  use appropriate function to get view matrix
        return glm::lookAt(glm::vec3(Position.x, Position.y, Position.z), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }
};

#endif
