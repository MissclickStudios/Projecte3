#include "Application.h"

#include "GameObject.h"

#include "M_Window.h"
#include "M_Camera3D.h"
#include "M_Editor.h"
#include "M_Scene.h"
#include "M_ResourceManager.h"

#include "C_Material.h"
#include "C_Canvas.h"
#include "C_Transform.h"
#include "C_Camera.h"

#include "R_Shader.h"

#include "E_Viewport.h"

#include "C_UI_Text.h"



#include "MemoryManager.h"

#include "JSONParser.h"

#include "Dependencies/FreeType/include/ft2build.h"
#include "Dependencies/FreeType/include/freetype/freetype.h"

#pragma comment (lib, "Source/Dependencies/FreeType/win32/freetype.lib")

C_UI_Text::C_UI_Text(GameObject* owner, Rect2D rect) : Component(owner, ComponentType::UI_TEXT),
VAO(0),
VBO(0),
color(1,1,1,1)
{
	text = "Some Text";
	rShader = App->resourceManager->GetShader("FontShader");
	GenerateTextureID();
	LoadBuffers();

	SetW(0.001);
	SetH(0.002);
	SetX(0);
	SetY(0);
}

C_UI_Text::~C_UI_Text()
{

}

bool C_UI_Text::Update()
{
	bool ret = true;

	if (IsActive() == false)
		return ret;

	C_Canvas* canvas = GetOwner()->parent->GetComponent<C_Canvas>();
	if (canvas == nullptr)
		return ret;	

	return ret;
}

bool C_UI_Text::CleanUp()
{
	bool ret = true;

	return ret;
}

void C_UI_Text::LoadBuffers()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}


bool C_UI_Text::SaveState(ParsonNode& root) const
{
	bool ret = true;

	root.SetNumber("Type", (uint)GetType());

	ParsonNode textNode = root.SetNode("Text");

	textNode.SetString("Text", text.c_str());
	float4 newColor = { color.r, color.g, color.b, color.a };
	textNode.SetFloat4("Color", newColor);
	textNode.SetNumber("X", GetRect().x);
	textNode.SetNumber("Y", GetRect().y);
	textNode.SetNumber("W", GetRect().w);
	textNode.SetNumber("H", GetRect().h);

	return ret;
}

bool C_UI_Text::LoadState(ParsonNode& root)
{
	bool ret = true;

	ParsonNode textNode = root.GetNode("Text");
	text = textNode.GetString("Text");
	float4 newColor = (textNode.GetFloat4("Color"));
	color = Color(newColor.x, newColor.y, newColor.z, newColor.w);
	Rect2D r;
	r.x = textNode.GetNumber("X");
	r.y = textNode.GetNumber("Y");
	r.w = textNode.GetNumber("W");
	r.h = textNode.GetNumber("H");
	SetRect(r);

	return ret;
}

Rect2D C_UI_Text::GetRect() const
{
	return rect;
}
Color C_UI_Text::GetColor() const
{
	return color;
}
const char* C_UI_Text::GetText() const
{
	return text.c_str();
}
void C_UI_Text::SetText(const char* text)
{
	this->text = text;
}
void C_UI_Text::SetColor(Color color)
{
	this->color = color;
}
void C_UI_Text::SetRect(const Rect2D& rect)
{
	this->rect = rect;
}

void C_UI_Text::SetX(const float x)
{
	this->rect.x = x;
}

void C_UI_Text::SetY(const float y)
{
	this->rect.y = y;
}

void C_UI_Text::SetW(const float w)
{
	this->rect.w = w;
}

void C_UI_Text::SetH(const float h)
{
	this->rect.h = h;
}


void C_UI_Text::RenderText( )
{
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(rShader->shaderProgramID);

	C_Canvas* canvas = GetOwner()->parent->GetComponent<C_Canvas>();

	float x = canvas->GetPosition().x + GetRect().x;
	float y = canvas->GetPosition().y + GetRect().y;

	float4x4 projectionMatrix = float4x4::FromTRS(float3(x, y, 0), Quat::FromEulerXYZ(0, 0, 0), float3(GetRect().w, GetRect().h, 1)).Transposed();


	rShader->SetUniformVec4f("textColor", (GLfloat*)&color);
	rShader->SetUniformMatrix4("projection", projectionMatrix.ptr());

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);


	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = Characters[*c];

		glBindTexture(GL_TEXTURE_2D, ch.textureID);

		float xpos = (x + ch.bearing.x) ;
		float ypos = (y - (ch.size.y - ch.bearing.y)) ;

		float w = ch.size.x ;
		float h = ch.size.y ;

		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

		glDrawArrays(GL_TRIANGLES, 0, 6);


		x += (ch.advance >> 6) ;
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
}

void C_UI_Text::GenerateTextureID()
{
	if (FT_Init_FreeType(&ft))
	{
		LOG("ERROR::FREETYPE: Could not init FreeType Library");
		return;
	}
	if (FT_New_Face(ft, "Assets/Fonts/arial.ttf", 0, &face))
	{
		LOG("ERROR::FREETYPE: Failed to load font");
		return;
	}
	
	FT_Set_Pixel_Sizes(face, 0, 48);

	if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
	{
		LOG("ERROR::FREETYTPE: Failed to load Glyph");
		return ;
	}
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	for (unsigned char c = 0; c < 128; c++)
	{
		// load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			LOG("ERROR::FREETYTPE: Failed to load Glyph");
			continue;
		}
		// generate texture
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		// set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		Character character = {
			texture,
			float2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			float2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};
		Characters.insert(std::pair<char, Character>(c, character));
	}

	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}
