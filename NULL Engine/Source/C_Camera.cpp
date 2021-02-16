#include "JSONParser.h"

#include "Application.h"
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

C_Camera::C_Camera(GameObject* owner) : Component(owner, COMPONENT_TYPE::CAMERA),
frustum_planes				(nullptr),
frustum_vertices			(nullptr),
min_fov						(MIN_FOV),
max_fov						(MAX_FOV),
is_culling					(false),
in_orthogonal_view			(false),
hide_frustum				(false),
update_projection_matrix	(true)
{
	frustum_planes		= new Plane[NUM_FRUSTUM_PLANES];
	frustum_vertices	= new float3[NUM_FRUSTUM_VERTICES];

	InitFrustum();
}

C_Camera::~C_Camera()
{
	delete[] frustum_planes;
	delete[] frustum_vertices;
}

bool C_Camera::Update()
{
	bool ret = true;



	return ret;
}

bool C_Camera::CleanUp()
{
	bool ret = true;

	if (is_culling)
	{
		App->scene->SetCullingCamera(nullptr);
	}

	return ret;
}

bool C_Camera::SaveState(ParsonNode& root) const
{
	bool ret = true;

	root.SetNumber("Type", (uint)GetType());

	root.SetNumber("MinFOV", min_fov);
	root.SetNumber("MaxFOV", max_fov);

	root.SetBool("Culling", is_culling);
	root.SetBool("OrthogonalView", in_orthogonal_view);
	root.SetBool("HideFrustum", hide_frustum);

	return ret;
}

bool C_Camera::LoadState(ParsonNode& root)
{
	bool ret = true;

	min_fov				= (uint)root.GetNumber("MinFOV");
	max_fov				= (uint)root.GetNumber("MaxFOV");

	is_culling			= root.GetBool("Culling");
	in_orthogonal_view	= root.GetBool("OrthogonalView");
	hide_frustum		= root.GetBool("HideFrustum");

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
	float4x4 world_transform	= this->GetOwner()->GetComponent<C_Transform>()->GetWorldTransform();
	float3x4 world_matrix		= float3x4::identity;

	world_matrix.SetTranslatePart(world_transform.TranslatePart());
	world_matrix.SetRotatePart(world_transform.RotatePart());
	world_matrix.Scale(world_transform.GetScale());

	frustum.SetWorldMatrix(world_matrix);

	UpdateFrustumPlanes();
	UpdateFrustumVertices();

	update_projection_matrix = true;
}

Frustum C_Camera::GetFrustum() const
{
	return frustum;
}
void C_Camera::SetUpdateProjectionMatrix(const bool& set_to)
{
	update_projection_matrix = set_to;
}

bool C_Camera::GetUpdateProjectionMatrix() const
{
	return update_projection_matrix;
}

float* C_Camera::GetOGLViewMatrix()
{
	static float4x4 view_matrix;

	view_matrix = frustum.ViewMatrix();
	view_matrix.Transpose();

	return (float*)view_matrix.v;
}

float* C_Camera::GetOGLProjectionMatrix()
{
	static float4x4 projection_matrix;

	projection_matrix = frustum.ProjectionMatrix().Transposed();
	
	return (float*)projection_matrix.v;
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

	float3 new_Z				= float3(target - frustum.Pos()).Normalized();											// Constructing the new forward vector of the camera.
	float3x3 look_at_matrix		= float3x3::LookAt(frustum.Front(), new_Z, frustum.Up(), float3::unitY);				// Using the LookAt() method built in MathGeoLib to generate the mat.

	frustum.SetFront(look_at_matrix.MulDir(frustum.Front()).Normalized());
	frustum.SetUp(look_at_matrix.MulDir(frustum.Up()).Normalized());

	float4x4 world_matrix = frustum.WorldMatrix();
	this->GetOwner()->GetComponent<C_Transform>()->SetWorldTransform(world_matrix);											// Setting the updated world transform.
}

void C_Camera::Move(const float3& velocity)
{
	this->GetOwner()->GetComponent<C_Transform>()->Translate(velocity);
}

void C_Camera::Rotate(const float3x3& rotation_matrix)
{
	float4x4 world_matrix = frustum.WorldMatrix();

	world_matrix.SetRotatePart(rotation_matrix);

	this->GetOwner()->GetComponent<C_Transform>()->SetWorldTransform(world_matrix);											// Setting the updated world transform.
}

void C_Camera::Focus(const float3& target, const float& distance_to_target)
{
	float abs_distance = (distance_to_target < 0) ? -distance_to_target : distance_to_target;

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
	frustum.GetPlanes(frustum_planes);
}

void C_Camera::UpdateFrustumVertices()
{
	frustum.GetCornerPoints(frustum_vertices);
}

Plane* C_Camera::GetFrustumPlanes() const
{
	return frustum_planes;
}

float3* C_Camera::GetFrustumVertices() const
{
	return frustum_vertices;
}

