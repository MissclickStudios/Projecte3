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
VBO(0)
{
	text = "Some Text";
	rShader = App->resourceManager->GetShader("FontShader");
	GenerateTextureID();
	LoadBuffers();
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

	if (GetRect().w > canvas->GetRect().w)
		SetW(canvas->GetRect().w);

	if (GetRect().h > canvas->GetRect().h)
		SetH(canvas->GetRect().h);

	

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

	ParsonNode text = root.SetNode("Text");

	text.SetNumber("X", GetRect().x);
	text.SetNumber("Y", GetRect().y);
	text.SetNumber("W", GetRect().w);
	text.SetNumber("H", GetRect().h);

	return ret;
}

bool C_UI_Text::LoadState(ParsonNode& root)
{
	bool ret = true;

	ParsonNode text = root.GetNode("Text");

	Rect2D r;

	r.x = text.GetNumber("X");
	r.y = text.GetNumber("Y");
	r.w = text.GetNumber("W");
	r.h = text.GetNumber("H");

	SetRect(r);

	return ret;
}

Rect2D C_UI_Text::GetRect() const
{
	return rect;
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


void C_UI_Text::RenderText(std::string text, float x, float y, float scale, float3 color)
{
	/*glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);*/
	
	
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);



	glUseProgram(rShader->shaderProgramID);
	
	//float4x4 projection = float4x4::OrthographicProjectionXY();
	//float4x4 projection = float4x4::OrthographicProjection(App->camera->GetCurrentCamera()->GetFrustum().NearPlane());
	//float4x4 projection = float4x4::OpenGLOrthoProjRH(App->camera->GetCurrentCamera()->GetNearPlaneDistance(), App->camera->GetCurrentCamera()->GetFarPlaneDistance(), GetRect().w, GetRect().h);
	
	//float4x4 projection = App->camera->GetCurrentCamera()->GetFrustum().ProjectionMatrix().Transposed();

	C_Canvas* canvas = GetOwner()->parent->GetComponent<C_Canvas>();
	/*	float4x4 projection = canvas->GetOwner()->transform->GetWorldTransform().Transposed();*/

	x = canvas->GetPosition().x + GetRect().x - GetRect().w / 2;
	y = canvas->GetPosition().y + GetRect().y - GetRect().h / 2;

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);


	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = Characters[*c];

		glBindTexture(GL_TEXTURE_2D, ch.textureID);

		float4x4 projection = float4x4::FromTRS(float3(x, y, 0), Quat::FromEulerXYZ(0, 0, 0), float3(scale, scale, 1)).Transposed();
		

		rShader->SetUniformVec3f("textColor", (GLfloat*)&color);
		rShader->SetUniformMatrix4("projection", projection.ptr());


		float xpos = (x + ch.Bearing.x) * scale;
		float ypos = (y - (ch.Size.y - ch.Bearing.y)) * scale;

		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;

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


		x += (ch.Advance >> 6) * scale;
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

	/*if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
	{
		LOG("ERROR::FREETYTPE: Failed to load Glyph");
		return ;
	}*/
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

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
		// now store character for later use
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
