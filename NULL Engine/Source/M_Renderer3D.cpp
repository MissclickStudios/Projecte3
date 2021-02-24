#include "Profiler.h"													
#include "OpenGL.h"														
#include "Time.h"

#include "Macros.h"														
#include "Log.h"														

#include "Color.h"														
#include "Icons.h"														
#include "Primitive.h"													

#include "Application.h"												
#include "M_Window.h"													
#include "M_Camera3D.h"													
#include "M_Input.h"													
#include "M_FileSystem.h"												
#include "M_ResourceManager.h"											
#include "M_Editor.h"													

#include "R_Mesh.h"														
#include "R_Material.h"													
#include "R_Texture.h"
#include "R_Shader.h"

#include "I_Textures.h"													

#include "GameObject.h"													
#include "C_Mesh.h"														
#include "C_Material.h"													
#include "C_Camera.h"													

#include "M_Renderer3D.h"												

#include "MemoryManager.h"
//////////////////////////////////////////////////
#include "I_Shaders.h"							//TODO: erase
/////////////////////////////////////////////////

#pragma comment (lib, "glu32.lib")    /* link OpenGL Utility lib     */	
#pragma comment (lib, "opengl32.lib") /* link Microsoft OpenGL lib   */	
#pragma comment (lib, "Source/Dependencies/Assimp/libx86/assimp.lib")	

#define WORLD_GRID_SIZE		64
#define CHECKERS_WIDTH		64
#define CHECKERS_HEIGHT		64
#define STANDARD_LINE_WIDTH	1.0f
#define BASE_LINE_WIDTH		3.0f

M_Renderer3D::M_Renderer3D(bool isActive) : Module("Renderer3D", isActive), 
context						(),
vsync						(VSYNC),
sceneFramebuffer			(0),
depthBuffer				(0),
sceneRenderTexture		(0),
depthBufferTexture		(0),
gameFramebuffer			(0),
debugTextureId			(0)
{
	InitDebugVariables();
}

// Destructor
M_Renderer3D::~M_Renderer3D()
{

}

// Called before render is available
bool M_Renderer3D::Init(ParsonNode& configuration)
{
	LOG("Creating 3D Renderer context");
	bool ret = true;

	ret = InitOpenGL();																			// Initializing OpenGL. (Context and initial configuration)

	OnResize();																					// Projection matrix recalculation to keep up with the resizing of the window.
	
	Importer::Textures::Init();																	// Initializing DevIL.

	CreatePrimitiveExamples();																	// Adding one of each available primitice to the primitives vector for later display.

	//InitFramebuffers();
	LoadDebugTexture();

	return ret;
}

bool M_Renderer3D::Start()
{
	bool ret = true;

	InitEngineIcons();

	return ret;
}

// PreUpdate: clear buffer
UpdateStatus M_Renderer3D::PreUpdate(float dt)
{	
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);

	C_Camera* currentCamera = App->camera->GetCurrentCamera();
	if (currentCamera != nullptr)
	{
		if (currentCamera->GetUpdateProjectionMatrix())
		{
			RecalculateProjectionMatrix();
			currentCamera->SetUpdateProjectionMatrix(false);
		}
	}

	glLoadMatrixf((GLfloat*)currentCamera->GetOGLViewMatrix());

	// light 0 on cam pos
	float3 cameraPos = float3::zero;
	if (App->camera->GetCurrentCamera() != nullptr)
	{
		cameraPos = App->camera->GetPosition();
	}
	else
	{
		cameraPos = float3(0.0f, 20.0f, 0.0f);
	}

	lights[0].SetPos(cameraPos.x, cameraPos.y, cameraPos.z);

	for (uint i = 0; i < MAX_LIGHTS; ++i)
	{
		lights[i].Render();
	}

	// --- RENDERER SHORTCUTS
	RendererShortcuts();

	return UpdateStatus::CONTINUE;
}

// PostUpdate present buffer to screen
UpdateStatus M_Renderer3D::PostUpdate(float dt)
{	
	BROFILER_CATEGORY("M_Renderer3D PostUpdate", Profiler::Color::Chartreuse);
	
	RenderScene();

	App->editor->RenderEditorPanels();

	SDL_GL_SwapWindow(App->window->GetWindow());

	return UpdateStatus::CONTINUE;
}

// Called before quitting
bool M_Renderer3D::CleanUp()
{
	LOG("Destroying 3D Renderer");

	for (uint i = 0; i < primitives.size(); ++i)
	{
		RELEASE(primitives[i]);
	}
	
	primitives.clear();

	Importer::Textures::CleanUp();																// Shutting down DevIL. 

	FreeBuffers();

	SDL_GL_DeleteContext(context);

	return true;
}

bool M_Renderer3D::LoadConfiguration(ParsonNode& root)
{
	bool ret = true;

	return ret;
}

bool M_Renderer3D::SaveConfiguration(ParsonNode& root) const
{
	bool ret = true;

	return ret;
}

// ----------- RENDERER METHODS -----------
bool M_Renderer3D::InitDebugVariables()
{
	bool ret = true;
	
	worldGridSize				= WORLD_GRID_SIZE;
	
	worldGridColor				= White;
	wireframeColor				= Yellow;
	vertexNormalsColor			= Yellow;													// TODO: Use other color?
	faceNormalsColor			= Magenta;
	aabbColor					= Green;
	obbColor					= Orange;
	frustumColor				= Red;
	rayColor					= Cyan;
	boneColor					= Pink;

	worldGridLineWidth		= STANDARD_LINE_WIDTH;
	wireframeLineWidth		= STANDARD_LINE_WIDTH;
	vertexNormalsWidth		= BASE_LINE_WIDTH;
	faceNormalsWidth			= BASE_LINE_WIDTH;

	aabbEdgeWidth				= BASE_LINE_WIDTH;
	obbEdgeWidth				= BASE_LINE_WIDTH;
	frustumEdgeWidth			= BASE_LINE_WIDTH;
	rayWidth					= BASE_LINE_WIDTH;
	boneWidth					= BASE_LINE_WIDTH;

	renderWorldGrid			= true;	
	renderWorldAxis			= true;
	renderWireframes		= false;
	renderWertexNormals		= false;
	renderFaceNormals		= false;
	renderBoundingBoxes		= false;
	renderSkeletons			= false;
	renderPrimitiveExamples	= false;

	return ret;
}

