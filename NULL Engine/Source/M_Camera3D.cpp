#include "Time.h"

#include "Application.h"															// ATTENTION: Globals.h already included in Application.h.
#include "M_Window.h"
#include "M_Input.h"
#include "M_Editor.h"
#include "M_Scene.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_Camera.h"

#include "M_Camera3D.h"

#include "MemoryManager.h"

#define MOVEMENT_SPEED 24.0f
#define ROTATION_SPEED 0.05f
#define ZOOM_SPEED 300.0f

M_Camera3D::M_Camera3D(bool is_active) : Module("Camera3D", is_active),
master_camera		(nullptr),
current_camera		(nullptr),
draw_last_raycast	(false)
{
	CreateMasterCamera();

	position_origin		= float3(60.0f, 40.0f, 60.0f);								//
	reference_origin	= float3(0.0f, 0.0f, 0.0f);									//
	reference			= reference_origin;											// 

	movement_speed		= MOVEMENT_SPEED;
	rotation_speed		= ROTATION_SPEED;
	zoom_speed			= ZOOM_SPEED;
}

M_Camera3D::~M_Camera3D()
{
	current_camera = nullptr;
	
	master_camera->CleanUp();
	RELEASE(master_camera);
}

// -----------------------------------------------------------------
bool M_Camera3D::Init(ParsonNode& root)
{
	//Position.x = root.GetNumber("X");
	//Position.y = root.GetNumber("Y");
	//Position.z = root.GetNumber("Z");
	
	//master_camera->GetComponent<C_Transform>()->SetLocalPosition(float3(60.0f, 40.0f, 60.0f));
	master_camera->GetComponent<C_Transform>()->SetLocalPosition(float3(6.5f, 4.0f, 7.0f));
	LookAt(reference);
	//current_camera->UpdateFrustumTransform();

	return true;
}

// -----------------------------------------------------------------
bool M_Camera3D::Start()
{
	LOG("Setting up the camera");
	bool ret = true;

	return ret;
}

// -----------------------------------------------------------------
bool M_Camera3D::CleanUp()
{
	LOG("Cleaning camera");

	return true;
}

// -----------------------------------------------------------------
bool M_Camera3D::LoadConfiguration(ParsonNode& configuration)
{
	bool ret = true;

	//Position.x = root.GetNumber("X");
	//Position.y = root.GetNumber("Y");
	//Position.z = root.GetNumber("Z");

	return ret;
}

bool M_Camera3D::SaveConfiguration(ParsonNode& configuration) const
{
	bool ret = true;

	//root.SetNumber("X", Position.x);
	//root.SetNumber("Y", Position.y);
	//root.SetNumber("Z", Position.z);

	LOG("SAVED CAMERA INFO");

	return ret;
}

// -----------------------------------------------------------------
UPDATE_STATUS M_Camera3D::Update(float dt)
{
	if (App->editor->ViewportIsHovered())
	{	
		if (!App->editor->HoveringGuizmo())
		{
			if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_STATE::KEY_DOWN)
			{
				CastRay();
			}
		}
		
		if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_STATE::KEY_REPEAT)
		{
			WASDMovement();

			FreeLookAround();
		}

		if (App->input->GetKey(SDL_SCANCODE_LALT) == KEY_STATE::KEY_REPEAT)
		{
			if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_STATE::KEY_REPEAT)
			{
				if (App->scene->GetSelectedGameObject() != nullptr)
				{
					if (App->scene->GetSelectedGameObject()->GetComponent<C_Camera>() != current_camera)
					{
						reference = App->scene->GetSelectedGameObject()->GetComponent<C_Transform>()->GetWorldPosition();
					}
				}
				else
				{
					reference = float3::zero;
				}
				
				Orbit();
			}
		}

		if (App->input->GetMouseButton(SDL_BUTTON_MIDDLE) == KEY_STATE::KEY_REPEAT)
		{
			PanCamera();
		}

		if (App->input->GetMouseZ() != 0)
		{
			Zoom();
		}

		if (App->input->GetKey(SDL_SCANCODE_LCTRL) == KEY_STATE::KEY_IDLE)
		{
			if (App->input->GetKey(SDL_SCANCODE_O) == KEY_STATE::KEY_DOWN)
			{
				ReturnToWorldOrigin();
			}
		}

		if (App->input->GetKey(SDL_SCANCODE_F) == KEY_STATE::KEY_DOWN)
		{
			float3 target = App->scene->GetSelectedGameObject()->GetComponent<C_Transform>()->GetWorldPosition();
			Focus(target);
		}
	}

	return UPDATE_STATUS::CONTINUE;
}

