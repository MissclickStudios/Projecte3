#include "Application.h"

#include "M_Window.h"
#include "M_Camera3D.h"

#include "GameObject.h"

#include "C_Camera.h"
#include "C_Transform.h"
#include "C_Transform2D.h"


C_Transform2D::C_Transform2D(GameObject* owner) : Component(owner, ComponentType::TRANSFORM2D)
{
	
}

C_Transform2D::~C_Transform2D()
{

}

bool C_Transform2D::Update()
{
	bool ret = true;

	return ret;
}

bool C_Transform2D::CleanUp()
{
	bool ret = true;

	return ret;
}

bool C_Transform2D::SaveState(ParsonNode& root) const
{
	bool ret = true;

	return ret;
}

bool C_Transform2D::LoadState(ParsonNode& root)
{
	bool ret = true;

	return ret;
}

float2 C_Transform2D::GetPosition()const
{
	return position;
}

float3 C_Transform2D::GetRotation()const
{
	return rotation;
}

float2 C_Transform2D::GetSize()const
{
	return size;
}

float4x4 C_Transform2D::GetGlobalMatrix() const
{
	return global_matrix;
}

float4x4 C_Transform2D::GetGlobalMatrixTransposed() const
{
	return global_matrix_transposed;
}

void C_Transform2D::SetTransformation(float3 pos, Quat rot, float2 size, float3 pivot)
{
	position = { pos.x,pos.y };
	z_depth = pos.z;
	rotation = QuaternionToEuler(rot);
	this->size = size;
	pivot_position = { pivot.x,pivot.y };

	relative_size.x = App->window->GetWidth() / size.x;
	relative_size.y = App->window->GetHeight() / size.y;
	relative_position.x = App->window->GetWidth() / position.x;
	relative_position.y = App->window->GetHeight() / position.y;
	needed_to_update = true;

}

void C_Transform2D::SetPivot(float3 pivot)
{
	pivot_position = { pivot.x,pivot.y };
}

void C_Transform2D::SetPosition(float2 pos)
{
	position = pos;
	relative_position.x = App->window->GetWidth() / position.x;
	relative_position.y = App->window->GetHeight() / position.y;
	needed_to_update = true;
}


void C_Transform2D::SetRotation(Quat rot)
{
	rotation = QuaternionToEuler(rot);
	needed_to_update = true;
}

void C_Transform2D::SetRotation(float3 rot)
{
	rotation = rot;
	needed_to_update = true;
}

void C_Transform2D::SetSize(float2 size)
{
	this->size = size;
	relative_size.x = App->window->GetWidth() / size.x;
	relative_size.y = App->window->GetHeight() / size.y;
	needed_to_update = true;
}

void C_Transform2D::UpdateZ()
{
	if (GetOwner()->components.size() > 1) {
		z_depth_with_layers = z_depth * ((UIElement*)GetOwner()->components[1])->uiLayer + z_depth;
	}
	else {
		z_depth_with_layers = z_depth;
	}
}

bool C_Transform2D::UpdateMatrixBillboard()
{
	C_Transform* trans = App->camera->GetCurrentCamera()->GetOwner()->GetComponent<C_Transform>();
	Quat rot = trans->GetWorldRotation();
	float3 pos = trans->GetWorldPosition();
	float4x4 matrix_billboard_last_frame = matrix_billboard;
	matrix_billboard = matrix_billboard.FromTRS(pos, rot, { 1,1,1 });

	return matrix_billboard.Equals(matrix_billboard_last_frame);

}

void C_Transform2D::SetMatrices()
{

	float3 pivot_world = { pivot_position.x + position.x, pivot_position.y + position.y, 0 };
	float3 rotation_in_gradians = rotation * DEGTORAD;
	Quat rotate = Quat::identity * Quat::identity.RotateZ(rotation_in_gradians.z);

	matrix_pivot = matrix_pivot.FromTRS(pivot_world, rotate, { 1,1,1 });


	local_matrix = local_matrix.FromTRS({ -pivot_position.x,-pivot_position.y,z_depth_with_layers }, Quat::identity, { size.x,size.y,1 });

	if (GetOwner()->parent != nullptr) {
		C_Transform2D* parent_trans2D = GetOwner()->parent->GetComponent<C_Transform2D>();
		if (parent_trans2D != nullptr) {
			matrix_parent = parent_trans2D->GetGlobalMatrix();
			matrix_parent = parent_trans2D->matrix_billboard.Inverted() * matrix_parent;
		}
		else {
			C_Transform* parent_trans = GetOwner()->parent->GetComponent<C_Transform>();
			matrix_parent = parent_trans->GetWorldTransform();
		}
	}
	else matrix_parent = float4x4::identity;

	global_matrix = matrix_billboard * matrix_parent * matrix_pivot * local_matrix;
	global_matrix_transposed = global_matrix.Transposed();
}

