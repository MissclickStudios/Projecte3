#include "OpenGL.h"

#include "Application.h"														// Globals.h already included in Application.h.

#include "Primitive.h"

// ------------------------------------------------------------
Primitive::Primitive() : transform(float4x4::identity), color(White), wire(false), axis(false), type(PRIMITIVE_TYPES::NONE)
{
	VAO = 0;
	
	for (uint i = 0; i < (uint)BUFFER_TYPE::MAX_BUFFER_TYPES; ++i)				// Initializing the primitive's buffers (vertices, indices...).
	{
		buffers[i]		= 0;
		bufferSize[i]	= 0;
	}
}

// ------------------------------------------------------------
PRIMITIVE_TYPES Primitive::GetType() const
{
	return type;
}

void Primitive::Update()
{

}

// ------------------------------------------------------------
void Primitive::Render() const
{
	glPushMatrix();
	glMultMatrixf((GLfloat*)&transform.Transposed());

	InnerRender();

	glPopMatrix();
}

void Primitive::RenderByIndices()
{
	glPushMatrix();
	glMultMatrixf((GLfloat*)&transform.Transposed());

	IndicesRender();

	glPopMatrix();
}

// ------------------------------------------------------------
void Primitive::LoadBuffersOnMemory()
{
	glGenVertexArrays(1, (GLuint*)&VAO);																// Generating a vertex array object that will store all buffer objects.
	glBindVertexArray(VAO);																				// --------

	glGenBuffers(1, (GLuint*)&buffers[(uint)BUFFER_TYPE::VERTICES]);									// Generating a vertex buffer that will store the vertex positions of the primitives.
	glBindBuffer(GL_ARRAY_BUFFER, buffers[(uint)BUFFER_TYPE::VERTICES]);								// --------
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), &vertices[0], GL_STATIC_DRAW);		// -------- 

	glGenBuffers(1, (GLuint*)&buffers[(uint)BUFFER_TYPE::INDICES]);										// Generating an index buffer that will store the indices of each of the primitives.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[(uint)BUFFER_TYPE::INDICES]);							// --------
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices.size(), &indices[0], GL_STATIC_DRAW);	// --------
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindVertexArray(0);
}

// ------------------------------------------------------------
void Primitive::InnerRender() const
{
	glPointSize(5.0f);

	glBegin(GL_POINTS);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);

	glEnd();

	glPointSize(1.0f);
}

void Primitive::IndicesRender()
{
	return;
}

// ------------------------------------------------------------
void Primitive::SetPos(float x, float y, float z)
{
	transform.Translate(x, y, z);
}

float3 Primitive::GetPos() const										// In a 4x4 matrix, the top-left 3x3 matrix is the rotation and the left column's first 3 elements are the position.
{
	return transform.TranslatePart();
}

// ------------------------------------------------------------
void Primitive::SetRotation(const float3 &u, const float& angle)
{
	transform.SetRotatePart(u, angle);
}

// ------------------------------------------------------------
void Primitive::Scale(float x, float y, float z)
{
	transform.Scale(x, y, z);
}

// CUBE ============================================

P_Cube::P_Cube(const float3& _size, float mass) : Primitive(), size(_size), loaded_in_array(false), loaded_in_indices(false)
{
	type = PRIMITIVE_TYPES::CUBE;
}

float3 P_Cube::GetSize() const
{
	return size;
}

void P_Cube::DirectRender() const
{	
	float sx = size.x * 0.5f;
	float sy = size.y * 0.5f;
	float sz = size.z * 0.5f;

	glBegin(GL_QUADS);
	// ------------ QUAD ------------
	/*glNormal3f(0.0f, 0.0f, 1.0f);			// ABDC
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-sx, -sy,  sz);				// A
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f( sx,	-sy,  sz);				// B
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f( sx,	 sy,  sz);				// D
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-sx,  sy,  sz);				// C

	glNormal3f(0.0f, 0.0f, -1.0f);			// FEGH
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f( sx, -sy, -sz);				// F
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-sx, -sy, -sz);				// E
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-sx,  sy, -sz);				// G
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f( sx,  sy, -sz);				// H

	glNormal3f(1.0f, 0.0f, 0.0f);			//BFHD
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(sx, -sy,  sz);				// B
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(sx, -sy, -sz);				// F
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(sx,  sy, -sz);				// H
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(sx,  sy,  sz);				// D

	glNormal3f(-1.0f, 0.0f, 0.0f);			// EACG
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-sx, -sy, -sz);				// E
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-sx, -sy,  sz);				// A
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-sx,  sy,  sz);				// C
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-sx,  sy, -sz);				// G

	glNormal3f(0.0f, 1.0f, 0.0f);			// CDHG
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-sx, sy,  sz);				// C
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f( sx, sy,  sz);				// D
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f( sx, sy, -sz);				// H
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-sx, sy, -sz);				// G

	glNormal3f(0.0f, -1.0f, 0.0f);			// EFBA
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-sx, -sy, -sz);				// E
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f( sx, -sy, -sz);				// F
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f( sx, -sy,  sz);				// B
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-sx, -sy,  sz);*/			// A

	// ------------ TRIANGLES ------------
	glBegin(GL_TRIANGLES);

	glTexCoord2f(0.0f, 0.0f);			// ABC
	glVertex3f(-sx, -sy, sz);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f( sx, -sy, sz);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-sx,  sy, sz);

	glTexCoord2f(1.0f, 0.0f);			// BDC
	glVertex3f( sx, -sy, sz);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f( sx,  sy, sz);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-sx,  sy, sz);

	glTexCoord2f(0.0f, 0.0f);			// BFD
	glVertex3f(sx, -sy,  sz);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(sx, -sy, -sz);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(sx,  sy,  sz);

	glTexCoord2f(1.0f, 0.0f);			// FHD
	glVertex3f(sx, -sy, -sz);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(sx,  sy, -sz);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(sx,  sy,  sz);

	glTexCoord2f(0.0f, 0.0f);			// FEH
	glVertex3f( sx, -sy, -sz);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-sx, -sy, -sz);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f( sx,  sy, -sz);

	glTexCoord2f(1.0f, 0.0f);			// EGH
	glVertex3f(-sx, -sy, -sz);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-sx,  sy, -sz);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f( sx,  sy, -sz);

	glTexCoord2f(0.0f, 0.0f);			// EAG
	glVertex3f(-sx, -sy, -sz);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-sx, -sy,  sz);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-sx,  sy, -sz);

	glTexCoord2f(1.0f, 0.0f);			// ACG
	glVertex3f(-sx, -sy,  sz);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-sx,  sy,  sz);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-sx,  sy, -sz);

	glTexCoord2f(0.0f, 0.0f);			// EFA
	glVertex3f(-sx, -sy, -sz);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f( sx, -sy, -sz);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-sx, -sy,  sz);

	glTexCoord2f(1.0f, 0.0f);			// FBA
	glVertex3f( sx, -sy, -sz);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f( sx, -sy,  sz);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-sx, -sy,  sz);

	glTexCoord2f(0.0f, 0.0f);			// CDG
	glVertex3f(-sx, sy,  sz);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f( sx, sy,  sz);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-sx, sy, -sz);

	glTexCoord2f(1.0f, 0.0f);			// DHG
	glVertex3f( sx, sy,  sz);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f( sx, sy, -sz);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-sx, sy, -sz);

	glEnd();
}

