#include "Time.h"

#include "Application.h"															// ATTENTION: Globals.h already included in Application.h.
#include "M_Window.h"
#include "M_Input.h"
#include "M_Editor.h"
#include "M_Scene.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_Camera.h"
#include "C_AudioListener.h"

#include "M_Camera3D.h"

#include "MemoryManager.h"

#define MOVEMENT_SPEED 24.0f
#define ROTATION_SPEED 0.05f
#define ZOOM_SPEED 300.0f

M_Camera3D::M_Camera3D(bool isActive) : Module("Camera3D", isActive),
masterCamera		(nullptr),
currentCamera		(nullptr),
drawLastRaycast	(false)
{
	CreateMasterCamera();

	positionOrigin		= float3(60.0f, 40.0f, 60.0f);								//
	referenceOrigin		= float3(0.0f, 0.0f, 0.0f);									//
	reference			= referenceOrigin;											// 

	movementSpeed		= MOVEMENT_SPEED;
	rotationSpeed		= ROTATION_SPEED;
	zoomSpeed			= ZOOM_SPEED;
}

M_Camera3D::~M_Camera3D()
{
	currentCamera = nullptr;
	
	masterCamera->CleanUp();
	RELEASE(masterCamera);
}

// -----------------------------------------------------------------
bool M_Camera3D::Init(ParsonNode& root)
{
	//Position.x = root.GetNumber("X");
	//Position.y = root.GetNumber("Y");
	//Position.z = root.GetNumber("Z");
	
	//master_camera->GetComponent<C_Transform>()->SetLocalPosition(float3(60.0f, 40.0f, 60.0f));
	//masterCamera->GetComponent<C_Transform>()->SetLocalPosition(float3(6.5f, 4.0f, 7.0f));
	masterCamera->GetComponent<C_Transform>()->SetLocalPosition(float3(125.0f, 80.0f, 135.0f));
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
UpdateStatus M_Camera3D::Update(float dt)
{
	if (App->editor->ViewportIsHovered())
	{	
		if (!App->editor->HoveringGuizmo())
		{
			if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KeyState::KEY_DOWN)
			{
				CastRay();
			}
		}
		
		if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KeyState::KEY_REPEAT)
		{
			WASDMovement();

			FreeLookAround();
		}

		if (App->input->GetKey(SDL_SCANCODE_LALT) == KeyState::KEY_REPEAT)
		{
			if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KeyState::KEY_REPEAT)
			{
				if (App->scene->GetSelectedGameObject() != nullptr)
				{
					if (App->scene->GetSelectedGameObject()->GetComponent<C_Camera>() != currentCamera)
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

		if (App->input->GetMouseButton(SDL_BUTTON_MIDDLE) == KeyState::KEY_REPEAT)
		{
			PanCamera();
		}

		if (App->input->GetMouseZ() != 0)
		{
			Zoom();
		}

		if (App->input->GetKey(SDL_SCANCODE_LCTRL) == KeyState::KEY_IDLE)
		{
			if (App->input->GetKey(SDL_SCANCODE_O) == KeyState::KEY_DOWN)
			{
				ReturnToWorldOrigin();
			}
		}

		if (App->input->GetKey(SDL_SCANCODE_F) == KeyState::KEY_DOWN)
		{
			float3 target = App->scene->GetSelectedGameObject()->GetComponent<C_Transform>()->GetWorldPosition();
			Focus(target);
		}
	}

	return UpdateStatus::CONTINUE;
}

// -----------------------------------------------------------------
void M_Camera3D::CreateMasterCamera()
{
	masterCamera = new GameObject();
	masterCamera->SetName("MasterCamera");
	masterCamera->CreateComponent(ComponentType::CAMERA);
	
	C_Camera* cCamera = masterCamera->GetComponent<C_Camera>();
	cCamera->SetFarPlaneDistance(1000.0f);
	SetCurrentCamera(cCamera);
	
	if (App != nullptr)
	{
		float winWidth		= (float)App->window->GetWidth();
		float winHeight	= (float)App->window->GetHeight();
		
		cCamera->SetAspectRatio(winWidth/ winHeight);
	}
}

C_Camera* M_Camera3D::GetCurrentCamera() const
{
	return currentCamera;
}

void M_Camera3D::SetCurrentCamera(C_Camera* cCamera)
{
	if (cCamera == nullptr)
	{
		LOG("[ERROR] Camera: Could not set a new current camera! Error: Given Camera Component was nullptr.");
		return;
	}

	if (cCamera->GetOwner() == nullptr)																						// Highly unlikely case. Gets checked just to make sure.
	{
		LOG("[ERROR] Camera: Could not set a new current camera! Error: Given Camera Component's owner was nullptr.");
		return;
	}

	if (currentCamera != nullptr)
	{
		currentCamera->SetFrustumIsHidden(false);
	}
	
	cCamera->SetFrustumIsHidden(true);
	currentCamera = cCamera;
	currentCamera->SetUpdateProjectionMatrix(true);
	
	if (App != nullptr)																										// TMP (?)
	{
		currentCamera->SetAspectRatio(((float)App->window->GetWidth()) / ((float)App->window->GetHeight()));
	}
}

void M_Camera3D::SetMasterCameraAsCurrentCamera()
{
	currentCamera->SetFrustumIsHidden(false);
	
	if (masterCamera == nullptr)
	{
		LOG("[ERROR] Camera: Could not set the master camera as the current camera! Error: Master Camera was nullptr.");
		LOG("[WARNING] Camera: Created a new Master Camera. Reason: Master Camera was nullptr!");
		
		CreateMasterCamera();
	}

	C_Camera* cCamera = masterCamera->GetComponent<C_Camera>();
	if (cCamera == nullptr)
	{
		LOG("[ERROR] Camera: Could not set the master camera as the current camera! Error: Master Camera did not have a Camera Component.");
		LOG("[WARNING] Camera: Created a new Camera Component for the Master Camera. Reason: Master Camera did not have a Camera Component!");

		masterCamera->CreateComponent(ComponentType::CAMERA);
	}

	currentCamera = cCamera;
	currentCamera->SetUpdateProjectionMatrix(true);
}

// -----------------------------------------------------------------
void M_Camera3D::PointAt(const float3& position, const float3& target, bool orbit)
{
	currentCamera->PointAt(position, target);										
	reference = target;

	if(!orbit)
	{
		reference = position;

		Frustum frustum = currentCamera->GetFrustum();
		currentCamera->SetPosition(frustum.Pos() + frustum.Front() * 0.05f);
	}
}

// -----------------------------------------------------------------
void M_Camera3D::LookAt(const float3& spot)											// Almost identical to PointAt except only the reference and XYZ are updated. DOES NOT TRANSLATE.
{
	currentCamera->LookAt(spot);
	reference = spot;
}

// -----------------------------------------------------------------
void M_Camera3D::Focus(const float3& target, const float& distanceFromTarget)
{
	currentCamera->Focus(target, distanceFromTarget);
	reference = target;
}

// -----------------------------------------------------------------
void M_Camera3D::Move(const float3& velocity)
{
	currentCamera->Move(velocity);
	reference += velocity;
}

// -----------------------------------------------------------------
void M_Camera3D::ReturnToWorldOrigin()
{
	PointAt(positionOrigin, referenceOrigin, true);
}

// -----------------------------------------------------------------
void M_Camera3D::WASDMovement()
{
	float3 newPosition		= float3::zero;
	Frustum frustum			= currentCamera->GetFrustum();
	float movSpeed			= movementSpeed * Time::Real::GetDT();
	
	if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KeyState::KEY_REPEAT)								// --- CAMERA MOVEMEMENT BOOST
	{																									// 
		movSpeed = movementSpeed * 2 * Time::Real::GetDT();											// 
	}																									// ---------------------------


	if (App->input->GetKey(SDL_SCANCODE_W) == KeyState::KEY_REPEAT)									// --- FORWARD/BACKARD MOVEMENT (+Z/-Z)
	{																									// 
		newPosition += frustum.Front() * movSpeed;													// 
	}																									// 
	if (App->input->GetKey(SDL_SCANCODE_S) == KeyState::KEY_REPEAT)									// 
	{																									// 
		newPosition -= frustum.Front() * movSpeed;													// 
	}																									// ----------------------------------------


	if (App->input->GetKey(SDL_SCANCODE_A) == KeyState::KEY_REPEAT)									// --- LEFT/RIGHT MOVEMENT (STRAFE -X/+X)
	{																									// 										
		newPosition -= frustum.WorldRight() * movSpeed;												// 										
	}																									// 										
	if (App->input->GetKey(SDL_SCANCODE_D) == KeyState::KEY_REPEAT)									// 										
	{																									// 										
		newPosition += frustum.WorldRight() * movSpeed;												// 										
	}																									// ----------------------------------------


	if (App->input->GetKey(SDL_SCANCODE_Q) == KeyState::KEY_REPEAT)									// --- UPWARD/DOWNWARD MOVEMENT (+Y/-Y)
	{																									// 
		newPosition += frustum.Up() * movSpeed;														// 
	}																									// 
	if (App->input->GetKey(SDL_SCANCODE_E) == KeyState::KEY_REPEAT)									// 
	{																									// 
		newPosition -= frustum.Up() * movSpeed;														// 
	}																									// ------------------------------------

	Move(newPosition);
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
	Frustum frustum			= currentCamera->GetFrustum();
	float2 mouseMotion		= App->editor->GetWorldMouseMotionThroughEditor();
	float sensitivity		= rotationSpeed * Time::Real::GetDT();

	float3 newZ = frustum.Pos() - reference;

	if (mouseMotion.x != 0.0f)
	{
		Quat newX = Quat(frustum.Up(), -mouseMotion.x * sensitivity);
		newZ = newX.Transform(newZ);
	}
	
	if (mouseMotion.y != 0.0f)
	{
		Quat newY = Quat(frustum.WorldRight(), -mouseMotion.y * sensitivity);
		newZ = newY.Transform(newZ);
	}
	
	float3 newPosition = newZ + reference;

	PointAt(newPosition, reference, true);
}

