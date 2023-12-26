# Volume Renderer: A Computer Graphics Project

Welcome to my project, **Volume Renderer**, a testament to the power of computer graphics. This project was undertaken during the Monsoon 2023 semester at the Indraprastha Institute of Information Technology, Delhi (IIITD).

Volume Renderer is a unique blend of technical prowess and creative vision. It incorporates a **Transfer Function** and **Surface Shading** to create visually stunning and scientifically accurate representations of volumetric data.

The Transfer Function plays a crucial role in determining how data values are mapped to color and opacity, providing us with the flexibility to highlight specific features of interest within the volume. On the other hand, Surface Shading adds depth and realism to the rendered images by simulating the way light interacts with the surfaces.

We invite you to explore our project and witness the fascinating interplay of science and art in computer graphics.

## Introduction

Volume rendering is a powerful visualization technique used in various fields such as medical imaging, scientific visualization, and computer-aided design. This project focuses on implementing volume rendering with the added feature of a transfer function, allowing users to customize the appearance of the rendered volume.

  ![Human Foot With Transfer Function](https://github.com/DeepanshuDabas03/Volume_Renderer/blob/main/Output/Images/Foot_TF.png)

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
## Output: Grayscale vs Transfer Functions
<ul>
 <li>  <h3>Human Foot</h3>
  <img src="https://github.com/DeepanshuDabas03/Volume_Renderer/blob/main/Output/Images/Foot_GrayScale.png"  width=50% height=50%>
    <img src="https://github.com/DeepanshuDabas03/Volume_Renderer/blob/main/Output/Images/Foot_Back_GrayScale.png.png" width=50% height=50%>
<img src="https://github.com/DeepanshuDabas03/Volume_Renderer/blob/main/Output/Images/Foot_TF.png"  width=50% height=50%>
<img src="https://github.com/DeepanshuDabas03/Volume_Renderer/blob/main/Output/Images/Foot_Back_TF.png"  width=50% height=50%>

   </li>
   <li>
<h3>Bonzai</h3>
<img src="https://github.com/DeepanshuDabas03/Volume_Renderer/blob/main/Output/Images/bonzai_gray_scale.png"  width=50% height=50%>
<img src="https://github.com/DeepanshuDabas03/Volume_Renderer/blob/main/Output/Images/bonzai_TF.png"  width=50% height=50%>
     </li>
   <li> <h3> MRI Ventricles </h3>
<img src="https://github.com/DeepanshuDabas03/Volume_Renderer/blob/main/Output/Images/mri_ventricle_gray_scale.png"  width=50% height=50%>
<img src="https://github.com/DeepanshuDabas03/Volume_Renderer/blob/main/Output/Images/mri_ventricle_TF.png"  width=50% height=50%>
     </li>
     <li>
   <h3> Anuerism </h3>
<img src="https://github.com/DeepanshuDabas03/Volume_Renderer/blob/main/Output/Images/anuerism_gray_scale.png"  width=50% height=50%>
<img src="https://github.com/DeepanshuDabas03/Volume_Renderer/blob/main/Output/Images/anureism_TF.png"  width=50% height=50%>
       </li>
       <li>
  <h3> Human Skull </h3>
<img src="https://github.com/DeepanshuDabas03/Volume_Renderer/blob/main/Output/Images/Skull_gray_scale.png"  width=50% height=50%>
</li>

</ul>

## Report And Presentations

- **Video Demo**: Available in the provided <a href="https://github.com/DeepanshuDabas03/Volume_Renderer/tree/main/Output/Videos"> directory</a>.
- **Report**: Can be accessed  <a href="https://github.com/DeepanshuDabas03/Volume_Renderer/blob/main/Reports%20and%20Presentation/Report.pdf" >here </a>.
- **Presentation**: View it at the following link. <a href="https://github.com/DeepanshuDabas03/Volume_Renderer/blob/main/Reports%20and%20Presentation/Presentation.pdf">Presentation </a> 

## Cleanup
A cleanup script is provided run by :
- Provide running permission to clean_up.sh by chmod +x clean_up.sh
- run clean_up.sh by ./clean_up.sh

## Contact Information
- **Name**: [Deepanshu Dabas]
- **Email**: [deepanshu21249@iiitd.ac.in]
- **LinkedIn**: [https://www.linkedin.com/in/deepanshu-dabas-29318b222/]

## Credits
This project wouldn't have been possible without the valuable assets provided by **Professor Dr. Ojaswa Sharma**. The code for features such as eyeball tracking, view transformation, and project transformation were adapted from assignments provided in the Computer Graphics course. I express my sincere gratitude to professor for his guidance and resources.
