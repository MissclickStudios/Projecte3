#include <algorithm>

#include "JSONParser.h"
#include "Profiler.h"													
//#include "OpenGL.h"
#include "Dependencies/glew/include/glew.h"

#include "MC_Time.h"

#include "Macros.h"														
#include "Log.h"														
												
#include "Icons.h"														
#include "Primitive.h"
#include "Light.h"
#include "DirectionalLight.h"

#include "Application.h"												
#include "M_Window.h"													
#include "M_Camera3D.h"													
#include "M_Input.h"													
#include "M_FileSystem.h"												
#include "M_ResourceManager.h"																								
#include "M_Scene.h"
#include "M_Detour.h"
#include "M_UISystem.h"

#include "R_Mesh.h"														
#include "R_Material.h"													
#include "R_Texture.h"
#include "R_Shader.h"
#include "R_NavMesh.h"

#include "I_Textures.h"													
#include "I_Shaders.h"							//TODO: erase

#include "GameObject.h"	
#include "C_Transform.h"
#include "C_Mesh.h"														
#include "C_Material.h"													
#include "C_Camera.h"	
#include "C_Light.h"
#include "C_Canvas.h"
#include "C_UI_Image.h"
#include "C_UI_Text.h"
#include "C_UI_Button.h"

#include "Renderer.h"
#include "RE_Circle.h"

#include "M_Renderer3D.h"

#include "MemoryManager.h"

#include "MathGeoLib/include/Math/float4x4.h"

#pragma comment (lib, "glu32.lib")    /* link OpenGL Utility lib     */	
#pragma comment (lib, "opengl32.lib") /* link Microsoft OpenGL lib   */	
#pragma comment (lib, "Source/Dependencies/Assimp/assimp-vc142-mt.lib")	
#pragma comment (lib, "Source/Dependencies/glew/libx86/glew32.lib")

#define WORLD_GRID_SIZE		64
#define CHECKERS_WIDTH		64
#define CHECKERS_HEIGHT		64
#define STANDARD_LINE_WIDTH	1.0f
#define BASE_LINE_WIDTH		3.0f

M_Renderer3D::M_Renderer3D(bool isActive) : Module("Renderer3D", isActive), 
context				(),
vsync				(VSYNC),
sceneFramebuffer	(0),
depthBuffer			(0),
sceneRenderTexture	(0),
depthBufferTexture	(0),
gameFramebuffer		(0),
debugTextureId		(0)
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

	//CreatePrimitiveExamples();																	// Adding one of each available primitice to the primitives vector for later display.

	//InitFramebuffers();
	LoadDebugTexture();

	SetVsync(configuration.GetBool("Vsync"));

	renderWorldGrid		= configuration.GetBool("renderWorldGrid");
	renderWorldAxis		= configuration.GetBool("renderWorldAxis");
	renderWireframes	= configuration.GetBool("renderWireFrame");
	renderVertexNormals = configuration.GetBool("renderVertexNormals");
	renderFaceNormals	= configuration.GetBool("renderFaceNormals");
	renderBoundingBoxes = configuration.GetBool("renderBoundingBoxes");
	renderSkeletons		= configuration.GetBool("renderSkeletons");
	renderColliders		= configuration.GetBool("renderColliders");
	renderCanvas		= configuration.GetBool("renderCanvas");

	worldGridColor		= configuration.GetColor("worldGridColor");
	wireframeColor		= configuration.GetColor("wireframeColor");
	vertexNormalsColor	= configuration.GetColor("vertexNormalsColor");
	faceNormalsColor	= configuration.GetColor("faceNormalsColor");

	aabbColor			= configuration.GetColor("aabbColor");
	obbColor			= configuration.GetColor("obbColor");
	frustumColor		= configuration.GetColor("frustumColor");
	rayColor			= configuration.GetColor("rayColor");
	boneColor			= configuration.GetColor("boneColor");

	if (App->gameState == GameState::PLAY)
	{
		renderWorldGrid		= false;
		renderWorldAxis		= false;
		renderWireframes	= false;
		renderVertexNormals = false;
		renderFaceNormals	= false;
		renderBoundingBoxes = false;
		renderSkeletons		= false;
		renderColliders		= false;
		renderCanvas		= false;
	}

	return ret;
}

bool M_Renderer3D::Start()
{
	InitEngineIcons();

	//SetUp the Skybox 
	defaultSkyBox.SetUpSkyBoxBuffers();

	defaultSkyBox.CreateSkybox();

	GenScreenBuffer();

	particleShader = App->resourceManager->GetShader("ParticleShader");

	return true;
}

// PreUpdate: clear buffer
UpdateStatus M_Renderer3D::PreUpdate(float dt)
{	
	OPTICK_CATEGORY("M_Renderer3D PreUpdate", Optick::Category::Module)

	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
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

	// --- RENDERER SHORTCUTS
	if (App->gameState != GameState::PLAY)
		RendererShortcuts();

	return UpdateStatus::CONTINUE;
}

