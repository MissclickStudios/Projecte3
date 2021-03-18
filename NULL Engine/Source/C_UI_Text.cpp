#include "Application.h"

#include "GameObject.h"

#include "M_Camera3D.h"
#include "M_Editor.h"
#include "M_Scene.h"

#include "C_Material.h"
#include "C_Canvas.h"
#include "C_Transform.h"
#include "C_Camera.h"

#include "E_Viewport.h"

#include "C_UI_Text.h"


#include "MemoryManager.h"

#include "JSONParser.h"

C_UI_Text::C_UI_Text(GameObject* owner, Rect2D rect) : Component(owner, ComponentType::UI_TEXT)
{
	text = "H";
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

void C_UI_Text::Draw2D()
{
	FT_Face face = App->uiSystem->standardFace;

	int bbox_ymax = face->bbox.yMax / 64;
	int glyph_width = face->glyph->metrics.width / 64;
	int advance = face->glyph->metrics.horiAdvance / 64;
	int x_off = (advance - glyph_width) / 2;
	int y_off = bbox_ymax - face->glyph->metrics.horiBearingY / 64;

	FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

	// x and y are the coordinates of the current drawing origin, starting
	//  at the top-left of the bounding box.
	int x = rect.x;
	int y = rect.y;
	for (int h = 0; h < (int)face->glyph->bitmap.rows; h++)
	{
		// row_offset is the distance from the top of the framebuffer
		//   of the text bounding box
		int row_offset = y + h + y_off;
		for (int w = 0; w < (int)face->glyph->bitmap.width; w++)
		{
			unsigned char p = face->glyph->bitmap.buffer[h * face->glyph->bitmap.pitch + w];

			// Don't draw a zero value, unless you want to fill the bounding
			//   box with black.
			if (p)
				Draw2DCharacter(face, x + w + x_off, row_offset);
		}
	}
	// Move the x position, ready for the next character.
	x += advance;


}

void C_UI_Text::Draw2DCharacter(FT_Face id, float x, float y)
{
	C_Canvas* canvas = GetOwner()->parent->GetComponent<C_Canvas>();
	if (canvas == nullptr) return;

	glPushMatrix();
	glMultMatrixf((GLfloat*)&GetOwner()->parent->GetComponent<C_Transform>()->GetWorldTransform().Transposed());

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	glBindTexture(GL_TEXTURE_2D, (GLuint)id->glyph);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex2f(canvas->GetPosition().x + GetRect().x - GetRect().w / 2, canvas->GetPosition().y + GetRect().y - GetRect().h / 2);
	glTexCoord2f(1, 0); glVertex2f(canvas->GetPosition().x + GetRect().x + GetRect().w / 2, canvas->GetPosition().y + GetRect().y - GetRect().h / 2);
	glTexCoord2f(1, 1); glVertex2f(canvas->GetPosition().x + GetRect().x + GetRect().w / 2, canvas->GetPosition().y + GetRect().y + GetRect().h / 2);
	glTexCoord2f(0, 1); glVertex2f(canvas->GetPosition().x + GetRect().x - GetRect().w / 2, canvas->GetPosition().y + GetRect().y + GetRect().h / 2);
	glEnd();

	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, 0);
}

