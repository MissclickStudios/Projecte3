#include "MathGeoLib/include/Geometry/LineSegment.h"
#include "Profiler.h"

#include "FileSystemDefinitions.h"
#include "JSONParser.h"
#include "Time.h"
#include "EasingFunctions.h"

#include "Channel.h"
#include "BoneLink.h"
#include "AnimatorClip.h"

#include "Application.h"
#include "M_ResourceManager.h"

#include "R_Mesh.h"
#include "R_Animation.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_Mesh.h"

#include "C_Animator.h"

#include "MemoryManager.h"

typedef std::map<double, float3>::const_iterator	PositionKeyframe;
typedef std::map<double, Quat>::const_iterator		RotationKeyframe;
typedef std::map<double, float3>::const_iterator	ScaleKeyframe;

C_Animator::C_Animator(GameObject* owner) : Component(owner, ComponentType::ANIMATOR),
currentClip		(nullptr),
blendingClip	(nullptr),
rootBone		(nullptr), 
animatorState	(AnimatorState::STOP)
{	
	blendFrames		= 0;

	playbackSpeed	= 1.0f;
	interpolate		= true;
	loopAnimation	= false;
	playOnStart		= true;
	cameraCulling	= true;
	showBones		= false;

	needsInit		= true;

}

C_Animator::~C_Animator()
{
	currentClip		= nullptr;
	blendingClip	= nullptr;
	rootBone		= nullptr;
}

