#include "MathGeoLib/include/Geometry/LineSegment.h"

#include "Profiler.h"
#include "JSONParser.h"

#include "Time.h"

#include "Channel.h"
#include "AnimatorClip.h"

#include "Application.h"
#include "FileSystemDefinitions.h"

#include "M_ResourceManager.h"

#include "R_Animation.h"

#include "GameObject.h"
#include "C_Transform.h"

#include "C_Animator.h"

#include "MemoryManager.h"

typedef std::map<double, float3>::const_iterator	PositionKeyframe;
typedef std::map<double, Quat>::const_iterator		RotationKeyframe;
typedef std::map<double, float3>::const_iterator	ScaleKeyframe;

C_Animator::C_Animator(GameObject* owner) : Component(owner, ComponentType::ANIMATOR),
currentClip		(nullptr),
blendingClip	(nullptr),
currentRootBone	(nullptr)
{
	blendFrames = 0;

	play	= false;
	pause	= false;
	step	= false;
	stop	= true;

	playbackSpeed	= 1.0f;
	interpolate		= true;
	loopAnimation	= false;
	playOnStart		= true;
	cameraCulling	= true;
	showBones		= false;
}

C_Animator::~C_Animator()
{
	currentClip		= nullptr;
	blendingClip	= nullptr;
	currentRootBone	= nullptr;
}

bool C_Animator::Update()
{
	BROFILERCATEGORY("Animation Component Update", Profiler::Color::DarkSlateBlue);
	
	bool ret = true;

	AddAnimationsToAdd();

	if (play || step)
	{
		if (currentClip != nullptr)
		{
			StepAnimation();
		}

		step = false;
	}

	return ret;
}

bool C_Animator::CleanUp()
{
	bool ret = true;

	for (uint i = 0; i < animations.size(); ++i)
	{
		App->resourceManager->FreeResource(animations[i]->GetUID());
	}

	animations.clear();
	animationBones.clear();
	clips.clear();

	currentBones.clear();
	blendingBones.clear();

	displayBones.clear();

	animationsToAdd.clear();

	return ret;
}

bool C_Animator::SaveState(ParsonNode& root) const
{
	bool ret = true;

	root.SetNumber("Type", (double)GetType());

	// Animations
	ParsonArray animationsArray = root.SetArray("Animations");
	for (auto animation = animations.cbegin(); animation != animations.cend(); ++animation)
	{
		ParsonNode animationNode = animationsArray.SetNode((*animation)->GetName());

		animationNode.SetNumber("UID", (*animation)->GetUID());
		animationNode.SetString("Name", (*animation)->GetAssetsFile());
		animationNode.SetString("Path", (*animation)->GetLibraryPath());
		animationNode.SetString("File", (*animation)->GetLibraryFile());
	}

	// Clips
	ParsonArray clipsArray = root.SetArray("Clips");
	for (auto clip = clips.cbegin(); clip != clips.cend(); ++clip)
	{
		if (strstr(clip->first.c_str(), "Default") != nullptr)
		{
			continue;
		}
		
		ParsonNode clipNode = clipsArray.SetNode(clip->second.GetName());
		clip->second.SaveState(clipNode);
	}

	// Current Clip
	if (currentClip != nullptr)
	{
		ParsonNode currentClipNode = root.SetNode("CurrentClip");
		currentClipNode.SetString("AnimationName", currentClip->GetAnimationName());
		currentClipNode.SetString("Name", currentClip->GetName());
	}

	return ret;
}

bool C_Animator::LoadState(ParsonNode& root)
{
	bool ret = true;

	ParsonArray animationsArray = root.GetArray("Animations");
	for (uint i = 0; i < animationsArray.size; ++i)
	{
		ParsonNode animationNode = animationsArray.GetNode(i);
		if (!animationNode.NodeIsValid())
		{
			continue;
		}

		std::string assetsPath = ASSETS_MODELS_PATH + std::string(animationNode.GetString("Name"));
		App->resourceManager->AllocateResource((uint32)animationNode.GetNumber("UID"), assetsPath.c_str());
		
		R_Animation* rAnimation = (R_Animation*)App->resourceManager->RequestResource((uint32)animationNode.GetNumber("UID"));
		if (rAnimation != nullptr)
		{
			animationsToAdd.push_back(rAnimation);
			//AddAnimation(rAnimation);
		}
	}

	ParsonArray clipsArray = root.GetArray("Clips");
	for (uint i = 0; i < clipsArray.size; ++i)
	{
		ParsonNode clipNode = clipsArray.GetNode(i);
		AnimatorClip clip = AnimatorClip();

		clip.LoadState(clipNode);

		clips.emplace(clip.GetName(), clip);
	}

	ParsonNode currentClipNode = root.GetNode("CurrentClip");
	auto item = clips.find(currentClipNode.GetString("Name"));
	if (item != clips.end())
	{
		SetCurrentClip(&item->second);
	}

	return ret;
}

