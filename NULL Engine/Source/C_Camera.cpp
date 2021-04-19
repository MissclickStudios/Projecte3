#include "JSONParser.h"

#include "Application.h"
#include "VariableDefinitions.h"
#include "FileSystemDefinitions.h"
#include "Log.h"
#include "M_Window.h"
#include "M_Scene.h"

#include "GameObject.h"
#include "C_Transform.h"

#include "C_Camera.h"

#include "MemoryManager.h"

#define NUM_FRUSTUM_PLANES		6																			// A Frustum will ALWAYS be composed by 6 Planes.
#define NUM_FRUSTUM_VERTICES	8																			// As frustums are CUBOIDS, they will ALWAYS be composed by 8 Vertices.

#define MIN_FOV 1
#define MAX_FOV 120

C_Camera::C_Camera(GameObject* owner) : Component(owner, ComponentType::CAMERA),
frustumPlanes(nullptr),
frustumVertices(nullptr),
minFov(MIN_FOV),
maxFov(MAX_FOV),
isCulling(false),
inOrthogonalView(false),
hideFrustum(false),
updateProjectionMatrix(true)
{
	frustumPlanes	= new Plane[NUM_FRUSTUM_PLANES];
	frustumVertices	= new float3[NUM_FRUSTUM_VERTICES];

	InitFrustum();
}

C_Camera::~C_Camera()
{
	delete[] frustumPlanes;
	delete[] frustumVertices;
}

bool C_Camera::Update()
{
	return true;
}

bool C_Camera::CleanUp()
{
	if (isCulling)
	{
		App->scene->SetCullingCamera(nullptr);
	}

	return true;
}

bool C_Camera::SaveState(ParsonNode& root) const
{
	bool ret = true;

	root.SetNumber("Type", (uint)GetType());

	root.SetNumber("MinFOV", minFov);
	root.SetNumber("MaxFOV", maxFov);

	root.SetBool("Culling", isCulling);
	root.SetBool("OrthogonalView", inOrthogonalView);
	root.SetBool("HideFrustum", hideFrustum);

	return ret;
}

bool C_Camera::LoadState(ParsonNode& root)
{
	bool ret = true;

	minFov = (uint)root.GetNumber("MinFOV");
	maxFov = (uint)root.GetNumber("MaxFOV");

	isCulling = root.GetBool("Culling");
	inOrthogonalView = root.GetBool("OrthogonalView");
	hideFrustum	= root.GetBool("HideFrustum");

	return ret;
}

// --- FRUSTUM METHODS ---
void C_Camera::InitFrustum()
{
	frustum.SetKind(FrustumSpaceGL, FrustumRightHanded);

	frustum.SetPos(float3(0.0f, 0.0f, 0.0f));
	frustum.SetFront(float3::unitZ);
	frustum.SetUp(float3::unitY);

	frustum.SetViewPlaneDistances(1.0f, 300.0f);
	frustum.SetPerspective(1.0f, 1.0f);

	//frustum.SetOrthographic(10.0f, 10.0f);
	//SetVerticalFOV(90);

	UpdateFrustumPlanes();
	UpdateFrustumVertices();
}

void C_Camera::UpdateFrustumTransform()
{
	float4x4 worldTransform	= this->GetOwner()->GetComponent<C_Transform>()->GetWorldTransform();
	float3x4 worldMatrix = float3x4::identity;

	worldMatrix.SetTranslatePart(worldTransform.TranslatePart());
	worldMatrix.SetRotatePart(worldTransform.RotatePart());
	worldMatrix.Scale(worldTransform.GetScale());

	frustum.SetWorldMatrix(worldMatrix);

	UpdateFrustumPlanes();
	UpdateFrustumVertices();

	updateProjectionMatrix = true;
}

Frustum C_Camera::GetFrustum() const
{
	return frustum;
}
void C_Camera::SetUpdateProjectionMatrix(const bool& setTo)
{
	updateProjectionMatrix = setTo;
}

bool C_Camera::GetUpdateProjectionMatrix() const
{
	return updateProjectionMatrix;
}

float* C_Camera::GetOGLViewMatrix()
{
	static float4x4 viewMatrix;

	viewMatrix = frustum.ViewMatrix();
	viewMatrix.Transpose();

	return (float*)viewMatrix.v;
}


float* C_Camera::GetOGLProjectionMatrix()
{
	static float4x4 projectionMatrix;

	projectionMatrix = frustum.ProjectionMatrix().Transposed();
	
	return (float*)projectionMatrix.v;
}

math::float4x4 C_Camera::GetViewMatrixTransposed() const
{
	math::float4x4 matrix = frustum.ViewMatrix();
	return matrix.Transposed();
}