void C_Transform2D::SetMatricesWithNewParent(float4x4 parent_global_matrix)
{
	UpdateMatrixBillboard();
	float4x4 local_matrix_with_rotation = parent_global_matrix.Inverted() * matrix_billboard.Inverted() * global_matrix;
	float3 pos, s;
	Quat rot;
	local_matrix_with_rotation.Decompose(pos, rot, s);
	rotation = QuaternionToEuler(rot);

	local_matrix = matrix_pivot.Inverted() * local_matrix_with_rotation;

	local_matrix.Decompose(pos, rot, s);

	position = { pos.x,pos.y };
	z_depth = pos.z;
	if (GetOwner()->components.size() > 1) {
		z_depth_with_layers = z_depth * ((UIElement*)GetOwner()->components[1])->uiLayer + z_depth;
	}
	else {
		z_depth_with_layers = z_depth;
	}
	size = { s.x,s.y };
	relative_size.x = App->window->GetWidth() / size.x;
	relative_size.y = App->window->GetHeight() / size.y;
	relative_position.x = App->window->GetWidth() / position.x;
	relative_position.y = App->window->GetHeight() / position.y;
	needed_to_update = true;
	needed_to_update_only_children = true;
}


float3 C_Transform2D::QuaternionToEuler(Quat q)
{
	float3 rotation_euler = q.ToEulerXYZ();
	rotation_euler *= RADTODEG;
	return rotation_euler;
}

Quat C_Transform2D::EulerToQuaternion(float3 eu)
{
	eu *= DEGTORAD;
	Quat q = Quat::FromEulerXYZ(eu.x, eu.y, eu.z);
	return q;
}

Quat C_Transform2D::LookAt(const float3& point)
{

	float3 vector = { position.x, position.x, 0 };
	vector = point - vector;

	float3x3 matrix = float3x3::LookAt(float3::unitZ, vector.Normalized(), float3::unitY, float3::unitY);
	return matrix.ToQuat();

}

float2 C_Transform2D::CalculateMovement(float4x4 matrix, float2 goal)
{
	float2 movement;
	movement.y = (matrix[0][0] * (goal.y - matrix[1][3]) - matrix[1][0] * (goal.x - matrix[0][3])) / (matrix[1][1] * matrix[0][0] - matrix[0][1] * matrix[1][0]);
	movement.x = (goal.x - matrix[0][1] * movement.y - matrix[0][3]) / matrix[0][0];
	float second_option = (goal.y - matrix[1][1] * movement.y - matrix[1][3]) / matrix[1][0];
	if (movement.x != goal.x && ((second_option - goal.x<0 && second_option - goal.x> movement.x) || (second_option - goal.x > 0 && second_option - goal.x < movement.x))) {
		movement.x = second_option;
	}

	return movement;
}

void C_Transform2D::GetTrianglePoints(float2& min_p, float2& max_p, float2& third_p, float2& third_p_second_tri)
{

	
	Quat q = q.RotateZ(-rotation.z * DEGTORAD);

	for (int i = 0; i < 4; i++) {
		float2 point;
		if (i == 0) {
			point = min_p;
		}
		else if (i == 1) {
			point = max_p;
		}
		else if (i == 2) {
			point = third_p;
		}
		else {
			point = third_p_second_tri;
		}

		point.x *= size.x;
		point.y *= size.y;


		point.x -= pivot_position.x;
		point.y += pivot_position.y;

		//ROTATE
		float3 point3 = { point.x,point.y,0 };
		point3 = q * point3;

		point = { point3.x,point3.y };

		point.x += pivot_position.x;
		point.y -= pivot_position.y;

		point.x += position.x;
		point.y -= position.y;

		point.x += App->window->GetWidth() / 2;
		point.y += App->window->GetHeight() / 2;

		if (i == 0) {
			min_p = point;
		}
		else if (i == 1) {
			max_p = point;
		}
		else if (i == 2) {
			third_p = point;
		}
		else {
			third_p_second_tri = point;
		}
	}

}

float C_Transform2D::GetDepth()
{
	return z_depth;
}