// --- C_ANIMATION METHODS ---
bool C_Animator::StepAnimation()
{
	bool ret = true;

	if (!CurrentClipIsValid())
	{
		return false;
	}
	
	bool success = StepClips();
	if (!success)
	{
		return false;
	}

	for (uint i = 0; i < currentBones.size(); ++i)
	{
		const BoneLink& bone = currentBones[i];
		
		C_Transform* cTransform = bone.gameObject->GetComponent<C_Transform>();
		if (cTransform == nullptr)
		{
			LOG("[WARNING] Animation Component: GameObject { %s } did not have a Transform Component!", bone.gameObject->GetName());
			continue;
		}

		const Transform& originalTransform = Transform(cTransform->GetLocalTransform());
		
		if (interpolate)
		{
			Transform& interpolatedTransform = GetInterpolatedTransform(currentClip->GetAnimationFrame(), bone.channel, originalTransform);
			
			//LOG("ANIMATION FRAME { %.3f }, ANIMATION TICK { %u }", currentClip->GetAnimationFrame(), currentClip->GetAnimationTick());

			if (BlendingClipExists())
			{
				interpolatedTransform = GetBlendedTransform(blendingClip->GetAnimationFrame(), blendingBones[i].channel, interpolatedTransform);
			}

			cTransform->ImportTransform(interpolatedTransform);
		}
		else
		{
			if (currentClip->inNewTick)
			{
				Transform& poseToPoseTransform = GetPoseToPoseTransform(currentClip->GetAnimationTick(), bone.channel, originalTransform);

				if (BlendingClipExists())
				{
					poseToPoseTransform = GetBlendedTransform(blendingClip->GetAnimationTick(), blendingBones[i].channel, poseToPoseTransform);
				}

				cTransform->ImportTransform(poseToPoseTransform);
			}
		}
	}

	UpdateDisplayBones();

	return ret;
}

bool C_Animator::StepClips()
{
	bool ret = true;

	bool currentExists	= CurrentClipExists();
	bool blendingExists	= BlendingClipExists();

	if (!currentExists && !blendingExists)
	{
		LOG("[ERROR] Animator Component: Could not Step Clips! Error: There were no Current or Blending Clips set.");
		return false;
	}
	if (!currentExists && blendingExists)
	{
		SwitchBlendingToCurrent();
	}

	if (BlendingClipExists())
	{
		if (blendingClip->GetAnimationFrame() > (float)(blendingClip->GetStart() + blendFrames))
		{
			SwitchBlendingToCurrent();
		}
	}
	
	float dt		= (App->play) ? Time::Game::GetDT() : Time::Real::GetDT();											// In case a clip preview is needed outside Game Mode.
	float stepValue	= dt * playbackSpeed;

	if (CurrentClipExists())
	{
		bool success = currentClip->StepClip(stepValue);
		if (!success)
		{
			if (BlendingClipExists())
			{
				blendingClip->StepClip(stepValue);
				SwitchBlendingToCurrent();
				return true;
			}
			else
			{
				if (!currentClip->IsLooped())
				{
					Stop();
					ResetBones();
					return false;
				}
			}
		}

		if (BlendingClipExists())
		{
			blendingClip->StepClip(stepValue);
		}
	}

	return ret;
}

bool C_Animator::BlendAnimation()
{
	bool ret = true;



	return ret;
}

bool C_Animator::CurrentClipIsValid()
{
	bool ret = true;
	
	if (currentClip == nullptr)
	{
		if (blendingClip != nullptr)
		{
			SwitchBlendingToCurrent();
			ret = true;
		}
		else
		{
			ret = false;
		}
	}

	return ret;
}