// PostUpdate present buffer to screen
UpdateStatus M_Renderer3D::PostUpdate(float dt)
{	
	OPTICK_CATEGORY("M_Renderer3D PostUpdate", Optick::Category::Module);

	RenderScene();

	glClear(GL_COLOR_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D, gameFramebuffer);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex2f(0, 0);
	glTexCoord2f(1, 0);
	glVertex2f(500, 0);
	glTexCoord2f(1, 1);
	glVertex2f(500, 500);
	glTexCoord2f(0, 1);
	glVertex2f(0, 500);
	glEnd();
	glFlush();

	//Render systems from other modules (example ImGUi)
	for (std::vector<Module*>::const_iterator it = PostSceneRenderModules.cbegin(); it != PostSceneRenderModules.cend(); ++it) 
	{
		if ((*it)->IsActive())
			(*it)->PostSceneRendering();
	}

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
	vsync = root.GetBool("Vsync");

	return true;
}

bool M_Renderer3D::SaveConfiguration(ParsonNode& root) const
{
	root.SetBool("Vsync", vsync);

	uint	worldGridSize;																		//
	
	root.SetFloat4("worldGridColor", float4(&worldGridColor));
	root.SetFloat4("wireframeColor", float4(&wireframeColor));
	root.SetFloat4("vertexNormalsColor", float4(&vertexNormalsColor));
	root.SetFloat4("faceNormalsColor", float4(&faceNormalsColor));

	root.SetFloat4("aabbColor", float4(&aabbColor));
	root.SetFloat4("obbColor", float4(&obbColor));
	root.SetFloat4("frustumColor", float4(&frustumColor));
	root.SetFloat4("rayColor", float4(&rayColor));
	root.SetFloat4("boneColor", float4(&boneColor));

	root.SetBool("renderWorldGrid",renderWorldGrid);	
	root.SetBool("renderWorldAxis", renderWorldAxis);
	root.SetBool("renderWireFrame", renderWireframes);
	root.SetBool("renderVertexNormals", renderVertexNormals);
	root.SetBool("renderFaceNormals", renderFaceNormals);
	root.SetBool("renderBoundingBoxes", renderBoundingBoxes);
	root.SetBool("renderSkeletons", renderSkeletons);
	root.SetBool("renderColliders", renderColliders);
	root.SetBool("renderCanvas", renderCanvas);

	return true;
}

// ----------- RENDERER METHODS -----------
bool M_Renderer3D::InitDebugVariables()
{
	bool ret = true;
	
	worldGridSize			= WORLD_GRID_SIZE;
	
	worldGridColor			= White;
	wireframeColor			= Yellow;
	vertexNormalsColor		= Yellow;													// TODO: Use other color?
	faceNormalsColor		= Magenta;
	aabbColor				= Green;
	obbColor				= Orange;
	frustumColor			= Red;
	rayColor				= Cyan;
	boneColor				= Pink;

	worldGridLineWidth		= STANDARD_LINE_WIDTH;
	wireframeLineWidth		= STANDARD_LINE_WIDTH;
	vertexNormalsWidth		= BASE_LINE_WIDTH;
	faceNormalsWidth		= BASE_LINE_WIDTH;

	aabbEdgeWidth			= BASE_LINE_WIDTH;
	obbEdgeWidth			= BASE_LINE_WIDTH;
	frustumEdgeWidth		= BASE_LINE_WIDTH;
	rayWidth				= BASE_LINE_WIDTH;
	boneWidth				= BASE_LINE_WIDTH;

	renderWorldGrid			= true;	
	renderWorldAxis			= true;
	renderWireframes		= false;
	renderVertexNormals		= false;
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

	ret = InitGlew();																										// Initializing Glew.

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
			LOG("[ERROR] Error initializing OpenGL! %s\n", glewGetErrorString(error));
			ret = false;
		}

		//Initialize Modelview Matrix
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		//Check for error
		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			LOG("[ERROR] Error initializing OpenGL! %s\n", glewGetErrorString(error));
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
			LOG("[ERROR] Error initializing OpenGL! %s\n", glewGetErrorString(error));
			ret = false;
		}

		GLfloat LightModelAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LightModelAmbient);

		GLfloat MaterialAmbient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, MaterialAmbient);

		GLfloat MaterialDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialDiffuse);

		SetGLFlag(GL_DEPTH_TEST, true);
		SetGLFlag(GL_CULL_FACE, false);
		SetGLFlag(GL_LIGHTING, true);
		SetGLFlag(GL_COLOR_MATERIAL, true);
		SetGLFlag(GL_TEXTURE_2D, true);

		SetGLFlag(GL_ALPHA_TEST, false);
		glAlphaFunc(GL_GREATER, 0.20f);													// Have alpha test in c_material (color alpha)?

		SetGLFlag(GL_BLEND, true);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_LIGHTING);

		//Outline Settings
		//glDepthFunc(GL_LESS);
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
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
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthBufferTexture, 0);

	// --- DEPTH & STENCIL BUFFERS ---
	glGenRenderbuffers(1, (GLuint*)&depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, App->window->GetWidth(), App->window->GetHeight());
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		LOG("[ERROR] Renderer 3D: Could not generate the scene's frame buffer! Error: %s", glewGetErrorString(glGetError()));

	// --- UNBINDING THE FRAMEBUFFER ---
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void M_Renderer3D::LoadDebugTexture()
{
	GLubyte checkerImage[CHECKERS_HEIGHT][CHECKERS_WIDTH][4];							// HEIGHT columns, WIDTH rows and 4 variables per checker (for RGBA purposes).

	for (int i = 0; i < CHECKERS_HEIGHT; ++i)											// There will be CHECKERS_WIDTH rows per column.
	{
		for (int j = 0; j < CHECKERS_WIDTH; ++j)										// There will be an RGBA value per checker.
		{
			int color = ((((i & 0x8) == 0) ^ ((j & 0x8) == 0))) * 255;					// Getting the checker's color (white or black) according to the iteration indices and bitwise ops.

			checkerImage[i][j][0] = (GLubyte)color;										// R
			checkerImage[i][j][1] = (GLubyte)color;										// G
			checkerImage[i][j][2] = (GLubyte)color;										// B
			checkerImage[i][j][3] = (GLubyte)255;										// A
		}
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);												// Sets the pixel storage modes. GL_UNPACK_ALIGNMENT specifies the alignment requirements for the
	// --->																				// start of each pixel row in memory. 1 means that the alignment requirements will be byte-alignment.
	glGenTextures(1, &debugTextureId);													// Generate texture names. Returns n names in the given buffer. GL_INVALID_VALUE if n is negative.
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
	if (App->input->GetKey(SDL_SCANCODE_LCTRL) == KeyState::KEY_REPEAT)
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
}