void C_UI_Text::Draw3D()
{
	if (GetOwner()->GetComponent<C_Material>() == nullptr) return;

	glPushMatrix();
	glMultMatrixf((GLfloat*)&GetOwner()->GetComponent<C_Transform>()->GetWorldTransform().Transposed());

	glEnable(GL_BLEND);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	uint32 id = GetOwner()->GetComponent<C_Material>()->GetTextureID();
	glBindTexture(GL_TEXTURE_2D, id); // Not sure
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex2f(GetRect().x - GetRect().w / 2, GetRect().y - GetRect().h / 2);
	glTexCoord2f(1, 0); glVertex2f(GetRect().x + GetRect().w / 2, GetRect().y - GetRect().h / 2);
	glTexCoord2f(1, 1); glVertex2f(GetRect().x + GetRect().w / 2, GetRect().y + GetRect().h / 2);
	glTexCoord2f(0, 1); glVertex2f(GetRect().x - GetRect().w / 2, GetRect().y + GetRect().h / 2);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_BLEND);

	glPopMatrix();

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
	//// activate corresponding render state	
	//glUniform3f(glGetUniformLocation(s.Program, "textColor"), color.x, color.y, color.z);
	//glActiveTexture(GL_TEXTURE0);
	//glBindVertexArray(VAO);

	//// iterate through all characters
	//std::string::const_iterator c;
	//for (c = text.begin(); c != text.end(); c++)
	//{
	//	Character ch = Characters[*c];

	//	float xpos = x + ch.Bearing.x * scale;
	//	float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

	//	float w = ch.Size.x * scale;
	//	float h = ch.Size.y * scale;
	//	// update VBO for each character
	//	float vertices[6][4] = {
	//		{ xpos,     ypos + h,   0.0f, 0.0f },
	//		{ xpos,     ypos,       0.0f, 1.0f },
	//		{ xpos + w, ypos,       1.0f, 1.0f },

	//		{ xpos,     ypos + h,   0.0f, 0.0f },
	//		{ xpos + w, ypos,       1.0f, 1.0f },
	//		{ xpos + w, ypos + h,   1.0f, 0.0f }
	//	};
	//	// render glyph texture over quad
	//	glBindTexture(GL_TEXTURE_2D, ch.textureID);
	//	// update content of VBO memory
	//	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	//	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//	// render quad
	//	glDrawArrays(GL_TRIANGLES, 0, 6);
	//	// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
	//	x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
	//}
	//glBindVertexArray(0);
	//glBindTexture(GL_TEXTURE_2D, 0);
}

