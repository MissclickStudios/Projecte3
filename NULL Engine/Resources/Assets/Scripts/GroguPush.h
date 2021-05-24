#include "Script.h"
#include "ScriptMacros.h"
#include "MathGeoLib/include/Math/float3.h"

class GameObject;
class C_BoxCollider;
class C_AudioSource;
class GameManager;

class SCRIPTS_API GroguPush : public Object ALLOWED_INHERITANCE
{
public:

	GroguPush();
	~GroguPush();

	void Start() override;
	void Update() override;
	void CleanUp() override;

	void OnTriggerRepeat(GameObject* object) override;
	void OnTriggerExit(GameObject* object) override;
	float3 abilityRadius = float3::one;

	float abilityPower = 2000000.0f;
	float abilityCooldown = 5.0f;

private:

	GameObject* player = nullptr;
	std::string playerName = "Mandalorian";

	C_BoxCollider* abilityCollider = nullptr;

	bool toExplode = false;
	bool exploded = false;

	Timer abilityCooldownTimer;

	GameManager* gameManager = nullptr;

public:

	bool doAbility = false;
};
//
SCRIPTS_FUNCTION GroguPush* CreateGroguPush();