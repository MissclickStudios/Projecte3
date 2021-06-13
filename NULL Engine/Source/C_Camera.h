#ifndef __C_CAMERA_H__
#define __C_CAMERA_H__

#include "MathGeoFrustum.h"
#include "MathGeoLib/include/Math/float2.h"
#include "MathGeoLib/include/Math/Quat.h"
#include "Component.h"

class ParsonNode;
class GameObject;

typedef unsigned int uint;

class MISSCLICK_API C_Camera : public Component
{
public:
	C_Camera(GameObject* owner);
	~C_Camera();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	static inline ComponentType GetType() { return ComponentType::CAMERA; }								// This is needed to use templeates for functions such as GetComponent<>();

public:																										// --- FRUSTUM METHODS
	void InitFrustum();
	void UpdateFrustumTransform();
	Frustum GetFrustum() const;

	void SetUpdateProjectionMatrix(const bool& setTo);
	bool GetUpdateProjectionMatrix() const;

	float* GetOGLViewMatrix();
	float* GetOGLProjectionMatrix();

	math::float4x4 GetViewMatrixTransposed() const;
	math::float4x4 GetProjectionMatrixTransposed() const;

public:																										// --- CAMERA TRANSFORM
	void PointAt(const float3& position, const float3& target);
	void LookAt(const float3& target);
	void Move(const float3& velocity);
	void Rotate(const float3x3& rotationMatrix);
	void Focus(const float3& target, const float& distanceToTarget = 10.0f);

	void SetPosition(const float3& position);

public:																										// --- FRUSTUM CULLING
	void UpdateFrustumPlanes();
	void UpdateFrustumVertices();
	Plane* GetFrustumPlanes() const;
	float3* GetFrustumVertices() const;

	bool FrustumCointainsAABB(const AABB& aabb) const;										// Returns true if the Frustum contains the whole AABB.
	bool FrustumIntersectsAABB(const AABB& aabb) const;										// Returns true if the Frustum contains at least one vertex of the AABB.

public:																										// --- GET/SET FRUSTUM SETTINGS
	float GetAspectRatio() const;
	void SetAspectRatio(const float& aspectRatio);

	float GetNearPlaneDistance() const;
	float GetFarPlaneDistance() const;
	float GetHorizontalFOV() const;
	float GetVerticalFOV () const;

	void SetNearPlaneDistance(const float& nearDistance);
	void SetFarPlaneDistance(const float& farDistance);
	void SetHorizontalFOV(const float& horizontalFov);									// Chosen FOV adaptation: VFOV locked , HFOV adapted to aspect_ratio.
	void SetVerticalFOV(const float& verticalFov);

	void GetMinMaxFOV(uint& minFov, uint& maxFov) const;
	void SetMinMaxFOV(const uint& minFov, const uint& maxFov);

public:																										// --- CAMERA FLAGS	
	bool IsCulling() const;
	bool OrthogonalView() const;
	bool FrustumIsHidden() const;

	void SetIsCulling(const bool& setTo);
	void SetOrthogonalView(const bool& setTo);
	void SetFrustumIsHidden(const bool& setTo);

private:
	Frustum frustum;																						//

	Plane* frustumPlanes;																					// --- FRUSTUM CULLING
	float3* frustumVertices;																				// -------------------
	
	uint minFov;																						// --- FRUSTUM SETTINGS 
	uint maxFov;																						// --------------------

	bool isCulling;																						// --- CAMERA FLAGS
	bool inOrthogonalView;																				//
	bool hideFrustum;																					// ----------------
	
	bool updateProjectionMatrix;																		// --- FRUSTUM UPDATES
};

#endif // !__CAMERA_H__