bool M_Renderer3D::InitOpenGL()
{
	bool ret = true;
	
	//Create OpenGL Context
	context = SDL_GL_CreateContext(App->window->GetWindow());
	if (context == NULL)
	{
		LOG("[ERROR] OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	ret = InitGlew();																					// Initializing Glew.

	if (ret == true)
	{
		//Use Vsync
		if (vsync)
		{
			if (SDL_GL_SetSwapInterval(1) < 0)
			{
				LOG("[ERROR] Unable to set Vsync! SDL Error: %s\n", SDL_GetError());
			}
			else
			{
				LOG("[STATUS] Vsync is activated!");
			}
		}
		else
		{
			if (SDL_GL_SetSwapInterval(0) < 0)
			{
				LOG("[ERROR] Unable to set frame update interval to immediate! SDL Error: %s\n", SDL_GetError());
			}
			else
			{
				LOG("[STATUS] Vsync is deactivated!");
			}
		}

		//Initialize Projection Matrix
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		//Check for error
		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			LOG("[ERROR] Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		//Initialize Modelview Matrix
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		//Check for error
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			LOG("[ERROR] Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glClearDepth(1.0f);

		//Initialize clear color
		glClearColor(0.f, 0.f, 0.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);

		//Check for error
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			LOG("[ERROR] Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		GLfloat LightModelAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LightModelAmbient);

		lights[0].ref = GL_LIGHT0;
		lights[0].ambient.Set(0.25f, 0.25f, 0.25f, 1.0f);
		lights[0].diffuse.Set(0.75f, 0.75f, 0.75f, 1.0f);
		lights[0].SetPos(0.0f, 0.0f, 2.5f);
		lights[0].Init();

		GLfloat MaterialAmbient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, MaterialAmbient);

		GLfloat MaterialDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialDiffuse);

		lights[0].Active(true);

		SetGLFlag(GL_DEPTH_TEST, true);
		SetGLFlag(GL_CULL_FACE, true);
		SetGLFlag(GL_LIGHTING, true);
		SetGLFlag(GL_COLOR_MATERIAL, true);
		SetGLFlag(GL_TEXTURE_2D, true);

		SetGLFlag(GL_ALPHA_TEST, true);
		glAlphaFunc(GL_GREATER, 0.20f);													// Have alpha test in c_material (color alpha)?

		SetGLFlag(GL_BLEND, true);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	return ret;
}

bool M_Renderer3D::InitGlew()
{
	bool ret = true;

	// Initializing glew.
	GLenum glewInitReturn = glewInit();												// glew must be initialized after an OpenGL rendering context has been created.

	if (glewInitReturn != GLEW_NO_ERROR)
	{
		LOG("[ERROR] GLEW could not initialize! SDL_Error: %s\n", SDL_GetError());

		ret = false;
	}

	return ret;
}

void M_Renderer3D::OnResize()
{	
	App->window->RecalculateWindowSize();
	
	float winWidth	= (float)App->window->GetWidth();
	float winHeight	= (float)App->window->GetHeight();

	glViewport(0, 0, (GLsizei)winWidth, (GLsizei)winHeight);

	if (App->camera->GetCurrentCamera() != nullptr)
	{
		App->camera->GetCurrentCamera()->SetAspectRatio(winWidth / winHeight);
	}
	else
	{
		LOG("[ERROR] Renderer 3D: Could not recalculate the aspect ratio! Error: Current Camera was nullptr.");
	}

	RecalculateProjectionMatrix();

	InitFramebuffers();
}

void M_Renderer3D::RecalculateProjectionMatrix()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if (App->camera->GetCurrentCamera() != nullptr)
	{
		glLoadMatrixf((GLfloat*)App->camera->GetCurrentCamera()->GetOGLProjectionMatrix());
	}
	else
	{
		LOG("[ERROR] Renderer 3D: Could not recalculate the projection matrix! Error: Current Camera was nullptr.");
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void M_Renderer3D::InitEngineIcons()
{
	uint32 animationIconUid	= App->resourceManager->LoadFromLibrary(ANIMATION_ICON_PATH);
	uint32 fileIconUid		= App->resourceManager->LoadFromLibrary(FILE_ICON_PATH);
	uint32 folderIconUid	= App->resourceManager->LoadFromLibrary(FOLDER_ICON_PATH);
	uint32 materialIconUid	= App->resourceManager->LoadFromLibrary(MATERIAL_ICON_PATH);
	uint32 modelIconUid		= App->resourceManager->LoadFromLibrary(MODEL_ICON_PATH);

	R_Texture* animationIcon	= (R_Texture*)App->resourceManager->RequestResource(animationIconUid);
	R_Texture* fileIcon			= (R_Texture*)App->resourceManager->RequestResource(fileIconUid);
	R_Texture* folderIcon		= (R_Texture*)App->resourceManager->RequestResource(folderIconUid);
	R_Texture* materialIcon		= (R_Texture*)App->resourceManager->RequestResource(materialIconUid);
	R_Texture* modelIcon		= (R_Texture*)App->resourceManager->RequestResource(modelIconUid);

	engineIcons.animationIcon	= animationIcon;
	engineIcons.fileIcon		= fileIcon;
	engineIcons.folderIcon		= folderIcon;
	engineIcons.materialIcon	= materialIcon;
	engineIcons.modelIcon		= modelIcon;

}

void M_Renderer3D::InitFramebuffers()
{
	glGenFramebuffers(1, (GLuint*)&sceneFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, sceneFramebuffer);
	
	// --- SCENE RENDER TEXTURE ---
	glGenTextures(1, (GLuint*)&sceneRenderTexture);
	glBindTexture(GL_TEXTURE_2D, sceneRenderTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, App->window->GetWidth(), App->window->GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// --- DEPTH BUFFER TEXTURE ---
	glGenTextures(1, (GLuint*)&depthBufferTexture);
	glBindTexture(GL_TEXTURE_2D, depthBufferTexture);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	//glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT, App->window->GetWidth(), App->window->GetHeight());
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, App->window->GetWidth(), App->window->GetHeight(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sceneRenderTexture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBufferTexture, 0);

	// --- DEPTH & STENCIL BUFFERS ---
	glGenRenderbuffers(1, (GLuint*)&depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, App->window->GetWidth(), App->window->GetHeight());
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		LOG("[ERROR] Renderer 3D: Could not generate the scene's frame buffer! Error: %s", gluErrorString(glGetError()));
	}

	// --- UNBINDING THE FRAMEBUFFER ---
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void M_Renderer3D::LoadDebugTexture()
{
	GLubyte checkerImage[CHECKERS_HEIGHT][CHECKERS_WIDTH][4];						// HEIGHT columns, WIDTH rows and 4 variables per checker (for RGBA purposes).

	for (int i = 0; i < CHECKERS_HEIGHT; ++i)											// There will be CHECKERS_WIDTH rows per column.
	{
		for (int j = 0; j < CHECKERS_WIDTH; ++j)										// There will be an RGBA value per checker.
		{
			int color = ((((i & 0x8) == 0) ^ ((j & 0x8) == 0))) * 255;					// Getting the checker's color (white or black) according to the iteration indices and bitwise ops.

			checkerImage[i][j][0] = (GLubyte)color;									// R
			checkerImage[i][j][1] = (GLubyte)color;									// G
			checkerImage[i][j][2] = (GLubyte)color;									// B
			checkerImage[i][j][3] = (GLubyte)255;										// A
		}
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);												// Sets the pixel storage modes. GL_UNPACK_ALIGNMENT specifies the alignment requirements for the
	// --->																				// start of each pixel row in memory. 1 means that the alignment requirements will be byte-alignment.
	glGenTextures(1, &debugTextureId);												// Generate texture names. Returns n names in the given buffer. GL_INVALID_VALUE if n is negative.
	glBindTexture(GL_TEXTURE_2D, debugTextureId);										// Bind a named texture to a texturing target. Binds the buffer with the given target (GL_TEXTURE_2D).

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);						// Set texture parameters. WRAP_S: Set the wrap parameters for tex. coord. S.. GL_REPEAT is the default.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);						// WRAP_T: Set the wrap parameters for the texture coordinate r
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);					// MAG_FILTER: Sets the tex. magnification process param.. GL_NEAREST rets the val. of nearest tex elem.
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);					// MIN_FILTER: Sets the texture minimization process parameters. " ".
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);	// MIPMAP_NEAREST: Same as GL_NEAREST but works with the mipmaps generated by glGenerateMipmap() to
	// --->																				// handle the process of resizing a tex. Takes the mipmap that most closely matches the size of the px.

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CHECKERS_WIDTH, CHECKERS_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkerImage);

	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void M_Renderer3D::FreeBuffers()
{
	glDeleteRenderbuffers(1, (GLuint*)&depthBuffer);
	glDeleteTextures(1, (GLuint*)&sceneRenderTexture);
	glDeleteFramebuffers(1, (GLuint*)&sceneFramebuffer);

	glDeleteTextures(1, (GLuint*)&debugTextureId);
}

void M_Renderer3D::RendererShortcuts()
{
	if (App->input->GetKey(SDL_SCANCODE_F1) == KeyState::KEY_DOWN)
	{
		renderWorldGrid = !renderWorldGrid;
	}

	if (App->input->GetKey(SDL_SCANCODE_F2) == KeyState::KEY_DOWN)
	{
		renderWorldAxis = !renderWorldAxis;
	}
	
	if (App->input->GetKey(SDL_SCANCODE_F3) == KeyState::KEY_DOWN)
	{
		SetRenderWireframes(!renderWireframes);
	}

	if (App->input->GetKey(SDL_SCANCODE_F7) == KeyState::KEY_DOWN)
	{
		SetGLFlag(GL_TEXTURE_2D, !GetGLFlag(GL_TEXTURE_2D));
	}

	if (App->input->GetKey(SDL_SCANCODE_F8) == KeyState::KEY_DOWN)
	{
		SetGLFlag(GL_COLOR_MATERIAL, !GetGLFlag(GL_COLOR_MATERIAL));
	}

	if (App->input->GetKey(SDL_SCANCODE_KP_PLUS) == KeyState::KEY_DOWN)
	{
		if (App->camera->GetCurrentCamera() != nullptr)
		{
			float currentFov = App->camera->GetCurrentCamera()->GetVerticalFOV();
			App->camera->GetCurrentCamera()->SetVerticalFOV(currentFov + 5.0f);
		}
	}
	if (App->input->GetKey(SDL_SCANCODE_KP_MINUS) == KeyState::KEY_DOWN)
	{
		if (App->camera->GetCurrentCamera() != nullptr)
		{
			float current_fov = App->camera->GetCurrentCamera()->GetVerticalFOV();
			App->camera->GetCurrentCamera()->SetVerticalFOV(current_fov - 5.0f);
		}
	}
}

void M_Renderer3D::RenderScene()
{	
	glBindFramebuffer(GL_FRAMEBUFFER, sceneFramebuffer);
	glClear(GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);

	if (renderWorldGrid)
	{
		DrawWorldGrid(worldGridSize);
	}

	if (renderWorldAxis)
	{
		DrawWorldAxis();
	}

	RenderMeshes();
	RenderCuboids();
	//RenderRays();
	RenderSkeletons();
	
	if (App->camera->DrawLastRaycast())
	{
		RayRenderer last_ray = RayRenderer(App->camera->lastRaycast, rayColor, rayWidth);
		last_ray.Render();
	}

	//PrimitiveDrawExamples p_ex = PrimitiveDrawExamples();
	//p_ex.DrawAllExamples();

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	if (renderPrimitiveExamples)
	{
		for (uint i = 0; i < primitives.size(); ++i)
		{
			primitives[i]->RenderByIndices();
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void M_Renderer3D::DrawWorldGrid(const int& size)
{
	glColor4f(worldGridColor.r, worldGridColor.g, worldGridColor.b, worldGridColor.a);
	glLineWidth(worldGridLineWidth);
	glBegin(GL_LINES);

	float destination = (float)size;
	
	for (float origin = -destination; origin <= destination; origin += 1.0f)
	{
		glVertex3f( origin,		 0.0f, -destination);
		glVertex3f( origin,		 0.0f,  destination);
		glVertex3f(-destination, 0.0f,  origin);
		glVertex3f( destination, 0.0f,  origin);
	}

	glEnd();
	glLineWidth(STANDARD_LINE_WIDTH);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void M_Renderer3D::DrawWorldAxis()
{
	glLineWidth(2.0f);

	glBegin(GL_LINES);

	glColor4f(1.0f, 0.0f, 0.0f, 1.0f);											// X Axis.
	glVertex3f(0.0f, 0.0f, 0.0f);		glVertex3f(1.0f,  0.0f, 0.0f);
	glVertex3f(1.0f, 0.1f, 0.0f);		glVertex3f(1.1f, -0.1f, 0.0f);
	glVertex3f(1.1f, 0.1f, 0.0f);		glVertex3f(1.0f, -0.1f, 0.0f);

	glColor4f(0.0f, 1.0f, 0.0f, 1.0f);											// Y Axis.
	glVertex3f( 0.0f,  0.0f,  0.0f);	glVertex3f(0.0f, 1.0f,  0.0f);
	glVertex3f(-0.05f, 1.25f, 0.0f);	glVertex3f(0.0f, 1.15f, 0.0f);
	glVertex3f( 0.05f, 1.25f, 0.0f);	glVertex3f(0.0f, 1.15f, 0.0f);
	glVertex3f( 0.0f,  1.15f, 0.0f);	glVertex3f(0.0f, 1.05f, 0.0f);

	glColor4f(0.0f, 0.0f, 1.0f, 1.0f);											// Z Axis.
	glVertex3f( 0.0f,   0.0f, 0.0f);	glVertex3f( 0.0f,   0.0f, 1.0f);
	glVertex3f(-0.05f,  0.1f, 1.05f);	glVertex3f( 0.05f,  0.1f, 1.05f);
	glVertex3f( 0.05f,  0.1f, 1.05f);	glVertex3f(-0.05f, -0.1f, 1.05f);
	glVertex3f(-0.05f, -0.1f, 1.05f);	glVertex3f( 0.05f, -0.1f, 1.05f);

	glEnd();

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glLineWidth(STANDARD_LINE_WIDTH);
}

void M_Renderer3D::AddRenderersBatch(const std::vector<MeshRenderer>& meshRenderers, const std::vector<CuboidRenderer>& cuboidRenderers, 
										const std::vector<SkeletonRenderer>& skeletonRenderers)
{	
	for (uint i = 0; i < meshRenderers.size(); ++i)
	{
		this->meshRenderers.push_back(meshRenderers[i]);
	}

	for (uint i = 0; i < cuboidRenderers.size(); ++i)
	{
		this->cuboidRenderers.push_back(cuboidRenderers[i]);
	}

	for (uint i = 0; i < skeletonRenderers.size(); ++i)
	{
		this->skeletonRenderers.push_back(skeletonRenderers[i]);
	}
}

void M_Renderer3D::RenderMeshes()
{
	for (uint i = 0; i < meshRenderers.size(); ++i)
	{
		meshRenderers[i].Render();
	}

	meshRenderers.clear();
}

void M_Renderer3D::RenderCuboids()
{
	glDisable(GL_LIGHTING);

	for (uint i = 0; i < cuboidRenderers.size(); ++i)
	{	
		cuboidRenderers[i].Render();
	}

	cuboidRenderers.clear();

	glEnable(GL_LIGHTING);
}

void M_Renderer3D::RenderRays()
{
	glDisable(GL_LIGHTING);
	glBegin(GL_LINES);

	// TODO: HAVE A VECTOR FOR RAYS TO RENDER

	glEnd();
	glEnable(GL_LIGHTING);
}

void M_Renderer3D::RenderSkeletons()
{
	glDisable(GL_LIGHTING);

	for (uint i = 0; i < skeletonRenderers.size(); ++i)
	{
		skeletonRenderers[i].Render();
	}

	skeletonRenderers.clear();

	glEnable(GL_LIGHTING);
}

void M_Renderer3D::DeleteFromMeshRenderers(C_Mesh* cMeshToDelete)
{
	for (uint i = 0; i < meshRenderers.size(); ++i)
	{
		if (meshRenderers[i].cMesh == cMeshToDelete)
		{
			meshRenderers.erase(meshRenderers.begin() + i);
		}
	}
}

void M_Renderer3D::DeleteFromMeshRenderers(R_Mesh* rMeshToDelete)
{
	for (uint i = 0; i < meshRenderers.size(); ++i)
	{
		if (meshRenderers[i].cMesh->GetMesh() == rMeshToDelete)
		{
			//mesh_renderers[i].c_mesh->SetMesh(nullptr);
			meshRenderers.erase(meshRenderers.begin() + i);
		}
	}
}

void M_Renderer3D::ClearRenderers()
{
	meshRenderers.clear();
	cuboidRenderers.clear();
}

void M_Renderer3D::AddPrimitive(Primitive* primitive)
{
	primitives.push_back(primitive);
}

void M_Renderer3D::CreatePrimitiveExamples()
{
	P_Cube* cube			= new P_Cube();
	P_Sphere* sphere		= new P_Sphere();
	P_Cylinder* cylinder	= new P_Cylinder();
	P_Pyramid* pyramid		= new P_Pyramid();

	sphere->SetPos(2.0f, 0.0f, 0.0f);
	cylinder->SetPos(4.0f, 0.0f, 0.0f);
	pyramid->SetPos(12.0f, 0.0f, 0.0f);

	primitives.push_back(cube);
	primitives.push_back(sphere);
	primitives.push_back(cylinder);
	primitives.push_back(pyramid);
}

/*void M_Renderer3D::GenerateBuffers(const R_Mesh* mesh)
{
	if (!mesh->vertices.empty())
	{
		glGenBuffers(1, (GLuint*)&mesh->VBO);																			// Generates the Vertex Buffer Object
		glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);																		// Binds VBO with the GL_ARRAY_BUFFER biding point (target):
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh->vertices.size(), &mesh->vertices[0], GL_STATIC_DRAW);		// Inits the data stored inside VBO and specifies how the data will be accessed.
	}

	if (!mesh->normals.empty())
	{
		glGenBuffers(1, (GLuint*)&mesh->NBO);
		glBindBuffer(GL_ARRAY_BUFFER, mesh->NBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh->normals.size(), &mesh->normals[0], GL_STATIC_DRAW);
	}

	if (!mesh->texCoords.empty())
	{
		glGenBuffers(1, (GLuint*)&mesh->TBO);
		glBindBuffer(GL_ARRAY_BUFFER, mesh->TBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh->texCoords.size(), &mesh->texCoords[0], GL_STATIC_DRAW);
	}

	if (!mesh->indices.empty())
	{
		glGenBuffers(1, (GLuint*)&mesh->IBO);																			// Generates the Index Buffer Object
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->IBO);																// Binds IBO with the GL_ARRAY_BUFFER biding point (target):
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * mesh->indices.size(), &mesh->indices[0], GL_STATIC_DRAW);	// Inits the data stored inside IBO and specifies how the data will be accessed.
	}
}*/

Icons M_Renderer3D::GetEngineIcons() const
{
	return engineIcons;
}

uint M_Renderer3D::GetDebugTextureID() const
{
	return debugTextureId;
}

uint M_Renderer3D::GetSceneFramebuffer() const
{
	return sceneFramebuffer;
}

uint M_Renderer3D::GetSceneRenderTexture() const
{
	return sceneRenderTexture;
}

uint M_Renderer3D::GetGameFramebuffer() const
{
	return gameFramebuffer;
}

uint M_Renderer3D::GetDepthBuffer() const
{
	return depthBuffer;
}

uint M_Renderer3D::GetDepthBufferTexture() const
{
	return depthBufferTexture;
}

const char* M_Renderer3D::GetDrivers() const
{
	return SDL_GetCurrentVideoDriver();													// SDL_GetCurrentVideoDriver() returns NULL if no driver has been initialized.
}

bool M_Renderer3D::GetVsync() const
{
	return vsync;
}

void M_Renderer3D::SetVsync(const bool& setTo)
{
	if (setTo != vsync)
	{
		vsync = setTo;
		
		int result = SDL_GL_SetSwapInterval(vsync ? 1 : 0);								// 0 for immediate updates, 1 for updates synchronized with vertical retrace.

		if (result < 0)																	// SDL_GL_SetSwapInterval() returns 0 on success and -1 on failure.
		{
			LOG("[WARNING] Unable to set Vsync! SDL Error: %s\n", SDL_GetError()); 
		}
		else
		{
			LOG("[STATUS] Vsync has been %s", vsync ? "activated" : "deactivated");
		}
	}
}

bool M_Renderer3D::GetGLFlag(GLenum flag) const
{
	return glIsEnabled(flag);
}

bool M_Renderer3D::GetGLFlag(RendererFlags flag) const
{
	return glIsEnabled((GLenum)flag);
}

void M_Renderer3D::SetGLFlag(GLenum flag, bool setTo)
{
	if (setTo != (bool)glIsEnabled(flag))
	{
		setTo ? glEnable(flag) : glDisable(flag);
	}
}

void M_Renderer3D::SetGLFlag(RendererFlags flag, bool setTo)
{
	GLenum glFlag = (GLenum)flag;
	
	if (setTo != (bool)glIsEnabled(glFlag))
	{
		setTo ? glEnable(glFlag) : glDisable(glFlag);
	}
}

uint M_Renderer3D::GetWorldGridSize() const
{
	return worldGridSize;
}

Color M_Renderer3D::GetWorldGridColor() const
{
	return worldGridColor;
}

Color M_Renderer3D::GetWireframeColor() const
{
	return wireframeColor;
}

Color M_Renderer3D::GetVertexNormalsColor() const
{
	return vertexNormalsColor;
}

Color M_Renderer3D::GetFaceNormalsColor() const
{
	return faceNormalsColor;
}

Color M_Renderer3D::GetAABBColor() const
{
	return aabbColor;
}

Color M_Renderer3D::GetOBBColor() const
{
	return obbColor;
}

Color M_Renderer3D::GetFrustumColor() const
{
	return frustumColor;
}

Color M_Renderer3D::GetRayColor() const
{
	return rayColor;
}

Color M_Renderer3D::GetBoneColor() const
{
	return boneColor;
}

float M_Renderer3D::GetWorldGridLineWidth() const
{
	return worldGridLineWidth;
}

float M_Renderer3D::GetWireframeLineWidth() const
{
	return wireframeLineWidth;
}

float M_Renderer3D::GetVertexNormalsWidth() const
{
	return vertexNormalsWidth;
}

float M_Renderer3D::GetFaceNormalsWidth() const
{
	return faceNormalsWidth;
}

float M_Renderer3D::GetAABBEdgeWidth() const
{
	return aabbEdgeWidth;
}

float M_Renderer3D::GetOBBEdgeWidth() const
{
	return obbEdgeWidth;
}

float M_Renderer3D::GetFrustumEdgeWidth() const
{
	return frustumEdgeWidth;
}

float M_Renderer3D::GetRayWidth() const
{
	return rayWidth;
}

float M_Renderer3D::GetBoneWidth() const
{
	return boneWidth;
}

bool M_Renderer3D::GetRenderWorldGrid() const
{
	return renderWorldGrid;
}

bool M_Renderer3D::GetRenderWorldAxis() const
{
	return renderWorldAxis;
}

bool M_Renderer3D::GetRenderWireframes() const
{
	return renderWireframes;
}

bool M_Renderer3D::GetRenderVertexNormals() const
{
	return renderWertexNormals;
}

bool M_Renderer3D::GetRenderFaceNormals() const
{
	return renderFaceNormals;
}

bool M_Renderer3D::GetRenderBoundingBoxes() const
{
	return renderBoundingBoxes;
}

bool M_Renderer3D::GetRenderSkeletons() const
{
	return renderSkeletons;
}

bool M_Renderer3D::GetRenderPrimitiveExamples() const
{
	return renderPrimitiveExamples;
}

void M_Renderer3D::SetWorldGridSize(const uint& worldGridSize)
{
	this->worldGridSize = worldGridSize;
}

void M_Renderer3D::SetWorldGridColor(const Color& worldGridColor)
{
	this->worldGridColor = worldGridColor;
}

void M_Renderer3D::SetWireframeColor(const Color& wireframeColor)
{
	this->wireframeColor = wireframeColor;
}

void M_Renderer3D::SetVertexNormalsColor(const Color& vertexNormalsColor)
{
	this->vertexNormalsColor = vertexNormalsColor;
}

void M_Renderer3D::SetFaceNormalsColor(const Color& faceNormalsColor)
{
	this->faceNormalsColor = faceNormalsColor;
}

void M_Renderer3D::SetAABBColor(const Color& aabbColor)
{
	this->aabbColor = aabbColor;
}

void M_Renderer3D::SetOBBColor(const Color& obbColor)
{
	this->obbColor = obbColor;
}

void M_Renderer3D::SetFrustumColor(const Color& frustumColor)
{
	this->frustumColor = frustumColor;
}

void M_Renderer3D::SetRayColor(const Color& rayColor)
{
	this->rayColor = rayColor;
}

void M_Renderer3D::SetBoneColor(const Color& boneColor)
{
	this->boneColor = boneColor;
}

void M_Renderer3D::SetWorldGridLineWidth(const float& worldGridLineWidth)
{
	this->worldGridLineWidth = worldGridLineWidth;
}

void M_Renderer3D::SetWireframeLineWidth(const float& wireframeLineWidth)
{
	this->wireframeLineWidth = wireframeLineWidth;
}

void M_Renderer3D::SetVertexNormalsWidth(const float& vertexNormalsWidth)
{
	this->vertexNormalsWidth = vertexNormalsWidth;
}

void M_Renderer3D::SetFaceNormalsWidth(const float& faceNormalsWidth)
{
	this->faceNormalsWidth = faceNormalsWidth;
}

void M_Renderer3D::SetAABBEdgeWidth(const float& aabbEdgeWidth)
{
	this->aabbEdgeWidth = aabbEdgeWidth;
}

void M_Renderer3D::SetOBBEdgeWidth(const float& obbEdgeWidth)
{
	this->obbEdgeWidth = obbEdgeWidth;
}

void M_Renderer3D::SetFrustumEdgeWidth(const float& frustumEdgeWidth)
{
	this->frustumEdgeWidth = frustumEdgeWidth;
}

void M_Renderer3D::SetRayWidth(const float& rayWidth)
{
	this->rayWidth = rayWidth;
}

void M_Renderer3D::SetBoneWidth(const float& boneWidth)
{
	this->boneWidth = boneWidth;
}

void M_Renderer3D::SetRenderWorldGrid(const bool& setTo)
{
	renderWorldGrid = setTo;
}

void M_Renderer3D::SetRenderWorldAxis(const bool& setTo)
{
	renderWorldAxis = setTo;
}

void M_Renderer3D::SetRenderWireframes(const bool& setTo)
{
	if (setTo != renderWireframes)
	{
		renderWireframes = setTo;

		if (renderWireframes)
		{
			//glColor4fv(wireframe_color.C_Array());
			//glLineWidth(wireframe_line_width);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			SetGLFlag(GL_TEXTURE_2D, false);
			SetGLFlag(GL_LIGHTING, false);
		}
		else
		{
			//glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			//glLineWidth(STANDARD_LINE_WIDTH);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			SetGLFlag(GL_TEXTURE_2D, true);
			SetGLFlag(GL_LIGHTING, true);
		}
	}
}

void M_Renderer3D::SetRenderVertexNormals(const bool& setTo)
{
	renderWertexNormals = setTo;
}

void M_Renderer3D::SetRenderFaceNormals(const bool& setTo)
{
	renderFaceNormals = setTo;
}

void M_Renderer3D::SetRenderBoundingBoxes(const bool& setTo)
{
	renderBoundingBoxes = setTo;
}

void M_Renderer3D::SetRenderSkeletons(const bool& setTo)
{
	renderSkeletons = setTo;
}

void M_Renderer3D::SetRenderPrimtiveExamples(const bool& setTo)
{
	renderPrimitiveExamples = setTo;
}

// --- RENDERER STRUCTURES METHODS ---
// --- MESH RENDERER METHODS
MeshRenderer::MeshRenderer(const float4x4& transform, C_Mesh* cMesh, C_Material* cMaterial) :
transform	(transform),
cMesh		(cMesh),
cMaterial	(cMaterial)
{

}

void MeshRenderer::Render()
{

	R_Mesh* rMesh = cMesh->GetMesh();

	if (rMesh == nullptr)
	{
		LOG("[ERROR] Renderer 3D: Could not render Mesh! Error: R_Mesh* was nullptr.");
		return;
	}

	//glMatrixMode(GL_MODELVIEW);
	//glPushMatrix();
	//glMultMatrixf((GLfloat*)&transform.Transposed());												// OpenGL requires that the 4x4 matrices are column-major instead of row-major.

	ApplyDebugParameters();																			// Enable Wireframe Mode for this specific mesh, etc.
	ApplyTextureAndMaterial();																		// Apply resource texture or default texture, mesh color...
	ApplyShader();
	//glEnableClientState(GL_VERTEX_ARRAY);															// Enables the vertex array for writing and to be used during rendering.
	//glEnableClientState(GL_NORMAL_ARRAY);															// Enables the normal array for writing and to be used during rendering.
	//glEnableClientState(GL_TEXTURE_COORD_ARRAY);													// Enables the texture coordinate array for writing and to be used during rendering.

	//glBindBuffer(GL_ARRAY_BUFFER, rMesh->TBO);														// Will bind the buffer object with the mesh->TBO identifyer for rendering.
	//glTexCoordPointer(2, GL_FLOAT, 0, nullptr);														// Specifies the location and data format of an array of tex coords to use when rendering.

	//glBindBuffer(GL_ARRAY_BUFFER, rMesh->NBO);														// The normal buffer is bound so the normal positions can be interpreted correctly.
	//glNormalPointer(GL_FLOAT, 0, nullptr);															// 

	//glBindBuffer(GL_ARRAY_BUFFER, rMesh->VBO);														// The vertex buffer is bound so the vertex positions can be interpreted correctly.
	//glVertexPointer(3, GL_FLOAT, 0, nullptr);														// Specifies the location and data format of an array of vert coords to use when rendering.

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rMesh->IBO);												// Will bind the buffer object with the mesh->IBO identifyer for rendering.
	
	glBindVertexArray(rMesh->VAO);
	glDrawElements(GL_TRIANGLES, rMesh->indices.size(), GL_UNSIGNED_INT, nullptr);					// 

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);														// Clearing the buffers.
	//glBindBuffer(GL_ARRAY_BUFFER, 0);																// 												

	//glDisableClientState(GL_TEXTURE_COORD_ARRAY);													// Disabling the client-side capabilities enabled at the beginning.
	//glDisableClientState(GL_NORMAL_ARRAY);															// 
	//glDisableClientState(GL_VERTEX_ARRAY);															// Disabling GL_TEXTURE_COORD_ARRAY, GL_NORMAL_ARRAY and GL_VERTEX_ARRAY.

	glBindTexture(GL_TEXTURE_2D, 0);																// ---------------------
	
	ClearTextureAndMaterial();																		// Clear the specifications applied in ApplyTextureAndMaterial().
	ClearDebugParameters();																			// Clear the specifications applied in ApplyDebugParameters().
	ClearShader();
	// --- DEBUG DRAW ---
	if (rMesh->drawVertexNormals || App->renderer->GetRenderVertexNormals())
	{
		RenderVertexNormals(rMesh);
	}

	if (rMesh->drawFaceNormals || App->renderer->GetRenderFaceNormals())
	{
		RenderFaceNormals(rMesh);
	}

	//glPopMatrix();
}

void MeshRenderer::RenderVertexNormals(const R_Mesh* rMesh)
{
	if (rMesh == nullptr)
	{
		LOG("[ERROR] Renderer 3D: Could not render Mesh Vertex Normals! Error: R_Mesh* was nullptr.");
		return;
	}
	
	if (rMesh->vertices.size() != rMesh->normals.size())
	{
		LOG("[ERROR] Renderer 3D: Could not render Mesh Vertex Normals! Error: Num Vertices != Num Normals.");
		return;
	}

	std::vector<float> vertices	= rMesh->vertices;
	std::vector<float> normals	= rMesh->normals;

	glColor4fv(App->renderer->GetVertexNormalsColor().C_Array());
	glLineWidth(App->renderer->GetVertexNormalsWidth());
	glBegin(GL_LINES);

	for (uint i = 0; i < vertices.size(); i += 3)
	{
		glVertex3f(vertices[i], vertices[i + 1], vertices[i + 2]);
		glVertex3f(vertices[i] + normals[i], vertices[i + 1] + normals[i + 1], vertices[i + 2] + normals[i + 2]);
	}

	glEnd();
	glLineWidth(STANDARD_LINE_WIDTH);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// Clear vectors?
}

void MeshRenderer::RenderFaceNormals(const R_Mesh* rMesh)
{
	if (rMesh == nullptr)
	{
		LOG("[ERROR] Renderer 3D: Could not render Mesh Face Normals! Error: R_Mesh* was nullptr.");
		return;
	}
	
	if (rMesh->vertices.size() != rMesh->normals.size())
	{
		LOG("[ERROR] Renderer 3D: Could not render Mesh Face Normals! Error: Num Vertices != Num Normals.");
		return;
	}

	std::vector<Triangle> vertexFaces;
	std::vector<Triangle> normalFaces;
	GetFaces(rMesh, vertexFaces, normalFaces);

	if (vertexFaces.size() != normalFaces.size())
	{
		LOG("[ERROR] Renderer 3D: Could not render Mesh Face Normals! Error: Num Vertex Faces != Num Normal Faces.");
		return;
	}

	glColor4fv(App->renderer->GetFaceNormalsColor().C_Array());
	glLineWidth(App->renderer->GetFaceNormalsWidth());
	glBegin(GL_LINES);


	for (uint i = 0; i < vertexFaces.size(); ++i)
	{
		float3 origin		= vertexFaces[i].CenterPoint();											// Center vector of Face Vertices
		float3 destination	= origin + normalFaces[i].CenterPoint();									// Center of Face Vertices + Center of Face Normals.

		glVertex3f(origin.x, origin.y, origin.z);
		glVertex3f(destination.x, destination.y, destination.z);
	}

	glEnd();
	glLineWidth(STANDARD_LINE_WIDTH);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	vertexFaces.clear();
	normalFaces.clear();
}

void MeshRenderer::GetFaces(const R_Mesh* rMesh, std::vector<Triangle>& vertexFaces, std::vector<Triangle>& normalFaces)
{
	std::vector<float3> vertices;
	std::vector<float3> normals;
	
	for (uint i = 0; i < rMesh->vertices.size(); i += 3)
	{
		float3 vertex;
		float3 normal;

		vertex.x = rMesh->vertices[i];
		vertex.y = rMesh->vertices[i + 1];
		vertex.z = rMesh->vertices[i + 2];

		normal.x = rMesh->normals[i];
		normal.y = rMesh->normals[i + 1];
		normal.z = rMesh->normals[i + 2];

		vertices.push_back(vertex);
		normals.push_back(normal);

		if (vertices.size() == 3 && normals.size() == 3)
		{
			vertexFaces.push_back(Triangle(vertices[0], vertices[1], vertices[2]));
			normalFaces.push_back(Triangle(normals[0], normals[1], normals[2]));

			vertices.clear();
			normals.clear();
		}
	}

	vertices.clear();
	normals.clear();
}

void MeshRenderer::ApplyDebugParameters()
{
	if (App->renderer->GetRenderWireframes() || cMesh->GetShowWireframe())
	{
		glColor4fv(App->renderer->GetWireframeColor().C_Array());
		glLineWidth(App->renderer->GetWireframeLineWidth());
	}

	if (cMesh->GetShowWireframe() && !App->renderer->GetRenderWireframes())
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
	}
}

void MeshRenderer::ClearDebugParameters()
{
	if (cMesh->GetShowWireframe() && !App->renderer->GetRenderWireframes())
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_LIGHTING);
	}

	glLineWidth(STANDARD_LINE_WIDTH);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void MeshRenderer::ApplyTextureAndMaterial()
{
	if (cMaterial != nullptr)
	{
		if (!cMaterial->IsActive())
		{
			glDisable(GL_TEXTURE_2D);
		}

		if (cMaterial->GetTexture() == nullptr)														// If the Material Component does not have a Texture Resource.
		{
			Color color = cMaterial->GetMaterialColour();
			glColor4f(color.r, color.g, color.b, color.a);												// Apply the diffuse color to the mesh.
		}
		else if (cMaterial->UseDefaultTexture())														// If the default texture is set to be used (bool use_default_texture)
		{
			glBindTexture(GL_TEXTURE_2D, App->renderer->GetDebugTextureID());							// Binding the texture that will be rendered. Index = 0 means we are clearing the binding.
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, cMaterial->GetTextureID());									// Binding the texture_id in the Texture Resource of the Material Component.
		}
	}
}

void MeshRenderer::ClearTextureAndMaterial()
{
	if (cMaterial != nullptr)
	{
		if (!cMaterial->IsActive())
		{
			glEnable(GL_TEXTURE_2D);
		}
	}
}

void MeshRenderer::ApplyShader()
{
	uint32 shaderProgram = 0;
	if (cMaterial != nullptr)
	{
		//if(cMaterial->GetShader()) shaderProgram = cMaterial->GetShader()->shaderProgramID;

		cMaterial->GetShader() ? shaderProgram = cMaterial->GetShader()->shaderProgramID : shaderProgram;

		shaderProgram ? shaderProgram : shaderProgram = SetDefaultShader(cMaterial);

		glUseProgram(shaderProgram);

		cMaterial->GetTexture() ? cMaterial->GetShader()->SetUniform1i("hasTexture", (GLint)true) : cMaterial->GetShader()->SetUniform1i("hasTexture", (GLint)false);

		if (shaderProgram != 0)
		{
			cMaterial->GetShader()->SetUniformVec4f("inColor", (GLfloat*)&cMaterial->GetMaterialColour());

			cMaterial->GetShader()->SetUniformMatrix4("modelMatrix", transform.Transposed().ptr());

			cMaterial->GetShader()->SetUniformMatrix4("viewMatrix", App->camera->GetCurrentCamera()->GetOGLViewMatrix());

			cMaterial->GetShader()->SetUniformMatrix4("projectionMatrix", App->camera->GetCurrentCamera()->GetOGLProjectionMatrix());

			cMaterial->GetShader()->SetUniform1f("time", Time::Game::GetTimeSinceStart());

			cMaterial->GetShader()->SetUniform1i("skybox", 11);

			cMaterial->GetShader()->SetUniformVec3f("cameraPosition", (GLfloat*)&App->camera->GetCurrentCamera()->GetFrustum().Pos());

			Importer::Shaders::SetShaderUniforms(cMaterial->GetShader());
		}
	}
}

uint32 MeshRenderer::SetDefaultShader(C_Material* cMaterial)
{
	R_Shader* rShader = nullptr;

	rShader = App->resourceManager->GetDefaultShader();	

	cMaterial->SetShader(rShader);

	return cMaterial->GetShaderProgramID();
}

void MeshRenderer::ClearShader()
{
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glUseProgram(0);
}



// --- CUBOID RENDERER METHODS
CuboidRenderer::CuboidRenderer(const float3* vertices, const Color& color, const float& edgeWidth) :
vertices	(vertices),
type		(CuboidType::NONE),
color		(color),
edgeWidth	(edgeWidth)
{

}

CuboidRenderer::CuboidRenderer(const float3* vertices, CuboidType type) :
vertices	(vertices),
type		(type),
color		(GetColorByType()),
edgeWidth	(GetEdgeWidthByType())
{

}

void CuboidRenderer::Render()
{
	// For a Cuboid with vertices ABCDEFGH
	GLfloat* A = (GLfloat*)&vertices[0];
	GLfloat* B = (GLfloat*)&vertices[1];
	GLfloat* C = (GLfloat*)&vertices[2];
	GLfloat* D = (GLfloat*)&vertices[3];
	GLfloat* E = (GLfloat*)&vertices[4];
	GLfloat* F = (GLfloat*)&vertices[5];
	GLfloat* G = (GLfloat*)&vertices[6];
	GLfloat* H = (GLfloat*)&vertices[7];

	glColor4f(color.r, color.g, color.b, color.a);
	glLineWidth(edgeWidth);
	glBegin(GL_LINES);

	// --- FRONT
	glVertex3fv(A);											// BOTTOM HORIZONTAL										// Firstly the Near Plane is constructed.
	glVertex3fv(B);											// -----------------
	glVertex3fv(D);											// TOP HORIZONTAL
	glVertex3fv(C);											// -------------

	glVertex3fv(B);											// LEFT VERTICAL
	glVertex3fv(D);											// -------------
	glVertex3fv(C);											// RIGHT VERTICAL
	glVertex3fv(A);											// --------------

	// --- BACK
	glVertex3fv(F);											// BOTTOM HORIZONTAL										// Secondly the Far Plane is constructed.
	glVertex3fv(E);											// -----------------
	glVertex3fv(G);											// TOP HORIZONTAL
	glVertex3fv(H);											// -------------- 

	glVertex3fv(E);											// LEFT VERTICAL 
	glVertex3fv(G);											// ------------- 
	glVertex3fv(H);											// RIGHT VERTICAL 
	glVertex3fv(F);											// -------------- 

	// --- RIGHT
	glVertex3fv(F);											// BOTTOM HORIZONTAL										// Lastly, the Near and Far Planes' corners are connected.
	glVertex3fv(B); 										// -----------------
	glVertex3fv(H); 										// TOP HORIZONTAL 
	glVertex3fv(D); 										// -------------- 
	
	// --- LEFT
	glVertex3fv(E);											// BOTTOM HORIZONTAL										// ---
	glVertex3fv(A);											// -----------------
	glVertex3fv(C);											// TOP HORIZONTAL 
	glVertex3fv(G);											// -------------- 

	glEnd();
	glLineWidth(STANDARD_LINE_WIDTH);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

Color CuboidRenderer::GetColorByType()
{
	switch (type)
	{
	case CuboidType::NONE:		{ return White; }								break;
	case CuboidType::AABB:		{ return App->renderer->GetAABBColor(); }		break;
	case CuboidType::OBB:		{ return App->renderer->GetOBBColor(); }		break;
	case CuboidType::FRUSTUM:	{ return App->renderer->GetFrustumColor(); }	break;
	}

	return White;
}

float CuboidRenderer::GetEdgeWidthByType()
{
	switch (type)
	{
	case CuboidType::NONE:		{ return BASE_LINE_WIDTH; }							break;
	case CuboidType::AABB:		{ return App->renderer->GetAABBEdgeWidth(); }		break;
	case CuboidType::OBB:		{ return App->renderer->GetOBBEdgeWidth(); }		break;
	case CuboidType::FRUSTUM:	{ return App->renderer->GetFrustumEdgeWidth(); }	break;
	}

	return STANDARD_LINE_WIDTH;
}

// --- RAY RENDERER METHODS
RayRenderer::RayRenderer(const LineSegment& ray, const Color& color, const float& rayWidth) :
ray			(ray),
color		(color),
rayWidth	(rayWidth)
{
	
}

RayRenderer::RayRenderer(const LineSegment& ray) : 
ray			(ray),
color		(App->renderer->GetRayColor()),
rayWidth	(App->renderer->GetRayWidth())
{

}

void RayRenderer::Render()
{
	GLfloat A[3] = { ray.a.x, ray.a.y , ray.a.z };
	GLfloat B[3] = { ray.b.x, ray.b.y , ray.b.z };

	glColor4f(color.r, color.g, color.b, color.a);

	glLineWidth(rayWidth);
	glBegin(GL_LINES);

	glVertex3fv(A);
	glVertex3fv(B);

	glEnd();
	glLineWidth(STANDARD_LINE_WIDTH);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

// --- SKELETON RENDERER METHODS
SkeletonRenderer::SkeletonRenderer(const std::vector<LineSegment>& bones, const Color& color, const float& boneWidth) :
bones		(bones),
color		(color),
boneWidth	(boneWidth)
{

}

SkeletonRenderer::SkeletonRenderer(const std::vector<LineSegment>& bones) :
bones		(bones),
color		(App->renderer->GetBoneColor()),
boneWidth	(App->renderer->GetBoneWidth())
{

}

void SkeletonRenderer::Render()
{
	GLfloat A[3]	= {};
	GLfloat B[3]	= {};
	uint bytes		= sizeof(float) * 3;

	glColor4f(color.r, color.g, color.b, color.a);
	glLineWidth(boneWidth);
	glBegin(GL_LINES);

	for (uint i = 0; i < bones.size(); ++i)
	{
		memcpy(A, (const void*)bones[i].a.ptr(), bytes);
		memcpy(B, (const void*)bones[i].b.ptr(), bytes);

		glVertex3fv(A);
		glVertex3fv(B);
	}

	glEnd();
	glLineWidth(STANDARD_LINE_WIDTH);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}