void C_Animator::SwitchBlendingToCurrent()
{
	if (currentClip != nullptr)
	{
		currentClip->playing = false;
		currentClip->ClearClip();
		ClearCurrentClip();
	}

	SetCurrentClip(blendingClip);
	ClearBlendingClip();
}

void C_Animator::ResetBones()
{
	if (currentClip == nullptr)
	{
		LOG("[ERROR] Animator Component: Could not Reset Bones! Error: Current Clip was nullptr.");
		return;
	}
	
	for (auto bone = currentBones.cbegin(); bone != currentBones.cend(); ++bone)
	{
		const Transform& transform = Transform(bone->gameObject->GetComponent<C_Transform>()->GetLocalTransform());
		const Transform& interpolatedTransform = GetInterpolatedTransform((double)currentClip->GetStart(), bone->channel, transform);

		bone->gameObject->GetComponent<C_Transform>()->ImportTransform(interpolatedTransform);
	}

	UpdateDisplayBones();
}

void C_Animator::AddAnimationsToAdd()
{
	if (!animationsToAdd.empty())
	{
		for (uint i = 0; i < animationsToAdd.size(); ++i)
		{
			AddAnimation(animationsToAdd[i]);
		}

		animationsToAdd.clear();
	}
}

Transform C_Animator::GetInterpolatedTransform(const double& keyframe, const Channel& channel, const Transform& originalTransform) const
{
	float3	interpolatedPosition	= GetInterpolatedPosition(keyframe, channel, originalTransform.position);
	Quat	interpolatedRotation	= GetInterpolatedRotation(keyframe, channel, originalTransform.rotation);
	float3	interpolatedScale		= GetInterpolatedScale(keyframe, channel, originalTransform.scale);

	if (channel.type == ChannelType::ROTATION)
	{
		
		interpolatedPosition = originalTransform.position;
	}

	return Transform(interpolatedPosition, interpolatedRotation, interpolatedScale);
}

const float3 C_Animator::GetInterpolatedPosition(const double& keyframe, const Channel& channel, const float3& originalPosition) const
{
	if (!channel.HasPositionKeyframes())
	{
		return originalPosition;
	}

	PositionKeyframe prevKeyframe = channel.GetClosestPrevPositionKeyframe(keyframe);
	PositionKeyframe nextKeyframe = channel.GetClosestNextPositionKeyframe(keyframe);

	float rate = (float)(keyframe / nextKeyframe->first);
	float3 ret = (prevKeyframe == nextKeyframe) ? prevKeyframe->second : prevKeyframe->second.Lerp(nextKeyframe->second, rate);

	return ret;
}

const Quat C_Animator::GetInterpolatedRotation(const double& keyframe, const Channel& channel, const Quat& originalRotation) const
{
	if (!channel.HasRotationKeyframes())
	{
		return originalRotation;
	}

	RotationKeyframe prevKeyframe = channel.GetClosestPrevRotationKeyframe(keyframe);
	RotationKeyframe nextKeyframe = channel.GetClosestNextRotationKeyframe(keyframe);

	float rate = (float)(keyframe / nextKeyframe->first);
	Quat ret = (prevKeyframe == nextKeyframe) ? prevKeyframe->second : prevKeyframe->second.Slerp(nextKeyframe->second, rate);

	return ret;
}

const float3 C_Animator::GetInterpolatedScale(const double& keyframe, const Channel& channel, const float3& originalScale) const
{
	if (!channel.HasScaleKeyframes())
	{
		return originalScale;
	}

	ScaleKeyframe prevKeyframe = channel.GetClosestPrevScaleKeyframe(keyframe);
	ScaleKeyframe nextKeyframe = channel.GetClosestNextScaleKeyframe(keyframe);

	float rate = (float)(keyframe / nextKeyframe->first);
	float3 ret = (prevKeyframe == nextKeyframe) ? prevKeyframe->second : prevKeyframe->second.Lerp(nextKeyframe->second, rate);

	return ret;
}

