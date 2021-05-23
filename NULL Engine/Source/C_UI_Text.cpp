#include "Application.h"

#include "GameObject.h"

#include "M_Window.h"
#include "M_Camera3D.h"
#include "M_Scene.h"
#include "M_ResourceManager.h"

#include "Dependencies/glew/include/glew.h"
//#include "OpenGL.h"

#include "C_Material.h"
#include "C_Canvas.h"
#include "C_Transform.h"
#include "C_Camera.h"
#include "C_UI_Text.h"

#include "R_Shader.h"

#include "MemoryManager.h"

#include "JSONParser.h"

#include "Dependencies/FreeType/include/ft2build.h"
#include "Dependencies/FreeType/include/freetype/freetype.h"

#pragma comment (lib, "Source/Dependencies/FreeType/win32/freetype.lib")

C_UI_Text::C_UI_Text(GameObject* owner, Rect2D rect) : C_UI(owner, ComponentType::UI_TEXT,false, rect),
VAO(0),
VBO(0),
color(1,0,0,1)
{
	text = "Some Text";
	rShader = App->resourceManager->GetShader("FontShader");
	GenerateTextureID("Assets/Fonts/in_game.ttf");
	LoadBuffers();

}

C_UI_Text::~C_UI_Text()
{
	GameObject* parent = GetOwner()->parent;
	if (parent)
	{
		C_Canvas* canvas = parent->GetComponent<C_Canvas>();
		if (canvas)
			canvas->RemoveUiElement(this);
	}
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
	root.SetNumber("Type", (uint)GetType());
	root.SetString("Text", text.c_str());
	root.SetFloat4("Color", { color.r, color.g, color.b, color.a });
	root.SetNumber("X", rect.x);
	root.SetNumber("Y", rect.y);
	root.SetNumber("W", rect.w);
	root.SetNumber("H", rect.h);

	root.SetInteger("childOrder", childOrder);
	return true;
}

bool C_UI_Text::LoadState(ParsonNode& root)
{
	/*ParsonNode node = root.GetNode("text");
	text = node.GetString("Text");
	float4 newColor = (node.GetFloat4("Color"));
	color = Color(newColor.x, newColor.y, newColor.z, newColor.w);
	rect.x = node.GetNumber("X");
	rect.y = node.GetNumber("Y");
	rect.w = node.GetNumber("W");
	rect.h = node.GetNumber("H");*/

	text = root.GetString("Text");
	float4 newColor = (root.GetFloat4("Color"));
	color = Color(newColor.x, newColor.y, newColor.z, newColor.w);
	rect.x = root.GetNumber("X");
	rect.y = root.GetNumber("Y");
	rect.w = root.GetNumber("W");
	rect.h = root.GetNumber("H");

	childOrder = root.GetInteger("childOrder");

	return true;
}

Color C_UI_Text::GetColor() const
{
	return color;
}

const char* C_UI_Text::GetText() const
{
	return text.c_str();
}

float C_UI_Text::GetFontSize() const
{
	return fontSize;
}

uint C_UI_Text::GetHSpaceBetween() const
{
	return hSpaceBetween;
}

float C_UI_Text::GetVSpaceBetween() const
{
	return vSpaceBetween;
}


void C_UI_Text::SetText(const char* text)
{
	this->text = text;
}

void C_UI_Text::SetColor(Color color)
{
	this->color = color;
}

void C_UI_Text::SetFontSize(const float fontSize)
{
	this->fontSize = fontSize;
}

void C_UI_Text::SetHSpaceBetween(const uint hSpaceBetween)
{
	this->hSpaceBetween = hSpaceBetween;
}

void C_UI_Text::SetVSpaceBetween(const float vSpaceBetween)
{
	this->vSpaceBetween = vSpaceBetween;
}


void C_UI_Text::Draw2D( )
{
	glEnable(GL_CULL_FACE);
	//glEnable(GL_BLEND); enabled in draw 2d render ui
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(rShader->shaderProgramID);

	C_Canvas* canvas = GetOwner()->parent->GetComponent<C_Canvas>();

	float x = canvas->GetPosition().x + GetRect().x;
	float y = canvas->GetPosition().y + GetRect().y;

	float4x4 projectionMatrix = float4x4::FromTRS(float3(x, y, 0), Quat::FromEulerXYZ(0, 0, 0), float3(fontSize, fontSize, 1)).Transposed();


	rShader->SetUniformVec4f("textColor", (GLfloat*)&color);
	rShader->SetUniformMatrix4("projection", projectionMatrix.ptr());

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	int i = 0;
	uint it = 0;
	bool rowHead = true;
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); ++c)
	{
		Character ch = Characters[*c];

		glBindTexture(GL_TEXTURE_2D, ch.textureID);

		float xpos = (x + ch.bearing.x);
		float ypos = (y - (ch.size.y - ch.bearing.y));

		float w = ch.size.x;
		float h = ch.size.y;

		ypos = ypos - vSpaceBetween * 5 * i;

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

		if (x > rect.w * 15000)
		{
			rowHead = true;
			x = 0;
			i++;
		}
		else
		{
			char j = text.at(it);
			std::string converter = " ";
			char k = converter.at(0);
			if (j == k && rowHead)
			{
				// I know it's weirde but I can't think of another way to do it :/
			}
			else
			{
				float adv = ch.advance + hSpaceBetween * 10;
				x += ((uint)adv >> 6);
			}

			rowHead = false;
		}
	

		++it;
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
}

void C_UI_Text::GenerateTextureID(std::string fontName)
{
	if (FT_Init_FreeType(&ft))
	{
		LOG("ERROR::FREETYPE: Could not init FreeType Library");
		return;
	}
	if (FT_New_Face(ft, fontName.c_str(), 0, &face))
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

	Characters.clear();
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
