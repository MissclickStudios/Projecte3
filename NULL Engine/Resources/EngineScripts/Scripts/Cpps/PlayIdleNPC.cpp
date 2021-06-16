#include "PlayIdleNPC.h"
#include "GameObject.h"
#include "C_Animator.h"
#include "Random.h"

NPCidlePlay::NPCidlePlay() : Script()
{
}

NPCidlePlay::~NPCidlePlay()
{
}

void NPCidlePlay::Awake()
{
	animatorNPC = gameObject->GetComponent<C_Animator>();
}

void NPCidlePlay::Start()
{
	offset_toStart=Random::LCG::GetBoundedRandomFloat(1.f, 3.0f);
	startAnimationTimer.Start();

}

void NPCidlePlay::Update()
{

	if (startAnimationTimer.ReadSec() > offset_toStart) {

		if (!idle_playing) {
			animatorNPC->PlayClip("Preview", "idleNPC", 0u);
			idle_playing = true;
			startAnimationTimer.Stop();
		}
	}
}