Transform C_Animator::GetPoseToPoseTransform(const uint& tick, const Channel& channel, const Transform& originalTransform) const
{
	const float3& position	= GetPoseToPosePosition(tick, channel, originalTransform.position);
	const Quat& rotation	= GetPoseToPoseRotation(tick, channel, originalTransform.rotation);
	const float3& scale		= GetPoseToPoseScale(tick, channel, originalTransform.scale);
	
	return Transform(position, rotation, scale);
}

const float3 C_Animator::GetPoseToPosePosition(const uint& tick, const Channel& channel, const float3& originalPosition) const
{
	if (!channel.HasPositionKeyframes()) 
	{ 
		return originalPosition; 
	}

	return channel.GetPositionKeyframe(tick)->second;
}

const Quat C_Animator::GetPoseToPoseRotation(const uint& tick, const Channel& channel, const Quat& originalRotation) const
{
	if (!channel.HasRotationKeyframes())
	{
		return originalRotation;
	}

	return channel.GetRotationKeyframe(tick)->second;
}

const float3 C_Animator::GetPoseToPoseScale(const uint& tick, const Channel& channel, const float3& originalScale) const
{
	if (!channel.HasScaleKeyframes())
	{
		return originalScale;
	}

	return channel.GetScaleKeyframe(tick)->second;
}

Transform C_Animator::GetBlendedTransform(const double& blendedKeyframe, const Channel& blendedChannel, const Transform& originalTransform) const
{
	const float3& position	= GetBlendedPosition(blendedKeyframe, blendedChannel, originalTransform.position);
	const Quat& rotation	= GetBlendedRotation(blendedKeyframe, blendedChannel, originalTransform.rotation);
	const float3& scale		= GetBlendedScale(blendedKeyframe, blendedChannel, originalTransform.scale);

	return Transform(position, rotation, scale);
}

const float3 C_Animator::GetBlendedPosition(const double& blendingKeyframe, const Channel& blendingChannel, const float3& originalPosition) const
{
	if (!blendingChannel.HasPositionKeyframes())
	{
		return originalPosition;
	}

	float3 position			= GetInterpolatedPosition(blendingKeyframe, blendingChannel, originalPosition);

	double blendFrame		= blendingKeyframe - blendingClip->GetStart();
	float blendRate		= (float)(blendFrame / blendFrames);
	float3 blendedPosition	= originalPosition.Lerp(position, blendRate);
	
	return blendedPosition;
}

const Quat C_Animator::GetBlendedRotation(const double& blendingKeyframe, const Channel& blendingChannel, const Quat& originalRotation) const
{
	if (!blendingChannel.HasRotationKeyframes())
	{
		return originalRotation;
	}

	Quat rotation			= GetInterpolatedRotation(blendingKeyframe, blendingChannel, originalRotation);

	double blendFrame		= blendingKeyframe - blendingClip->GetStart();
	float blendRate		= (float)(blendFrame / blendFrames);
	Quat blendedRotation	= originalRotation.Slerp(rotation, blendRate);

	return blendedRotation;
}

const float3 C_Animator::GetBlendedScale(const double& blendingKeyframe, const Channel& blendingChannel, const float3& originalScale) const
{
	if (!blendingChannel.HasScaleKeyframes())
	{
		return originalScale;
	}

	float3 scale = GetInterpolatedScale(blendingKeyframe, blendingChannel, originalScale);

	double blendFrame = blendingKeyframe - blendingClip->GetStart();
	float blendRate = (float)(blendFrame / blendFrames);
	float3 blendedScale	= originalScale.Lerp(scale, blendRate);

	return blendedScale;
}

void C_Animator::FindAnimationBones(const R_Animation* rAnimation)
{
	if (rAnimation == nullptr)
	{
		return;
	}
	if (rAnimation->channels.empty())
	{
		return;
	}

	std::vector<BoneLink> links;
	bool success = FindBoneLinks(rAnimation, links);
	if (success)
	{
		animationBones.emplace(rAnimation->GetUID(), links);

		GameObject* rootBone = FindRootBone(links);
		if (rootBone != nullptr)
		{
			SetRootBone(rootBone);
			UpdateDisplayBones();
		}
	}
}