math::float4x4 C_Camera::GetProjectionMatrixTransposed() const
{
	math::float4x4 matrix = frustum.ProjectionMatrix();
	return matrix.Transposed();
}

void C_Camera::PointAt(const float3& position, const float3& target)
{
	SetPosition(position);
	LookAt(target);
}

void C_Camera::LookAt(const float3& target)
{	
	//float3 Z = float3(location - frustum.Pos()).Normalized();															// Could construct the rotation matrix and multiply it by
	//float3 X = float3(float3::unitY.Cross(Z)).Normalized();															// the world transform. It does not quite work however.
	//float3 Y = Z.Cross(X);																							// 
	//float3x3 look_at_matrix = float3x3(X, Y, Z);																		// ------------------------------------------------------

	float3 newZ = float3(target - frustum.Pos()).Normalized();											// Constructing the new forward vector of the camera.
	float3x3 lookAtMatrix = float3x3::LookAt(frustum.Front(), newZ, frustum.Up(), float3::unitY);				// Using the LookAt() method built in MathGeoLib to generate the mat.

	frustum.SetFront(lookAtMatrix.MulDir(frustum.Front()).Normalized());
	frustum.SetUp(lookAtMatrix.MulDir(frustum.Up()).Normalized());

	float4x4 worldMatrix = frustum.WorldMatrix();
	this->GetOwner()->GetComponent<C_Transform>()->SetWorldTransform(worldMatrix);											// Setting the updated world transform.
}

void C_Camera::Move(const float3& velocity)
{
	this->GetOwner()->GetComponent<C_Transform>()->Translate(velocity);
}

void C_Camera::Rotate(const float3x3& rotationMatrix)
{
	float4x4 worldMatrix = frustum.WorldMatrix();

	worldMatrix.SetRotatePart(rotationMatrix);

	this->GetOwner()->GetComponent<C_Transform>()->SetWorldTransform(worldMatrix);											// Setting the updated world transform.
}

void C_Camera::Focus(const float3& target, const float& distanceToTarget)
{
	float abs_distance = (distanceToTarget < 0) ? -distanceToTarget : distanceToTarget;

	float3 distance = float3(target - frustum.Pos()).Normalized();

	float3 position = frustum.Pos() - target;

	position = target + distance * position.Length();						// While FreeLookAround() rotates the camera on its axis, this method also translates the camera.

	PointAt(position, target);
}

void C_Camera::SetPosition(const float3& position)
{
	this->GetOwner()->GetComponent<C_Transform>()->SetWorldPosition(position);
}

void C_Camera::UpdateFrustumPlanes()
{
	frustum.GetPlanes(frustumPlanes);
}

void C_Camera::UpdateFrustumVertices()
{
	frustum.GetCornerPoints(frustumVertices);
}

Plane* C_Camera::GetFrustumPlanes() const
{
	return frustumPlanes;
}

float3* C_Camera::GetFrustumVertices() const
{
	return frustumVertices;
}

bool C_Camera::FrustumCointainsAABB(const AABB& aabb) const
{
	float3 aabbVertices[8];
	aabb.GetCornerPoints(aabbVertices);

	uint planesContainingAllVerts = 0;																	// Can be used to check whether or not the geometric figure is intersected.

	for (uint p = 0; p < 6; ++p)
	{
		bool allVertsAreIn = true;
		uint vertsInPlane = 8;

		for (uint v = 0; v < 8; ++v)
		{
			if (frustumPlanes[p].IsOnPositiveSide(aabbVertices[v]))										// IsOnPositiveSide() returns true if the given point is behind the frustum plane.
			{
				allVertsAreIn = false;
				--vertsInPlane;
			}
		}

		if (vertsInPlane == 0)
		{
			return false;
		}

		if (allVertsAreIn)
		{
			++planesContainingAllVerts;
		}
	}

	if (planesContainingAllVerts == 6)
	{
		return true;
	}

	return false;
}

bool C_Camera::FrustumIntersectsAABB(const AABB& aabb) const
{
	float3 aabbVertices[8];
	aabb.GetCornerPoints(aabbVertices);

	for (uint p = 0; p < 6; ++p)
	{
		uint vertsInPlane = 8;

		for (uint v = 0; v < 8; ++v)
		{
			if (frustumPlanes[p].IsOnPositiveSide(aabbVertices[v]))
			{
				--vertsInPlane;
			}
		}

		if (vertsInPlane == 0)
		{
			return false;
		}
	}

	return true;
}

float C_Camera::GetAspectRatio() const
{
	return frustum.AspectRatio();
}

