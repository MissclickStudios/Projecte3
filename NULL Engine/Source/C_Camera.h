#ifndef __C_CAMERA_H__
#define __C_CAMERA_H__

#include "MathGeoFrustum.h"
#include "MathGeoLib/include/Math/float2.h"
#include "Component.h"

class ParsonNode;
class GameObject;

typedef unsigned int uint;

class C_Camera : public Component
{
public:
	C_Camera(GameObject* owner);
	~C_Camera();

	bool Update		() override;
	bool CleanUp	() override;

	bool SaveState	(ParsonNode& root) const override;
	bool LoadState	(ParsonNode& root) override;

	static inline COMPONENT_TYPE GetType() { return COMPONENT_TYPE::CAMERA; }								// This is needed to use templeates for functions such as GetComponent<>();

public:																										// --- FRUSTUM METHODS
	void		InitFrustum					();
	void		UpdateFrustumTransform		();
	Frustum		GetFrustum					() const;

	void		SetUpdateProjectionMatrix	(const bool& set_to);
	bool		GetUpdateProjectionMatrix	() const;

	float*		GetOGLViewMatrix			();
	float*		GetOGLProjectionMatrix		();

public:																										// --- CAMERA TRANSFORM
	void		PointAt							(const float3& position, const float3& target);
	void		LookAt							(const float3& target);
	void		Move							(const float3& velocity);
	void		Rotate							(const float3x3& rotation_matrix);
	void		Focus							(const float3& target, const float& distance_to_target = 10.0f);

	void		SetPosition						(const float3& position);

public:																										// --- FRUSTUM CULLING
	void		UpdateFrustumPlanes			();
	void		UpdateFrustumVertices		();
	Plane*		GetFrustumPlanes			() const;
	float3*		GetFrustumVertices			() const;

	bool		FrustumCointainsAABB		(const AABB& aabb) const;										// Returns true if the Frustum contains the whole AABB.
	bool		FrustumIntersectsAABB		(const AABB& aabb) const;										// Returns true if the Frustum contains at least one vertex of the AABB.

public:																										// --- GET/SET FRUSTUM SETTINGS
	float		GetAspectRatio				() const;
	void		SetAspectRatio				(const float& aspect_ratio);

	float		GetNearPlaneDistance		() const;
	float		GetFarPlaneDistance			() const;
	float		GetHorizontalFOV			() const;
	float		GetVerticalFOV				() const;

	void		SetNearPlaneDistance		(const float& near_distance);
	void		SetFarPlaneDistance			(const float& far_distance);
	void		SetHorizontalFOV			(const float& horizontal_fov);									// Chosen FOV adaptation: VFOV locked , HFOV adapted to aspect_ratio.
	void		SetVerticalFOV				(const float& vertical_fov);

	void		GetMinMaxFOV				(uint& min_fov, uint& max_fov) const;
	void		SetMinMaxFOV				(const uint& min_fov, const uint& max_fov);

public:																										// --- CAMERA FLAGS	
	bool		IsCulling					() const;
	bool		OrthogonalView				() const;
	bool		FrustumIsHidden				() const;

	void		SetIsCulling				(const bool& set_to);
	void		SetOrthogonalView			(const bool& set_to);
	void		SetFrustumIsHidden			(const bool& set_to);

private:
	Frustum frustum;																						//

	Plane*	frustum_planes;																					// --- FRUSTUM CULLING
	float3* frustum_vertices;																				// -------------------
	
	uint	min_fov;																						// --- FRUSTUM SETTINGS 
	uint	max_fov;																						// --------------------

	bool	is_culling;																						// --- CAMERA FLAGS
	bool	in_orthogonal_view;																				//
	bool	hide_frustum;																					// ----------------
	
	bool	update_projection_matrix;																		// --- FRUSTUM UPDATES
};

#endif // !__CAMERA_H__