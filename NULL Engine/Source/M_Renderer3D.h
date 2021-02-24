#ifndef __M_RENDERER_3D_H__
#define __M_RENDERER_3D_H__

#include <vector>

#include "MathGeoTransform.h"
#include "MathGeoLib/include/Geometry/LineSegment.h"
#include "MathGeoLib/include/Geometry/Triangle.h"

#include "Icons.h"

#include "Module.h"
#include "Light.h"

struct Color;
class ParsonNode;

class R_Model;
class R_Mesh;
class R_Material;
class R_Texture;

class C_Mesh;	
class C_Material;

class Primitive;

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
	FRUSTUM
};

struct MeshRenderer
{
	MeshRenderer(const float4x4& transform, C_Mesh* cMesh, C_Material* cMaterial);							// Will render the given mesh at the given position with the given mat & tex.

	void Render						();

	void RenderVertexNormals		(const R_Mesh* rMesh);
	void RenderFaceNormals			(const R_Mesh* rMesh);

	void GetFaces					(const R_Mesh* rMesh, std::vector<Triangle>& vertexFaces, std::vector<Triangle>& normalFaces);

	void ApplyDebugParameters		();
	void ClearDebugParameters		();
	
	void ApplyTextureAndMaterial	();
	void ClearTextureAndMaterial	();

	float4x4	transform;
	C_Mesh*		cMesh;
	C_Material*	cMaterial;
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
	const Color		color;
	const float		edgeWidth;
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

	const std::vector<LineSegment>	bones;
	const Color						color;
	const float						boneWidth;
};

#define MAX_LIGHTS 8

class M_Renderer3D : public Module
{
public:
	M_Renderer3D(bool isActive = true);
	~M_Renderer3D();

	bool			Init				(ParsonNode& configuration) override;
	bool			Start				() override;
	UpdateStatus	PreUpdate			(float dt) override;
	UpdateStatus	PostUpdate			(float dt) override;
	bool			CleanUp				() override;
	
	bool			LoadConfiguration	(ParsonNode& root) override;
	bool			SaveConfiguration	(ParsonNode& root) const override;

public:
	bool			InitDebugVariables			();
	
	bool			InitOpenGL					();
	bool			InitGlew					();
	void			OnResize					();
	void			RecalculateProjectionMatrix	();

	void			InitEngineIcons();

	void			InitFramebuffers			();
	void			LoadDebugTexture			();
	void			FreeBuffers					();

	void			RendererShortcuts			();

	void			RenderScene					();

public:																											// --- RENDER GEOMETRY
	void			GenerateBuffers				(const R_Mesh* rMesh);
		
	void			DrawWorldGrid				(const int& size);
	void			DrawWorldAxis				();

	void			AddRenderersBatch			(const std::vector<MeshRenderer>& meshRenderers, const std::vector<CuboidRenderer>& cuboidRenderers, 
													const std::vector<SkeletonRenderer>& skeletonRenderers);
	void			RenderMeshes				();
	void			RenderCuboids				();
	void			RenderRays					();
	void			RenderSkeletons				();
	void			RenderUI();
	void			DeleteFromMeshRenderers		(C_Mesh* cMeshToDelete);
	void			DeleteFromMeshRenderers		(R_Mesh* rMeshToDelete);
	void			DeleteFromCuboids			(float3* cuboidToDelete);
	void			ClearRenderers				();																// Loading scene measure.

	void			AddPrimitive				(Primitive* primitive);
	void			CreatePrimitiveExamples		();


public:																											// --- GET/SET METHODS
	Icons			GetEngineIcons				() const;
	
	uint			GetDebugTextureID			() const;
	uint			GetSceneFramebuffer			() const;
	uint			GetSceneRenderTexture		() const;
	uint			GetGameFramebuffer			() const;

	uint			GetDepthBuffer				() const;
	uint			GetDepthBufferTexture		() const;

	const char*		GetDrivers					() const;														// 
	bool			GetVsync					() const;														// 
	void			SetVsync					(const bool& setTo);											// 

	bool			GetGLFlag					(GLenum flag) const;											// 
	bool			GetGLFlag					(RendererFlags flag) const;									// 
	void			SetGLFlag					(GLenum flag, bool setTo);										// 
	void			SetGLFlag					(RendererFlags flag, bool setTo);								// 

public:																											// --- DEBUG GET/SET METHODS
	uint			GetWorldGridSize			() const;

	Color			GetWorldGridColor			() const;
	Color			GetWireframeColor			() const;
	Color			GetVertexNormalsColor		() const;
	Color			GetFaceNormalsColor			() const;
	
	Color			GetAABBColor				() const;
	Color			GetOBBColor					() const;
	Color			GetFrustumColor				() const;
	Color			GetRayColor					() const;
	Color			GetBoneColor				() const;

