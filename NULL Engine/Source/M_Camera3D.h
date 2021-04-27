#ifndef __CAMERA_3D_H__
#define __CAMERA_3D_H__

#include "MathGeoLib/include/Geometry/LineSegment.h"
#include "Module.h"

class ParsonNode;
class GameObject;
class C_Camera;
class C_AudioListener;

class MISSCLICK_API M_Camera3D : public Module
{
public:
	M_Camera3D(bool isActive = true);
	~M_Camera3D();

	bool			Init				(ParsonNode& root) override;
	bool			Start				() override;
	UpdateStatus	Update				(float dt) override;
	bool			CleanUp				() override;

	bool			LoadConfiguration	(ParsonNode& configuration) override;
	bool			SaveConfiguration	(ParsonNode& configuration) const override;

public:
	void			CreateMasterCamera				();
	C_Camera*		GetCurrentCamera				() const;
	void			SetCurrentCamera				(C_Camera* cCamera);
	void			SetMasterCameraAsCurrentCamera	();

public:
	void			PointAt							(const float3& position, const float3& reference, bool orbit = false);
	void			LookAt							(const float3& Spot);
	void			Focus							(const float3& targetPosition, const float& distanceFromTarget = 10.0f);
	void			Move							(const float3& velocity);

	void			ReturnToWorldOrigin				();

public:																				// Camera3D Getters and Setters.
	float3			GetPosition						() const;
	float3			GetReference					() const;
	void			SetPosition						(const float3& position);
	void			SetReference					(const float3& reference);

	float			GetMovementSpeed				() const;
	float			GetRotationSpeed				() const;
	float			GetZoomSpeed					() const;
	void			SetMovementSpeed				(const float& movementSpeed);
	void			SetRotationSpeed				(const float& rotationSpeed);
	void			SetZoomSpeed					(const float& zoomSpeed);

	float3			GetMasterCameraPosition			() const;
	float3			GetMasterCameraRotation			() const;
	float3			GetMasterCameraScale			() const;
	void			SetMasterCameraPosition			(const float3& position);
	void			SetMasterCameraRotation			(const float3& rotation);
	void			SetMasterCameraScale			(const float3& scale);

public:																				//Ray Functionallity
	bool			DrawLastRaycast					() const;
	void			SetDrawLastRaycast				(const bool& setTo);
	void			CastRay							(const float2& position);

public:
	void FreeLookAround();															// Receives information about the mouse's motion values and rotates the camera on it's axis.
	void Orbit(const float2& orbitPoint);											// Will rotate the camera around a point specified by the reference var.. ~identical to FreeLookAround();
	void PanCamera(const float2& panPoint);										// Will translate the camera along XY (Right, Up). Moving the camera in the plane of the camera frame.
	void Zoom();																	// Will translate the camera along the Z (Forward) axis. The camera will move forwards and backwards. 
	void WASDMovement();															// Translates the camera along XYZ (Right, Up, Forward), which will act as the camera's axis.		

public:
	GameObject*		masterCamera;
	C_Camera*		currentCamera;

	float3			reference;														// Point where the camera will look at and rotate around.

	float3			positionOrigin;													// 
	float3			referenceOrigin;												//

	float			movementSpeed;													// 
	float			rotationSpeed;													// 
	float			zoomSpeed;														// 
	float			panSpeed = 7;
public:
	bool			drawLastRaycast;
	LineSegment		lastRaycast;
};

#endif // !__CAMERA_3D_H__