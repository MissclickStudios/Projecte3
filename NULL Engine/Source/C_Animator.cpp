#include "MathGeoLib/include/Geometry/LineSegment.h"
#include "Profiler.h"

#include "FileSystemDefinitions.h"
#include "JSONParser.h"
#include "MC_Time.h"
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
rootBone		(nullptr), 
animatorState	(AnimatorState::STOP)
{	
	playbackSpeed	= 1.0f;
	loopAnimation	= false;
	playOnStart		= true;
	cameraCulling	= true;
	showBones		= false;

	init = true;
}

C_Animator::~C_Animator()
{
	rootBone		= nullptr;
}

bool C_Animator::Start()
{
	/*GetAnimatedMeshes();

	FindBones();
	FindBoneLinks();
	UpdateDisplayBones();

	GenerateDefaultClips();*/
	
	return true;
}

bool C_Animator::Update()
{
	if (init)																								// Move Init to Start() later.
	{
		GetAnimatedMeshes();

		FindBones();
		FindBoneLinks();
		UpdateDisplayBones();

		GenerateDefaultClips();

		FindTracksRootBones();
		GeneratePreviewTrack();

		init = false;
	}
	
	if (showBones)
	{
		UpdateDisplayBones();
	}
	
	CheckGameState();

	if (animatorState == AnimatorState::PLAY || animatorState == AnimatorState::STEP)
	{
		StepAnimation();

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
	displayBones.clear();

	clips.clear();
	tracks.clear();

	return ret;
}

bool C_Animator::SaveState(ParsonNode& root) const
{
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

	// Tracks
	ParsonArray tracksArray = root.SetArray("Tracks");
	for (auto track = tracks.cbegin(); track != tracks.cend(); ++track)
	{
		if (strstr(track->first.c_str(), "Preview") != nullptr)
		{
			continue;
		}

		ParsonNode trackNode = tracksArray.SetNode(track->first.c_str());
		track->second.SaveState(trackNode);
	}

	return true;
}

bool C_Animator::LoadState(ParsonNode& root)
{
	// Animations
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

	// Clips
	ParsonArray clipsArray = root.GetArray("Clips");
	for (uint i = 0; i < clipsArray.size; ++i)
	{
		ParsonNode clipNode = clipsArray.GetNode(i);
		if (!clipNode.NodeIsValid())
		{
			continue;
		}

		AnimatorClip clip = AnimatorClip();
		clip.LoadState(clipNode);

		clips.emplace(clip.GetName(), clip);
	}

	// Tracks
	ParsonArray tracksArray = root.GetArray("Tracks");
	for (uint i = 0; i < tracksArray.size; ++i)
	{
		ParsonNode trackNode = tracksArray.GetNode(i);
		if (!trackNode.NodeIsValid())
		{
			continue;
		}

		AnimatorTrack track = AnimatorTrack();
		track.LoadState(trackNode);

		tracks.emplace(track.GetName(), track);
	}

	return true;
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

	if (tracks.empty())
		return false;

	float dt		= (App->gameState == GameState::PLAY) ? MC_Time::Game::GetDT() : MC_Time::Real::GetDT();			// In case a clip preview is needed outside Game Mode.
	float stepValue = dt * playbackSpeed;

	for (auto track = tracks.begin(); track != tracks.end(); ++track)
	{
		track->second.StepTrack(stepValue);
	}

	UpdateMeshSkinning();

	return true;
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
		AnimatorClip& defaultClip = AnimatorClip((*animation), defaultName, 0, (uint)(*animation)->GetDuration(), 1.0f, false);

		clips.emplace(defaultClip.GetName(), defaultClip);
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
	defaultClip				= AnimatorClip(rAnimation, defaultName, 0, (uint)rAnimation->GetDuration(), 1.0f, false);

	return true;
}

void C_Animator::FindTracksRootBones()
{
	if (tracks.empty())
		return;

	std::map<uint32, GameObject*> tmp;
	for (auto bone = bones.cbegin(); bone != bones.cend(); ++bone)
	{
		tmp.emplace((*bone)->GetUID(), (*bone));
	}

	for (auto track = tracks.begin(); track != tracks.end(); ++track)
	{
		auto bone = tmp.find(track->second.rootBoneUID);
		if (bone == tmp.end())
			continue;

		track->second.SetRootBone(bone->second);
	}

	tmp.clear();
}

void C_Animator::GeneratePreviewTrack()
{
	AddTrack(AnimatorTrack("Preview", rootBone));
}

// --- ANIMATION TRANSFORMS METHODS
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
		if (!bone->childs[i]->isBone)
			continue;

		LineSegment displayBone = { float3::zero, float3::zero };

		displayBone.a = boneTransform->GetWorldPosition();
		displayBone.b = bone->childs[i]->GetComponent<C_Transform>()->GetWorldPosition();

		displayBones.push_back(displayBone);

		GenerateBoneSegments(bone->childs[i]);
	}
}