void P_Cube::ArrayRender()
{	
	if (!loaded_in_array)
	{	
		uint position	= 0;
		uint normals	= 0;
		uint tex_coords = 0;

		const uint vertex_array_size = 108;									// 6 faces with 2 triangles per face --> 12 vertex with 3 coordinates and 3 vertices. TOTAL: 6 * 2 * 3 * 3 = 108
		const uint tex_coord_array_size = 72;								// Same as above but 2 coordinates instead of 3.

		float cube_vertex_coords[vertex_array_size] =						// Being ABCDEFGH all the vertex in a Cube. Starting at the front bottom left and ending at the back top right.
		{
			// --- (0.0f, 0.0f, 0.0f) AS THE CENTER
			// -- FRONT --
			-1.0f, -1.0f, 1.0f,		// ABC
			 1.0f, -1.0f, 1.0f,
			-1.0f,  1.0f, 1.0f,
			
			 1.0f, -1.0f, 1.0f,		// BDC
			 1.0f,  1.0f, 1.0f,
			-1.0f,  1.0f, 1.0f,
			
			// -- RIGHT --
			1.0f, -1.0f,  1.0f,		// BFD
			1.0f, -1.0f, -1.0f,
			1.0f,  1.0f,  1.0f,
			
			1.0f, -1.0f, -1.0f,		// FHD
			1.0f,  1.0f, -1.0f,
			1.0f,  1.0f,  1.0f,
			
			// -- BACK --
			 1.0f, -1.0f, -1.0f,	// FEH
			-1.0f, -1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,	// EGH
			-1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			
			// -- LEFT --
			-1.0f, -1.0f, -1.0f,	// EAG
			-1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f,
			
			-1.0f, -1.0f,  1.0f,	// ACG
			-1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f,
			
			// -- BOTTOM --
			-1.0f, -1.0f,  1.0f,	// AEF
			-1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			
			-1.0f, -1.0f,  1.0f,	// AFB
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f,  1.0f,
			
			// -- TOP --
			-1.0f, 1.0f,  1.0f,		// CDG
			 1.0f, 1.0f,  1.0f,
			-1.0f, 1.0f, -1.0f,
			
			 1.0f, 1.0f,  1.0f,		// DHG
			 1.0f, 1.0f, -1.0f,
			-1.0f, 1.0f, -1.0f
			// ------------------------------------
		};

		float cube_tex_coords[tex_coord_array_size] =
		{
			// --- FRONT ---
			0.0f, 0.0f,				// ABC
			1.0f, 0.0f,
			0.0f, 1.0f,

			1.0f, 0.0f,				// BDC
			1.0f, 1.0f,
			0.0f, 1.0f,

			// --- RIGHT ---
			0.0f, 0.0f,				// BFD
			1.0f, 0.0f,
			0.0f, 1.0f,

			1.0f, 0.0f,				// FHD
			0.0f, 1.0f,
			1.0f, 1.0f,

			// --- BACK ---
			0.0f, 0.0f,				// FEH
			1.0f, 0.0f,
			0.0f, 1.0f,

			1.0f, 0.0f,				// EGH
			1.0f, 1.0f,
			0.0f, 1.0f,

			// --- LEFT ---
			0.0f, 0.0f,				// EAG
			1.0f, 0.0f,
			0.0f, 1.0f,

			1.0f, 0.0f,				// ACG
			1.0f, 1.0f,
			0.0f, 1.0f,

			// --- BOTTOM ---
			0.0f, 0.0f,				// EFA
			1.0f, 0.0f,
			0.0f, 1.0f,

			1.0f, 0.0f,				// FBA
			1.0f, 1.0f,
			0.0f, 1.0f,

			// --- TOP ---
			0.0f, 0.0f,				// CDG
			1.0f, 0.0f,
			0.0f, 1.0f,

			1.0f, 0.0f,				// DHG
			1.0f, 1.0f,
			0.0f, 1.0f
		};

		for (uint i = 0, j = 0; i < vertex_array_size; ++i, ++j)				// Resizing the cube according to the size parameter.
		{
			if (j == 0)
			{
				//cube_vertex_coords[i] *= size.x;
				cube_vertex_coords[i] *= size.x * 0.5f;
			}
			else if (j == 1)
			{
				//cube_vertex_coords[i] *= size.y;
				cube_vertex_coords[i] *= size.y * 0.5f;
			}
			else if (j == 2)
			{
				//cube_vertex_coords[i] *= size.z;
				cube_vertex_coords[i] *= size.z * 0.5f;
				j = -1;
			}
		}

		/*glGenBuffers(1, (GLuint*)&position);
		glBindBuffer(GL_ARRAY_BUFFER, position);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 36 * 3, cube_vertex_coords, GL_STATIC_DRAW);*/
		
		VAO = 0;

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, (GLuint*)&position);
		glBindBuffer(GL_ARRAY_BUFFER, position);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertex_array_size, cube_vertex_coords, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
		glEnableVertexAttribArray(0);

		glGenBuffers(1, (GLuint*)&tex_coords);
		glBindBuffer(GL_ARRAY_BUFFER, tex_coords);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * tex_coord_array_size, cube_tex_coords, GL_STATIC_DRAW);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);

		loaded_in_array = true;
	}

	/*glEnableClientState(GL_VERTEX_ARRAY);
	
	glVertexPointer(3, GL_FLOAT, 0, nullptr);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	glDisableClientState(GL_VERTEX_ARRAY);*/

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void P_Cube::IndicesRender()
{	
	if (!loaded_in_indices)
	{
		const uint array_size = 24;						// 8 vertex by 3 coordinates per vertex.
		
		float cube_vertex_coords[array_size] =			// Being ABCDEFGH all the vertex in a Cube. Starting at the front bottom left and ending at the back top right.
		{
			// --- (0.0f, 0.0f, 0.0f) AS THE CENTER
			-1.0f, -1.0f,  1.0f,						// A --- i = 0
			 1.0f, -1.0f,  1.0f,						// B --- i = 1
			-1.0f,  1.0f,  1.0f,						// C --- i = 2
			 1.0f,  1.0f,  1.0f,						// D --- i = 3
			-1.0f, -1.0f, -1.0f,						// E --- i = 4
			 1.0f, -1.0f, -1.0f,						// F --- i = 5
			-1.0f,  1.0f, -1.0f,						// G --- i = 6
			 1.0f,  1.0f, -1.0f							// H --- i = 7
		};

		ApplyTransform(cube_vertex_coords, array_size);
		ApplySize(cube_vertex_coords, array_size);

		uint indices[36] =
		{
			0, 1, 2,				//ABC
			1, 3, 2,				//BDC
			
			1, 5, 3,				//BFD
			5, 7, 3,				//FHD
			
			4, 6, 7,				//EGH
			5, 4, 7,				//FEH
			
			4, 0, 6,				//EAG
			0, 2, 6,				//ACG
			
			0, 4, 5,				//AEF
			1, 0, 5,				//BAF
			
			2, 3, 6,				//CDG
			3, 7, 6,				//DHG
		};

		uint my_id = 0;
		uint my_indices = 0;

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, (GLuint*)&my_id);
		glBindBuffer(GL_ARRAY_BUFFER, my_id);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, cube_vertex_coords, GL_STATIC_DRAW);

		glGenBuffers(1, (GLuint*)&my_indices);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, my_indices);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * 36, indices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

		glBindVertexArray(0);

		loaded_in_indices = true;
	}

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);

	/*glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(transform.M[12], transform.M[13], transform.M[14]);
	//glRotatef(angle, x, y, z);
	//glScalef(x, y, z);

	glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, nullptr);

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);

	glDisableClientState(GL_VERTEX_ARRAY);

	glPopMatrix();*/
}