void M_Camera3D::PanCamera()
{
	float3 newX		= float3::zero;
	float3 newY		= float3::zero;
	float3 newPosition = float3::zero;

	Frustum frustum		= currentCamera->GetFrustum();
	float2 mouseMotion = App->editor->GetWorldMouseMotionThroughEditor();

	if (mouseMotion.x != 0)
	{
		newX = -mouseMotion.x * frustum.WorldRight() * Time::Real::GetDT();
	}

	if (mouseMotion.y != 0)
	{
		newY = mouseMotion.y * frustum.Up() * Time::Real::GetDT();
	}

	newPosition = newX + newY;
	
	Move(newPosition);
}

void M_Camera3D::Zoom()
{	
	Frustum frustum		= currentCamera->GetFrustum();
	float3 newZ		= frustum.Front() * (float)App->input->GetMouseZ() * zoomSpeed * Time::Real::GetDT();

	Move(newZ);
}

float3 M_Camera3D::GetPosition() const
{
	return currentCamera->GetFrustum().Pos();
}

float3 M_Camera3D::GetReference() const
{
	return reference;
}

void M_Camera3D::SetPosition(const float3& position)
{
	currentCamera->SetPosition(position);
}

void M_Camera3D::SetReference(const float3& reference)
{
	this->reference = reference;
}

