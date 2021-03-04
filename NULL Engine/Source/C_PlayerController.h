#ifndef	__C_PLAYERCONTROLLER__
#define	__C_PLAYERCONTROLLER__

#include "Component.h"
#include "MathGeoLib/include/Math/float3.h"

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

private: 

	void Move();
	void Rotate();

	float speed = 2.0f;
	float acceleration = 10.0f;
	float deceleration = 10.0f;
	float maxSpeed = 1.0f;
};

#endif // !__C_PLAYERCONTROLLER__