void P_Cube::ApplyTransform(float* coordinates, int array_size)
{
	float3 position = GetPos();

	for (int i = 0; i < array_size; ++i)
	{
		coordinates[i]		+= position.x;
		coordinates[++i]	+= position.y;
		coordinates[++i]	+= position.z;
	}
}

void P_Cube::ApplySize(float* coordinates, int array_size)
{
	for (int i = 0; i < array_size; ++i)					// Resizing the cube according to the size parameter.
	{
		coordinates[i]		*= size.x * 0.5f;
		coordinates[++i]	*= size.y * 0.5f;
		coordinates[++i]	*= size.z * 0.5f;
	}
}

// SPHERE ============================================

/*Sphere::Sphere(float _radius, float mass) : Primitive(), radius(_radius), rings(1), sectors(1), loaded_buffers(false)
{
	type = PRIMITIVE_TYPES::SPHERE;
}*/

P_Sphere::P_Sphere(float radius, uint rings, uint sectors) : Primitive(), radius(radius), rings(rings), sectors(sectors), loaded_buffers(false)
{
	type = PRIMITIVE_TYPES::SPHERE;
}

float P_Sphere::GetRadius() const
{
	return radius;
}

uint P_Sphere::GetRings() const
{
	return rings;
}

uint P_Sphere::GetSectors() const
{
	return sectors;
}