void C_Camera::SetAspectRatio(const float& aspectRatio)
{
	frustum.SetHorizontalFovAndAspectRatio(frustum.HorizontalFov(), aspectRatio);
	
	UpdateFrustumPlanes();
	UpdateFrustumVertices();

	updateProjectionMatrix = true;
}

float C_Camera::GetNearPlaneDistance() const
{
	return frustum.NearPlaneDistance();
}

float C_Camera::GetFarPlaneDistance() const
{
	return frustum.FarPlaneDistance();
}

float C_Camera::GetHorizontalFOV() const
{
	return (frustum.HorizontalFov() * RADTODEG);
}

float C_Camera::GetVerticalFOV() const
{
	return (frustum.VerticalFov() * RADTODEG);
}

void C_Camera::SetNearPlaneDistance(const float& nearDistance)
{
	if (nearDistance < 0)
	{
		LOG("[ERROR] Component Camera: Cannot set the new distance of the near plane! Error: New near distance < 0.");
		return;
	}

	if (nearDistance > frustum.FarPlaneDistance())
	{
		LOG("[ERROR] Component Camera: Cannot set the new distance of the near plane! Error: New near distance > far distance.");
		return;
	}

	frustum.SetViewPlaneDistances(nearDistance, frustum.FarPlaneDistance());

	UpdateFrustumPlanes();
	UpdateFrustumVertices();

	updateProjectionMatrix = true;
}

void C_Camera::SetFarPlaneDistance(const float& farDistance)
{
	if (farDistance < 0)
	{
		LOG("[ERROR] Component Camera: Cannot set the new distance of the far plane! Error: New far distance < 0.");
		return;
	}

	if (farDistance < frustum.NearPlaneDistance())
	{
		LOG("[ERROR] Component Camera: Cannot set the new distance of the far plane! Error: New far distance < near distance");
		return;
	}

	frustum.SetViewPlaneDistances(frustum.NearPlaneDistance(), farDistance);

	UpdateFrustumPlanes();
	UpdateFrustumVertices();

	updateProjectionMatrix = true;
}

void C_Camera::SetHorizontalFOV(const float& horizontalFov)
{
	/*if (horizontalFov < minFov)
	{
		LOG("[ERROR] Camera Component: Could not set the Horizontal FOV! Error: New Horizontal FOV < minFov");
		return;
	}

	if (horizontalFov > maxFov)
	{
		LOG("[ERROR] Camera Component: Could not set the Horizontal FOV! Error: New Horizontal FOV > maxFov");
		return;
	}*/
	
	frustum.SetHorizontalFovAndAspectRatio((horizontalFov * DEGTORAD), frustum.AspectRatio());

	UpdateFrustumPlanes();
	UpdateFrustumVertices();

	updateProjectionMatrix = true;
}

void C_Camera::SetVerticalFOV(const float& verticalFov)
{
	/*if (verticalFov < minFov)
	{
		LOG("[ERROR] Camera Component: Could not set the Vertical FOV! Error: New Vertical FOV < minFov");
		return;
	}

	if (verticalFov > maxFov)
	{
		LOG("[ERROR] Camera Component: Could not set the Vertical FOV! Error: New Vertical FOV > maxFov");
		return;
	}*/

	frustum.SetVerticalFovAndAspectRatio((verticalFov * DEGTORAD), frustum.AspectRatio());

	UpdateFrustumPlanes();
	UpdateFrustumVertices();

	updateProjectionMatrix = true;
}

void C_Camera::GetMinMaxFOV(uint& minFov, uint& maxFov) const
{
	minFov = this->minFov;
	maxFov = this->maxFov;
}

void C_Camera::SetMinMaxFOV(const uint& minFov, const uint& maxFov)
{
	this->minFov = minFov;
	this->maxFov = maxFov;
}

bool C_Camera::IsCulling() const
{
	return isCulling;
}

bool C_Camera::OrthogonalView() const
{
	return inOrthogonalView;
}

bool C_Camera::FrustumIsHidden() const
{
	return hideFrustum;
}

void C_Camera::SetIsCulling(const bool& setTo)
{
	isCulling = setTo;
	
	if (setTo)
	{
		App->scene->SetCullingCamera(this);
	}
	else
	{
		if (App->scene->GetCullingCamera() == this)
		{
			App->scene->SetCullingCamera(nullptr);
		}
	}
}

void C_Camera::SetOrthogonalView(const bool& setTo)
{
	inOrthogonalView = setTo;

	if (inOrthogonalView)
	{
		// SET TO ORTHOGONAL
		// --- Near plane size = Far plane size
		// --- Update View Matrix
	}
}

void C_Camera::SetFrustumIsHidden(const bool& setTo)
{
	hideFrustum = setTo;
}