void M_Renderer3D::RenderScene()
{	
	OPTICK_CATEGORY("Render Scene", Optick::Category::Rendering)

	glBindFramebuffer(GL_FRAMEBUFFER, sceneFramebuffer);
	glClear(GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_STENCIL_BUFFER_BIT);

	defaultSkyBox.RenderSkybox();

	if (renderWorldGrid)
		DrawWorldGrid(worldGridSize);

	if (renderWorldAxis)
		DrawWorldAxis();

	for (auto renderer = renderers.begin(); renderer != renderers.end(); ++renderer)
	{
		renderer->second->Render();

		renderer->second->CleanUp();
		RELEASE(renderer->second);
	}

	renderers.clear();
	
	// TMP
	/*static float4x4 dbTrnsfrm		= float4x4::FromTRS(float3(0.0f, 10.0f, 0.0f), Quat::FromEulerXYZ(90.0f * DEGTORAD, 0.0f, 0.0f), float3::one);
	static RE_Circle debugCircle	= RE_Circle(dbTrnsfrm, float3(0.0f, 10.0f, 0.0f), 5.0f, 20, 2.0f);
	debugCircle.Render();*/
	
	/*RenderMeshes();
	RenderCuboids();
	//RenderRays();
	RenderSkeletons();
	RenderParticles();*/

	
	if (App->camera->DrawLastRaycast())
	{
		RayRenderer last_ray = RayRenderer(App->camera->lastRaycast, rayColor, rayWidth);
		last_ray.Render();
	}

	if (App->detour->debugDraw && App->detour->navMeshResource != nullptr && App->detour->navMeshResource->navMesh != nullptr) {
		if (App->detour->renderMeshes.data() != nullptr)
		{
			R_Shader* shaderProgram = App->resourceManager->GetShader("DefaultShader");;

			for (int i = 0; i < App->detour->renderMeshes.size(); ++i)
			{

				glUseProgram(shaderProgram->shaderProgramID);

				shaderProgram->SetUniformMatrix4("modelMatrix", (GLfloat*)float4x4::identity.ptr());

				shaderProgram->SetUniformMatrix4("viewMatrix", App->camera->GetCurrentCamera()->GetViewMatrixTransposed().ptr());

				shaderProgram->SetUniformMatrix4("projectionMatrix", App->camera->GetCurrentCamera()->GetProjectionMatrixTransposed().ptr());

				shaderProgram->SetUniformVec3f("cameraPosition", (GLfloat*)&App->camera->GetCurrentCamera()->GetFrustum().Pos());

				glBindVertexArray(App->detour->renderMeshes[i]->rmesh->VAO);
				glDrawElements(GL_TRIANGLES, App->detour->renderMeshes[i]->rmesh->indices.size(), GL_UNSIGNED_INT, nullptr);
				glUseProgram(0);
				glBindVertexArray(0);	
			}
		}
	}

	//PrimitiveDrawExamples p_ex = PrimitiveDrawExamples();
	//p_ex.DrawAllExamples();

	for (uint i = 0; i < primitives.size(); ++i)
	{
		primitives[i]->RenderByIndices();
	}

	RenderUI();

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	
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

void M_Renderer3D::SetTo2DRenderSettings(const bool& setTo)
{
	if (setTo)
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-App->camera->GetCurrentCamera()->GetFrustum().NearPlaneWidth() / 2, App->camera->GetCurrentCamera()->GetFrustum().NearPlaneWidth() / 2, -App->camera->GetCurrentCamera()->GetFrustum().NearPlaneHeight() / 2, App->camera->GetCurrentCamera()->GetFrustum().NearPlaneHeight() / 2, 100.0f, -100.0f);
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(App->camera->GetCurrentCamera()->GetOGLViewMatrix());

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		glEnable(GL_BLEND);
	}
	else
	{
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(App->camera->GetCurrentCamera()->GetOGLProjectionMatrix());
		glMatrixMode(GL_MODELVIEW);

		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
	}
}