bool C_Camera::FrustumCointainsAABB(const AABB& aabb) const
{
	float3 aabb_vertices[8];
	aabb.GetCornerPoints(aabb_vertices);

	uint planes_containing_all_verts = 0;																	// Can be used to check whether or not the geometric figure is intersected.

	for (uint p = 0; p < 6; ++p)
	{
		bool all_verts_are_in	= true;
		uint verts_in_plane		= 8;

		for (uint v = 0; v < 8; ++v)
		{
			if (frustum_planes[p].IsOnPositiveSide(aabb_vertices[v]))										// IsOnPositiveSide() returns true if the given point is behind the frustum plane.
			{
				all_verts_are_in = false;
				--verts_in_plane;
			}
		}

		if (verts_in_plane == 0)
		{
			return false;
		}

		if (all_verts_are_in)
		{
			++planes_containing_all_verts;
		}
	}

	if (planes_containing_all_verts == 6)
	{
		return true;
	}

	return false;
}

bool C_Camera::FrustumIntersectsAABB(const AABB& aabb) const
{
	float3 aabb_vertices[8];
	aabb.GetCornerPoints(aabb_vertices);

	for (uint p = 0; p < 6; ++p)
	{
		uint verts_in_plane = 8;

		for (uint v = 0; v < 8; ++v)
		{
			if (frustum_planes[p].IsOnPositiveSide(aabb_vertices[v]))
			{
				--verts_in_plane;
			}
		}

		if (verts_in_plane == 0)
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

void C_Camera::SetAspectRatio(const float& aspect_ratio)
{
	frustum.SetHorizontalFovAndAspectRatio(frustum.HorizontalFov(), aspect_ratio);
	
	UpdateFrustumPlanes();
	UpdateFrustumVertices();

	update_projection_matrix = true;
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

void C_Camera::SetNearPlaneDistance(const float& near_distance)
{
	if (near_distance < 0)
	{
		LOG("[ERROR] Component Camera: Cannot set the new distance of the near plane! Error: New near distance < 0.");
		return;
	}

	if (near_distance > frustum.FarPlaneDistance())
	{
		LOG("[ERROR] Component Camera: Cannot set the new distance of the near plane! Error: New near distance > far distance.");
		return;
	}

	frustum.SetViewPlaneDistances(near_distance, frustum.FarPlaneDistance());

	UpdateFrustumPlanes();
	UpdateFrustumVertices();

	update_projection_matrix = true;
}

void C_Camera::SetFarPlaneDistance(const float& far_distance)
{
	if (far_distance < 0)
	{
		LOG("[ERROR] Component Camera: Cannot set the new distance of the far plane! Error: New far distance < 0.");
		return;
	}

	if (far_distance < frustum.NearPlaneDistance())
	{
		LOG("[ERROR] Component Camera: Cannot set the new distance of the far plane! Error: New far distance < near distance");
		return;
	}

	frustum.SetViewPlaneDistances(frustum.NearPlaneDistance(), far_distance);

	UpdateFrustumPlanes();
	UpdateFrustumVertices();

	update_projection_matrix = true;
}

void C_Camera::SetHorizontalFOV(const float& horizontal_fov)
{
	/*if (horizontal_fov < min_fov)
	{
		LOG("[ERROR] Camera Component: Could not set the Horizontal FOV! Error: New Horizontal FOV < min_fov");
		return;
	}

	if (horizontal_fov > max_fov)
	{
		LOG("[ERROR] Camera Component: Could not set the Horizontal FOV! Error: New Horizontal FOV > max_fov");
		return;
	}*/
	
	frustum.SetHorizontalFovAndAspectRatio((horizontal_fov * DEGTORAD), frustum.AspectRatio());

	UpdateFrustumPlanes();
	UpdateFrustumVertices();

	update_projection_matrix = true;
}

void C_Camera::SetVerticalFOV(const float& vertical_fov)
{
	/*if (vertical_fov < min_fov)
	{
		LOG("[ERROR] Camera Component: Could not set the Vertical FOV! Error: New Vertical FOV < min_fov");
		return;
	}

	if (vertical_fov > max_fov)
	{
		LOG("[ERROR] Camera Component: Could not set the Vertical FOV! Error: New Vertical FOV > max_fov");
		return;
	}*/

	frustum.SetVerticalFovAndAspectRatio((vertical_fov * DEGTORAD), frustum.AspectRatio());

	UpdateFrustumPlanes();
	UpdateFrustumVertices();

	update_projection_matrix = true;
}

void C_Camera::GetMinMaxFOV(uint& min_fov, uint& max_fov) const
{
	min_fov = this->min_fov;
	max_fov = this->max_fov;
}

void C_Camera::SetMinMaxFOV(const uint& min_fov, const uint& max_fov)
{
	this->min_fov = min_fov;
	this->max_fov = max_fov;
}

bool C_Camera::IsCulling() const
{
	return is_culling;
}

bool C_Camera::OrthogonalView() const
{
	return in_orthogonal_view;
}

bool C_Camera::FrustumIsHidden() const
{
	return hide_frustum;
}

void C_Camera::SetIsCulling(const bool& set_to)
{
	is_culling = set_to;
	
	if (set_to)
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

void C_Camera::SetOrthogonalView(const bool& set_to)
{
	in_orthogonal_view = set_to;

	if (in_orthogonal_view)
	{
		// SET TO ORTHOGONAL
		// --- Near plane size = Far plane size
		// --- Update View Matrix
	}
}

void C_Camera::SetFrustumIsHidden(const bool& set_to)
{
	hide_frustum = set_to;
}