bool C_Animator::FindBoneLinks(const R_Animation* rAnimation, std::vector<BoneLink>& links)
{
	if (rAnimation == nullptr)
	{
		LOG("[ERROR] Animator Component: Could not find Bone Links! Error: Given R_Animation* was nullptr.");
		return false;
	}
	if (rAnimation->channels.empty())
	{
		LOG("[ERROR] Animator Component: Could not find { %s }'s Bone Links! Error: R_Animation* had no channels.");
		return false;
	}
	if (this->GetOwner()->childs.empty())
	{
		LOG("[ERROR] Animator Component: Could not find { %s }'s Bone Links! Error: Component Owner { %s } had no Childs.", this->GetOwner()->GetName());
		return false;
	} 
	
	std::map<std::string, GameObject*> childs;
	this->GetOwner()->GetAllChilds(childs);

	for (auto channel = rAnimation->channels.cbegin(); channel != rAnimation->channels.cend(); ++channel)						// Trying out the auto keyword
	{
		auto goItem = childs.find(channel->name);
		if (goItem != childs.end())
		{
			goItem->second->is_bone = true;
			links.push_back(BoneLink((*channel), goItem->second));
		}
	}

	childs.clear();

	return true;
}

GameObject* C_Animator::FindRootBone(const std::vector<BoneLink>& links)
{	
	for (auto link = links.cbegin(); link != links.cend(); ++link)																// Trying out the auto keyword
	{
		if (link->gameObject->parent == nullptr)
		{
			continue;
		}

		if (!link->gameObject->parent->is_bone)
		{
			return link->gameObject;
		}
	}

	return nullptr;
}

void C_Animator::SetRootBone(const GameObject* rootBone)
{
	if (rootBone == nullptr)
	{
		LOG("[ERROR] Animator Component: Could not Set Root Bone! Error: Given GameObject* was nullptr.");
		return;
	}
	
	if (currentRootBone == nullptr)
	{
		currentRootBone = rootBone;
	}
	else
	{
		if (currentRootBone != rootBone)
		{
			LOG("[WARNING] Animator Component: Disparity between root bones detected! A: [%s], B: [%s]", currentRootBone->GetName(), rootBone->GetName());
		}
	}
}

void C_Animator::UpdateDisplayBones()
{
	displayBones.clear();
	
	if (currentRootBone != nullptr)
	{
		GenerateBoneSegments(currentRootBone);
	}

	return;
}

void C_Animator::GenerateBoneSegments(const GameObject* bone)
{
	if (bone == nullptr)
	{
		LOG("[ERROR] Animation Component: Could not Generate Bone Segments! Error: Given GameObject* was nullptr.");
		return;
	}
	if (bone->childs.empty() || !bone->is_bone)
	{
		return;
	}
	
	C_Transform* boneTransform = bone->GetComponent<C_Transform>();

	for (uint i = 0; i < bone->childs.size(); ++i)
	{
		LineSegment displayBone = { float3::zero, float3::zero };

		displayBone.a = boneTransform->GetWorldPosition();
		displayBone.b = bone->childs[i]->GetComponent<C_Transform>()->GetWorldPosition();

		displayBones.push_back(displayBone);

		GenerateBoneSegments(bone->childs[i]);
	}
}

bool C_Animator::GenerateDefaultClip(const R_Animation* rAnimation, AnimatorClip& defaultClip)
{
	if (rAnimation == nullptr)
	{
		LOG("[ERROR] Animator Component: Could not Generate Default Clip! Error: Given R_Animation* was nullptr.");
		return false;
	}
	
	std::string defaultName	= rAnimation->GetName() + std::string(" Default");
	defaultClip				= AnimatorClip(rAnimation, defaultName, 0, (uint)rAnimation->GetDuration(), false);

	return true;
}

void C_Animator::SortBoneLinksByHierarchy(const std::vector<BoneLink>& boneLinks, const GameObject* rootBone, std::vector<BoneLink>& sorted)
{
	if (rootBone == nullptr)
	{
		return;
	}
	
	if (rootBone == currentRootBone)
	{
		for (uint j = 0; j < boneLinks.size(); ++j)
		{
			if (boneLinks[j].channel.name == rootBone->GetName())
			{
				sorted.push_back(boneLinks[j]);
			}
		}
	}

	for (uint i = 0; i < rootBone->childs.size(); ++i)
	{
		for (uint j = 0; j < boneLinks.size(); ++j)
		{
			if (boneLinks[j].channel.name == rootBone->childs[i]->GetName())
			{
				sorted.push_back(boneLinks[j]);
			}
		}
	}

	for (uint i = 0; i < rootBone->childs.size(); ++i)
	{
		SortBoneLinksByHierarchy(boneLinks, rootBone->childs[i], sorted);
	}
}