// -----------------------------------------------------------------
void M_Camera3D::CreateMasterCamera()
{
	master_camera = new GameObject();
	master_camera->SetName("MasterCamera");
	master_camera->CreateComponent(COMPONENT_TYPE::CAMERA);
	
	C_Camera* c_camera = master_camera->GetComponent<C_Camera>();
	c_camera->SetFarPlaneDistance(1000.0f);
	SetCurrentCamera(c_camera);
	
	if (App != nullptr)
	{
		float win_width		= (float)App->window->GetWidth();
		float win_height	= (float)App->window->GetHeight();
		
		c_camera->SetAspectRatio(win_width/ win_height);
	}
}

C_Camera* M_Camera3D::GetCurrentCamera() const
{
	return current_camera;
}

void M_Camera3D::SetCurrentCamera(C_Camera* c_camera)
{
	if (c_camera == nullptr)
	{
		LOG("[ERROR] Camera: Could not set a new current camera! Error: Given Camera Component was nullptr.");
		return;
	}

	if (c_camera->GetOwner() == nullptr)																						// Highly unlikely case. Gets checked just to make sure.
	{
		LOG("[ERROR] Camera: Could not set a new current camera! Error: Given Camera Component's owner was nullptr.");
		return;
	}

	if (current_camera != nullptr)
	{
		current_camera->SetFrustumIsHidden(false);
	}
	
	c_camera->SetFrustumIsHidden(true);
	current_camera = c_camera;
	current_camera->SetUpdateProjectionMatrix(true);
	
	if (App != nullptr)																										// TMP (?)
	{
		current_camera->SetAspectRatio(((float)App->window->GetWidth()) / ((float)App->window->GetHeight()));
	}
}

void M_Camera3D::SetMasterCameraAsCurrentCamera()
{
	current_camera->SetFrustumIsHidden(false);
	
	if (master_camera == nullptr)
	{
		LOG("[ERROR] Camera: Could not set the master camera as the current camera! Error: Master Camera was nullptr.");
		LOG("[WARNING] Camera: Created a new Master Camera. Reason: Master Camera was nullptr!");
		
		CreateMasterCamera();
	}

	C_Camera* c_camera = master_camera->GetComponent<C_Camera>();
	if (c_camera == nullptr)
	{
		LOG("[ERROR] Camera: Could not set the master camera as the current camera! Error: Master Camera did not have a Camera Component.");
		LOG("[WARNING] Camera: Created a new Camera Component for the Master Camera. Reason: Master Camera did not have a Camera Component!");

		master_camera->CreateComponent(COMPONENT_TYPE::CAMERA);
	}

	current_camera = c_camera;
	current_camera->SetUpdateProjectionMatrix(true);
}

// -----------------------------------------------------------------
void M_Camera3D::PointAt(const float3& position, const float3& target, bool orbit)
{
	current_camera->PointAt(position, target);										
	reference = target;

	if(!orbit)
	{
		reference = position;

		Frustum frustum = current_camera->GetFrustum();
		current_camera->SetPosition(frustum.Pos() + frustum.Front() * 0.05f);
	}
}

// -----------------------------------------------------------------
void M_Camera3D::LookAt(const float3& spot)											// Almost identical to PointAt except only the reference and XYZ are updated. DOES NOT TRANSLATE.
{
	current_camera->LookAt(spot);
	reference = spot;
}

// -----------------------------------------------------------------
void M_Camera3D::Focus(const float3& target, const float& distance_from_target)
{
	current_camera->Focus(target, distance_from_target);
	reference = target;
}

// -----------------------------------------------------------------
void M_Camera3D::Move(const float3& velocity)
{
	current_camera->Move(velocity);
	reference += velocity;
}

// -----------------------------------------------------------------
void M_Camera3D::ReturnToWorldOrigin()
{
	PointAt(position_origin, reference_origin, true);
}

