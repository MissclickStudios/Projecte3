#ifndef __C_UI_TEXT_H__
#define __C_UI_TEXT_H__

#include <string>
#include "Macros.h"

#include "M_UISystem.h"
#include "Color.h"
#include "Component.h"
#include <map>

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

class MISSCLICK_API C_UI_Text : public Component
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
	void RenderText();
	void GenerateTextureID(std::string fontName);

public:

	Rect2D GetRect() const;
	Color GetColor() const;
	const char* GetText() const;
	float GetFontSize() const;
	uint GetHSpaceBetween() const;
	float GetVSpaceBetween() const;

	void SetText(const char* text);
	void SetColor(Color color);
	void SetRect(const Rect2D& rect);
	void SetX(const float x);
	void SetY(const float y);
	void SetW(const float w);
	void SetH(const float h);
	void SetFontSize(const float fontSize);
	void SetHSpaceBetween(const uint hSpaceBetween);
	void SetVSpaceBetween(const float vSpaceBetween);
	
private:

	Rect2D rect = { 0,0,0.001,0.001 };

	std::string text; 

	unsigned char image[640][480];

	Color color;

	FT_Face face;

	FT_Library ft;

	std::map<char, Character> Characters;

	R_Shader* rShader;

	uint VAO;
	uint VBO;

	float fontSize = 0.001;
	uint hSpaceBetween = 0; // extra space between letters
	float vSpaceBetween = 15; // space between rows

};

#endif // !__C_UI_TEXT_H__