void P_Sphere::SetRings(uint rings)
{
	this->rings = rings;

	loaded_buffers = false;
}

void P_Sphere::SetSectors(uint sectors)
{
	this->sectors = sectors;

	loaded_buffers = false;
}

void P_Sphere::IndicesRender()
{	
	if (!loaded_buffers)
	{
		float const ring_constant = 1.0f / (float)(rings - 1);
		float const sector_constant = 1.0f / (float)(sectors - 1);

		const uint vertices_size	= rings * sectors * 3;											// rings per sectors per ring per 3 coordinates per vertex. Total: rings * sectors * 3;
		const uint normals_size		= rings * sectors * 3;
		const uint tex_coords_size	= rings * sectors * 2;
		const uint indices_size		= rings * sectors * 4;

		vertices.resize(vertices_size);
		normals.resize(normals_size);
		uvs.resize(tex_coords_size);
		indices.resize(indices_size);

		std::vector<float>::iterator vert = vertices.begin();
		std::vector<float>::iterator norm = normals.begin();
		std::vector<float>::iterator texc = uvs.begin();
		
		int v = 0;
		for (uint ring = 0; ring < rings; ++ring)
		{
			for (uint sector = 0; sector < sectors; ++sector)
			{
				float const x = cos(2 * PI * sector * sector_constant) * sin(PI * ring * ring_constant);
				float const y = sin(-HALF_PI + PI * ring * ring_constant);
				float const z = sin(2 * PI * sector * sector_constant) * sin(PI * ring * ring_constant);

				*vert++ = x * radius;																// Calculating the position coordinates of each vertex
				*vert++ = y * radius;																// according to the axis, the ring and sector consts
				*vert++ = z * radius;																// and the radius.

				*norm++ = x;																		// Calculating the normal vector coordinates of each vertex
				*norm++ = y;																		//
				*norm++ = z;																		//

				*texc++ = sector * sector_constant;													//
				*texc++ = ring * ring_constant;														//
			}
		}

		std::vector<uint>::iterator ind = indices.begin();
		for (uint ring = 0; ring < rings; ++ring)
		{
			for (uint sector = 0; sector < sectors; ++sector)
			{
				*ind++ = ring * sectors + sector;
				*ind++ = ring * sectors + (sector + 1);
				*ind++ = (ring + 1) * sectors + (sector + 1);
				*ind++ = (ring + 1) * sectors + sector;
			}
		}

		LoadBuffersOnMemory();
		
		/*uint my_vertices = 0;
		uint my_indices = 0;
		
		glGenBuffers(1, (GLuint*)&my_vertices);
		glBindBuffer(GL_ARRAY_BUFFER, my_vertices);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

		glGenBuffers(1, (GLuint*)&my_indices);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, my_indices);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices.size(), &indices[0], GL_STATIC_DRAW);*/

		loaded_buffers = true;
	}

	GLfloat x = (GLfloat)transform.TranslatePart().x;
	GLfloat y = (GLfloat)transform.TranslatePart().y;
	GLfloat z = (GLfloat)transform.TranslatePart().z;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(x, y, z);
	
	/*glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, nullptr);

	glDrawElements(GL_QUADS, indices.size(), GL_UNSIGNED_INT, nullptr);

	glDisableClientState(GL_VERTEX_ARRAY);*/

	glBindVertexArray(VAO);
	glDrawElements(GL_QUADS, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glPopMatrix();
}

void P_Sphere::InnerRender() const
{
	//glutSolidSphere(radius, 25, 25);
}

// CYLINDER ============================================
P_Cylinder::P_Cylinder(float radius, float height, uint sectors, float mass) : Primitive(), radius(radius), height(height), sectors(sectors), loaded_in_buffers(false)
{
	type = PRIMITIVE_TYPES::CYLINDER;
}

float P_Cylinder::GetRadius() const
{
	return radius;
}

float P_Cylinder::GetHeight() const
{
	return height;
}

uint P_Cylinder::GetSectors() const
{
	return sectors;
}

void P_Cylinder::SetSectors(uint sectors)
{
	this->sectors = sectors;
	loaded_in_buffers = false;
}

std::vector<float> P_Cylinder::GetCircularVertices()
{
	float sector_step = 2 * PI / sectors;			// The angle between cylinder sectors is: a = 2*PI * (current_sector / total_sectors). To get the step: (2 * PI) / total_sectors.
	float sector_angle;

	float x;										// To improve readability.
	float y;
	float z;

	std::vector<float> vertices;

	for (uint i = 0; i < sectors; ++i)
	{
		sector_angle = i * sector_step;				// Getting the angle for the current sector.

		x = sin(sector_angle);
		y = 0.0f;
		z = cos(sector_angle);

		vertices.push_back(x);
		vertices.push_back(y);
		vertices.push_back(z);
	}

	return vertices;
}

void P_Cylinder::IndicesRender()
{
	if (!loaded_in_buffers)
	{
		vertices.clear();
		normals.clear();
		uvs.clear();
		indices.clear();

		std::vector<float> circular_vertices = GetCircularVertices();

		ConstructCoverVertices(circular_vertices);

		uint base_center_index = (int)vertices.size() / 3;						// Gets the index of the vertex at the center of the cylinder's base.
		uint top_center_index = base_center_index + sectors + 1;				// Gets the index of the vertex at the center of the cylinder's top.

		ConstructBaseAndTopVertices(circular_vertices);

		ConstructIndices(base_center_index, top_center_index);

		LoadBuffersOnMemory();

		/*uint my_vert = 0;
		uint my_indices = 1;

		glGenBuffers(1, (GLuint*)&my_vert);
		glBindBuffer(GL_ARRAY_BUFFER, my_vert);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

		glGenBuffers(1, (GLuint*)&my_indices);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, my_indices);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices.size(), &indices[0], GL_STATIC_DRAW);*/

		/*glGenBuffers(1, (GLuint*)&buffers[(uint)BUFFER_TYPE::VERTICES]);				// Generating a vertex buffer that will store the vertex positions of the primitives.
		glBindBuffer(GL_ARRAY_BUFFER, buffers[(uint)BUFFER_TYPE::VERTICES]);			// --------
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), &vertices[0], GL_STATIC_DRAW);	// --------

		glGenBuffers(1, (GLuint*)&buffers[(uint)BUFFER_TYPE::INDICES]);					// Generating an index buffer that will store the indices of each of the primitives.
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[(uint)BUFFER_TYPE::INDICES]);		// --------
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices.size(), &indices[0], GL_STATIC_DRAW);	// --------*/

		loaded_in_buffers = true;
	}

	GLfloat x = (GLfloat)transform.TranslatePart().x;
	GLfloat y = (GLfloat)transform.TranslatePart().y;
	GLfloat z = (GLfloat)transform.TranslatePart().z;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(x, y, z);

	/*glEnableClientState(GL_VERTEX_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, NULL);

	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

	glDisableClientState(GL_VERTEX_ARRAY);*/

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glPopMatrix();
}

void P_Cylinder::ConstructCoverVertices(std::vector<float> circular_vertices)
{
	// CONSTRUCTING COVER VERTICES
	for (int ring = 0; ring < 2; ++ring)													// Goes from -h/2 to h/2. Maybe change it so it goes from 0 to 2 directly.
	{
		float h = -height * 0.5f + ring * height;											// As i progresses, the height will go from -h/2 to h/2;
		float t = 1.0f - ring;																// Vertical UV Coords. Goes from 1 to 0.

		for (uint sector = 0, i = 0; sector < sectors; ++sector, i += 3)						// Goes through all the sector vertices in a single ring of the cylinder.
		{
			float const x = circular_vertices[i];											// X coordinate. 
			float const y = circular_vertices[i + 1];										// Y coordinate. As the index is increased, it accesses the next element in the vertices vector. (y)
			float const z = circular_vertices[i + 2];										// Z coordinate. As the index is increased, it accesses the next element in the vertices vector. (z)

			// --- POSITION VECTOR
			vertices.push_back(x * radius);													// vx
			vertices.push_back(h);															// vy
			vertices.push_back(z * radius);													// vz

			// --- NORMAL VECTOR
			normals.push_back(x);															// nx
			normals.push_back(y);															// ny
			normals.push_back(z);															// nz

			// --- TEXTURE COORDS
			uvs.push_back((float)sector / sectors);											// s
			uvs.push_back(t);																// t
		}
	}
}

void P_Cylinder::ConstructBaseAndTopVertices(std::vector<float> circular_vertices)
{
	// CONSTRUCTING BASE AND TOP VERTICES
	for (int ring = 0; ring < 2; ++ring)
	{
		float const h = -height * 0.5f + ring * height;										// -h/2 to h/2
		float const ny = -1.0f + ring * 2.0f;												// Vertical Normal Coords. Goes from -1 to 1;

		vertices.push_back(0.0f);															// Constructing a new center point.
		vertices.push_back(h);																// 
		vertices.push_back(0.0f);															// 
																							// 
		normals.push_back(0.0f);															// 
		normals.push_back(ny);																// 
		normals.push_back(0.0f);															// 
																							// 
		uvs.push_back(0.5f);																// 
		uvs.push_back(0.5f);																// --------------------------------

		for (uint sector = 0, i = 0; sector < sectors; ++sector, i += 3)
		{
			float const x = circular_vertices[i];
			float const y = circular_vertices[i + 1];
			float const z = circular_vertices[i + 2];

			vertices.push_back(x * radius);													// vx
			vertices.push_back(h);															// vy
			vertices.push_back(z * radius);													// vz

			normals.push_back(0.0f);														// nx
			normals.push_back(ny);															// ny
			normals.push_back(0.0f);														// nz

			uvs.push_back(-x * 0.5f + 0.5f);												// s
			uvs.push_back(-y * 0.5f + 0.5f);												// t
		}
	}
}

void P_Cylinder::ConstructIndices(uint base_center_index, uint top_center_index)
{
	uint base_index = 0;														// First vertex index at base.
	uint top_index = sectors /*+ 1*/;											// First vertex index at top.

	// COVER INDICES
	for (uint sector = 0; sector < sectors; ++sector, ++base_index, ++top_index)
	{	
		// 2 triangles per sector

		if (top_index < (sectors * 2) - 1)										// If the current triangles are not the last ones.
		{
			indices.push_back(base_index);										// b_i => b_i + 1 => t_i  ==  ABC
			indices.push_back(base_index + 1);									// The triangles must be constructed in counter-clockwise order.
			indices.push_back(top_index);										// Otherwise the constructed triangles will be culled by the backface culling.
			
			indices.push_back(top_index);										// t_i => b_i + 1 => t_i + 1  ==  CBD
			indices.push_back(base_index + 1);									// 
			indices.push_back(top_index + 1);									// ---------------------------------- 
		}
		else
		{
			indices.push_back(base_index);										// As index (sectors * 2) corresponds to the center vertex of the base,
			indices.push_back(0);												// when top_index == (sectors * 2) - 1 it means that its the
			indices.push_back(top_index);										// last 2 triangles and we need the indices from the beginning.
			
			indices.push_back(top_index);
			indices.push_back(0);
			indices.push_back(base_index + 1);
		}
	}

	// BASE INDICES
	for (uint sector = 0, circle_index = base_center_index + 1; sector < sectors; ++sector, ++circle_index)
	{
		if (sector < sectors - 1)
		{
			indices.push_back(base_center_index);							// Constructing the base triangles indices.
			indices.push_back(circle_index + 1);							// The first index of the triangle will always
			indices.push_back(circle_index);								// be the one at the center of the base.
		}
		else
		{
			indices.push_back(base_center_index);							// Constructing the last triangle indices of the base.
			indices.push_back(base_center_index + 1);						//
			indices.push_back(circle_index);								// ---------------------------------------------------
		}
	}

	// TOP INDICES
	for (uint sector = 0, circle_index = top_center_index + 1; sector < sectors; ++sector, ++circle_index)
	{
		if (sector < sectors - 1)
		{
			indices.push_back(top_center_index);							// The circle index refers to the indexes that are at the borders
			indices.push_back(circle_index);								// of the base or the top. In this case it refers to the ones at the top.
			indices.push_back(circle_index + 1);							//
		}
		else
		{
			indices.push_back(top_center_index);							// 
			indices.push_back(circle_index);								// 
			indices.push_back(top_center_index + 1);						// ---------------------------------------------------------------------- 
		}
	}
}

void P_Cylinder::InnerRender() const
{
	float4x4 rotateMat = float4x4::identity * Quat::RotateZ(90.0f * DEGTORAD);

	glPushMatrix();
	glMultMatrixf((GLfloat*)&rotateMat);

	int n = 30;

	// Cylinder Bottom
	glBegin(GL_POLYGON);
	
	for(int i = 360; i >= 0; i -= (360 / n))
	{
		float a = (float)i * PI / 180; // degrees to radians
		glVertex3f(-height * 0.5f, radius * cos(a), radius * sin(a));
		//glVertex3f(radius * sin(a), -height * 0.5f, radius * cos(a));
	}
	glEnd();

	// Cylinder Top
	glBegin(GL_POLYGON);
	glNormal3f(0.0f, 0.0f, 1.0f);
	for(int i = 0; i <= 360; i += (360 / n))
	{
		float a = (float)i * PI / 180; // degrees to radians
		glVertex3f(height * 0.5f, radius * cos(a), radius * sin(a));
		//glVertex3f(radius * sin(a), height * 0.5f, radius * cos(a));
	}
	glEnd();

	// Cylinder "Cover"
	glBegin(GL_QUAD_STRIP);
	for(int i = 0; i < 480; i += (360 / n))
	{
		float a = (float)i * PI / 180; // degrees to radians

		glVertex3f(height*0.5f,  radius * cos(a), radius * sin(a) );
		glVertex3f(-height*0.5f, radius * cos(a), radius * sin(a) );
		//glVertex3f(radius * sin(a), height * 0.5f, radius * cos(a));
		//glVertex3f(radius * sin(a), -height * 0.5f, radius * cos(a));
	}
	glEnd();

	glPopMatrix();
}

// PYRAMID ============================================
P_Pyramid::P_Pyramid(float3 size) : Primitive(), size(size), loaded_in_buffers(false)
{
	type = PRIMITIVE_TYPES::PYRAMID;
}

void P_Pyramid::InnerRender() const
{

}

void P_Pyramid::IndicesRender()
{	
	if (!loaded_in_buffers)
	{
		float vertices_ex[15] =
		{
			 0.0f,  1.015f,  0.0f,		// A --- i = 0		// Height of a given equilateral pyramid: (sqrt(6) / 3) * a; where a is the length of the sides of the base.
			-1.0f, -0.615f,  1.0f,		// B --- i = 1
			 1.0f, -0.615f,  1.0f,		// C --- i = 2
			-1.0f, -0.615f, -1.0f,		// D --- i = 3
			 1.0f, -0.615f, -1.0f		// E --- i = 4
		};

		uint indices_ex[18] =
		{
			0, 1, 2,					// ABC --- FRONT
			0, 2, 4,					// ACE --- RIGHT
			0, 4, 3,					// AED --- BACK
			0, 3, 1,					// ADB --- LEFT

			1, 3, 4,					// BDE --- BASE
			1, 4, 2						// BEC ---  -
		};

		uint my_vertices	= 0;
		uint my_indices		= 0;

		/*for (uint i = 0; i < 15; ++i)
		{
			vertices.push_back(vertices_ex[i]);
		}

		for (uint i = 0; i < 18; ++i)
		{
			indices.push_back(indices_ex[i]);
		}

		LoadBuffersOnMemory();*/

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, (GLuint*)&my_vertices);
		glBindBuffer(GL_ARRAY_BUFFER, my_vertices);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 15, vertices_ex, GL_STATIC_DRAW);

		glGenBuffers(1, (GLuint*)&my_indices);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, my_indices);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * 24, indices_ex, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);

		loaded_in_buffers = true;
	}

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	//glMatrixMode(GL_MODELVIEW);
	//glPushMatrix();
	//glTranslatef(transform.M[12], transform.M[13], transform.M[14]);

	//glEnableClientState(GL_VERTEX_ARRAY);

	//glVertexPointer(3, GL_FLOAT, 0, nullptr);

	////glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 2);
	//glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_INT, nullptr);

	//glDisableClientState(GL_VERTEX_ARRAY);

	//glPopMatrix();
}