// -----------------------------------------------------------------
void M_Camera3D::WASDMovement()
{
	float3 new_position		= float3::zero;
	Frustum frustum			= current_camera->GetFrustum();
	float mov_speed			= movement_speed * Time::Real::GetDT();
	
	if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_STATE::KEY_REPEAT)								// --- CAMERA MOVEMEMENT BOOST
	{																									// 
		mov_speed = movement_speed * 2 * Time::Real::GetDT();											// 
	}																									// ---------------------------


	if (App->input->GetKey(SDL_SCANCODE_W) == KEY_STATE::KEY_REPEAT)									// --- FORWARD/BACKARD MOVEMENT (+Z/-Z)
	{																									// 
		new_position += frustum.Front() * mov_speed;													// 
	}																									// 
	if (App->input->GetKey(SDL_SCANCODE_S) == KEY_STATE::KEY_REPEAT)									// 
	{																									// 
		new_position -= frustum.Front() * mov_speed;													// 
	}																									// ----------------------------------------


	if (App->input->GetKey(SDL_SCANCODE_A) == KEY_STATE::KEY_REPEAT)									// --- LEFT/RIGHT MOVEMENT (STRAFE -X/+X)
	{																									// 										
		new_position -= frustum.WorldRight() * mov_speed;												// 										
	}																									// 										
	if (App->input->GetKey(SDL_SCANCODE_D) == KEY_STATE::KEY_REPEAT)									// 										
	{																									// 										
		new_position += frustum.WorldRight() * mov_speed;												// 										
	}																									// ----------------------------------------


	if (App->input->GetKey(SDL_SCANCODE_Q) == KEY_STATE::KEY_REPEAT)									// --- UPWARD/DOWNWARD MOVEMENT (+Y/-Y)
	{																									// 
		new_position += frustum.Up() * mov_speed;														// 
	}																									// 
	if (App->input->GetKey(SDL_SCANCODE_E) == KEY_STATE::KEY_REPEAT)									// 
	{																									// 
		new_position -= frustum.Up() * mov_speed;														// 
	}																									// ------------------------------------

	Move(new_position);
}

void M_Camera3D::FreeLookAround()
{
	/*Frustum frustum = current_camera->GetFrustum();
	float2 mouse_motion = App->editor->GetScreenMouseMotionThroughEditor();
	float sensitivity = rotation_speed * App->GetDt();

	float3 X = float3::zero;
	float3 Y = float3::zero;
	float3 Z = float3::zero;
	
	float3 new_Z = frustum.Pos() - reference;

	if (mouse_motion.x != 0.0f)
	{
		X = Quat(frustum.Up(), -mouse_motion.x * sensitivity).ToEulerXYZ();
	}

	if (mouse_motion.y != 0.0f)
	{
		Y = Quat(frustum.WorldRight(), -mouse_motion.y * sensitivity).ToEulerXYZ();
	}

	Z = X.Cross(Y);

	float3x3 rotation_matrix = float3x3(X, Y, Z);

	current_camera->Rotate(rotation_matrix);*/
	
	// Free Look
	/*int dx = -App->input->GetMouseXMotion();							// Motion value registered by the mouse in the X axis. Negated so the camera behaves like it should.
	int dy = -App->input->GetMouseYMotion();							// Motion value registered by the mouse in the Y axis. Negated so the camera behaves like it should.

	float sensitivity = rotation_speed * App->GetDt();					// Factor that will be applied to dx before constructing the angle with which to rotate the vectors.

	if (dx != 0)														// --- 
	{
		float delta_X = (float)dx * sensitivity;						// The value of the angle that we will rotate the camera by is very, very small, as it will be applied each frame.

		X = rotate(X, delta_X, vec3(0.0f, 1.0f, 0.0f));					// All vectors of the camera (X = Right, Y = Up, Z = Forward), will be rotated by the value of the angle (delta_X)
		Y = rotate(Y, delta_X, vec3(0.0f, 1.0f, 0.0f));					// The axis of rotation will be Y (yaw), not to confuse with the Y vector, which belongs to the camera.
		Z = rotate(Z, delta_X, vec3(0.0f, 1.0f, 0.0f));					// Keep in mind that X(Right) will always remain axis aligned.
	}

	if (dy != 0)														// Same as above but only affecting the Y and Z vectors, as X will act as the pivot of the rotation.
	{
		float delta_Y = (float)dy * sensitivity;

		Y = rotate(Y, delta_Y, X);										// As stated above, X(Right) will be used as the X axis (pitch) as, even if it is rotated, it will always be perfectly
		Z = rotate(Z, delta_Y, X);										// axis aligned in space, at least for this case.

		if (Y.y < 0.0f)													// If the y component of the Y(Up) vector is negative.
		{
			Z = vec3(0.0f, Z.y > 0.0f ? 1.0f : -1.0f, 0.0f);			// The y component of Z(Forward) will be recalculated.
			Y = cross(Z, X);											// A new Y(Up) vector orthogonal to both Z(Forward) and X(Right) (cross product) will be calculated.
		}
	}*/
}