// --- C_ANIMATOR INITIALIZATION METHODS
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
				(*bone)->isBone = true;
			}
		}
	}
}

std::vector<BoneLink>* C_Animator::GetAnimationBoneLinks(uint32 UID)
{
	if (UID == 0)
		return nullptr;

	auto boneLinks = animationBones.find(UID);
	if (boneLinks != animationBones.end())
	{
		return &boneLinks->second;
	}

	return nullptr;
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

	return true;
}

bool C_Animator::EditClip(const char* originalClipName, const AnimatorClip& editedClip)
{	
	if (clips.find(originalClipName) == clips.end())
		return false;
	
	if (clips.find(editedClip.GetName()) != clips.end())
	{
		LOG("[ERROR] Animator Component: Could not Edit Clip! Error: There was another clip with the same name as the given editedClip.");
		return false;
	}

	DeleteClip(originalClipName);

	return AddClip(editedClip);
}

bool C_Animator::DeleteClip(const char* clipName)
{
	auto clipToDelete = clips.find(clipName);
	if (clipToDelete == clips.end())
	{
		LOG("[ERROR] Animator Component: Could not Delete Clip { %s }! Error: Could not find Clip with the given name.", clipName);
		return false;
	}
	
	for (auto track = tracks.begin(); track != tracks.end(); ++track)
	{
		if (track->second.GetCurrentClip() == &clipToDelete->second)													// Comparing pointers to ascertain whether or not there is a match.
		{
			track->second.FreeCurrentClip();
		}

		if (track->second.GetBlendingClip() == &clipToDelete->second)
		{
			track->second.FreeBlendingClip();
		}
	}

	return (clips.erase(clipName) == 1);																				// std::map::erase() returns the amount of elements erased.
}

bool C_Animator::AddTrack(const AnimatorTrack& track)
{
	if (track.GetRootBone() == nullptr)
	{
		LOG("[ERROR] Animator Component: Could not Add Track { %s }! Error: Given AnimatorTrack's rootBone was nullptr.", track.GetName());
		return false;
	}
	if (tracks.find(track.GetName()) != tracks.end())
	{
		LOG("[ERROR] Animator Component: Could not Add Track { %s }! Error: A track with the same name already exists.", track.GetName());
		return false;
	}

	return (tracks.emplace(track.GetName(), track)).second;																					// The "second" element is a bool that is true
}																																			// if the insertion took place.

bool C_Animator::EditTrack(const char* originalTrackName, const AnimatorTrack& editedTrack)
{
	if (tracks.find(originalTrackName) == tracks.end())
		return false;

	DeleteTrack(originalTrackName);

	return AddTrack(editedTrack);
}

bool C_Animator::DeleteTrack(const char* trackName)
{	
	if (tracks.find(trackName) == tracks.end())
	{
		LOG("[ERROR] Animator Component: Could not Delete Track { %s }! Error: Could not find Track with the given name.", trackName);
		return false;
	}
	
	return (tracks.erase(trackName) == 1);
}

void C_Animator::PlayClip(const char* trackName, const char* clipName, uint blendFrames)
{
	auto track	= tracks.find(trackName);
	auto clip	= clips.find(clipName);
	
	if (track == tracks.end())																							// Generate one instead of just not playing the clip?
	{
		LOG("[ERROR] Animator Component: Could not Play Clip! Error: Could not find any track with the given name!");
		return;
	}
	if (clip == clips.end())
	{
		LOG("[ERROR] Animator Component: Could not Play Clip! Error: Could not find any clip with the given name!");
		return;
	}
	
	track->second.PlayClip(&clip->second, GetAnimationBoneLinks(clip->second.GetAnimation()->GetUID()), blendFrames);

	Play();
}

void C_Animator::PlayClip(const char* trackName, const char* clipName, float blendTime)
{
	auto track	= tracks.find(trackName);
	auto clip	= clips.find(clipName);

	if (track == tracks.end())
	{
		LOG("[ERROR] Animator Component: Could not Play Clip! Error: Could not find any track with the given name!");
		return;
	}
	if (clip == clips.end())
	{
		LOG("[ERROR] Animator Component: Could not Play Clip! Error: Could not find any clip with the given name!");
		return;
	}

	track->second.PlayClip(&clip->second, GetAnimationBoneLinks(clip->second.GetAnimation()->GetUID()), (uint)(blendTime * clip->second.GetAnimationTicksPerSecond()));

	Play();
}

bool C_Animator::Play(bool applyToTracks)
{
	if (animatorState == AnimatorState::PLAY)
	{
		LOG("[WARNING] Animator Component: Animator was already in { PLAY } State!");
		return true;
	}
	
	animatorState = AnimatorState::PLAY;

	if (applyToTracks)
	{
		for (auto track = tracks.begin(); track != tracks.end(); ++track)
		{
			track->second.Play();
		}
	}

	return (animatorState == AnimatorState::PLAY);
}