// LINE ==================================================
P_Line::P_Line() : Primitive(), origin(float3::zero), destination(float3::one)
{
	type = PRIMITIVE_TYPES::LINE;
}

P_Line::P_Line(const float3& A, const float3& B) : Primitive(), origin(A), destination(B)
{
	type = PRIMITIVE_TYPES::LINE;
}

float3 P_Line::GetOrigin() const
{
	return origin;
}

float3 P_Line::GetDestination() const
{
	return destination;
}

void P_Line::InnerRender() const
{
	glLineWidth(2.0f);

	glBegin(GL_LINES);

	glVertex3f(origin.x, origin.y, origin.z);
	glVertex3f(destination.x, destination.y, destination.z);

	glEnd();

	glLineWidth(1.0f);
}

void P_Line::IndicesRender()
{
	InnerRender();
}

// PLANE ==================================================
P_Plane::P_Plane(const float3& _normal) : Primitive(), normal(_normal)
{
	type = PRIMITIVE_TYPES::PLANE;
}

float3 P_Plane::GetNormal() const
{
	return normal;
}

void P_Plane::InnerRender() const
{
	glLineWidth(1.0f);

	glBegin(GL_LINES);

	float d = 200.0f;

	for(float i = -d; i <= d; i += 1.0f)
	{
		glVertex3f(i, 0.0f, -d);
		glVertex3f(i, 0.0f, d);
		glVertex3f(-d, 0.0f, i);
		glVertex3f(d, 0.0f, i);
	}

	glEnd();
}

