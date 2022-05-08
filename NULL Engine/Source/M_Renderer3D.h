#ifndef __M_RENDERER_3D_H__
#define __M_RENDERER_3D_H__

#include <vector>
#include <map>

#include "MathGeoTransform.h"
#include "MathGeoLib/include/Geometry/LineSegment.h"
#include "MathGeoLib/include/Geometry/Triangle.h"

#include "Icons.h"
#include "Light.h"
#include "Renderer.h"

#include "SkyBox.h"

#include "Module.h"

class ParsonNode;

class R_Model;
class R_Mesh;
class R_Material;
class R_Texture;
class R_Shader;

class C_Transform;
class C_Mesh;	
class C_Material;

class Primitive;
class GameObject;

typedef unsigned int GLenum;

enum class RendererFlags																						// Employed to avoid having OpenGL deps. in some files (M_Editor & "E_" files)
{
	DEPTH_TEST		= 0x0B71,																					// 0x0B71 = GL_DEPTH_TEST
	CULL_FACE		= 0x0B44,																					// 0x0B44 = GL_CULL_FACE
	LIGHTING		= 0x0B50,																					// 0x0B50 = GL_LIGHTING
	COLOR_MATERIAL	= 0x0B57,																					// 0x0B57 = GL_COLOR_MATERIAL
	TEXTURE_2D		= 0x0DE1,																					// 0x0DE1 = GL_TEXTURE_2D
	ALPHA_TEST		= 0x0BC0,																					// 0x0BC0 = GL_ALPHA_TEST
	BLEND			= 0x0BE2																					// 0x0BE2 = GL_BLEND
};

enum class CuboidType
{
	NONE,
	AABB,
	OBB,
	FRUSTUM,
	COLLIDER
};

const float ParticlesCoords[] = {
1, 1,
1, 0,
0, 0,
1,0,
};

struct MeshRenderer
{
	MeshRenderer(C_Transform* cTransform, C_Mesh* cMesh, C_Material* cMaterial);								// Will render the given mesh at the given position with the given mat & tex.

	void Render();

	void RenderVertexNormals(const R_Mesh* rMesh);
	void RenderFaceNormals(const R_Mesh* rMesh);

	void GetFaces(const R_Mesh* rMesh, std::vector<Triangle>& vertexFaces, std::vector<Triangle>& normalFaces);

	void ApplyDebugParameters();
	void ClearDebugParameters();
	
	void ApplyTextureAndMaterial();
	void ClearTextureAndMaterial();

	void RenderOutline(R_Mesh* rMesh);

	void ApplyShader();
	uint32 SetDefaultShader(C_Material* cMaterial);
	void ClearShader();

	std::vector<float4x4>*	boneTransforms;

	C_Transform*			cTransform;
	C_Mesh*					cMesh;
	C_Material*				cMaterial;

	bool					renderLast;
};

struct CuboidRenderer																							// Will render the wireframe of any given geometric form with 8 vertices.
{	
	CuboidRenderer(const float3* vertices, const Color& color, const float& edgeWidth);
	CuboidRenderer(const float3* vertices, CuboidType type);

	void Render();

	Color GetColorByType();
	float GetEdgeWidthByType();

	const float3*	vertices;
	CuboidType		type;
	Color			color;
	float			edgeWidth;
};

struct RayRenderer
{
	RayRenderer(const LineSegment& ray, const Color& color, const float& rayWidth);
	RayRenderer(const LineSegment& ray);

	void Render();

	const LineSegment	ray;
	const Color			color;
	const float			rayWidth;
};

struct SkeletonRenderer
{
	SkeletonRenderer(const std::vector<LineSegment>& bones, const Color& color, const float& boneWidth);
	SkeletonRenderer(const std::vector<LineSegment>& bones);

	void Render();

	std::vector<LineSegment>	bones;
	Color						color;
	float						boneWidth;
};

struct ParticleRenderer
{
	ParticleRenderer(R_Texture* mat, Color color, const float4x4 transform);

	void Render();
	void LoadBuffers();

	uint		VAO;

	R_Texture*	mat;
	R_Shader*	shader;
	Color		color;
	float4x4	transform;
};

#define MAX_LIGHTS 8

class MISSCLICK_API M_Renderer3D : public Module
{
public:
	M_Renderer3D(bool isActive = true);
	~M_Renderer3D();