void M_Camera3D::Orbit()								// Almost identical to FreeLookAround(), but instead of only modifying XYZ, the position of the camera is also modified.
{	
	Frustum frustum			= current_camera->GetFrustum();
	float2 mouse_motion		= App->editor->GetWorldMouseMotionThroughEditor();
	float sensitivity		= rotation_speed * Time::Real::GetDT();

	float3 new_Z = frustum.Pos() - reference;

	if (mouse_motion.x != 0.0f)
	{
		Quat new_X = Quat(frustum.Up(), -mouse_motion.x * sensitivity);
		new_Z = new_X.Transform(new_Z);
	}
	
	if (mouse_motion.y != 0.0f)
	{
		Quat new_Y = Quat(frustum.WorldRight(), -mouse_motion.y * sensitivity);
		new_Z = new_Y.Transform(new_Z);
	}
	
	float3 new_position = new_Z + reference;

	PointAt(new_position, reference, true);
}

void M_Camera3D::PanCamera()
{
	float3 new_X		= float3::zero;
	float3 new_Y		= float3::zero;
	float3 new_position = float3::zero;

	Frustum frustum		= current_camera->GetFrustum();
	float2 mouse_motion = App->editor->GetWorldMouseMotionThroughEditor();

	if (mouse_motion.x != 0)
	{
		new_X = -mouse_motion.x * frustum.WorldRight() * Time::Real::GetDT();
	}

	if (mouse_motion.y != 0)
	{
		new_Y = mouse_motion.y * frustum.Up() * Time::Real::GetDT();
	}

	new_position = new_X + new_Y;
	
	Move(new_position);
}

void M_Camera3D::Zoom()
{	
	Frustum frustum		= current_camera->GetFrustum();
	float3 new_Z		= frustum.Front() * (float)App->input->GetMouseZ() * zoom_speed * Time::Real::GetDT();

	Move(new_Z);
}

float3 M_Camera3D::GetPosition() const
{
	return current_camera->GetFrustum().Pos();
}

float3 M_Camera3D::GetReference() const
{
	return reference;
}

void M_Camera3D::SetPosition(const float3& position)
{
	current_camera->SetPosition(position);
}

void M_Camera3D::SetReference(const float3& reference)
{
	this->reference = reference;
}

float M_Camera3D::GetMovementSpeed() const
{
	return movement_speed;
}

float M_Camera3D::GetRotationSpeed() const
{
	return rotation_speed;
}

float M_Camera3D::GetZoomSpeed() const
{
	return zoom_speed;
}

void M_Camera3D::SetMovementSpeed(const float& movement_speed)
{
	this->movement_speed = movement_speed;
}

void M_Camera3D::SetRotationSpeed(const float& rotation_speed)
{
	this->rotation_speed = rotation_speed;
}

void M_Camera3D::SetZoomSpeed(const float& zoom_speed)
{
	this->zoom_speed = zoom_speed;
}

float3 M_Camera3D::GetMasterCameraPosition() const
{
	return master_camera->GetComponent<C_Transform>()->GetWorldPosition();
}

float3 M_Camera3D::GetMasterCameraRotation() const
{
	//return master_camera->GetTransformComponent()->GetWorldEulerRotation();
	return master_camera->GetComponent<C_Transform>()->GetLocalEulerRotation();
}

float3 M_Camera3D::GetMasterCameraScale() const
{
	return master_camera->GetComponent<C_Transform>()->GetWorldScale();
}

void M_Camera3D::SetMasterCameraPosition(const float3& position)
{
	master_camera->GetComponent<C_Transform>()->SetWorldPosition(position);
}

void M_Camera3D::SetMasterCameraRotation(const float3& rotation)
{
	//master_camera->GetTransformComponent()->SetWorldRotation(rotation);
	master_camera->GetComponent<C_Transform>()->SetLocalRotation(rotation);
}

void M_Camera3D::SetMasterCameraScale(const float3& scale)
{
	master_camera->GetComponent<C_Transform>()->SetWorldScale(scale);
}

void M_Camera3D::CastRay()
{	
	float2 mouse_pos = App->editor->GetWorldMousePositionThroughEditor();

	float norm_mouse_X = mouse_pos.x / (float)App->window->GetWidth();
	float norm_mouse_Y = mouse_pos.y / (float)App->window->GetHeight();

	float ray_origin_X = (norm_mouse_X - 0.5f) * 2;
	float ray_origin_Y = (norm_mouse_Y - 0.5f) * 2;
	
	last_raycast = current_camera->GetFrustum().UnProjectLineSegment(ray_origin_X, ray_origin_Y);

	App->scene->SelectGameObjectThroughRaycast(last_raycast);
}

bool M_Camera3D::DrawLastRaycast() const
{
	return draw_last_raycast;
}

void M_Camera3D::SetDrawLastRaycast(const bool& set_to)
{
	draw_last_raycast = set_to;
}