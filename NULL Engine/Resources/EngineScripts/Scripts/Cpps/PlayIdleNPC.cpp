#include "PlayIdleNPC.h"
#include "GameObject.h"
#include "C_Animator.h"

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

}

void NPCidlePlay::Update()
{
	animatorNPC->PlayClip("Preview", "idleNPC", 0u);
}