//
//namespace freetype {
//
//	// This Function Gets The First Power Of 2 >= The
//	// Int That We Pass It.
//	inline int next_p2(int a)
//	{
//		int rval = 1;
//		// rval<<=1 Is A Prettier Way Of Writing rval*=2;
//		while (rval < a) rval <<= 1;
//		return rval;
//	}
//
//	// Create A Display List Corresponding To The Given Character.
//	void make_dlist(FT_Face face, char ch, GLuint list_base, GLuint* tex_base) 
//	{
//
//		// The First Thing We Do Is Get FreeType To Render Our Character
//		// Into A Bitmap.  This Actually Requires A Couple Of FreeType Commands:
//
//		// Load The Glyph For Our Character.
//		if (FT_Load_Glyph(face, FT_Get_Char_Index(face, ch), FT_LOAD_DEFAULT))
//			throw std::runtime_error("FT_Load_Glyph failed");
//
//		// Move The Face's Glyph Into A Glyph Object.
//		FT_GlyphSlot glyph;
//		if (FT_Get_Glyph(face->glyph, &glyph))
//			throw std::runtime_error("FT_Get_Glyph failed");
//
//		// Convert The Glyph To A Bitmap.
//		FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, 0, 1);
//		FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;
//
//		// This Reference Will Make Accessing The Bitmap Easier.
//		FT_Bitmap& bitmap = bitmap_glyph->bitmap;
//
//		// Use Our Helper Function To Get The Widths Of
//		// The Bitmap Data That We Will Need In Order To Create
//		// Our Texture.
//		int width = next_p2(bitmap.width);
//		int height = next_p2(bitmap.rows);
//
//		// Allocate Memory For The Texture Data.
//		GLubyte* expanded_data = new GLubyte[2 * width * height];
//
//		// Here We Fill In The Data For The Expanded Bitmap.
//		// Notice That We Are Using A Two Channel Bitmap (One For
//		// Channel Luminosity And One For Alpha), But We Assign
//		// Both Luminosity And Alpha To The Value That We
//		// Find In The FreeType Bitmap.
//		// We Use The ?: Operator To Say That Value Which We Use
//		// Will Be 0 If We Are In The Padding Zone, And Whatever
//		// Is The FreeType Bitmap Otherwise.
//		for (int j = 0; j < height; j++) 
//		{
//			for (int i = 0; i < width; i++) 
//			{
//				expanded_data[2 * (i + j * width)] = expanded_data[2 * (i + j * width) + 1] =
//					(i >= bitmap.width || j >= bitmap.rows) ?
//					0 : bitmap.buffer[i + bitmap.width * j];
//			}
//		}
//
//		// Now We Just Setup Some Texture Parameters.
//		glBindTexture(GL_TEXTURE_2D, tex_base[ch]);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//
//		// Here We Actually Create The Texture Itself, Notice
//		// That We Are Using GL_LUMINANCE_ALPHA To Indicate That
//		// We Are Using 2 Channel Data.
//		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
//			GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, expanded_data);
//
//		// With The Texture Created, We Don't Need The Expanded Data Anymore.
//		delete[] expanded_data;
//
//		// Now We Create The Display List
//		glNewList(list_base + ch, GL_COMPILE);
//
//		glBindTexture(GL_TEXTURE_2D, tex_base[ch]);
//
//		glPushMatrix();
//
//		// First We Need To Move Over A Little So That
//		// The Character Has The Right Amount Of Space
//		// Between It And The One Before It.
//		glTranslatef(bitmap_glyph->left, 0, 0);
//
//		// Now We Move Down A Little In The Case That The
//		// Bitmap Extends Past The Bottom Of The Line
//		// This Is Only True For Characters Like 'g' Or 'y'.
//		glTranslatef(0, bitmap_glyph->top - bitmap.rows, 0);
//
//		// Now We Need To Account For The Fact That Many Of
//		// Our Textures Are Filled With Empty Padding Space.
//		// We Figure What Portion Of The Texture Is Used By
//		// The Actual Character And Store That Information In
//		// The x And y Variables, Then When We Draw The
//		// Quad, We Will Only Reference The Parts Of The Texture
//		// That Contains The Character Itself.
//		float   x = (float)bitmap.width / (float)width,
//			y = (float)bitmap.rows / (float)height;
//
//		// Here We Draw The Texturemapped Quads.
//		// The Bitmap That We Got From FreeType Was Not
//		// Oriented Quite Like We Would Like It To Be,
//		// But We Link The Texture To The Quad
//		// In Such A Way That The Result Will Be Properly Aligned.
//		glBegin(GL_QUADS);
//		glTexCoord2d(0, 0); glVertex2f(0, bitmap.rows);
//		glTexCoord2d(0, y); glVertex2f(0, 0);
//		glTexCoord2d(x, y); glVertex2f(bitmap.width, 0);
//		glTexCoord2d(x, 0); glVertex2f(bitmap.width, bitmap.rows);
//		glEnd();
//		glPopMatrix();
//		glTranslatef(face->glyph->advance.x >> 6, 0, 0);
//
//		// Increment The Raster Position As If We Were A Bitmap Font.
//		// (Only Needed If You Want To Calculate Text Length)
//		// glBitmap(0,0,0,0,face->glyph->advance.x >> 6,0,NULL);
//
//		// Finish The Display List
//		glEndList();
//	}
//
//	void font_data::init(const char* fname, unsigned int h) 
//	{
//		// Allocate Some Memory To Store The Texture Ids.
//		textures = new GLuint[128];
//
//		this->h = h;
//
//		// Create And Initilize A FreeType Font Library.
//		FT_Library library;
//		if (FT_Init_FreeType(&library))
//			throw std::runtime_error("FT_Init_FreeType failed");
//
//		// The Object In Which FreeType Holds Information On A Given
//		// Font Is Called A "face".
//		FT_Face face;
//
//		// This Is Where We Load In The Font Information From The File.
//		// Of All The Places Where The Code Might Die, This Is The Most Likely,
//		// As FT_New_Face Will Fail If The Font File Does Not Exist Or Is Somehow Broken.
//		if (FT_New_Face(library, fname, 0, &face))
//			throw std::runtime_error("FT_New_Face failed (there is probably a problem with your font file)");
//
//		// For Some Twisted Reason, FreeType Measures Font Size
//		// In Terms Of 1/64ths Of Pixels.  Thus, To Make A Font
//		// h Pixels High, We Need To Request A Size Of h*64.
//		// (h << 6 Is Just A Prettier Way Of Writing h*64)
//		FT_Set_Char_Size(face, h << 6, h << 6, 96, 96);
//
//		// Here We Ask OpenGL To Allocate Resources For
//		// All The Textures And Display Lists Which We
//		// Are About To Create. 
//		list_base = glGenLists(128);
//		glGenTextures(128, textures);
//
//		// This Is Where We Actually Create Each Of The Fonts Display Lists.
//		for (unsigned char i = 0; i < 128; i++)
//			make_dlist(face, i, list_base, textures);
//
//		// We Don't Need The Face Information Now That The Display
//		// Lists Have Been Created, So We Free The Assosiated Resources.
//		FT_Done_Face(face);
//
//		// Ditto For The Font Library.
//		FT_Done_FreeType(library);
//	}
//
//	void font_data::clean() 
//	{
//		glDeleteLists(list_base, 128);
//		glDeleteTextures(128, textures);
//		delete[] textures;
//	}
//
//	// A Fairly Straightforward Function That Pushes
//	// A Projection Matrix That Will Make Object World
//	// Coordinates Identical To Window Coordinates.
//	inline void pushScreenCoordinateMatrix() 
//	{
//		glPushAttrib(GL_TRANSFORM_BIT);
//		GLint   viewport[4];
//		glGetIntegerv(GL_VIEWPORT, viewport);
//		glMatrixMode(GL_PROJECTION);
//		glPushMatrix();
//		glLoadIdentity();
//		gluOrtho2D(viewport[0], viewport[2], viewport[1], viewport[3]);
//		glPopAttrib();
//	}
//
//	// Pops The Projection Matrix Without Changing The Current
//	// MatrixMode.
//	inline void pop_projection_matrix() 
//	{
//		glPushAttrib(GL_TRANSFORM_BIT);
//		glMatrixMode(GL_PROJECTION);
//		glPopMatrix();
//		glPopAttrib();
//	}
//
//	vector<string> lines;
//	for (const char* c = text; *c; c++) 
//	{
//		if (*c == '\n') 
//		{
//			string line;
//			for (const char* n = start_line; n < c; n++) line.append(1, *n);
//			lines.push_back(line);
//			start_line = c + 1;
//		}
//	}
//	if (start_line) 
//	{
//		string line;
//		for (const char* n = start_line; n < c; n++) line.append(1, *n);
//		lines.push_back(line);
//	}
//
//	glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TRANSFORM_BIT);
//	glMatrixMode(GL_MODELVIEW);
//	glDisable(GL_LIGHTING);
//	glEnable(GL_TEXTURE_2D);
//	glDisable(GL_DEPTH_TEST);
//	glEnable(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//
//	glListBase(font);
//
//
//	float modelview_matrix[16];
//	glGetFloatv(GL_MODELVIEW_MATRIX, modelview_matrix);
//
//	// This Is Where The Text Display Actually Happens.
//	// For Each Line Of Text We Reset The Modelview Matrix
//	// So That The Line's Text Will Start In The Correct Position.
//	// Notice That We Need To Reset The Matrix, Rather Than Just Translating
//	// Down By h. This Is Because When Each Character Is
//	// Drawn It Modifies The Current Matrix So That The Next Character
//	// Will Be Drawn Immediately After It. 
//	for (int i = 0; i < lines.size(); i++) 
//	{
//		glPushMatrix();
//		glLoadIdentity();
//		glTranslatef(x, y - h * i, 0);
//		glMultMatrixf(modelview_matrix);
//
//		// The Commented Out Raster Position Stuff Can Be Useful If You Need To
//		// Know The Length Of The Text That You Are Creating.
//		// If You Decide To Use It Make Sure To Also Uncomment The glBitmap Command
//		// In make_dlist().
//			// glRasterPos2f(0,0);
//		glCallLists(lines[i].length(), GL_UNSIGNED_BYTE, lines[i].c_str());
//		// float rpos[4];
//		// glGetFloatv(GL_CURRENT_RASTER_POSITION ,rpos);
//		// float len=x-rpos[0]; (Assuming No Rotations Have Happend)
//
//		glPopMatrix();
//	}
//
//	glPopAttrib();
//
//	pop_projection_matrix();
//}
//
//