void M_Renderer3D::RenderUI()
{
	OPTICK_CATEGORY("RenderUI", Optick::Category::Rendering)
	SetTo2DRenderSettings(true);
	/*
	if (App->camera->currentCamera != App->camera->masterCamera->GetComponent<C_Camera>())
		canvas->Draw2D();
	else
		canvas->Draw3D();
	*/
	if (App->gameState != GameState::PLAY)
	{
		const std::vector<C_Canvas*> canvasToDraw = App->uiSystem->GetAllCanvas();
		for (std::vector<C_Canvas*>::const_iterator it = canvasToDraw.cbegin(); it != canvasToDraw.cend(); ++it)
		{
			//TODO: DRAW THE UI ELEMENTS ON 3D
			// SetTo2DRenderSettings(false);
			//(*rit)->Draw3D(renderCanvas);
			//SetTo2DRenderSettings(false);
			if ((*it)->debugDraw)
				(*it)->Draw2D(renderCanvas);
		}
		SetTo2DRenderSettings(false);
		return;
	}
	std::list<C_Canvas*> canvasToDraw = App->uiSystem->GetActiveCanvas();
	for (std::list<C_Canvas*>::reverse_iterator rit = canvasToDraw.rbegin(); rit != canvasToDraw.rend(); ++rit)
	{
		//TODO: DRAW THE UI ELEMENTS ON 3D
		// SetTo2DRenderSettings(false);
		//(*rit)->Draw3D(renderCanvas);
		//SetTo2DRenderSettings(false);
		//if ((*rit)->IsActive())
		(*rit)->Draw2D(renderCanvas);
	}
	SetTo2DRenderSettings(false);
}

void M_Renderer3D::RenderFramebufferTexture()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glViewport(0, 0, App->window->GetWidth(), App->window->GetHeight());
	glClear(GL_COLOR_BUFFER_BIT);

	if (!screenShader) screenShader = App->resourceManager->GetShader("ScreenShader");

	glUseProgram(screenShader->shaderProgramID);

	App->scene->DoSceneTransition(screenShader, 1);
	
	glBindVertexArray(quadScreenVAO);
	glDisable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D, sceneRenderTexture);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	glEnable(GL_DEPTH_TEST);
}

void M_Renderer3D::DrawWorldAxis()
{
	glLineWidth(2.0f);

	glBegin(GL_LINES);

	glColor4f(1.0f, 0.0f, 0.0f, 1.0f);											// X Axis.
	glVertex3f(0.0f, 0.0f, 0.0f);		glVertex3f(1.0f, 0.0f, 0.0f);
	glVertex3f(1.0f, 0.1f, 0.0f);		glVertex3f(1.1f, -0.1f, 0.0f);
	glVertex3f(1.1f, 0.1f, 0.0f);		glVertex3f(1.0f, -0.1f, 0.0f);

	glColor4f(0.0f, 1.0f, 0.0f, 1.0f);											// Y Axis.
	glVertex3f(0.0f, 0.0f, 0.0f);	glVertex3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-0.05f, 1.25f, 0.0f);	glVertex3f(0.0f, 1.15f, 0.0f);
	glVertex3f(0.05f, 1.25f, 0.0f);	glVertex3f(0.0f, 1.15f, 0.0f);
	glVertex3f(0.0f, 1.15f, 0.0f);	glVertex3f(0.0f, 1.05f, 0.0f);

	glColor4f(0.0f, 0.0f, 1.0f, 1.0f);											// Z Axis.
	glVertex3f(0.0f, 0.0f, 0.0f);	glVertex3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-0.05f, 0.1f, 1.05f);	glVertex3f(0.05f, 0.1f, 1.05f);
	glVertex3f(0.05f, 0.1f, 1.05f);	glVertex3f(-0.05f, -0.1f, 1.05f);
	glVertex3f(-0.05f, -0.1f, 1.05f);	glVertex3f(0.05f, -0.1f, 1.05f);

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