bool C_Animator::Pause(bool applyToTracks)
{
	if (animatorState == AnimatorState::PAUSE)
	{
		LOG("[WARNING] Animator Component: Animator is already in { PAUSE } State!");
		return true;
	}
	if (animatorState == AnimatorState::STOP)
	{
		LOG("[WARNING] Animator Component: Cannot Pause a Stopped Animation!");
		return false;
	}

	animatorState = AnimatorState::PAUSE;

	if (applyToTracks)
	{
		for (auto track = tracks.begin(); track != tracks.end(); ++track)
		{
			track->second.Pause();
		}
	}

	return (animatorState == AnimatorState::PAUSE);
}

bool C_Animator::Step(bool applyToTracks)
{	
	if (animatorState == AnimatorState::STEP)
	{
		LOG("[WARNING] Animator Component: Animator is already in { STEP } state!");
		return true;
	}
	if (animatorState != AnimatorState::PAUSE)
	{
		LOG("[WARNING] Animator Component: Only Paused Animations can be Stepped!");
		return false;
	}

	animatorState = AnimatorState::STEP;

	if (applyToTracks)
	{
		for (auto track = tracks.begin(); track != tracks.end(); ++track)
		{
			track->second.Step();
		}
	}

	return (animatorState == AnimatorState::STEP);
}

bool C_Animator::Stop(bool applyToTracks)
{
	animatorState = AnimatorState::STOP;

	if (applyToTracks)
	{
		for (auto track = tracks.begin(); track != tracks.end(); ++track)
		{
			track->second.Stop();
		}
	}

	return (animatorState == AnimatorState::STOP);
}

// --- GET/SET METHODS
AnimatorClip C_Animator::GetClip(const char* clipName) const
{
	auto clip = clips.find(clipName);
	return (clip != clips.end()) ? clip->second : AnimatorClip(nullptr, "[NONE]", 0, 0, 1.0f, false);
}

AnimatorClip* C_Animator::GetClipAsPtr(const char* clipName)
{
	auto clip = clips.find(clipName);
	return (clip != clips.end()) ? &clip->second : nullptr;
}

AnimatorClip C_Animator::GetClipByIndex(uint index) const
{
	
	
	return AnimatorClip();
}

/*void C_Animator::SetCurrentClipByIndex(uint index)
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
}*/

AnimatorTrack C_Animator::GetTrack(const char* trackName) const
{
	auto track = tracks.find(trackName);
	return (track != tracks.end()) ? track->second : AnimatorTrack("[NONE]", nullptr);
}

AnimatorTrack* C_Animator::GetTrackAsPtr(const char* trackName)
{
	auto track = tracks.find(trackName);
	return (track != tracks.end()) ? &track->second : nullptr;
}

AnimatorTrack C_Animator::GetTrackByIndex(uint index) const
{
	
	
	return AnimatorTrack();
}

std::vector<R_Animation*>* C_Animator::GetAnimationsAsPtr()
{
	return &animations;
}

std::vector<GameObject*>* C_Animator::GetBonesAsPtr()
{
	return &bones;
}

std::map<std::string, AnimatorClip>* C_Animator::GetClipsAsPtr()
{
	return &clips;
}

std::map<std::string, AnimatorTrack>* C_Animator::GetTracksAsPtr()
{
	return &tracks;
}

std::vector<LineSegment> C_Animator::GetDisplayBones() const
{
	return displayBones;
}

void C_Animator::GetClipNamesAsVector(std::vector<const char*>& clipNames) const
{
	if (clips.empty())
		return;
	
	for (auto clip = clips.cbegin(); clip != clips.cend(); ++clip)
	{
		clipNames.push_back(clip->first.c_str());
	}
}

void C_Animator::GetTrackNamesAsVector(std::vector<const char*>& trackNames) const
{
	if (tracks.empty())
		return;

	for (auto track = tracks.cbegin(); track != tracks.cend(); ++track)
	{
		trackNames.push_back(track->first.c_str());
	}
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

int C_Animator::GetIndexByAnimation(const R_Animation* rAnimation) const
{
	if (rAnimation == nullptr)
	{
		LOG("[ERROR] Animator Component: Could not Get Index by Animation! Error: Given R_Animation* was nullptr.");
		return -1;
	}

	for (uint i = 0; i < animations.size(); ++i)
	{
		if ((animations[i]->GetDuration() == rAnimation->GetDuration()) && (animations[i]->GetName() == std::string(rAnimation->GetName())))			// TMP. Check is too rudimentary.
		{
			return i;
		}
	}

	return -1;
}

float C_Animator::GetPlaybackSpeed() const
{
	return playbackSpeed;
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