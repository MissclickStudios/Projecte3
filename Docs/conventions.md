# C++ Coding Style Conventions

Here it is a list with some of the conventions used in this project:

## Project Conventions:

### Pushes:

- They should be regular, clean and concise.
- If there is a pushed feature that requires caution from others it should be explained in the push description.
- Users should caution whenever they are going to work on or modify a file that is not expected of them to change.

### Issues:

- For bug/crash reporting GitHub issues will be used, follow the guidelines defined by Docs/Issues/issues_guidelines.md .

### Folders:

Folder Name | TitleCase | `Assets`
Assets File Name | snake_case | `rock_01.fbx`

Script Naming | Class title

---

---

## Code Conventions


Code element | Convention | Example
--- | :---: | ---
Defines | ALL_CAPS | `#define PLATFORM_DESKTOP`
Macros | ALL_CAPS | `#define MIN(a,b) (((a)<(b))?(a):(b))`
Variables | camelCase | `int screenWidth = 0;`
Local variables | camelCase | `Vector2 playerPosition;`
Global variables | camelCase | `bool fullscreen = false;`
Constants | camelCase | `const int maxValue = 8`
Pointers | MyType* pointer | `Texture2D* array;`
float values | always x.xf | `float value = 10.0f`
Operators | value1 * value2 | `int product = value * 6;`
Operators | value1 / value2 | `int division = value / 4;`
Operators | value1 + value2 | `int sum = value + 10;`
Operators | value1 - value2 | `int sub = value - 5;` 
Enum Class | TitleCase | `TextureFormat`
Enum members | ALL_CAPS | `UNCOMPRESSED_R8G8B8`
Struct | TitleCase | `struct Texture2D`
Struct members | camelCase | `texture.id`
Functions | TitleCase | `InitWindow()`
Class | TitleCase | `class Player`
Class fields | camelCase | `Vector2 position`
Class methods | TitleCase | `GetPosition()`
Ternary Operator | (condition)? result1 : result2 | `printf("Value is 0: %s", (value == 0)? "yes" : "no");`



### Naming Exceptions

 - Variable names that contain acronyms will have said acronyms in all caps:

  ```c++
  uint UID = 0;
  int maxFPS = 60;
  ```

  

 - Children of main classes will be named with X_ after the TitleCase name. X Being the first letter of the parent class.

  ```c++
  class Component
  {
  public:
  	// Methods
  	
  public:
  	// Variables
  }
  
  class C_Transform : public Component
  {
  public:
  	// Methods
  	
  public:
  	// Variables
  }
  ```



### Indentation:

 - Project uses aligned braces or curly brackets:

   ```c++
   void SomeFunction()
   {
   	// TODO: Do something here!
   }
   ```
 - if() statements with only one line of code:
```c++
if(a >= 1)
    // One-liner code
```

 - Switch() statements with only one line of code should not use enters and will look like this:
```c++
switch (type)
{
    case RESOURCE_TYPE::MODEL:		success = Importer::Scenes::Load(buffer, (R_Model*)resource);  		break;
    case RESOURCE_TYPE::MESH:		success = Importer::Meshes::Load(buffer, (R_Mesh*)resource);		break;
    case RESOURCE_TYPE::MATERIAL:	success = Importer::Materials::Load(buffer, (R_Material*)resource);	break;
}
```



### Others:

 - Project uses **TABS** instead of 4 spaces.
 - Trailing spaces MUST be avoided!