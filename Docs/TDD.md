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

- [SDL](https://www.sdl.com/es/) v.2.0
- [OpenGL](https://www.opengl.org//) v4.6
- [MathGeoLib](https://github.com/juj/MathGeoLib) v1.5
- [Parson](https://github.com/kgabis/parson) v1.1.0
- [Assimp](https://www.assimp.org/) v3.1.1
- [DevIL](http://openil.sourceforge.net/) v1.8.0
- [glew](http://glew.sourceforge.net/) v2.1.0
- [ImGui](https://github.com/ocornut/imgui) v1.79
- [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo) v1.61
- [PhysFS](https://icculus.org/physfs/) v3.0.2
- [PhysX](https://github.com/NVIDIAGameWorks/PhysX) v.X.X
- [Wwise](https://www.audiokinetic.com/products/wwise/) v.2019.2.9
- [mmgr](https://github.com/RIscRIpt/mmgr) Unknown


## 4. Desired Technical Features







## 5. Branching Policy

### Use

- Branches shall be used to maintain a healthy parallel and continuous development. Merges will be made on the weekly meetings, when close to a delivery or when a feature is properly or fully implemented in its respective branch.

### Merges

- Branch merge should only be committed once the code has been tested and is working as intended. The programmer responsible of developing the branch will be the one committing the merge, the Programming Lead can also take its place. If it's the programmer doing it then it preferred that the Programming Lead is supervising the operation.

### Branch layout

- **Main Branch** will be kept for merging once the engine has reached a new stable version with major completed features.

- **Development Branch** is the branch which will be the one the other branches will merge. It will be subject to the new features putting them together. Once there is a stable version it will be able to be merged into main.

- A part from the previous branches other ones will be created to develop new major features to the engine. They will be merged into development once it is

## 6. Repository Policy

- There will exist two different repositories to develop this project. One will hold the engine's development and the other one will hold the game's development with the executable engine.
- Issues will be posted on the engine's Issues page, a label will qualify if the issue is regarding the engine or the game itself.



## 7. Coding Conventions



## 8. Data Layout

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



## 9. Scheduling

Delivery dates with it's respective expected features:

- **Concept Discovery:** February 23th

Get a perspective on the full project and the desired features, 

- **Vertical Slice 1:**  March 9th

Have most of the game features in a very early development state.

- **Vertical Slice 2:**  March 23th

Have all the game features in a very early development state.

- **Alpha 1:**  April 13th

- **Alpha 2:**  April 27th

- **Alpha 3:**  May 11th

- **Beta:** May 25th

- **Gold:** June 7th to 16th 



## 10. Build Delivery Method

- Release through Github

## 11. Version Control

- {List containing all the current and previous versions and should detail what was implemented in each one of them}

## Bibliography:

- [Final TDD Reference](https://github.com/DevCrumbs/Warcraft-II/wiki/7.-Tech-Design-Document)
- [Enric's TDD Research](https://github.com/EnricGDV/Research-TDD)

