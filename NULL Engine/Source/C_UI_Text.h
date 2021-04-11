#ifndef __C_UI_TEXT_H__
#define __C_UI_TEXT_H__

#include <string>

#include "M_UISystem.h"
#include "OpenGL.h"
#include "Color.h"
#include "Component.h"

struct FT_FaceRec_;
typedef FT_FaceRec_* FT_Face;
struct FT_LibraryRec_;
typedef FT_LibraryRec_* FT_Library;

class R_Shader;

struct Character 
{
	uint	textureID;
	float2	size;		
	float2	bearing;
	uint	advance;	
};

class NULL_API C_UI_Text : public Component
{
public:
	C_UI_Text(GameObject* owner, Rect2D rect = { 0,0,50,50 });
	~C_UI_Text();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	static inline ComponentType GetType() { return ComponentType::UI_TEXT; }

public:
	void LoadBuffers();

	//void Draw2D();
	//void Draw2DCharacter(FT_Face id, float x, float y);
	//void Draw3D();

	void RenderText();

	void GenerateTextureID();

public:
	Rect2D GetRect() const;
	Color GetColor() const;
	const char* GetText() const;

	void SetText(const char* text);
	void SetColor(Color color);
	void SetRect(const Rect2D& rect);
	void SetX(const float x);
	void SetY(const float y);
	void SetW(const float w);
	void SetH(const float h);

	

private:

	Rect2D rect = { 0,0,50,50 };

	std::string text; 

	unsigned char image[640][480];

	Color color;

	FT_Face face;

	FT_Library ft;

	std::map<char, Character> Characters;

	R_Shader* rShader;

	uint VAO;
	uint VBO;

};

#endif // !__C_UI_TEXT_H__