	float			GetWorldGridLineWidth		() const;
	float			GetWireframeLineWidth		() const;
	float			GetVertexNormalsWidth		() const;
	float			GetFaceNormalsWidth			() const;

	float			GetAABBEdgeWidth			() const;
	float			GetOBBEdgeWidth				() const;
	float			GetFrustumEdgeWidth			() const;
	float			GetRayWidth					() const;
	float			GetBoneWidth				() const;

	bool			GetRenderWorldGrid			() const;														// 
	bool			GetRenderWorldAxis			() const;														// 
	bool			GetRenderWireframes			() const;														// 
	bool			GetRenderVertexNormals		() const;														// 
	bool			GetRenderFaceNormals		() const;														// 
	bool			GetRenderBoundingBoxes		() const;														// 
	bool			GetRenderSkeletons			() const;														// 
	bool			GetRenderPrimitiveExamples	() const;														// 

	void			SetWorldGridSize			(const uint& worldGridSize);

	void			SetWorldGridColor			(const Color& worldGridColor);
	void			SetWireframeColor			(const Color& wireframeColor);
	void			SetVertexNormalsColor		(const Color& vertexNormalsColor);
	void			SetFaceNormalsColor			(const Color& faceNormalsColor);

	void			SetAABBColor				(const Color& aabbColor);
	void			SetOBBColor					(const Color& obbColor);
	void			SetFrustumColor				(const Color& frustumColor);
	void			SetRayColor					(const Color& rayColor);
	void			SetBoneColor				(const Color& boneColor);

	void			SetWorldGridLineWidth		(const float& worldGridLineWidth);
	void			SetWireframeLineWidth		(const float& wireframeLineWidth);
	void			SetVertexNormalsWidth		(const float& vertexNormalsWidth);
	void			SetFaceNormalsWidth			(const float& faceNormalsWidth);

	void			SetAABBEdgeWidth			(const float& aabbEdgeWidth);
	void			SetOBBEdgeWidth				(const float& obbEdgeWidth);
	void			SetFrustumEdgeWidth			(const float& frustumEdgeWidth);
	void			SetRayWidth					(const float& rayWidth);
	void			SetBoneWidth				(const float& boneWidth);

	void			SetRenderWorldGrid			(const bool& setTo);											// 
	void			SetRenderWorldAxis			(const bool& setTo);											// 
	void			SetRenderWireframes			(const bool& setTo);											// 
	void			SetRenderVertexNormals		(const bool& setTo);											// 
	void			SetRenderFaceNormals		(const bool& setTo);											// 
	void			SetRenderBoundingBoxes		(const bool& setTo);											// 
	void			SetRenderSkeletons			(const bool& setTo);
	void			SetRenderPrimtiveExamples	(const bool& setTo);											// 

public:
	Light					lights[MAX_LIGHTS];																	// 
	SDL_GLContext			context;																			// 

	std::vector<Primitive*>	primitives;

private:
	std::vector<MeshRenderer>		meshRenderers;
	std::vector<CuboidRenderer>		cuboidRenderers;
	std::vector<SkeletonRenderer>	skeletonRenderers;
	
	Icons					engineIcons;

	uint					sceneFramebuffer;
	uint					depthBuffer;
	uint					sceneRenderTexture;
	uint					depthBufferTexture;
	uint					gameFramebuffer;
	uint					debugTextureId;

	bool					vsync;																				// Will keep track of whether or not the vsync is currently active.

private:																										// --- DEBUG VARIABLES ---		// TODO: CREATE A "DEBUGSETTINGS" STRUCTURE
	uint					worldGridSize;																		//

	Color					worldGridColor;																		//
	Color					wireframeColor;																		//
	Color					vertexNormalsColor;																	//
	Color					faceNormalsColor;																	//
	
	Color					aabbColor;																			// 
	Color					obbColor;																			// 
	Color					frustumColor;																		// 
	Color					rayColor;																			// 
	Color					boneColor;																			// 
	
	float					worldGridLineWidth;
	float					wireframeLineWidth;
	float					vertexNormalsWidth;
	float					faceNormalsWidth;

	float					aabbEdgeWidth;																		// 
	float					obbEdgeWidth;																		// 
	float					frustumEdgeWidth;																	// 
	float					rayWidth;																			// 
	float					boneWidth;																			// 
	
	bool					renderWorldGrid;																	// 
	bool					renderWorldAxis;																	// 
	bool					renderWireframes;																	//
	bool					renderWertexNormals;																// 
	bool					renderFaceNormals;																	// 
	bool					renderBoundingBoxes;																// 
	bool					renderSkeletons;																	//

	bool					renderPrimitiveExamples;															//
};

#endif // !__M_RENDERER_3D_H__