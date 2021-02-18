# Technical Design Document
In this document all the technical specifications related with the project will be laid out. 

This project will be developed in C++.

## 1.Technical Goals

All the main technical goals that have been set for this project will be stated below:

- Usable Game Engine
- 3D Isometric World
- Procedural level generation



### Target Platform
The target platform that has been set for this project will have the following specifications:

|     Hardware    |         Requirements        |  Minimum Requirements  |
| --------------- | --------------------------- | ---------------------- |
|       OS        | Windows 10 *version*        | Windows 7 Professional |
|       CPU       | Intel Core i7-9750H 2.6Hz   | Intel Core i3          |
|       GPU       | NVIDIA GeForce RTX 2060 6GB | Integrated             |
|       RAM       | 16GB                        | 100 MB                 |
| Free Disk Space | ???                         | 200 MB                 |

- Must correctly run in the university computers.



### Performance Budget
The final build must run at 60fps in the target platform described above.
The base screen resolution will be 1280x720px. Moreover Fullscreen mode, as well as Windowed mode, must be supported.




## 2. Development Hardware
{Minimum requirements of the hardware used to develop the project}
Hardware Used:
- PC windows



## 3. Development Software
The following programs will be employed in order to correctly develop the project:

**Code related software:**

- git

- GitHub Desktop
- Visual Studio 2019
- Wwise
- Brofiler




### External Libraries
All the external libraries that will be employed during the project's development will be shown down below with the version being used:

- [OpenGL](https://www.opengl.org//) v.X.X
- [MathGeoLib](https://github.com/juj/MathGeoLib) v.X.X
- [Parson](https://github.com/kgabis/parson) v.X.X
- [Assimp](https://www.assimp.org/) v.X.X
- [DevIL](http://openil.sourceforge.net/) v.X.X
- [glew](http://glew.sourceforge.net/) v.X.X
- [ImGui](https://github.com/ocornut/imgui) v.X.X
- [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo) v.X.X
- [PhysFS](https://icculus.org/physfs/) v.X.X
- [PhysX](https://github.com/NVIDIAGameWorks/PhysX) v.X.X
- [Wwise](https://www.audiokinetic.com/products/wwise/) v.X.X
- [mmgr](https://github.com/RIscRIpt/mmgr) v.X.X



## 4. Desired Technical Features



## 5. Branching Policy
- Branches shall be used to maintain a healthy parallel development. Merges will be made on the weekly meetings when close to a delivery or when a feature is properly or fully implemented.



## 5. Repository Policy

- There will exist two different repositories to develop this project. One will hold the engine's development and the other one will hold the game's development with the executable engine.
- Issues will be posted on the engine's Issues page, a label will qualify if the issue is regarding the engine or the game itself.



## 6. Coding Conventions





## 7. Data Layout
There are two main folders holding files for our project:

### 1. Docs

Holds all files used for the GitHub wiki. This includes .md files for text and images used in the wiki.

There are various folders used for images to have them better organized.

### 2. Engine Folder

Holds the full code structure as well as necessary Visual Studio files. This mainly holds .dll files

#### 	2.1 Resources

​		This is the folder the code uses as working directory. Inside there are folders holding the Assets, the Library and some Assets used by the engine.

#### 	2.2 Source

​		Holds the source code files. Inside there is the Dependencies folder which holds the various library files.



## 8. Scheduling

Delivery dates with it's respective expected features:

- **Concept Discovery:** February 23th

- **Vertical Slice 1:**  March 9th 

- **Vertical Slice 2:**  March 23th

- **Alpha 1:**  April 13th

- **Alpha 2:**  April 27th

- **Alpha 3:**  May 11th

- **Beta:** May 25th

- **Gold:** June 7th to 16th 



## 9. Build Delivery Method
- Release through Github




## 10. Version Control
- {List containing all the current and previous versions and should detail what was implemented in each one of them}




## Bibliography:
- [Final TDD Reference](https://github.com/DevCrumbs/Warcraft-II/wiki/7.-Tech-Design-Document)
- [Enric's TDD Research](https://github.com/EnricGDV/Research-TDD)