float M_Camera3D::GetMovementSpeed() const
{
	return movementSpeed;
}

float M_Camera3D::GetRotationSpeed() const
{
	return rotationSpeed;
}

float M_Camera3D::GetZoomSpeed() const
{
	return zoomSpeed;
}

void M_Camera3D::SetMovementSpeed(const float& movementSpeed)
{
	this->movementSpeed = movementSpeed;
}

void M_Camera3D::SetRotationSpeed(const float& rotationSpeed)
{
	this->rotationSpeed = rotationSpeed;
}

void M_Camera3D::SetZoomSpeed(const float& zoomSpeed)
{
	this->zoomSpeed = zoomSpeed;
}

float3 M_Camera3D::GetMasterCameraPosition() const
{
	return masterCamera->GetComponent<C_Transform>()->GetWorldPosition();
}

float3 M_Camera3D::GetMasterCameraRotation() const
{
	//return master_camera->GetTransformComponent()->GetWorldEulerRotation();
	return masterCamera->GetComponent<C_Transform>()->GetLocalEulerRotation();
}

float3 M_Camera3D::GetMasterCameraScale() const
{
	return masterCamera->GetComponent<C_Transform>()->GetWorldScale();
}

void M_Camera3D::SetMasterCameraPosition(const float3& position)
{
	masterCamera->GetComponent<C_Transform>()->SetWorldPosition(position);
}

void M_Camera3D::SetMasterCameraRotation(const float3& rotation)
{
	//master_camera->GetTransformComponent()->SetWorldRotation(rotation);
	masterCamera->GetComponent<C_Transform>()->SetLocalRotation(rotation);
}

void M_Camera3D::SetMasterCameraScale(const float3& scale)
{
	masterCamera->GetComponent<C_Transform>()->SetWorldScale(scale);
}

void M_Camera3D::CastRay()
{	
	float2 mousePos = App->editor->GetWorldMousePositionThroughEditor();

	float normMouseX = mousePos.x / (float)App->window->GetWidth();
	float normMouseY = mousePos.y / (float)App->window->GetHeight();

	float rayOriginX = (normMouseX - 0.5f) * 2;
	float rayOriginY = (normMouseY - 0.5f) * 2;
	
	lastRaycast = currentCamera->GetFrustum().UnProjectLineSegment(rayOriginX, rayOriginY);

	App->scene->SelectGameObjectThroughRaycast(lastRaycast);
}

bool M_Camera3D::DrawLastRaycast() const
{
	return drawLastRaycast;
}

void M_Camera3D::SetDrawLastRaycast(const bool& setTo)
{
	drawLastRaycast = setTo;
}