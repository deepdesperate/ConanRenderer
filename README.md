# ConanRenderer is a C-based Renderer using library SDL

## Overview:
ConanRenderer is a C-based 3D Rendering engine. It's core purpose is to understand the principles of 3D Graphics rendering through a software implementation, primarily leveraging the SDL library for screen and pixel management.

## ✨Technology
- C
- SDL

## 🚀Features
- Rendering Pipeline dives into matrices and their implementation for 3D Transformations.
- Load 3D models in popular .obj format.
- Supports texture mapping with image files.
- Decoding UV data for texture implementation.
- Shading Model for correct light placement and rendering.

## 📍The Process
- Input: The renderer first loads a 3D model (like an .obj mesh) along with its corresponding texture data (specifically .png files) and UV coordinates.
- Transformation (The Core Math): This is the heart of the process. The engine uses matrix mathematics to apply a series of transformations to the model's vertices:
- World: Position the object in the 3D scene.
- View: Adjust the position relative to the virtual camera.
- Projection: Convert the 3D coordinates into 2D perspective (making far objects appear smaller).
- Rasterization: Once the vertices are projected, the engine takes the resulting 2D triangles and determines which pixels on the screen they cover.
Shading/Texturing: It applies the loaded Base Color (.png) texture to the calculated pixels based on the model's UV map data.
- Display (SDL): The final calculated pixel colors are written to an image buffer, and the SDL library is used to display this rendered image on the window, completing one frame of the 3D scene.

## 🎯Future Plans
1. Extended File Support: Implementing Import Algo for .fbx and .gltf.
2. Texture Format Support: Support for different image formats .jpeg, .tiff
3. Scene Management: Implement Model Outliner to visualize and manage active meshes
4. Add Gizmo support and Functionality for real-time mesh translation within render window.

## 🚦Running the Project
To clone, build, and test the ConanRenderer, you will need a development environment for C projects that link the SDL Library.

1. Install SDL Library
2. `clone` the repository.
3. `make` to compile the C files
4. `./renderer` Execute the compiler renderer program.

## Preview:
![ConanRenderer](docs/images/ConanRenderer2.png)
![ConanRenderer](docs/images/ConanRenderer.png)