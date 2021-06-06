
#include "Script.h"
#include "ScriptMacros.h"
#include "Timer.h"
#include <vector>
#include <string>



class GameObject;
class C_Animator;

class SCRIPTS_API NPCidlePlay : public Script
{
public:

	NPCidlePlay();
	virtual ~NPCidlePlay();

	void Awake() override;
	void Start() override;
	void Update() override;


	std::string gameManagerName = "Game Manager";
	std::string playerName = "Mandalorian";


	C_Animator* animatorNPC;


	Timer startAnimationTimer;

	float offset_toStart;

	bool idle_playing = false;


};


SCRIPTS_FUNCTION NPCidlePlay* CreateNPCidlePlay() {


	NPCidlePlay* script = new NPCidlePlay();
	return script;
}