void M_Renderer3D::AddRenderersBatch(const std::multimap<float, Renderer*>& renderers)
{
	this->renderers = renderers;
}

void M_Renderer3D::RenderMeshes()
{	
	C_Camera* currentCamera = App->camera->GetCurrentCamera();
	float3 cameraPos = (currentCamera != nullptr) ? currentCamera->GetFrustum().Pos() : float3::zero;
	/*std::multimap<float, MeshRenderer> sortedRenderers;
	for (auto mRenderer = meshRenderers.cbegin(); mRenderer != meshRenderers.cend(); ++mRenderer)
	{
		float distanceToCamera = (*mRenderer).transform->GetWorldPosition().DistanceSq(cameraPos);
		sortedRenderers.emplace(distanceToCamera, (*mRenderer));
	}*/
	
	//std::sort(meshRenderers.begin(), meshRenderers.end(), [&cameraPos](MeshRenderer mRendererA, MeshRenderer mRendererB) { return (mRendererA.transform->GetWorldPosition().Distance(cameraPos)) > (mRendererB.transform->GetWorldPosition().Distance(cameraPos)); });
	std::sort(meshRenderers.begin(), meshRenderers.end(), [&cameraPos](MeshRenderer mRendererA, MeshRenderer mRendererB) { return (mRendererA.transform->GetWorldPosition().y) < (mRendererB.transform->GetWorldPosition().y); });
	
	for (uint i = 0; i < meshRenderers.size(); ++i)
	{

		if(meshRenderers[i].cMesh->GetOutlineMesh())
			meshRenderers[i].Render(true);
		else
			meshRenderers[i].Render(false);
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

void M_Renderer3D::AddParticle(const float4x4& transform, R_Texture* material, Color color, float distanceToCamera)
{
	ParticleRenderer pRenderer = ParticleRenderer(material, color, transform);
	pRenderer.shader = particleShader;
	particles.insert(std::map<float, ParticleRenderer>::value_type(distanceToCamera, pRenderer));
}

void M_Renderer3D::RenderParticles()
{
	std::map<float, ParticleRenderer>::reverse_iterator it;				//Render from far to close to the camera
	for (it = particles.rbegin(); it != particles.rend(); ++it)			
	{
		//DrawParticle(it->second);
		if(it->second.mat != nullptr)
			it->second.Render();
	}
	particles.clear();
}

void M_Renderer3D::DrawParticle(ParticleRenderer& renderParticle)
{
	glPushMatrix();
	glMultMatrixf((float*)&renderParticle.transform);

	////Binding particle Texture
	//if (renderParticle.mat)
	//{
	//	if (R_Texture* rTex = renderParticle.mat.)
	//	{
	//		if (rTex && rTex->buffer != 0)
	//		{
	//			glBindTexture(GL_TEXTURE_2D, rTex->buffer);
	//		}
	//	}
	//}

	glColor4f(0.0f, 1.0f, 1.0f, 1.0f);

	//Drawing to tris in direct mode
	glBegin(GL_TRIANGLES);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(.5f, -.5f, .0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-.5f, .5f, .0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-.5f, -.5f, .0f);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(.5f, -.5f, .0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(.5f, .5f, .0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-.5f, .5f, .0f);

	glEnd();
	glPopMatrix();
	glBindTexture(GL_TEXTURE_2D, 0);
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
	renderers.clear();
	
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

	vsync ? App->framesAreCapped = false : App->framesAreCapped = true;
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
	return renderVertexNormals;
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

bool M_Renderer3D::GetRenderColliders() const
{
	return renderColliders;
}

bool M_Renderer3D::GetRenderCanvas() const
{
	return renderCanvas;
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
			//SetGLFlag(GL_LIGHTING, false);
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
	renderVertexNormals = setTo;
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

void M_Renderer3D::SetRenderColliders(const bool& setTo)
{
	renderColliders = setTo;
}

void M_Renderer3D::SetRenderCanvas(const bool& setTo)
{
	renderCanvas = setTo;
}

void M_Renderer3D::AddPostSceneRenderModule(Module* module)
{
	PostSceneRenderModules.push_back(module);
}

GameObject* M_Renderer3D::GenerateSceneLight(Color diffuse, Color ambient, Color specular, LightType lightType)
{
	//Create GameObject
	GameObject* light;
	std::string name;
	switch (lightType)
	{
	case LightType::DIRECTIONAL:	{ name = "DirectionalLight"; }	break;
	case LightType::POINTLIGHT:		{ name = "PointLight"; }		break;
	case LightType::SPOTLIGHT:		{ name = "SpotLight"; }			break;
	case LightType::NONE:			{ name = ""; }					break;
	}

	if (App->scene->SceneHasLights())
	{
		std::vector<GameObject*> pointLights;																		// ATTENTION: Only checking point lights? Shouldn't it be all lights?
		App->scene->GetPointLights(pointLights);

		name += std::to_string(pointLights.size());

		pointLights.clear();
	}

	light = App->scene->CreateGameObject(name.c_str(), App->scene->GetSceneRoot());

	//Create and Add Component
	Component* component = nullptr;
	component = new C_Light(light, lightType);
	light->components.push_back(component);

	//Set the light attributes
	C_Light* lightComp = light->GetComponent<C_Light>();
	switch (lightType)
	{
	case LightType::DIRECTIONAL: 
		lightComp->GetDirectionalLight()->Active(true);
		lightComp->GetDirectionalLight()->ambient = ambient;
		lightComp->GetDirectionalLight()->diffuse = diffuse;
		lightComp->GetDirectionalLight()->specular = specular;
		lightComp->GetDirectionalLight()->Init();
		break;
	case LightType::POINTLIGHT: 
		lightComp->GetPointLight()->Active(true);
		lightComp->GetPointLight()->ambient = ambient;
		lightComp->GetPointLight()->diffuse = diffuse;
		lightComp->GetPointLight()->specular = specular;
		lightComp->GetPointLight()->SetConstant(1.0f);
		lightComp->GetPointLight()->SetLinear(0.09f);
		lightComp->GetPointLight()->SetQuadratic(0.032f);
		lightComp->GetPointLight()->Init();
		break;
	case LightType::SPOTLIGHT: break;
	case LightType::NONE: break;
	}
	//App->scene->AddSceneLight(light);
	
	return light;
}

void M_Renderer3D::GenScreenBuffer()
{
	glGenVertexArrays(1, &quadScreenVAO);
	glBindVertexArray(quadScreenVAO);

	const GLfloat quadVertexBufferData[] = {
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f
	};

	GLuint quadVertexbuffer;
	glGenBuffers(1, &quadVertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quadVertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertexBufferData), quadVertexBufferData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));

}

// --- RENDERER STRUCTURES METHODS ---
// --- MESH RENDERER METHODS
MeshRenderer::MeshRenderer(C_Transform* transform, C_Mesh* cMesh,  C_Material* cMaterial) :
transform	(transform),
cMesh		(cMesh),
cMaterial	(cMaterial)
{

}

void MeshRenderer::Render(bool outline)
{
	R_Mesh* rMesh = cMesh->GetMesh();
	
	std::string name = transform->GetOwner()->GetName();

	if (strcmp(transform->GetOwner()->GetName(), "Blaster") == 0)
	{
		LOG("BRUH");
	}

	if (rMesh == nullptr)
	{
		LOG("[ERROR] Renderer 3D: Could not render Mesh! Error: R_Mesh* was nullptr.");
		return;
	}

	glEnable(GL_BLEND);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glClear(GL_STENCIL_BUFFER_BIT);

	if (outline)
	{
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
	}
	else
	{
		glStencilMask(0x00);
	}

	ApplyDebugParameters();																									// Enable Wireframe Mode for this specific mesh, etc.
	ApplyTextureAndMaterial();																								// Apply resource texture or default texture, mesh color...
	ApplyShader();																											// 

	if (cMesh->GetSkinnedMesh() == nullptr)
	{
		glBindVertexArray(rMesh->VAO);
		glDrawElements(GL_TRIANGLES, rMesh->indices.size(), GL_UNSIGNED_INT, nullptr);
	}
	else
	{
		glBindVertexArray(cMesh->GetSkinnedMesh()->VAO);
		glDrawElements(GL_TRIANGLES, cMesh->GetSkinnedMesh()->indices.size(), GL_UNSIGNED_INT, nullptr);
	}
	if (outline) RenderOutline(rMesh);

	ClearShader();	

	glBindVertexArray(0);																									//

	glBindTexture(GL_TEXTURE_2D, 0);																						// ---------------------
	
	ClearTextureAndMaterial();																								// Clear the specifications applied in ApplyTextureAndMaterial().
	ClearDebugParameters();																									// Clear the specifications applied in ApplyDebugParameters().

	/*glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);*/
	

	// --- DEBUG DRAW ---
	if (rMesh->drawVertexNormals || App->renderer->GetRenderVertexNormals())
	{
		RenderVertexNormals(rMesh);
	}

	if (rMesh->drawFaceNormals || App->renderer->GetRenderFaceNormals())
	{
		RenderFaceNormals(rMesh);
	}
}

void MeshRenderer::RenderOutline(R_Mesh* rMesh)
{
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilMask(0x00);

	uint32 shaderProgram = 0;
	if (cMaterial != nullptr)
	{
		R_Shader* tempShader;
		tempShader = App->resourceManager->GetShader("OutlineShader");

		tempShader ? shaderProgram = tempShader->shaderProgramID : shaderProgram;

		glUseProgram(shaderProgram);

		if (shaderProgram != 0)
		{

			tempShader->SetUniform1f("outlineThickness", cMesh->GetOutlineThickness());

			tempShader->SetUniformVec4f("outlineColor", (GLfloat*)&cMesh->GetOutlineColor());

			tempShader->SetUniformMatrix4("modelMatrix", transform->GetWorldTransform().Transposed().ptr());

			tempShader->SetUniformMatrix4("viewMatrix", App->camera->GetCurrentCamera()->GetViewMatrixTransposed().ptr());

			tempShader->SetUniformMatrix4("projectionMatrix", App->camera->GetCurrentCamera()->GetProjectionMatrixTransposed().ptr());

			//Animations

			cMesh->GetBoneTranforms(boneTransforms);

			bool check = cMesh->GetSkinnedMesh() != nullptr;

			tempShader->SetUniform1i("activeAnimation", (check));

			if (!boneTransforms.empty())
			{
				tempShader->SetUniformMatrix4("finalBonesMatrices", (GLfloat*)&boneTransforms[0], boneTransforms.size());
			}

			boneTransforms.clear();
		}
	}

	if (cMesh->GetSkinnedMesh() == nullptr)
	{
		glBindVertexArray(rMesh->VAO);
		glDrawElements(GL_TRIANGLES, rMesh->indices.size(), GL_UNSIGNED_INT, nullptr);
	}
	else
	{
		glBindVertexArray(cMesh->GetSkinnedMesh()->VAO);
		glDrawElements(GL_TRIANGLES, cMesh->GetSkinnedMesh()->indices.size(), GL_UNSIGNED_INT, nullptr);
	}


	glStencilMask(0xFF);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
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
		//glDisable(GL_LIGHTING);
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
		std::vector<GameObject*> dirLights;
		std::vector<GameObject*> pointLights;
		App->scene->GetDirLights(dirLights);
		App->scene->GetPointLights(pointLights);


		cMaterial->GetShader() ? shaderProgram = cMaterial->GetShader()->shaderProgramID : shaderProgram;

		shaderProgram ? shaderProgram : shaderProgram = SetDefaultShader(cMaterial);

		glUseProgram(shaderProgram);

		cMaterial->GetTexture() ? cMaterial->GetShader()->SetUniform1i("hasTexture", (GLint)true) : cMaterial->GetShader()->SetUniform1i("hasTexture", (GLint)false);

		cMaterial->GetTakeDamage() ? cMaterial->GetShader()->SetUniform1i("takeDamage", (GLint)true) : cMaterial->GetShader()->SetUniform1i("takeDamage", (GLint)false);
		
		!dirLights.empty() ? cMaterial->GetShader()->SetUniform1i("useDirLight", (GLint)true) : cMaterial->GetShader()->SetUniform1i("useDirLight", (GLint)false);

		if (shaderProgram != 0)
		{
			//Model
			
			cMaterial->GetShader()->SetUniformVec4f("inColor", (GLfloat*)&cMaterial->GetMaterialColour());

			cMaterial->GetShader()->SetUniformVec4f("alternateColor", (GLfloat*)&cMaterial->GetAlternateColour());

			cMaterial->GetShader()->SetUniformMatrix4("modelMatrix", transform->GetWorldTransform().Transposed().ptr());

			cMaterial->GetShader()->SetUniformMatrix4("viewMatrix", App->camera->GetCurrentCamera()->GetViewMatrixTransposed().ptr());

			cMaterial->GetShader()->SetUniformMatrix4("projectionMatrix", App->camera->GetCurrentCamera()->GetProjectionMatrixTransposed().ptr());

			cMaterial->GetShader()->SetUniformVec3f("cameraPosition", (GLfloat*)&App->camera->GetCurrentCamera()->GetFrustum().Pos());
			

			cMaterial->GetShader()->SetUniform1f("deltaTime", MC_Time::Game::GetDT());

			cMaterial->GetShader()->SetUniform1f("Time", MC_Time::Game::GetTimeSinceStart());

 			//Skybox
			
			cMaterial->GetShader()->SetUniform1i("skybox", 11);


			//ANimations

			cMesh->GetBoneTranforms(boneTransforms);
			
			bool check = cMesh->GetSkinnedMesh() != nullptr;

			cMaterial->GetShader()->SetUniform1i("activeAnimation", (check));

			if (!boneTransforms.empty())
			{				
				cMaterial->GetShader()->SetUniformMatrix4("finalBonesMatrices", (GLfloat*)&boneTransforms[0], boneTransforms.size());
			}

			boneTransforms.clear();

			// Light 

			if (!dirLights.empty())
			{
				for (uint i = 0; i < dirLights.size(); i++)
				{
						cMaterial->GetShader()->SetUniformVec4f("dirLight.diffuse", (GLfloat*)&dirLights[i]->GetComponent<C_Light>()->GetDirectionalLight()->diffuse);
						cMaterial->GetShader()->SetUniformVec4f("dirLight.ambient", (GLfloat*)&dirLights[i]->GetComponent<C_Light>()->GetDirectionalLight()->ambient);
						cMaterial->GetShader()->SetUniformVec4f("dirLight.specular", (GLfloat*)&dirLights[i]->GetComponent<C_Light>()->GetDirectionalLight()->specular);
						cMaterial->GetShader()->SetUniformVec3f("dirLight.direction", (GLfloat*)&dirLights[i]->GetComponent<C_Light>()->GetDirectionalLight()->GetDirection());

						cMaterial->GetShader()->SetUniformVec3f("viewPos", (GLfloat*)&App->camera->GetCurrentCamera()->GetFrustum().Pos());
				}
			}

			if (!pointLights.empty())
			{
				for (uint i = 0; i < pointLights.size(); i++)
				{
					cMaterial->GetShader()->SetUniform1i("numPointLights", pointLights.size());
					cMaterial->GetShader()->SetUniformVec4f("pointLight[" + std::to_string(i) + "]" + ".diffuse", (GLfloat*)&pointLights[i]->GetComponent<C_Light>()->GetPointLight()->diffuse);
					cMaterial->GetShader()->SetUniformVec4f("pointLight[" + std::to_string(i) + "]" + ".ambient", (GLfloat*)&pointLights[i]->GetComponent<C_Light>()->GetPointLight()->ambient);
					cMaterial->GetShader()->SetUniformVec4f("pointLight[" + std::to_string(i) + "]" + ".specular", (GLfloat*)&pointLights[i]->GetComponent<C_Light>()->GetPointLight()->specular);
					cMaterial->GetShader()->SetUniform1f("pointLight[" + std::to_string(i) + "]" + ".constant", pointLights[i]->GetComponent<C_Light>()->GetPointLight()->GetConstant());
					cMaterial->GetShader()->SetUniform1f("pointLight[" + std::to_string(i) + "]" + ".linear", pointLights[i]->GetComponent<C_Light>()->GetPointLight()->GetLinear());
					cMaterial->GetShader()->SetUniform1f("pointLight[" + std::to_string(i) + "]" + ".quadratic", pointLights[i]->GetComponent<C_Light>()->GetPointLight()->GetQuadratic());
					cMaterial->GetShader()->SetUniformVec3f("pointLight[" + std::to_string(i) + "]" + ".position", (GLfloat*)&pointLights[i]->transform->GetWorldPosition());
				}
			}

			if(cMaterial->GetShader()) 
				Importer::Shaders::SetShaderUniforms(cMaterial->GetShader());

			dirLights.clear();
			pointLights.clear();
		}
	}
}

uint32 MeshRenderer::SetDefaultShader(C_Material* cMaterial)
{
	//Assign the default Shader
	if (!App->renderer->defaultShader) App->renderer->defaultShader = App->resourceManager->GetShader("DefaultShader");
	
	cMaterial->SetShader(App->renderer->defaultShader);

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
	case CuboidType::COLLIDER:	{ return Green; }								break;
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
	case CuboidType::COLLIDER:	{ return App->renderer->GetOBBEdgeWidth(); }		break;
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

// -- PARTICLE RENDERER METHODS
ParticleRenderer::ParticleRenderer(R_Texture* mat, Color color, const float4x4 transform) : 
mat(mat),
color(color),
transform(transform),
VAO(0),
shader(nullptr)
{

}

void ParticleRenderer::LoadBuffers()
{
	glGenBuffers(1, &VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VAO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(ParticlesCoords), ParticlesCoords, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (GLvoid*)0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ParticleRenderer::Render()
{
	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);

	glUseProgram(shader->shaderProgramID);

	glBindTexture(GL_TEXTURE_2D, mat->GetTextureID());

	shader->SetUniformMatrix4("modelMatrix", transform.ptr());

	shader->SetUniformMatrix4("viewMatrix", App->camera->GetCurrentCamera()->GetViewMatrixTransposed().ptr());

	shader->SetUniformMatrix4("projectionMatrix", App->camera->GetCurrentCamera()->GetProjectionMatrixTransposed().ptr());

	shader->SetUniformVec4f("color", (GLfloat*)&color);

	glBindBuffer(GL_ARRAY_BUFFER, VAO);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, 0);

	glUseProgram(0);

	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
}