void C_Animator::AddAnimation(R_Animation* rAnimation)
{
	if (rAnimation == nullptr)
	{
		LOG("[ERROR] Animator Component: Could not Add Animation to %s's Animation Component! Error: Argument R_Animation* was nullptr.", this->GetOwner()->GetName());
		return;
	}

	animations.push_back(rAnimation);

	FindAnimationBones(rAnimation);

	AnimatorClip defaultClip = AnimatorClip();
	bool success = GenerateDefaultClip(rAnimation, defaultClip);
	if (success)
	{
		clips.emplace(defaultClip.GetName(), defaultClip);

		if (currentClip == nullptr)
		{
			SetCurrentClip(&clips.find(defaultClip.GetName())->second);
		}
	}
}

bool C_Animator::AddClip(const AnimatorClip& clip)
{
	if (clip.GetAnimation() == nullptr)
	{
		LOG("[ERROR] Animator Component: Could not Add Clip { %s }! Error: Clip's R_Animation* was nullptr.", clip.GetName());
		return false;
	}
	if (clips.find(clip.GetName()) != clips.end())
	{
		LOG("[ERROR] Animator Component: Could not Add Clip { %s }! Error: A clip with the same name already exists.", clip.GetName());
		return false;
	}
	
	clips.emplace(clip.GetName(), clip);

	if (currentClip == nullptr)
	{
		currentClip = (AnimatorClip*)&clip;
	}

	return true;
}

void C_Animator::PlayClip(const std::string& clipName, const uint& blendFrames)
{
	auto item = clips.find(clipName);
	if (item == clips.end())
	{
		LOG("[ERROR] Animator Component: Could not Play Clip! Error: Could not find any clip with the given name!");
		return;
	}
	if (currentClip != nullptr && currentClip->GetName() == clipName)
	{
		return;
	}
	
	if (currentClip == nullptr || blendFrames == 0 || blendFrames > item->second.GetDuration())
	{
		Stop();
		SetCurrentClip(&item->second);
	}
	else
	{
		SetBlendingClip(&item->second, blendFrames);
	}

	Play();
}

bool C_Animator::Play()
{
	if (currentClip == nullptr)
	{
		return false;
	}
	
	play = true;

	pause = false;
	step = false;
	stop = false;

	currentClip->playing = true;

	if (BlendingClipExists()) 
	{ 
		blendingClip->playing = true; 
	};

	return play;
}

bool C_Animator::Pause()
{
	if (play)
	{
		pause = true;
		play = false;
		step = false;
	}
	else
	{
		LOG("[WARNING] Animation Component: Cannot Pause a Stopped Animation!");
	}

	return pause;
}

bool C_Animator::Step()
{	
	if (pause)
	{
		step = true;
	}
	else
	{
		LOG("[WARNING] Animation Component: Only Paused Animations can be Stepped!");
	}

	return step;
}

bool C_Animator::Stop()
{
	stop = true;

	play = false;
	pause = false;
	step = false;
	
	currentClip->playing = false;
	currentClip->ClearClip();

	if (BlendingClipExists())
	{
		blendingClip->playing = false;
		blendingClip->ClearClip();
	}

	return stop;
}

// --- DEBUG METHODS
bool C_Animator::StepToPrevKeyframe()
{
	if (play)
	{
		LOG("[ERROR] Animator Component: Could not Step Animation to Prev Keyframe! Error: Cannot step an animation that is being currently Played.");
		return false;
	}
	if (currentClip == nullptr)
	{
		LOG("[ERROR] Animator Component: Could not Step Animation to Prev Keyframe! Error: Current Clip (AnimatorClip*) was nullptr.");
		return false;
	}

	currentClip->StepClipToPrevKeyframe();

	for (uint i = 0; i < currentBones.size(); ++i)
	{
		const Transform& transform				= Transform(currentBones[i].gameObject->GetComponent<C_Transform>()->GetLocalTransform());
		const Transform& interpolatedTransform = GetInterpolatedTransform((double)currentClip->GetClipTick(), currentBones[i].channel, transform);

		currentBones[i].gameObject->GetComponent<C_Transform>()->ImportTransform(interpolatedTransform);
	}

	UpdateDisplayBones();

	return true;
}