void P_Plane::IndicesRender()
{
	InnerRender();
}

PrimitiveDrawExamples::PrimitiveDrawExamples() : Primitive(), size(1.0f), origin(float3::zero)
{

}

PrimitiveDrawExamples::PrimitiveDrawExamples(float size, float3 origin) : Primitive(), size(size), origin(origin)
{	
	transform.Translate(origin.x, origin.y, origin.z);
}

void PrimitiveDrawExamples::DrawAllExamples()
{
	GL_PointsExample();
	GL_LinesExample();
	GL_LineStripExample();
	GL_LineLoopExample();
	GL_PolygonExample();
	GL_TrianglesExample();
	GL_TriangleStripExample();
	GL_TriangleFanExample();
	GL_QuadsExample();
	GL_QuadStripExample();
}

void PrimitiveDrawExamples::GL_PointsExample(uint index)
{
	float offset = 4.0f * index;
	
	glPointSize(4.0f);

	glBegin(GL_POINTS);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	glVertex3f(0.0f + offset, 2.0f, 0.0f);						// A

	glVertex3f(1.5f + offset, 2.0f, 0.0f);						// B

	glVertex3f(2.0f + offset, 3.0f, 0.0f);						// C

	glVertex3f(1.5f + offset, 4.0f, 0.0f);						// D

	glVertex3f(0.0f + offset, 4.0f, 0.0f);						// E

	glVertex3f(-0.5f + offset, 3.0f, 0.0f);						// F


	glEnd();

	glPointSize(1.0f);
}