bool C_Animator::Update()
{
	if (needsInit)
	{
		GetAnimatedMeshes();
		FindBones();
		FindBoneLinks();
		GenerateDefaultClips();

		UpdateDisplayBones();

		needsInit = false;
	}

	if (showBones)
	{
		UpdateDisplayBones();
	}
	
	CheckGameState();

	if (animatorState == AnimatorState::PLAY || animatorState == AnimatorState::STEP)
	{
		if (currentClip != nullptr)
		{
			StepAnimation();
		}

		if (animatorState == AnimatorState::STEP)
		{
			Pause();
		}
	}

	return true;
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
	
	animatedMeshes.clear();

	bones.clear();
	currentBones = nullptr;
	blendingBones = nullptr;
	displayBones.clear();

	clips.clear();

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
			animations.push_back(rAnimation);
			//animationsToAdd.push_back(rAnimation);
			//AddAnimation(rAnimation);
		}
	}

	ParsonArray clipsArray = root.GetArray("Clips");
	for (uint i = 0; i < clipsArray.size; ++i)
	{
		ParsonNode clipNode = clipsArray.GetNode(i);
		AnimatorClip clip = AnimatorClip();

		/*if (animations.find((uint32)clipNode.GetNumber("AnimationUID")) != animations.end())
		{
			clip.SetAnimation(animations[(uint32)clipNode.GetNumber("AnimationUID")]);
		}*/

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
void C_Animator::CheckGameState()
{	
	if (App->gameState == GameState::STEP && animatorState != AnimatorState::STEP)
	{
		Step();
	}
	
	if (App->gameState == GameState::PAUSE && animatorState == AnimatorState::PLAY)
	{
		Pause();
	}
}

bool C_Animator::StepAnimation()
{	
	OPTICK_CATEGORY("Step Animation", Optick::Category::Animation);
	
	bool success = ValidateCurrentClip();
	if (!success)
	{
		return false;
	}
	
	success = StepClips();
	if (!success)
	{
		return false;
	}
	
	UpdateChannelTransforms();
	UpdateMeshSkinning();

	return true;
}

bool C_Animator::StepClips()
{	
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
		if (blendingClip->GetAnimationFrame() > (float)(blendingClip->GetStart() + blendFrames))						// ATTENTION HERE.
		{
			SwitchBlendingToCurrent();
		}
	}
	
	float dt		= (App->gameState == GameState::PLAY) ? Time::Game::GetDT() : Time::Real::GetDT();					// In case a clip preview is needed outside Game Mode.
	float stepValue	= dt * playbackSpeed;

	if (CurrentClipExists())
	{
		bool success = currentClip->StepClip(stepValue);
		if (!success)
		{
			if (BlendingClipExists())
			{	
				if ((blendingClip->GetAnimationFrame() - blendingClip->GetStart() / blendFrames) >= 1.0f)
				{
					blendingClip->StepClip(stepValue);																		// ATTENTION HERE
					SwitchBlendingToCurrent();
				}

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

	return true;
}

bool C_Animator::BlendAnimation()
{


	return true;
}

bool C_Animator::ValidateCurrentClip()
{	
	if (currentClip == nullptr)
	{
		if (blendingClip != nullptr)
		{
			SwitchBlendingToCurrent();
		}
		else
		{
			return false;
		}
	}

	return true;
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
	if (currentBones == nullptr)
	{
		LOG("[ERROR] Animator Component: Could not Reset Bones! Error: Current Bones was nullptr.");
		return;
	}

	for (auto bone = currentBones->cbegin(); bone != currentBones->cend(); ++bone)
	{
		//const Transform& transform = Transform(bone->gameObject->GetComponent<C_Transform>()->GetLocalTransform());
		//const Transform& interpolatedTransform = GetInterpolatedTransform((double)currentClip->GetStart(), bone->channel, transform);
		const Transform& interpolatedTransform = GetInterpolatedTransform((double)currentClip->GetStart(), bone->channel, bone->gameObject->GetComponent<C_Transform>());

		bone->gameObject->GetComponent<C_Transform>()->ImportTransform(interpolatedTransform);
	}

	UpdateDisplayBones();
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

// --- ANIMATION TRANSFORMS METHODS
void C_Animator::UpdateChannelTransforms()
{	
	OPTICK_CATEGORY("Update Channel Transforms", Optick::Category::Animation);
	
	if (currentBones == nullptr)
	{
		LOG("[ERROR] Animator Component: Could not Update Channel Transforms! Error: Current Bones was nullptr.");
		return;
	}

	for (uint i = 0; i < currentBones->size(); ++i)
	{
		const BoneLink& bone = currentBones->at(i);																									// (*currentBones)[i] would also be valid.
		
		C_Transform* cTransform = bone.gameObject->GetComponent<C_Transform>();
		if (cTransform == nullptr)
		{
			LOG("[WARNING] Animation Component: GameObject { %s } did not have a Transform Component!", bone.gameObject->GetName());
			continue;
		}

		if (interpolate)
		{
			Transform& interpolatedTransform = GetInterpolatedTransform(currentClip->GetAnimationFrame(), bone.channel, cTransform);
			
			if (BlendingClipExists())
			{
				if (blendingBones == nullptr)
				{
					LOG("[ERROR] Animator Component: Could not Get Blended Transform! Error: Blending Bones was nullptr.");
					break;
				}

				interpolatedTransform = GetBlendedTransform(blendingClip->GetAnimationFrame(), blendingBones->at(i).channel, interpolatedTransform);
			}

			cTransform->ImportTransform(interpolatedTransform);
		}
		else
		{
			if (currentClip->inNewTick)
			{
				Transform& poseToPoseTransform = GetPoseToPoseTransform(currentClip->GetAnimationTick(), bone.channel, cTransform);

				if (BlendingClipExists())
				{
					if (blendingBones == nullptr)
					{
						LOG("[ERROR] Animator Component: Could not Get Blended Transform! Error: Blending Bones was nullptr.");
						break;
					}
					
					poseToPoseTransform = GetBlendedTransform(blendingClip->GetAnimationTick(), blendingBones->at(i).channel, poseToPoseTransform);
				}

				cTransform->ImportTransform(poseToPoseTransform);
			}
		}
	}
}

void C_Animator::UpdateMeshSkinning()
{	
	OPTICK_CATEGORY("Update Mesh Skinning", Optick::Category::Animation);
	
	if (animatedMeshes.empty())
	{
		return;
	}
	
	bool success = false;
	for (auto mesh = animatedMeshes.begin(); mesh != animatedMeshes.end(); ++mesh)
	{
		success = (*mesh)->RefreshSkinning();
		if (success)
		{
			(*mesh)->AnimateMesh();
		}
	}
}

void C_Animator::UpdateDisplayBones()
{
	displayBones.clear();

	if (rootBone != nullptr)
	{
		GenerateBoneSegments(rootBone);
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
	if (bone->childs.empty() /*|| !bone->is_bone*/)
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

Transform C_Animator::GetInterpolatedTransform(double keyframe, const Channel& channel, C_Transform* originalTransform) const
{	
	OPTICK_CATEGORY("Get Interpolated Transform", Optick::Category::Animation);

	const float3&	newPosition	= (channel.HasPositionKeyframes()) ? GetInterpolatedPosition(keyframe, channel) : originalTransform->GetLocalPosition();
	const Quat&		newRotation	= (channel.HasRotationKeyframes()) ? GetInterpolatedRotation(keyframe, channel) : originalTransform->GetLocalRotation();
	const float3&	newScale	= (channel.HasScaleKeyframes()) ? GetInterpolatedScale(keyframe, channel) : originalTransform->GetLocalScale();

	return Transform(newPosition, newRotation, newScale);
}

const float3 C_Animator::GetInterpolatedPosition(double keyframe, const Channel& channel) const
{
	PositionKeyframe prevKeyframe = channel.GetClosestPrevPositionKeyframe(keyframe);
	PositionKeyframe nextKeyframe = channel.GetClosestNextPositionKeyframe(keyframe);

	if (prevKeyframe == nextKeyframe)
	{
		return prevKeyframe->second;
	}

	float rate	= (float)((keyframe - prevKeyframe->first) / (nextKeyframe->first - prevKeyframe->first));
	rate		= (rate > 1.0f) ? 1.0f : rate;																		// Safety Measure just in case rate is higher than 1.0f (100%).

	return (prevKeyframe->second.Lerp(nextKeyframe->second, rate));
}

const Quat C_Animator::GetInterpolatedRotation(double keyframe, const Channel& channel) const
{
	RotationKeyframe prevKeyframe = channel.GetClosestPrevRotationKeyframe(keyframe);
	RotationKeyframe nextKeyframe = channel.GetClosestNextRotationKeyframe(keyframe);

	if (prevKeyframe == nextKeyframe)
	{
		return prevKeyframe->second;
	}

	float rate	= (float)((keyframe - prevKeyframe->first) / (nextKeyframe->first - prevKeyframe->first));
	rate		= (rate > 1.0f) ? 1.0f : rate;

	return (prevKeyframe->second.Slerp(nextKeyframe->second, rate));
}

const float3 C_Animator::GetInterpolatedScale(double keyframe, const Channel& channel) const
{
	ScaleKeyframe prevKeyframe = channel.GetClosestPrevScaleKeyframe(keyframe);
	ScaleKeyframe nextKeyframe = channel.GetClosestNextScaleKeyframe(keyframe);

	if (prevKeyframe == nextKeyframe)
	{
		return prevKeyframe->second;
	}

	float rate	= (float)((keyframe - prevKeyframe->first) / (nextKeyframe->first - prevKeyframe->first));
	rate		= (rate > 1.0f) ? 1.0f : rate;

	return (prevKeyframe->second.Lerp(nextKeyframe->second, rate));
}

Transform C_Animator::GetPoseToPoseTransform(uint tick, const Channel& channel, C_Transform* originalTransform) const
{
	const float3&	position	= (channel.HasPositionKeyframes()) ? channel.GetPositionKeyframe(tick)->second : originalTransform->GetLocalPosition();
	const Quat&		rotation	= (channel.HasRotationKeyframes()) ? channel.GetRotationKeyframe(tick)->second : originalTransform->GetLocalRotation();
	const float3&	scale		= (channel.HasScaleKeyframes()) ? channel.GetScaleKeyframe(tick)->second : originalTransform->GetLocalScale();
	
	return Transform(position, rotation, scale);
}

Transform C_Animator::GetBlendedTransform(double bKeyframe, const Channel& bChannel, const Transform& originalTransform) const
{
	OPTICK_CATEGORY("Get Blended Transform", Optick::Category::Animation);
	
	if (!bChannel.HasPositionKeyframes() && !bChannel.HasRotationKeyframes() && !bChannel.HasScaleKeyframes()) { return originalTransform; }

	float bRate = (float)((bKeyframe - blendingClip->GetStart()) / blendFrames);
	bRate		= (bRate > 1.0f) ? 1.0f : bRate;

	const float3&	position	= (bChannel.HasPositionKeyframes()) ? originalTransform.position.Lerp((GetInterpolatedPosition(bKeyframe, bChannel)), bRate) : originalTransform.position;
	const Quat&		rotation	= (bChannel.HasRotationKeyframes()) ? originalTransform.rotation.Slerp((GetInterpolatedRotation(bKeyframe, bChannel)), bRate) : originalTransform.rotation;
	const float3&	scale		= (bChannel.HasScaleKeyframes()) ? originalTransform.scale.Lerp((GetInterpolatedScale(bKeyframe, bChannel)), bRate) : originalTransform.scale;

	return Transform(position, rotation, scale);
}

const float3 C_Animator::GetBlendedPosition(double bKeyframe, const Channel& bChannel, float bRate, const float3& originalPosition) const
{
	return (originalPosition.Lerp((GetInterpolatedPosition(bKeyframe, bChannel)), bRate));		// We get the interpolated value of the keyframe-channel and then lerp origin to value.
}

const Quat C_Animator::GetBlendedRotation(double bKeyframe, const Channel& bChannel, float bRate, const Quat& originalRotation) const
{
	return (originalRotation.Slerp((GetInterpolatedRotation(bKeyframe, bChannel)), bRate));
}

const float3 C_Animator::GetBlendedScale(double bKeyframe, const Channel& bChannel, float bRate, const float3& originalScale) const
{
	return (originalScale.Lerp((GetInterpolatedScale(bKeyframe, bChannel)), bRate));
}

void C_Animator::FindRootBone()
{
	std::vector<GameObject*> childs;
	this->GetOwner()->GetAllChilds(childs);
	for (auto animesh = animatedMeshes.begin(); animesh != animatedMeshes.end(); ++animesh)							// Finding root bone by cross-checking with animesh. Maybe overengineered?
	{
		std::map<std::string, uint> mapping = (*animesh)->GetMesh()->boneMapping;
		if (mapping.empty())
		{
			continue;
		}

		for (auto child = childs.begin(); child != childs.end(); ++child)
		{
			auto result = mapping.find((*child)->GetName());
			if (result != mapping.end())
			{
				rootBone = (*child);																				// 1st bone to be found in the mapping is the most likely to be the root.
				break;
			}
		}
	}
}

void C_Animator::SetRootBone(GameObject* rootBone)
{
	if (rootBone == nullptr)
	{
		LOG("[ERROR] Animator Component: Could not Set Root Bone! Error: Given GameObject* was nullptr.");
		return;
	}
	
	if (this->rootBone == nullptr)
	{
		this->rootBone = rootBone;

		for (auto cMesh = animatedMeshes.begin(); cMesh != animatedMeshes.end(); ++cMesh)
		{
			(*cMesh)->SetRootBone(this->rootBone);
		}
	}
	else
	{
		if (this->rootBone != rootBone)
		{
			LOG("[WARNING] Animator Component: Disparity between root bones detected! A: [%s], B: [%s]", this->rootBone->GetName(), rootBone->GetName());
		}
	}
}

GameObject* C_Animator::GetRootBone() const
{
	return rootBone;
}

void C_Animator::GetAnimatedMeshes()
{
	std::map<std::string, GameObject*> childs;
	this->GetOwner()->GetAllChilds(childs);
	std::vector<C_Mesh*> cMeshes;
	for (auto child = childs.begin(); child != childs.end(); ++child)
	{
		cMeshes.clear();
		child->second->GetComponents<C_Mesh>(cMeshes);
		for (uint i = 0; i < cMeshes.size(); ++i)
		{
			R_Mesh* rMesh = cMeshes[i]->GetMesh();
			if (rMesh != nullptr && !rMesh->boneMapping.empty())
			{
				animatedMeshes.push_back(cMeshes[i]);
				cMeshes[i]->SetAnimatorOwner(this->GetOwner());
			}
		}
	}
}

void C_Animator::FindBones()
{
	FindRootBone();

	if (rootBone != nullptr)
	{
		bones.push_back(rootBone);
		rootBone->GetAllChilds(bones);

		for (auto animesh = animatedMeshes.cbegin(); animesh != animatedMeshes.cend(); ++animesh)
		{
			(*animesh)->SetRootBone(rootBone);															// Setting the root bone for the Animated Mesh, not this Component. Necessary?
		}

		CrossCheckBonesWithMeshBoneMapping();
	}
}

void C_Animator::FindBoneLinks()
{
	if (animations.empty())
	{
		LOG("[ERROR] Animator Component: Could not find Bone Links! Error: Animations Vector was Empty.");
		return;
	}
	if (bones.empty())
	{
		LOG("[ERROR] Animator Component: Could not find Bone Links! Error: Root Bone was nullptr.");
		return;
	}

	std::map<std::string, GameObject*> boneMap;
	for (auto bone = bones.cbegin(); bone != bones.cend(); ++bone)														// Passing bones to a map for easy access.
	{
		boneMap.emplace((*bone)->GetName(), (*bone));
	}

	for (auto animation = animations.cbegin(); animation != animations.cend(); ++animation)
	{
		if ((*animation)->channels.empty())
		{
			continue;
		}
		
		std::vector<BoneLink> boneLinks;
		for (auto channel = (*animation)->channels.cbegin(); channel != (*animation)->channels.cend(); ++channel)
		{
			auto result = boneMap.find((*channel).name);
			if (result != boneMap.end())
			{
				boneLinks.push_back(BoneLink((*channel), result->second));
			}
		}

		if (boneLinks.empty())
		{
			LOG("[WARNING] Animator Component: Animation { %s } had no Bone Links!", (*animation)->GetName());
		}

		animationBones.emplace((*animation)->GetUID(), boneLinks);
		boneLinks.clear();
	}

	boneMap.clear();
}

void C_Animator::GenerateDefaultClips()
{
	if (animations.empty())
	{
		return;
	}

	for (auto animation = animations.begin(); animation < animations.end(); ++animation)
	{
		std::string defaultName = (*animation)->GetName() + std::string(" Default");
		AnimatorClip& defaultClip = AnimatorClip((*animation), defaultName, 0, (uint)(*animation)->GetDuration(), false);

		clips.emplace(defaultClip.GetName(), defaultClip);

		if (currentClip == nullptr)
		{
			SetCurrentClip(&clips.find(defaultClip.GetName())->second);															// .find instead of passing the clip to make sure it exists.
		}
	}
}

void C_Animator::CrossCheckBonesWithMeshBoneMapping()
{
	if (animatedMeshes.empty())
	{
		LOG("[ERROR] Animator Component: Could not Cross Check Bones! Error: Animated Meshes Vector was Empty.");
		return;
	}
	if (bones.empty())
	{
		LOG("[ERROR] Animator Component: Could not Cross Check Bones! Error: Bones Vector was Empty.");
		return;
	}

	for (auto animesh = animatedMeshes.begin(); animesh != animatedMeshes.end(); ++animesh)
	{
		std::map<std::string, uint> mapping = (*animesh)->GetMesh()->boneMapping;
		if (mapping.empty())
		{
			continue;
		}

		for (auto bone = bones.begin(); bone != bones.end(); ++bone)
		{
			auto result = mapping.find((*bone)->GetName());
			if (result != mapping.end())
			{
				(*bone)->is_bone = true;
			}
		}
	}
}

// --- PUBLIC C_ANIMATOR METHODS
void C_Animator::AddAnimation(R_Animation* rAnimation)
{
	if (rAnimation == nullptr)
	{
		LOG("[ERROR] Animator Component: Could not Add Animation to %s's Animation Component! Error: Argument R_Animation* was nullptr.", this->GetOwner()->GetName());
		return;
	}

	animations.push_back(rAnimation);

	//FindAnimationBones(rAnimation);

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

bool C_Animator::EditClip(const std::string& originalClipName, const R_Animation* rAnimation, const std::string& name, uint start, uint end, bool loop)
{	
	auto originalClip = clips.find(originalClipName);
	if (originalClip == clips.end())
	{
		return false;
	}

	originalClip->second.EditClip(rAnimation, name, start, end, loop);
	AddClip(originalClip->second);

	DeleteClip(originalClipName);
}

bool C_Animator::DeleteClip(const std::string& clipName)
{
	if (currentClip != nullptr && std::string(currentClip->GetName()) == clipName)
	{
		ClearCurrentClip();
	}
	if (blendingClip != nullptr && std::string(currentClip->GetName()) == clipName)
	{
		ClearBlendingClip();
	}
	
	return (clips.erase(clipName) == 1);																				// std::unordered_map::erase returns the amount of elements erased.
}

void C_Animator::PlayClip(const std::string& clipName, uint blendFrames)
{
	auto item = clips.find(clipName);
	if (item == clips.end())
	{
		LOG("[ERROR] Animator Component: Could not Play Clip! Error: Could not find any clip with the given name!");
		return;
	}
	if (currentClip != nullptr && currentClip->GetName() == clipName)													// This makes it so the user is unable to play the same clip twice.
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

void C_Animator::PlayClip(const std::string& clipName, float blendTime)
{
	auto item = clips.find(clipName);
	if (item == clips.end())
	{
		LOG("[ERROR] Animator Component: Could not Play Clip! Error: Could not find any clip with the given name!");
		return;
	}
	if (currentClip != nullptr && currentClip->GetName() == clipName)													// This makes it so the user is unable to play the same clip twice.
	{
		return;
	}

	blendFrames = blendTime * item->second.GetAnimationTicksPerSecond();

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
		LOG("[ERROR] Animator Component: Could not Play the AnimatorClip! Error: No Current Clip was set.");
		return false;
	}

	animatorState = AnimatorState::PLAY;

	if (CurrentClipExists())
		currentClip->playing = true;

	if (BlendingClipExists()) 
		blendingClip->playing = true;

	return (animatorState == AnimatorState::PLAY);
}

bool C_Animator::Pause()
{
	if (animatorState != AnimatorState::PAUSE)
	{
		(animatorState != AnimatorState::STOP) ? animatorState = AnimatorState::PAUSE : LOG("[WARNING] Animator Component: Cannot Pause a Stopped Animation!");
	}

	return (animatorState == AnimatorState::PAUSE);
}

bool C_Animator::Step()
{	
	if (animatorState != AnimatorState::STEP)
	{
		(animatorState == AnimatorState::PAUSE) ? animatorState = AnimatorState::STEP : LOG("[WARNING] Animator Component: Only Paused Animations can be Stepped!");
	}

	return (animatorState == AnimatorState::STEP);
}

bool C_Animator::Stop()
{
	animatorState = AnimatorState::STOP;

	if (CurrentClipExists())
	{
		currentClip->playing = false;
		currentClip->ClearClip();
	}
	if (BlendingClipExists())
	{
		blendingClip->playing = false;
		blendingClip->ClearClip();
	}

	return (animatorState == AnimatorState::STOP);
}

// --- DEBUG METHODS
bool C_Animator::StepToPrevKeyframe()
{
	if (animatorState == AnimatorState::PLAY)
	{
		LOG("[ERROR] Animator Component: Could not Step Animation to Prev Keyframe! Error: Cannot Step an unpaused animation.");
		return false;
	}
	if (currentClip == nullptr)
	{
		LOG("[ERROR] Animator Component: Could not Step Animation to Prev Keyframe! Error: Current Clip (AnimatorClip*) was nullptr.");
		return false;
	}
	if (currentBones == nullptr)
	{
		LOG("[ERROR] Animator Component: Could not Step Animation to Prev Keyframe! Error: Current Bones is nullptr.");
		return false;
	}

	currentClip->StepClipToPrevKeyframe();

	for (auto bone = currentBones->cbegin(); bone != currentBones->cend(); ++bone)
	{
		C_Transform* cTransform = (*bone).gameObject->GetComponent<C_Transform>();
		if (cTransform == nullptr)
		{
			continue;
		}

		cTransform->ImportTransform(GetInterpolatedTransform((double)currentClip->GetClipTick(), (*bone).channel, cTransform));
	}

	UpdateDisplayBones();

	return true;
}

bool C_Animator::StepToNextKeyframe()
{
	if (animatorState == AnimatorState::PLAY)
	{
		LOG("[ERROR] Animator Component: Could not Step Animation to Next Keyframe! Error: Cannot Step an unpaused animation.");
		return false;
	}
	if (currentClip == nullptr)
	{
		LOG("[ERROR] Animator Component: Could not Step Animation to Next Keyframe! Error: Current Clip (AnimatorClip*) was nullptr.");
		return false;
	}
	if (currentBones == nullptr)
	{
		LOG("[ERROR] Animator Component: Could not Step Animation to Next Keyframe! Error: Current Bones is nullptr");
		return false;
	}
	
	currentClip->StepClipToNextKeyframe();

	for (auto bone = currentBones->cbegin(); bone != currentBones->cend(); ++bone)
	{
		C_Transform* cTransform = (*bone).gameObject->GetComponent<C_Transform>();
		if (cTransform == nullptr)
		{
			continue;
		}

		cTransform->ImportTransform(GetInterpolatedTransform((double)currentClip->GetClipTick(), (*bone).channel, cTransform));
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
AnimatorClip C_Animator::GetClip(const char* clipName) const
{
	auto clip = clips.find(clipName);
	return (clip != clips.end()) ? clip->second : AnimatorClip(nullptr, "[NONE]", 0, 0, false);
}

AnimatorClip* C_Animator::GetClipAsPtr(const char* clipName)
{
	auto clip = clips.find(clipName);
	return (clip != clips.end()) ? &clip->second : nullptr;
}

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
	currentBones = &bones->second;

	//LOG("PLAYING CLIP { %s }", clip->GetName());

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
	
	LOG("BLENDING TO [%s]::[%u]", clip->GetName(), blendFrames);

	blendingClip		= clip;
	blendingBones		= &bones->second;
	this->blendFrames	= blendFrames;

	blendingClip->ClearClip();																					// Resetting the clip just in case.
}

void C_Animator::SetCurrentClipByIndex(uint index)
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
			currentBones = &(animationBones.find(clip.GetAnimation()->GetUID())->second);

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
	return (currentClip != nullptr);
}

bool C_Animator::BlendingClipExists() const
{
	return (blendingClip != nullptr);
}

void C_Animator::ClearCurrentClip()
{
	currentClip		= nullptr;
	currentBones	= nullptr;
}

void C_Animator::ClearBlendingClip()
{
	blendingClip	= nullptr;
	blendingBones	= nullptr;
	blendFrames		= 0;
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

std::string C_Animator::GetAnimatorStateAsString() const
{
	switch (animatorState)
	{
	case AnimatorState::PLAY:	{ return ("PLAY"); }	break;
	case AnimatorState::PAUSE:	{ return ("PAUSE"); }	break;
	case AnimatorState::STEP:	{ return ("STEP"); }	break;
	case AnimatorState::STOP:	{ return ("STOP"); }	break;
	}

	return ("[NONE]");
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

R_Animation* C_Animator::GetAnimationByIndex(uint index) const
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

void C_Animator::SetPlaybackSpeed(float playbackSpeed)
{
	this->playbackSpeed = playbackSpeed;
}

void C_Animator::SetInterpolate(bool setTo)
{
	interpolate = setTo;
}

void C_Animator::SetLoopAnimation(bool setTo)
{
	loopAnimation = setTo;
}

void C_Animator::SetPlayOnStart(bool setTo)
{
	playOnStart = setTo;
}

void C_Animator::SetCameraCulling(bool setTo)
{
	cameraCulling = setTo;
}

void C_Animator::SetShowBones(bool setTo)
{
	showBones = setTo;
}