	bool			Init(ParsonNode& configuration) override;
	bool			Start() override;
	UpdateStatus	PreUpdate(float dt) override;
	UpdateStatus	PostUpdate(float dt) override;
	bool			CleanUp() override;
	
	bool			LoadConfiguration(ParsonNode& root) override;
	bool			SaveConfiguration(ParsonNode& root) const override;

public:
	bool			InitDebugVariables();
	
	bool			InitOpenGL();
	bool			InitGlew();
	void			OnResize();
	void			RecalculateProjectionMatrix();

	void			InitEngineIcons();

	void			InitFramebuffers();
	void			ResizeFramebuffers();
	void			LoadDebugTexture();
	void			FreeBuffers();

	void			RendererShortcuts();

	void			RenderScene();

public:																											// --- RENDER GEOMETRY
	//void			GenerateBuffers(const R_Mesh* rMesh);
		
	void			DrawWorldGrid(const int& size);
	void			DrawWorldAxis();

	void			AddRenderersBatch(const std::vector<MeshRenderer>& meshRenderers, const std::vector<CuboidRenderer>& cuboidRenderers, 
										const std::vector<SkeletonRenderer>& skeletonRenderers);

	void			AddRenderersBatch(const std::multimap<float, Renderer*>& renderers);

	void			RenderMeshes(std::vector<MeshRenderer>& lastRenderers);
	void			RenderCuboids();
	void			RenderRays();
	void			RenderSkeletons();
	void			RenderUI();
	void			RenderParticles();

	void			RenderFramebufferTexture();
	void			DeleteFromMeshRenderers(C_Mesh* cMeshToDelete);
	void			DeleteFromMeshRenderers(R_Mesh* rMeshToDelete);
	void			DeleteFromCuboids(float3* cuboidToDelete);
	void			ClearRenderers();																			// Loading scene measure.

	void			AddPrimitive(Primitive* primitive);
	void			CreatePrimitiveExamples();


	void			AddParticle(const float4x4& transform, R_Texture* material, Color color, float distanceToCamera);
	void			DrawParticle(ParticleRenderer& renderParticle);

	void			SetTo2DRenderSettings(const bool& setTo);


public:																											// --- GET/SET METHODS
	Icons			GetEngineIcons() const;
	
	uint			GetDebugTextureID() const;
	uint			GetSceneFramebuffer() const;
	uint			GetSceneRenderTexture() const;
	uint			GetGameFramebuffer() const;

	uint			GetDepthBuffer() const;
	uint			GetDepthBufferTexture() const;

	const char*		GetDrivers() const;																			// 
	bool			GetVsync() const;																			// 
	void			SetVsync(const bool& setTo);																// 

	bool			GetGLFlag(GLenum flag) const;																// 
	bool			GetGLFlag(RendererFlags flag) const;														// 
	void			SetGLFlag(GLenum flag, bool setTo);															// 
	void			SetGLFlag(RendererFlags flag, bool setTo);													// 

public:																											// --- DEBUG GET/SET METHODS
	uint			GetWorldGridSize() const;

	Color			GetWorldGridColor() const;
	Color			GetWireframeColor() const;
	Color			GetVertexNormalsColor() const;
	Color			GetFaceNormalsColor() const;
	
	Color			GetAABBColor() const;
	Color			GetOBBColor() const;
	Color			GetFrustumColor() const;
	Color			GetRayColor() const;
	Color			GetBoneColor() const;

	float			GetWorldGridLineWidth() const;
	float			GetWireframeLineWidth() const;
	float			GetVertexNormalsWidth() const;
	float			GetFaceNormalsWidth() const;

	float			GetAABBEdgeWidth() const;
	float			GetOBBEdgeWidth() const;
	float			GetFrustumEdgeWidth() const;
	float			GetRayWidth() const;
	float			GetBoneWidth() const;

	bool			GetRenderWorldGrid() const;																	// 
	bool			GetRenderWorldAxis() const;																	// 
	bool			GetRenderWireframes() const;																// 
	bool			GetRenderVertexNormals() const;																// 
	bool			GetRenderFaceNormals() const;																// 
	bool			GetRenderBoundingBoxes() const;																// 
	bool			GetRenderSkeletons() const;																	// 
	bool			GetRenderPrimitiveExamples() const;															// 
	bool			GetRenderColliders() const;
	bool			GetRenderCanvas() const;

	void			SetWorldGridSize(const uint& worldGridSize);

