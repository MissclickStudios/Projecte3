## C++ Coding Style Conventions

Here it is a list with some of the conventions used in this project template:

Project Conventions:

Pushes:

- They should be regular, clean and concise.
- If there is a pushed feature that requires caution from others it should be explained in the push description.

Issues:

- Issues should be added when a possible bug or crash is noticed.
- The issue shall follow the template provided in Docs/Issues/issue_template.md.

Folders:

Folder Name | TitleCase | `Assets`
File Name | snake_case | `rock_01.fbx`



Code element | Convention | Example
--- | :---: | ---
Defines | ALL_CAPS | `#define PLATFORM_DESKTOP`
Macros | ALL_CAPS | `#define MIN(a,b) (((a)<(b))?(a):(b))`
Variables | lowerCase | `int screenWidth = 0;`
Local variables | lowerCase | `Vector2 playerPosition;`
Global variables | lowerCase | `bool fullscreen = false;`
Constants | lowerCase | `const int maxValue = 8`
Pointers | MyType* pointer | `Texture2D* array;`
float values | always x.xf | `float value = 10.0f`
Operators | value1 * value2 | `int product = value * 6;`
Operators | value1 / value2 | `int division = value / 4;`
Operators | value1 + value2 | `int sum = value + 10;`
Operators | value1 - value2 | `int res = value - 5;`
Enum | TitleCase | `TextureFormat`
Enum members | ALL_CAPS | `UNCOMPRESSED_R8G8B8`
Struct | TitleCase | `struct Texture2D`
Struct members |lowerCase | `texture.id`
Functions | TitleCase | `InitWindow()`
Class | TitleCase | `class Player`
Class fields | lowerCase | `Vector2 position`
Class methods | TitleCase | `GetPosition()`
Ternary Operator | (condition)? result1 : result2 | `printf("Value is 0: %s", (value == 0)? "yes" : "no");`

Identation:

 - Project uses aligned braces or curly brackets:
```c
void SomeFunction()
{
   // TODO: Do something here!
}
```

 - Project uses **TABS** instead of 4 spaces.

 - Trailing spaces MUST be avoided!