bool C_Animator::StepToNextKeyframe()
{
	if (play)
	{
		LOG("[ERROR] Animator Component: Could not Step Animation to Next Keyframe! Error: Cannot step an animation that is being currently Played.");
		return false;
	}
	if (currentClip == nullptr)
	{
		LOG("[ERROR] Animator Component: Could not Step Animation to Next Keyframe! Error: Current Clip (AnimatorClip*) was nullptr.");
		return false;
	}
	
	currentClip->StepClipToNextKeyframe();

	for (uint i = 0; i < currentBones.size(); ++i)
	{
		const Transform& transform				= Transform(currentBones[i].gameObject->GetComponent<C_Transform>()->GetLocalTransform());
		const Transform& interpolatedTransform = GetInterpolatedTransform((double)currentClip->GetClipTick(), currentBones[i].channel, transform);

		currentBones[i].gameObject->GetComponent<C_Transform>()->ImportTransform(interpolatedTransform);
	}

	UpdateDisplayBones();

	return false;
}

bool C_Animator::RefreshBoneDisplay()
{
	UpdateDisplayBones();

	return true;
}

// --- CURRENT/BLENDING ANIMATION METHODS
AnimatorClip* C_Animator::GetCurrentClip() const
{
	return currentClip;
}

AnimatorClip* C_Animator::GetBlendingClip() const
{
	return blendingClip;
}

void C_Animator::SetCurrentClip(AnimatorClip* clip)
{
	std::string errorString = "[ERROR] Animator Component: Could not Set Current Clip to { " + std::string(this->GetOwner()->GetName()) + " }'s Animator Component";
	
	if (clip == nullptr)
	{
		LOG("%s! Error: Given AnimatorClip* was nullptr.", errorString.c_str());
		return;
	}
	if (clips.find(clip->GetName()) == clips.end())
	{
		LOG("%s! Error: Could not find the given AnimatorClip* in the clips map.", errorString.c_str());
		return;
	}
	if (clip->GetAnimation() == nullptr)
	{
		LOG("%s! Error: Given AnimatorClip* had no R_Animation* assigned to it.", errorString.c_str());
		return;
	}
	
	auto bones = animationBones.find(clip->GetAnimation()->GetUID());
	if (bones == animationBones.end())
	{
		LOG("%s! Error: Could not find the Bones of the Clip's animation (R_Animation*).", errorString.c_str());
		return;
	}

	currentClip = clip;
	currentBones = bones->second;

	//currentClip->ClearClip();
}

void C_Animator::SetBlendingClip(AnimatorClip* clip, uint blendFrames)
{
	std::string errorString = "[ERROR] Animator Component: Could not Set Blending Clip in { " + std::string(this->GetOwner()->GetName()) + " }'s Animator Component";

	if (clip == nullptr)
	{
		LOG("%s! Error: Given AnimatorClip* was nullptr.", errorString.c_str());
		return;
	}
	if (clips.find(clip->GetName()) == clips.end())
	{
		LOG("%s! Error: Could not find the given AnimatorClip* in the clips map.", errorString.c_str());
		return;
	}
	if (clip->GetAnimation() == nullptr)
	{
		LOG("%s! Error: Given AnimatorClip* had no R_Animation* assigned to it.", errorString.c_str());
		return;
	}

	auto bones = animationBones.find(clip->GetAnimation()->GetUID());
	if (bones == animationBones.end())
	{
		LOG("%s! Error: Could not find the Bones of the Clip's animation (R_Animation*).");
		return;
	}
	
	blendingClip = clip;
	blendingBones = bones->second;
	this->blendFrames = blendFrames;

	blendingClip->ClearClip();																					// Resetting the clip just in case.
}

