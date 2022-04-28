## Building Our Project

In order to build and run this project you'll need to either download and build OpenXR from source, or comment out the #define VR line near the top of the GLViewFinalProject.h file.

Here's the link to the OpenXR repo: https://github.com/KhronosGroup/OpenXR-SDK

Follow the instructions on their ReadMe for building the project.

Copy the openxr_loaderd.lib to your lib64 folder, and copy the folder called openxr which contains all the include files to your include folder.

## Playing Our Game

The game can be controlled with mouse and keyboard or a controller.

To move the camera on mouse and keyboard, hold left click. Press 1 to move forward, press 2 to slow down, press 3 to shoot.

To move the camera on controller, use the D-pad. Press RB to move forward, LB to slow down, A to shoot. Press in the left stick to roll left, press in the right stick to roll right.