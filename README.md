# Richess
Richess is a cross-platform 3D chess game.<br />
Game has been written in C++ language.<br />
OpenGL and GLFW libraries have been used.<br />
Rendering is implemented using PBR techniques.<br />

![Screenshot](docs/images/screenshot.png)

## Features

* Comfortable gameplay for two players.
* Full implementation of chess rules.
* Highlighting of possible movements.
* Manual camera rotation and zoom.
* Separated camera settings for each player.
* Use of PBR materials and multiple light sources.
* Light source orbit.

## Build instructions

### Windows

* Clone the Richess game repository.
* Move into the root directory of repository.
* Run `cmake -G "Visual Studio 16 2019" -A Win32 -Bbuild -H.` command.
* Move into the `build` directory.
* Open `richess.sln` file using Visual Studio.
* Select `Release` solution configuration in Visual Studio.
* Build solution.
* Move into the `Release` directory.
* Run Richess.

### Linux

* Clone the Richess game repository.
* Move into the root directory of repository.
* Run `cmake -Bbuild -H.` command.
* Move into the `build` directory.
* Build the executable using `make` automation tool.
* Run Richess.
