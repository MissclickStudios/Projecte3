#ifndef __C_UI_TEXT_H__
#define __C_UI_TEXT_H__

#include <string>
#include "Macros.h"

#include "C_UI.h"
#include "Color.h"
#include <map>

struct FT_FaceRec_;
typedef FT_FaceRec_* FT_Face;
struct FT_LibraryRec_;
typedef FT_LibraryRec_* FT_Library;

class R_Shader;

struct Character 
{
	unsigned int textureID;
	float2	size;		
	float2	bearing;
	unsigned int advance;
};

class MISSCLICK_API C_UI_Text : public C_UI
{
public:
	C_UI_Text(GameObject* owner, Rect2D rect = { 0,0,0.001,0.001 });
	~C_UI_Text();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	static inline ComponentType GetType() { return ComponentType::UI_TEXT; }

public:

	void LoadBuffers();
	void HandleInput(C_UI** selectedUi)override {}
	void Draw2D()override;
	void Draw3D()override {} //TODO: 3dText !!??
	void GenerateTextureID(std::string fontName);

public:

	Color GetColor() const;
	const char* GetText() const;
	float GetFontSize() const;
	uint GetHSpaceBetween() const;
	float GetVSpaceBetween() const;

	void SetText(const char* text);
	void SetColor(Color color);
	void SetFontSize(const float fontSize);
	void SetHSpaceBetween(const uint hSpaceBetween);
	void SetVSpaceBetween(const float vSpaceBetween);
	
private:

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