#ifndef	__C_PLAYERMANAGER__
#define	__C_PLAYERMANAGER__

#include "Component.h"
#include "MathGeoLib/include/Math/float3.h"

class C_PlayerManager : public Component
{
public:

	C_PlayerManager(GameObject* owner);
	~C_PlayerManager();

	bool Update() override;
	bool CleanUp() override;

	void Move();		//
	void Rotate();
	void OnCollision();

public: 
	
	float3 center;
	float height;

	float currentVelocity;
	float minMoveDistance;

	//collision flags		//What part of the capsule collided with the environment
	//collider radius?
	//skin width?		    //distance that the collider allows being penetrated
	bool detectCollisions;

};

#endif // !__C_PLAYERMANAGER__
