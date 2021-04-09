#ifndef __C_UI_TEXT_H__
#define __C_UI_TEXT_H__

#include <string>

#include "M_UISystem.h"
#include "OpenGL.h"

#include "Component.h"

class R_Shader;

struct Character 
{
	uint	textureID;		// ID handle of the glyph texture
	float2	size;			// Size of glyph
	float2	bearing;		// Offset from baseline to left/top of glyph
	uint	advance;		// Offset to advance to next glyph
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

	void RenderText(std::string text, float x, float y, float scale, float3 color);

	void GenerateTextureID();

public:
	Rect2D GetRect() const;

	void SetRect(const Rect2D& rect);
	void SetX(const float x);
	void SetY(const float y);
	void SetW(const float w);
	void SetH(const float h);

	std::string text; // temp here

private:

	Rect2D rect = { 0,0,50,50 };

	unsigned char image[640][480];

	FT_UInt  glyphIndex;

	FT_Face face;

	FT_Library ft;

	std::map<char, Character> Characters;

	R_Shader* rShader;

	uint VAO;
	uint VBO;

};

#endif // !__C_UI_TEXT_H__