void PrimitiveDrawExamples::GL_LinesExample(uint index)
{
	float offset = 4.0f * index;
	
	glLineWidth(2.0f);
	
	glBegin(GL_LINES);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	glVertex3f(0.0f + offset, 2.0f, 0.0f);						// AB
	glVertex3f(1.5f + offset, 2.0f, 0.0f);

	glVertex3f(2.0f + offset, 3.0f, 0.0f);						// CD
	glVertex3f(1.5f + offset, 4.0f, 0.0f);

	glVertex3f(0.0f + offset, 4.0f, 0.0f);						// EF
	glVertex3f(-0.5f + offset, 3.0f, 0.0f);

	glEnd();

	glLineWidth(1.0f);
}

void PrimitiveDrawExamples::GL_LineStripExample(uint index)
{
	float offset = 4.0f * index;

	glLineWidth(2.0f);

	glBegin(GL_LINE_STRIP);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	glVertex3f(0.0f + offset, 2.0f, 0.0f);						// ABCDE --> Last segment is not drawn --> Still a line, not a polygon.
	glVertex3f(1.5f + offset, 2.0f, 0.0f);
	glVertex3f(2.0f + offset, 3.0f, 0.0f);
	glVertex3f(1.5f + offset, 4.0f, 0.0f);
	glVertex3f(0.0f + offset, 4.0f, 0.0f);
	glVertex3f(-0.5f + offset, 3.0f, 0.0f);

	glEnd();

	glLineWidth(1.0f);
}

void PrimitiveDrawExamples::GL_LineLoopExample(uint index)
{
	float offset = 4.0f * index;

	glLineWidth(2.0f);

	glBegin(GL_LINE_LOOP);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	glVertex3f(0.0f + offset, 2.0f, 0.0f);						// ABCDEF --> Closed loop --> Polygon but not considered as such by OpenGL (not filled).
	glVertex3f(1.5f + offset, 2.0f, 0.0f);
	glVertex3f(2.0f + offset, 3.0f, 0.0f);
	glVertex3f(1.5f + offset, 4.0f, 0.0f);
	glVertex3f(0.0f + offset, 4.0f, 0.0f);
	glVertex3f(-0.5f + offset, 3.0f, 0.0f);

	glEnd();

	glLineWidth(1.0f);
}

