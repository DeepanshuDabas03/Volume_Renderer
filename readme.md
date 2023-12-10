## Introduction

Volume rendering is a powerful visualization technique used in various fields such as medical imaging, scientific visualization, and computer-aided design. This project focuses on implementing volume rendering with the added feature of a transfer function, allowing users to customize the appearance of the rendered volume.

## Features

- **Volume Rendering:** Display 3D datasets as 2D images.
- **Transfer Function:** Customize color and opacity mapping for scalar values.
- **Interactive Controls:** User-friendly interface for manipulating the transfer function (Currently Not Working).

## Dependencies

Before building and running the application, make sure you have the following dependencies installed:

- OpenGL
- GLFW
- CMake : Minimum Version 22
- Make
- glm

## Build Instructions

Follow these steps to build the volume rendering application:
- Provide running permission to build.sh by chmod +x build.sh
- run build.sh by ./build.sh
- build will handle all the compilation task now and imgui will be printed directly.

## Usage
- In Main.cpp we can provide path of other data files also, currently we do not have support for providing using command line(will be added before final deadline)
- There are 5 data to visualise currently,can be changed later on by manipulating volume size currently 256x256x256

- Once the application is running, use the interactive controls to manipulate the transfer function and explore the volume rendering. Refer to the documentation for specific details on how to use the transfer function and customize the rendering
## Cleanup
A cleanup script is provided run by :
- Provide running permission to clean_up.sh by chmod +x clean_up.sh
- run clean_up.sh by ./clean_up.sh