	void			SetWorldGridColor(const Color& worldGridColor);
	void			SetWireframeColor(const Color& wireframeColor);
	void			SetVertexNormalsColor(const Color& vertexNormalsColor);
	void			SetFaceNormalsColor(const Color& faceNormalsColor);

	void			SetAABBColor(const Color& aabbColor);
	void			SetOBBColor(const Color& obbColor);
	void			SetFrustumColor(const Color& frustumColor);
	void			SetRayColor(const Color& rayColor);
	void			SetBoneColor(const Color& boneColor);

	void			SetWorldGridLineWidth(const float& worldGridLineWidth);
	void			SetWireframeLineWidth(const float& wireframeLineWidth);
	void			SetVertexNormalsWidth(const float& vertexNormalsWidth);
	void			SetFaceNormalsWidth(const float& faceNormalsWidth);

	void			SetAABBEdgeWidth(const float& aabbEdgeWidth);
	void			SetOBBEdgeWidth(const float& obbEdgeWidth);
	void			SetFrustumEdgeWidth(const float& frustumEdgeWidth);
	void			SetRayWidth(const float& rayWidth);
	void			SetBoneWidth(const float& boneWidth);

	void			SetRenderWorldGrid(const bool& setTo);														// 
	void			SetRenderWorldAxis(const bool& setTo);														// 
	void			SetRenderWireframes(const bool& setTo);														// 
	void			SetRenderVertexNormals(const bool& setTo);													// 
	void			SetRenderFaceNormals(const bool& setTo);													// 
	void			SetRenderBoundingBoxes(const bool& setTo);													// 
	void			SetRenderSkeletons(const bool& setTo);
	void			SetRenderPrimtiveExamples(const bool& setTo);												// 
	void			SetRenderColliders(const bool& setTo);
	void			SetRenderCanvas(const bool& setTo);

public:
	void			AddPostSceneRenderModule(Module* module);
	GameObject*		GenerateSceneLight(Color diffuse, Color ambient, Color specular, LightType lightType);

private:
	void			GenScreenBuffer();

public:
	SDL_GLContext	context;																					// 
	R_Shader*		defaultShader	= nullptr;
	R_Shader*		screenShader	= nullptr;
	R_Shader*		particleShader	= nullptr;
	Skybox			defaultSkyBox;


	std::multimap<float, Renderer*>	renderers;																	// Multimap with distance to camera and Renderers.
	std::vector<Primitive*>			primitives;

private:
	std::vector<MeshRenderer>		meshRenderers;
	std::vector<CuboidRenderer>		cuboidRenderers;
	std::vector<SkeletonRenderer>	skeletonRenderers;
	
	Icons	engineIcons;

	uint	sceneFramebuffer;
	uint	depthBuffer;
	uint	sceneRenderTexture;
	uint	depthBufferTexture;
	uint	gameFramebuffer;
	uint	debugTextureId;
	uint	quadScreenVAO;

	bool	vsync;																								// Will keep track of whether or not the vsync is currently active.

private:																										// --- DEBUG VARIABLES ---		// TODO: CREATE A "DEBUGSETTINGS" STRUCTURE
	uint	worldGridSize;		

	Color	worldGridColor;		
	Color	wireframeColor;		
	Color	vertexNormalsColor;	
	Color	faceNormalsColor;	
	
	Color	aabbColor;			
	Color	obbColor;			
	Color	frustumColor;		
	Color	rayColor;			
	Color	boneColor;			
	
	float	worldGridLineWidth;
	float	wireframeLineWidth;
	float	vertexNormalsWidth;
	float	faceNormalsWidth;

	float	aabbEdgeWidth;		
	float	obbEdgeWidth;		
	float	frustumEdgeWidth;	
	float	rayWidth;			
	float	boneWidth;			
	
	bool	renderWorldGrid;	
	bool	renderWorldAxis;	
	bool	renderWireframes;	
	bool	renderVertexNormals;
	bool	renderFaceNormals;	
	bool	renderBoundingBoxes;
	bool	renderSkeletons;	
	bool	renderColliders	= false;
	bool	renderCanvas	= false;

	bool	renderPrimitiveExamples;																			//

	std::vector<Module*> PostSceneRenderModules;

	std::map<float, ParticleRenderer> particles;						//map of the particles to render. It is ordered depending on the (float)distance to camera. Allows propper rendering
};

#endif // !__M_RENDERER_3D_H__