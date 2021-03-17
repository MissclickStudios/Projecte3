#ifndef	__C_PLAYERCONTROLLER__
#define	__C_PLAYERCONTROLLER__

#include "Component.h"
#include "MathGeoLib/include/Math/float3.h"

class C_AudioSource;
class C_RigidBody;


enum class Direction {
	NORTH,
	NORTH_WEST,
	WEST,
	SOUTH_WEST,
	SOUTH,
	SOUTH_EAST,
	EAST,
	NORTH_EAST,
};

class C_PlayerController : public Component
{
public:

	C_PlayerController(GameObject* owner);
	~C_PlayerController();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	static inline ComponentType GetType() { return ComponentType::PLAYER_CONTROLLER; }			// This is needed to be able to use templates for functions such as GetComponent<>();

	const float Speed() const { return speed; }
	void SetSpeed(float speed) { this->speed = speed; }

	const float Acceleration() const { return acceleration; }
	void SetAcceleration(float force) { this->acceleration = force; }
	const float Deceleration() const { return deceleration; }
	//The deceleration is internaly inverted, so setting a negative deceleration will cause an acceleration
	void SetDeceleration(float force) { this->deceleration = force; }



	const float BulletSpeed() const { return bulletSpeed; }
	void SetBulletSpeed(float speed) { bulletSpeed = speed; }

	Direction ReturnPlayerDirection();

	int dashCooldown = 50;

private: 

	void Move(C_RigidBody* rigidBody);
	void Dash(C_RigidBody * rigidBody,bool forward, bool backward, bool right, bool left);
	void Rotate();

	void StepSound(bool a, bool b, bool c, bool d );

	float2 MousePositionToWorldPosition(float mapPositionY = 0);

	float2 GetMovementVectorAxis();
	float2 GetAimVectorAxis();


	float speed = 30.0f;

	Direction playerDirection = Direction::NORTH;

	float acceleration = 2.0f;
	float deceleration = 2.0f;

	float bulletSpeed = 100.0f;

	bool rightDash = false;
	bool leftDash = false;

	//Temporal dash timer cause there is no dt in update method  
	int dashTimer = 0;

	float dashForce = 73200.0f;
};

#endif // !__C_PLAYERCONTROLLER__