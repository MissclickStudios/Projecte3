#include "JSONParser.h"

#include "MC_Time.h"

#include "Application.h"
#include "Log.h"
#include "M_Window.h"
#include "M_Input.h"
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

	reference = root.GetFloat3("reference");

	masterCamera->GetComponent<C_Transform>()->SetLocalPosition(root.GetFloat3("cameraPosition"));
	masterCamera->GetComponent<C_Transform>()->SetLocalRotation(Quat(root.GetFloat4("cameraRotation").ptr()));

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
	configuration.SetFloat3("cameraPosition", masterCamera->transform->GetLocalPosition());
	configuration.SetFloat4("cameraRotation", float4(masterCamera->transform->GetLocalRotation().ptr()));

	configuration.SetFloat3("reference", reference);

	return true;
}

// -----------------------------------------------------------------
UpdateStatus M_Camera3D::Update(float dt)
{
	masterCamera->GetComponent<C_Transform>()->GetWorldTransform();

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

bool M_Camera3D::DrawLastRaycast() const
{
	return drawLastRaycast;
}

void M_Camera3D::SetDrawLastRaycast(const bool& setTo)
{
	drawLastRaycast = setTo;
}

void M_Camera3D::CastRay(const float2& castPoint)
{
	float2 mousePos = castPoint;

	float normMouseX = mousePos.x / (float)App->window->GetWidth();
	float normMouseY = mousePos.y / (float)App->window->GetHeight();

	float rayOriginX = (normMouseX - 0.5f) * 2;
	float rayOriginY = (normMouseY - 0.5f) * 2;

	lastRaycast = App->camera->currentCamera->GetFrustum().UnProjectLineSegment(rayOriginX, rayOriginY);

	App->scene->SelectGameObjectThroughRaycast(lastRaycast);
}

void M_Camera3D::FreeLookAround()
{
	int dx = -App->input->GetMouseXMotion();
	int dy = -App->input->GetMouseYMotion();

	float sensitivity = 0.003f;
	//looking = false;

	

	// x motion make the camera rotate in Y absolute axis (0,1,0) (not local)
	if (dx != 0.f)
	{
		//Quat q = Quat::RotateY(dx * sensitivity);
		//currentCamera.frustum.SetFront(q.Mul(currentCamera->frustum.Front()).Normalized());
		//currentCamera->frustum.SetUp(q.Mul(currentCamera->frustum.Up()).Normalized());

		//masterCamera->transform->Rotate(q);
	}

	// y motion makes the camera rotate in X local axis, with tops
	if (dy != 0.f)
	{
		float4x4 prevTransform = masterCamera->transform->GetLocalTransform();

		Quat q = Quat::RotateX(dy * sensitivity);

		//float3 newUp;

		/*if (newUp.y > 0.0f)
		{
			masterCamera->transform->Rotate(q);
		}*/

	}
}

void M_Camera3D::Orbit(const float2& orbitPoint)								// Almost identical to FreeLookAround(), but instead of only modifying XYZ, the position of the camera is also modified.
{
	Frustum frustum = currentCamera->GetFrustum();
	float sensitivity = rotationSpeed * MC_Time::Real::GetDT();

	float3 newZ = frustum.Pos() - GetReference();

	if (orbitPoint.x != 0.0f)
	{
		Quat newX = Quat(frustum.Up(), -orbitPoint.x * sensitivity);
		newZ = newX.Transform(newZ);
	}

	if (orbitPoint.y != 0.0f)
	{
		Quat newY = Quat(frustum.WorldRight(), -orbitPoint.y * sensitivity);
		newZ = newY.Transform(newZ);
	}

	float3 newPosition = newZ + reference;

	PointAt(newPosition, reference, true);
}

void M_Camera3D::PanCamera(const float2& panPoint)
{
	float3 newX = float3::zero;
	float3 newY = float3::zero;
	float3 newPosition = float3::zero;

	Frustum frustum = currentCamera->GetFrustum();

	if (panPoint.x != 0)
	{
		newX = -panPoint.x * frustum.WorldRight() * panSpeed * MC_Time::Real::GetDT();
	}

	if (panPoint.y != 0)
	{
		newY = panPoint.y * frustum.Up() * panSpeed * MC_Time::Real::GetDT();
	}

	newPosition = newX + newY;

	Move(newPosition);
}

void M_Camera3D::Zoom()
{
	Frustum frustum = currentCamera->GetFrustum();
	float3 newZ = frustum.Front() * (float)App->input->GetMouseZ() * zoomSpeed * MC_Time::Real::GetDT();

	Move(newZ);
}

// -----------------------------------------------------------------
void M_Camera3D::WASDMovement()
{
	float3 newPosition = float3::zero;
	Frustum frustum = currentCamera->GetFrustum();
	float movSpeed = movementSpeed * MC_Time::Real::GetDT();

	if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KeyState::KEY_REPEAT)								// --- CAMERA MOVEMEMENT BOOST
	{																									// 
		movSpeed = movementSpeed * 2 * MC_Time::Real::GetDT();											// 
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