void PrimitiveDrawExamples::GL_PolygonExample(uint index)
{
	float offset = 4.0f * index;

	glLineWidth(2.0f);

	glBegin(GL_POLYGON);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	glVertex3f(0.0f + offset, 2.0f, 0.0f);						// ABCDEF --> Polygon and is considered as such by OpenGL (filled).
	glVertex3f(1.5f + offset, 2.0f, 0.0f);
	glVertex3f(2.0f + offset, 3.0f, 0.0f);
	glVertex3f(1.5f + offset, 4.0f, 0.0f);
	glVertex3f(0.0f + offset, 4.0f, 0.0f);
	glVertex3f(-0.5f + offset, 3.0f, 0.0f);

	glEnd();

	glLineWidth(1.0f);
}

void PrimitiveDrawExamples::GL_TrianglesExample(uint index)
{
	float offset = 3.95f * index;

	glLineWidth(2.0f);

	glBegin(GL_TRIANGLES);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	glVertex3f(0.0f + offset, 2.0f, 0.0f);						// ABC --> Same as GL_POLYGON but only for triangles.
	glVertex3f(3.0f + offset, 2.0f, 0.0f);
	glVertex3f(1.5f + offset, 4.0f, 0.0f);

	glEnd();

	glLineWidth(1.0f);
}

void PrimitiveDrawExamples::GL_TriangleStripExample(uint index)
{
	float offset = 4.0f * index;

	glLineWidth(2.0f);
	
	glBegin(GL_TRIANGLE_STRIP);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	glVertex3f(0.0f + offset, 4.0f, 0.0f);						// Vertex are positioned in a zig-zag and GL_TRIANGLE_STRIP closes the zig-zag in triangles.
	glVertex3f(1.5f + offset, 2.0f, 0.0f);
	glVertex3f(3.0f + offset, 4.0f, 0.0f);
	glVertex3f(4.5f + offset, 2.0f, 0.0f);
	glVertex3f(6.0f + offset, 4.0f, 0.0f);
	glVertex3f(7.5f + offset, 2.0f, 0.0f);
	glVertex3f(9.0f + offset, 4.0f, 0.0f);

	glEnd();

	glLineWidth(1.0f);

}

void PrimitiveDrawExamples::GL_TriangleFanExample(uint index)
{
	float offset = 5.20f * index;

	glLineWidth(2.0f);

	glBegin(GL_TRIANGLE_FAN);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	//glVertex3f(0.0f, 3.0f, 0.0f);												// Center of the triangle fan.
	glVertex3f(offset, 3.0f, 0.0f);												// Center of the triangle fan.

	for (int i = 360 - 30; i >= 0 - 30; i -= (360 / 6))							// i = 360 - 30  so it is the same as the previously drawn hexagons.
	{	
		float a = (float)i * PI / 180;											// Degrees to Radiants
		//glVertex3f(2.0f * sin(a), 2.0f * cos(a), 0.0f);						// Triangle fan centered at (0.0f, 0.0f, 0.0f)
		//glVertex3f((2.0f * sin(a)), (2.0f * cos(a)) + 3.0f, 0.0f);			// Triangle fan centered at (0.0f, 3.0f, 0.0f)
		glVertex3f((1.5f * sin(a)) + offset, (1.5f * cos(a)) + 3.0f, 0.0f);		// Triangle fan centered at (position_offset, 3.0f, 0.0f)
	}

	glEnd();

	glLineWidth(1.0f);
}

void PrimitiveDrawExamples::GL_QuadsExample(uint index)
{
	float offset = 5.05f * index;

	glLineWidth(2.0f);

	glBegin(GL_QUADS);								// Same as GL_TRIANGLES, like GL_POLYGON but only for quads.

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	
	glVertex3f(0.0f + offset, 2.0f, 0.0f);			// ABCD
	glVertex3f(2.0f + offset, 2.0f, 0.0f);			// Declaration order has to be counter-clockwise (Right Hand Rule). Otherwise the poligon will be facing backwards (Backface culling).
	glVertex3f(2.0f + offset, 4.0f, 0.0f);
	glVertex3f(0.0f + offset, 4.0f, 0.0f);

	glEnd();

	glLineWidth(1.0f);
}

void PrimitiveDrawExamples::GL_QuadStripExample(uint index)
{
	float offset = 5.0f * index;

	glLineWidth(2.0f);

	glBegin(GL_QUAD_STRIP);						// Very similar to GL_TRIANGLE_STRIP. The vertexs are declared in a zig-zag and they will be conected in quads.

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	glVertex3f(0.0f + offset, 4.0f, 0.0f);
	glVertex3f(0.0f + offset, 2.0f, 0.0f);
	glVertex3f(2.0f + offset, 4.0f, 0.0f);
	glVertex3f(2.0f + offset, 2.0f, 0.0f);
	glVertex3f(4.0f + offset, 4.0f, 0.0f);
	glVertex3f(4.0f + offset, 2.0f, 0.0f);
	glVertex3f(6.0f + offset, 4.0f, 0.0f);
	glVertex3f(6.0f + offset, 2.0f, 0.0f);

	glEnd();

	glLineWidth(1.0f);
}