void C_Animator::SetCurrentClipByIndex(const uint& index)
{
	if (index >= clips.size())
	{
		LOG("[ERROR] Animator Component: Could not Set Current Clip By Index! Error: Given Index was out of bounds.");
		return;
	}

	std::string errorString = "[ERROR] Animator Component: Could not Set Current Clip in { " + std::string(this->GetOwner()->GetName()) + " }'s Animator Component";

	uint i = 0;
	for (auto item = clips.cbegin(); item != clips.cend(); ++item)
	{
		if (i == index)																										// Dirty way of finding items in a map by index.
		{
			const AnimatorClip& clip = item->second;

			if (animationBones.find(clip.GetAnimation()->GetUID()) == animationBones.end())
			{
				LOG("%s! Error: Could not find the Bones of the Clip's animation (R_Animation*).");
				return;
			}

			currentClip	= (AnimatorClip*)&clip;
			currentBones = animationBones.find(clip.GetAnimation()->GetUID())->second;

			currentClip->ClearClip();

			return;
		}

		++i;
	}
}

/*void C_Animator::SetBlendingClipByIndex(const uint& index, const uint& blendFrames)
{

}*/

bool C_Animator::CurrentClipExists() const
{
	return (currentClip != nullptr) ? true : false;
}

bool C_Animator::BlendingClipExists() const
{
	return (blendingClip != nullptr) ? true : false;
}

void C_Animator::ClearCurrentClip()
{
	currentClip = nullptr;
	currentBones.clear();
}

void C_Animator::ClearBlendingClip()
{
	blendingClip = nullptr;
	blendFrames = 0;
	blendingBones.clear();
}

// --- GET/SET METHODS
std::vector<LineSegment> C_Animator::GetDisplayBones() const
{
	return displayBones;
}

std::vector<std::string> C_Animator::GetClipNamesAsVector() const
{
	std::vector<std::string> clipNames;

	for (auto clip = clips.cbegin(); clip != clips.cend(); ++clip)
	{
		clipNames.push_back(clip->first);
	}

	return clipNames;
}

std::string C_Animator::GetClipNamesAsString() const
{
	std::string clipNames = "";

	for (auto clip = clips.cbegin(); clip != clips.cend(); ++clip)
	{
		clipNames += clip->first.c_str();
		clipNames += '\0';
	}

	return clipNames;
}

std::string C_Animator::GetAnimationNamesAsString() const
{
	std::string animationNames = "";

	for (auto animation = animations.cbegin(); animation != animations.cend(); ++animation)
	{
		animationNames += (*animation)->GetName();
		animationNames += '\0';
	}

	return animationNames;
}

R_Animation* C_Animator::GetAnimationByIndex(const uint& index) const
{
	if (index >= animations.size())
	{
		LOG("[ERROR] Animator Component: Could not get Animation by Index! Error: Given index was out of bounds.");
		return nullptr;
	}

	return animations[index];
}

float C_Animator::GetPlaybackSpeed() const
{
	return playbackSpeed;
}

bool C_Animator::GetInterpolate() const
{
	return interpolate;
}

bool C_Animator::GetLoopAnimation() const
{
	return loopAnimation;
}

bool C_Animator::GetPlayOnStart() const
{
	return playOnStart;
}

bool C_Animator::GetCameraCulling() const
{
	return cameraCulling;
}

bool C_Animator::GetShowBones() const
{
	return showBones;
}

void C_Animator::SetPlaybackSpeed(const float& playbackSpeed)
{
	this->playbackSpeed = playbackSpeed;
}

void C_Animator::SetInterpolate(const bool& setTo)
{
	interpolate = setTo;
}

void C_Animator::SetLoopAnimation(const bool& setTo)
{
	loopAnimation = setTo;
}

void C_Animator::SetPlayOnStart(const bool& setTo)
{
	playOnStart = setTo;
}

void C_Animator::SetCameraCulling(const bool& setTo)
{
	cameraCulling = setTo;
}

void C_Animator::SetShowBones(const bool& setTo)
{
	showBones = setTo;
}

// --- BONE LINK METHODS
BoneLink::BoneLink() : 
channel(Channel()),
gameObject(nullptr)
{

}

BoneLink::BoneLink(const Channel& channel, GameObject* gameObject) : 
channel(channel),
gameObject(gameObject)
{

}