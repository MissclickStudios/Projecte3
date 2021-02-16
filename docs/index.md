## What is NULL Engine?

---

Download Latest Release: [Click Here](<https://github.com/BarcinoLechiguino/NULL_Engine/releases/tag/v0.75>)

---

NULL Engine is a basic 3D Graphics Engine for game development. It has been developed by a single [Bachelor's degree in Video Game Design and Development](<https://www.citm.upc.edu/ing/estudis/graus-videojocs/>) student at [UPC CITM](<https://www.citm.upc.edu/>) [Tech Talent Center](<https://www.talent.upc.edu/cat/school/ttc/>) in Barcelona for the Game Engines Subject.

NULL Engine has been coded from scratch in C and C++ along with the following libraries:

- [Standard Template Library](<https://en.wikipedia.org/wiki/Standard_Template_Library>)
- [Simple DirectMedia Layer](<https://libsdl.org/>)
- [OpenGL v4.6](<https://www.opengl.org/>)
- [Glew v2.1.0](<http://glew.sourceforge.net/>)
- [ImGui v1.79 (Docking Branch)](<https://github.com/ocornut/imgui/tree/docking>)
- [ImGuizmo v1.61](<https://github.com/CedricGuillemet/ImGuizmo>)
- [MatGeoLib v1.5](<https://github.com/juj/MathGeoLib>)
- [PhysFS v3.0.2](<https://icculus.org/physfs/>)
- [Parson v1.1.0](<https://github.com/kgabis/parson>)
- [Assimp v3.1.1](<http://www.assimp.org/index.php/downloads>)
- [DevIL v1.8.0](<http://openil.sourceforge.net/download.php>)
- [mmgr](<http://www.paulnettle.com/>)

---

---

## The Developer: Ángel González

<img src="Images/MyMugshot.png" alt="The Developer: Ángel González" height="300">

* Github: [Click Here](<https://github.com/BarcinoLechiguino>)
* LinkedIn: [Click Here](<https://www.linkedin.com/in/angel-gonzalez-jimenez/>)

---

---

## Core Sub-Systems

### File Importer

This system is in charge of importing any asset file into the Engine as well as Saving the asset files data into different custom file formats, which will be Loaded later on when required.

The File Importer, as mentioned above, is separated in two sub-systems:

* **Import:** Will be in charge of extracting the data stored in any asset file (.FBX, .PNG...) and storing it in their correspondent data structures inside the Engine (Ex: .PNG --> Texture Structure).

  

* **Save&Load:** Will be in charge of storing data from the aforementioned Engine data structures into different custom file formats as well as loading any custom file format into their correspondent Engine data structure.

  * This sub-system is specially important as loading data from custom file formats is several magnitudes faster than importing it from asset files.

---

### Game Objects & Components

Game Objects are entities structured after the Entity Component Structure (ECS).

Game Objects can be assigned the following components:

* **Transform:** Allows to edit a GameObject's Position, Rotation & Scale.

* **Mesh:** Allows to assign a 3D Mesh to a GameObject. The mesh will be later rendered on-screen.

* **Material:** Allows to assign a base color as well as textures to a GameObject's mesh.

* **Camera:** Allows to assign a Frustum representing a Camera to a GameObject as well as to customize the Camera's base values (FOV, Frustum Culling...).

* **Animator:** Allows to assign Animations to a Mesh as well as generate clips with any of the aforementioned animations as the base.

---

### Resource Manager

The Resource Manager is the system in charge of managing all the resources that the engine has access to. This system also manages the resources that the different Game Objects (and their components) that exist within any given scene are using.

Resources are Data Structures that are used for storing the data from the different elements that are employed by the Engine. (Ex: .PNG image --> Texture Resource).

This system generates .meta files for each Asset File with references to the resources they are using. Moreover, when dragging an file that is already imported into the scene, the system will load the file through the custom file formats instead than through the asset file itself. 

Last but not least, the Resource Manager keeps constantly in check that no resource that is already loaded in memory is loaded again. 

---

### Engine GUI

The Engine features an Editor through which many of the Engine's facilities can be tweaked and modified. 

Some of the aforementioned facilities are: 

* **Configuration Panel:** Placed at the right side of the window. Allows to modify the settings of the Application's Core Modules such as the Renderer Module (Ex: Enable GL_DEPTH_TEST...).

* **Hierarchy Panel:** Placed at the left side of the window. It shows all the game objects that exist in the current scene. Upon clicking any of them the Inspector panel will show all the components the selected game object has.

* **Inspector Panel:** Placed at the right side of the window (along with Configuration). It shows the components of a selected game object and allows to modify many of the component's parameters.

* **Project Panel:** Placed at the bottom of the window. It shows all the assets inside the Assets folder and allows to drag any of the shown files into the scene.

  

The Engine's GUI has been implemented with the help of the Dear ImGui Library.

---

---

## Skeletal Animation Sub-System

NULL Engine features a Skeletal Animation System that allows to import any animation, segment it into clips and execute them either in Editor Mode (preview display) or Game Mode (In-Game Execution).

### Importing Animations

To import any animation the following process needs to be followed:

1. Drop the animation's .FBX or .OBJ file in the Assets Folder ("Assets/").
2. Run the engine and wait until the file has been imported.
3. Once in the Engine, access the location of the animation's file through the Project Panel.
4. Once the file is found, just drag and drop the file into the scene while in Editor Mode.

**Disclaimer:** This process can be applied to importing ANY file.

---

### Managing Animations Through the Editor

Once the animation has been loaded into scene, an Animator Component should have been created in the parent Game Object of the Model (first Game Object in the Model's Hierarchy). The Animator will have already loaded all of the animations that the Asset File contained.

This component is composed by:

* **Settings & Controls:** 
  * Allows to select an animation clip as well as Play, Pause, Step or Stop it. 
  * It also allows to modify some Animator Settings such as Playback Speed or Show Bones.
  * Last but not least, relevant data of the Current Clip as well as the animation it was created from is displayed.

* **Clip Manager:** 

  * Allows to create clips from any of the animations assigned to the Animator Component

  * To create a Clip the next process needs to be followed:

    1. Select the Base Animation

    2. Choose the Clip's name. (Disclaimer: All Clip names MUST be unique)

    3. Choose at which frame of the animation the clip starts.

    4. Choose at which frame of the animation the clip ends.

    5. Choose whether or not the Clip should be looped or not.

    6. Choose whether or not the Clip is interruptible.

    7. Click the "Create" button.

       

  * The Clip Manager also shows all the Clips that exist within the Animator Component.

---

### Executing Animations

**Editor Controls**

As mentioned above in the Settings & Controls sub-section, clip execution controls can be found in the Animator Component's Settings & Controls Tab.

* Select the Clip that will be executed and:
  * To Play the animation, just press the "Play" button.
  * To Pause the animation, just press the "Pause" button while the animation is Playing.
  * To Step the animation, just press the "Step" button while the animation is Paused.
  * To Stop the animation, just press the "Stop" button while the animation is Playing or Paused.

  ---

**In-Game Controls**

If an appropiate trigger has been set in code, then just enter Game Mode by pressing the "Play" button at the top left corner of the Application's window and trigger the animation to happen.

* Ex: `if (pressed_key == KP_2) { PlayClip("Attack"); }`

  ---

### Animator Component & Creating Clips Demonstration

**Animator Component:**
![Animator Component GIF](<Gifs/AnimatorComponent.gif>)

**Clip Manager:**
![Clip Manager GIF](<Gifs/ClipManager.gif>)
  ---

---

### In-Game Triggers Demonstration

![In-Game Triggers GIF](<Gifs/AnimationTriggers.gif>)

---

---

## Authoring Scenes with NULL Engine

* Engine Showcase: [![Engine Showcase](https://img.youtube.com/vi/lFkq9amrakY/0.jpg)](https://youtu.be/lFkq9amrakY)

---

---

## Engine Controls

### Camera Controls

**MOUSE**

* **Right Click:** Selects the Game Object under the mouse.
* **Left Alt + Right Click + Drag:** Rotates the camera around a target.
* **Wheel Click + Drag:** Pans the camera in the plane of the camera frame.
* **Mouse Wheel:** Zooms the camera in its Z axis.
* **Left Alt + Left Click**: Rotate around a reference point.

**KEYBOARD**

* **W Key:** Moves the camera forwards on it's X axis.

* **A Key:** Strafes the camera to the left on it's Z axis.

* **S Key:** Moves the camera backwards on it's X axis.

* **D Key:** Strafes the camera to the right on it's Z axis.

* **Q Key:** Moves the camera vertically up on its Y axis.

* **E Key:** Moves the camera vertically down on its Y axis.

* **SHIFT Key:** Duplicates the camera's movement speed.

* **O Key: ** Moves the camera back to the world origin.

* **KP PLUS:** Increses the FOV of the current camera by 5 degrees.

* **KP MINUS:** Decreases the FOV of the current camera by 5 degrees.

  

### Serialization Controls

* **Ctrl + S:** Serializes the current scene.

* **Ctrl + O:** Opens the "Load File" window, from which any file in "Assets/" can be loaded.

  

### Shortcuts

* **1 Key:** Enable/Disable the Configuration Panel.
* **2 Key:** Enable/Disable the Hierarchy Panel.
* **3 Key:** Enable/Disable the Inspector Panel.
* **4 Key:** Enable/Disable the Console Panel.
* **8 Key:** Enable/Disable the About Panel.
* **9 Key:** Enable/Disable the ImGui Demo Panels.
* **ESCAPE Key:** Enable/Disable the Close App Popup.

---

---

## Links

* Repository: [Click Here](<https://github.com/BarcinoLechiguino/NULL_Engine>) 
* Latest Release: [Click Here](<https://github.com/BarcinoLechiguino/NULL_Engine/releases/tag/v0.75>)

---

---

## License

MIT License

Copyright (c) 2021 [Angel Gonzalez T.]

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
