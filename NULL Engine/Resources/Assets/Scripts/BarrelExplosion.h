#include "Script.h"
#include "ScriptMacros.h"
#include "MathGeoLib/include/Math/float3.h"

class GameObject;
class C_BoxCollider;
class C_AudioSource;
class Object;
class ExplosiveBarrel;

class SCRIPTS_API BarrelExplosion : public Object ALLOWED_INHERITANCE 
{
public:

	BarrelExplosion();
	~BarrelExplosion();

	void Start()override;
	void Update() override;
	void CleanUp()override;

	void OnTriggerRepeat(GameObject* object) override;

	int damage = 0;
	float power = 20.0f;
	float stunTime = 0.75f;

	bool stun = false;
};

SCRIPTS_FUNCTION BarrelExplosion* CreateBarrelExplosion();