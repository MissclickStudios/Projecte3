#include "MathGeoLib/include/Geometry/LineSegment.h"

#include "Profiler.h"
#include "JSONParser.h"

#include "Time.h"

#include "Channel.h"
#include "AnimatorClip.h"

#include "Application.h"
#include "M_ResourceManager.h"

#include "R_Animation.h"

#include "GameObject.h"
#include "C_Transform.h"

#include "C_Animator.h"

typedef std::map<double, float3>::const_iterator	PositionKeyframe;
typedef std::map<double, Quat>::const_iterator		RotationKeyframe;
typedef std::map<double, float3>::const_iterator	ScaleKeyframe;

C_Animator::C_Animator(GameObject* owner) : Component(owner, COMPONENT_TYPE::ANIMATOR),
current_clip		(nullptr),
blending_clip		(nullptr),
current_root_bone	(nullptr)
{
	blend_frames		= 0;

	play				= false;
	pause				= false;
	step				= false;
	stop				= true;

	playback_speed		= 1.0f;
	interpolate			= true;
	loop_animation		= false;
	play_on_start		= true;
	camera_culling		= true;
	show_bones			= false;
}

C_Animator::~C_Animator()
{
	current_clip		= nullptr;
	blending_clip		= nullptr;
	current_root_bone	= nullptr;
}

bool C_Animator::Update()
{
	BROFILER_CATEGORY("Animation Component Update", Profiler::Color::DarkSlateBlue);
	
	bool ret = true;

	AddAnimationsToAdd();

	if (play || step)
	{
		if (current_clip != nullptr)
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
		App->resource_manager->FreeResource(animations[i]->GetUID());
	}

	animations.clear();
	animation_bones.clear();
	clips.clear();

	current_bones.clear();
	blending_bones.clear();

	display_bones.clear();

	animations_to_add.clear();

	return ret;
}

bool C_Animator::SaveState(ParsonNode& root) const
{
	bool ret = true;

	root.SetNumber("Type", (double)GetType());

	// Animations
	ParsonArray animations_array = root.SetArray("Animations");
	for (auto animation = animations.cbegin(); animation != animations.cend(); ++animation)
	{
		ParsonNode animation_node = animations_array.SetNode((*animation)->GetName());

		animation_node.SetNumber("UID", (*animation)->GetUID());
		animation_node.SetString("Name", (*animation)->GetAssetsFile());
		animation_node.SetString("Path", (*animation)->GetLibraryPath());
		animation_node.SetString("File", (*animation)->GetLibraryFile());
	}

	// Clips
	ParsonArray clips_array = root.SetArray("Clips");
	for (auto clip = clips.cbegin(); clip != clips.cend(); ++clip)
	{
		if (strstr(clip->first.c_str(), "Default") != nullptr)
		{
			continue;
		}
		
		ParsonNode clip_node = clips_array.SetNode(clip->second.GetName());
		clip->second.SaveState(clip_node);
	}

	// Current Clip
	if (current_clip != nullptr)
	{
		ParsonNode current_clip_node = root.SetNode("CurrentClip");
		current_clip_node.SetString("AnimationName", current_clip->GetAnimationName());
		current_clip_node.SetString("Name", current_clip->GetName());
	}

	return ret;
}

bool C_Animator::LoadState(ParsonNode& root)
{
	bool ret = true;

	ParsonArray animations_array = root.GetArray("Animations");
	for (uint i = 0; i < animations_array.size; ++i)
	{
		ParsonNode animation_node = animations_array.GetNode(i);
		if (!animation_node.NodeIsValid())
		{
			continue;
		}

		std::string assets_path = ASSETS_MODELS_PATH + std::string(animation_node.GetString("Name"));
		App->resource_manager->AllocateResource((uint32)animation_node.GetNumber("UID"), assets_path.c_str());
		
		R_Animation* r_animation = (R_Animation*)App->resource_manager->RequestResource((uint32)animation_node.GetNumber("UID"));
		if (r_animation != nullptr)
		{
			animations_to_add.push_back(r_animation);
			//AddAnimation(r_animation);
		}
	}

	ParsonArray clips_array = root.GetArray("Clips");
	for (uint i = 0; i < clips_array.size; ++i)
	{
		ParsonNode clip_node	= clips_array.GetNode(i);
		AnimatorClip clip		= AnimatorClip();

		clip.LoadState(clip_node);

		clips.emplace(clip.GetName(), clip);
	}

	ParsonNode current_clip_node = root.GetNode("CurrentClip");
	auto item = clips.find(current_clip_node.GetString("Name"));
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

	for (uint i = 0; i < current_bones.size(); ++i)
	{
		const BoneLink& bone = current_bones[i];
		
		C_Transform* c_transform = bone.game_object->GetComponent<C_Transform>();
		if (c_transform == nullptr)
		{
			LOG("[WARNING] Animation Component: GameObject { %s } did not have a Transform Component!", bone.game_object->GetName());
			continue;
		}

		const Transform& original_transform = Transform(c_transform->GetLocalTransform());
		
		if (interpolate)
		{
			Transform& interpolated_transform = GetInterpolatedTransform(current_clip->GetAnimationFrame(), bone.channel, original_transform);
			
			if (BlendingClipExists())
			{
				interpolated_transform = GetBlendedTransform(blending_clip->GetAnimationFrame(), blending_bones[i].channel, interpolated_transform);
			}

			c_transform->ImportTransform(interpolated_transform);
		}
		else
		{
			if (current_clip->in_new_tick)
			{
				Transform& pose_to_pose_transform = GetPoseToPoseTransform(current_clip->GetAnimationTick(), bone.channel, original_transform);

				if (BlendingClipExists())
				{
					pose_to_pose_transform = GetBlendedTransform(blending_clip->GetAnimationTick(), blending_bones[i].channel, pose_to_pose_transform);
				}

				c_transform->ImportTransform(pose_to_pose_transform);
			}
		}
	}

	UpdateDisplayBones();

	return ret;
}

bool C_Animator::StepClips()
{
	bool ret = true;

	bool current_exists		= CurrentClipExists();
	bool blending_exists	= BlendingClipExists();

	if (!current_exists && !blending_exists)
	{
		LOG("[ERROR] Animator Component: Could not Step Clips! Error: There were no Current or Blending Clips set.");
		return false;
	}
	if (!current_exists && blending_exists)
	{
		SwitchBlendingToCurrent();
	}

	if (BlendingClipExists())
	{
		if (blending_clip->GetAnimationFrame() > (float)(blending_clip->GetStart() + blend_frames))
		{
			SwitchBlendingToCurrent();
		}
	}
	
	float dt			= (App->play) ? Time::Game::GetDT() : Time::Real::GetDT();											// In case a clip preview is needed outside Game Mode.
	float step_value	= dt * playback_speed;

	if (CurrentClipExists())
	{
		bool success = current_clip->StepClip(step_value);
		if (!success)
		{
			if (BlendingClipExists())
			{
				blending_clip->StepClip(step_value);
				SwitchBlendingToCurrent();
				return true;
			}
			else
			{
				if (!current_clip->IsLooped())
				{
					Stop();
					ResetBones();
					return false;
				}
			}
		}

		if (BlendingClipExists())
		{
			blending_clip->StepClip(step_value);
		}
	}

	return ret;
}

bool C_Animator::BlendAnimation()
{
	bool ret = true;



	return ret;
}

bool C_Animator::ValidateCurrentClip()
{
	bool ret = true;
	
	if (current_clip == nullptr)
	{
		if (blending_clip != nullptr)
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
	if (current_clip != nullptr)
	{
		current_clip->playing = false;
		current_clip->ClearClip();
		ClearCurrentClip();
	}

	SetCurrentClip(blending_clip);
	ClearBlendingClip();
}

void C_Animator::ResetBones()
{
	if (current_clip == nullptr)
	{
		LOG("[ERROR] Animator Component: Could not Reset Bones! Error: Current Clip was nullptr.");
		return;
	}
	
	for (auto bone = current_bones.cbegin(); bone != current_bones.cend(); ++bone)
	{
		const Transform& transform = Transform(bone->game_object->GetComponent<C_Transform>()->GetLocalTransform());
		const Transform& interpolated_transform = GetInterpolatedTransform((double)current_clip->GetStart(), bone->channel, transform);

		bone->game_object->GetComponent<C_Transform>()->ImportTransform(interpolated_transform);
	}

	UpdateDisplayBones();
}

void C_Animator::AddAnimationsToAdd()
{
	if (!animations_to_add.empty())
	{
		for (uint i = 0; i < animations_to_add.size(); ++i)
		{
			AddAnimation(animations_to_add[i]);
		}

		animations_to_add.clear();
	}
}

Transform C_Animator::GetInterpolatedTransform(const double& keyframe, const Channel& channel, const Transform& original_transform) const
{
	float3	interpolated_position	= GetInterpolatedPosition(keyframe, channel, original_transform.position);
	Quat	interpolated_rotation	= GetInterpolatedRotation(keyframe, channel, original_transform.rotation);
	float3	interpolated_scale		= GetInterpolatedScale(keyframe, channel, original_transform.scale);

	return Transform(interpolated_position, interpolated_rotation, interpolated_scale);
}

const float3 C_Animator::GetInterpolatedPosition(const double& keyframe, const Channel& channel, const float3& original_position) const
{
	if (!channel.HasPositionKeyframes())
	{
		return original_position;
	}

	PositionKeyframe prev_keyframe = channel.GetClosestPrevPositionKeyframe(keyframe);
	PositionKeyframe next_keyframe = channel.GetClosestNextPositionKeyframe(keyframe);

	float rate = (float)(keyframe / next_keyframe->first);
	float3 ret = (prev_keyframe == next_keyframe) ? prev_keyframe->second : prev_keyframe->second.Lerp(next_keyframe->second, rate);

	return ret;
}

const Quat C_Animator::GetInterpolatedRotation(const double& keyframe, const Channel& channel, const Quat& original_rotation) const
{
	if (!channel.HasRotationKeyframes())
	{
		return original_rotation;
	}

	RotationKeyframe prev_keyframe = channel.GetClosestPrevRotationKeyframe(keyframe);
	RotationKeyframe next_keyframe = channel.GetClosestNextRotationKeyframe(keyframe);

	float rate	= (float)(keyframe / next_keyframe->first);
	Quat ret	= (prev_keyframe == next_keyframe) ? prev_keyframe->second : prev_keyframe->second.Slerp(next_keyframe->second, rate);

	return ret;
}

const float3 C_Animator::GetInterpolatedScale(const double& keyframe, const Channel& channel, const float3& original_scale) const
{
	if (!channel.HasScaleKeyframes())
	{
		return original_scale;
	}

	ScaleKeyframe prev_keyframe = channel.GetClosestPrevScaleKeyframe(keyframe);
	ScaleKeyframe next_keyframe = channel.GetClosestNextScaleKeyframe(keyframe);

	float rate = (float)(keyframe / next_keyframe->first);
	float3 ret = (prev_keyframe == next_keyframe) ? prev_keyframe->second : prev_keyframe->second.Lerp(next_keyframe->second, rate);

	return ret;
}

Transform C_Animator::GetPoseToPoseTransform(const uint& tick, const Channel& channel, const Transform& original_transform) const
{
	const float3& position	= GetPoseToPosePosition(tick, channel, original_transform.position);
	const Quat& rotation	= GetPoseToPoseRotation(tick, channel, original_transform.rotation);
	const float3& scale		= GetPoseToPoseScale(tick, channel, original_transform.scale);
	
	return Transform(position, rotation, scale);
}

const float3 C_Animator::GetPoseToPosePosition(const uint& tick, const Channel& channel, const float3& original_position) const
{
	if (!channel.HasPositionKeyframes()) 
	{ 
		return original_position; 
	}

	return channel.GetPositionKeyframe(tick)->second;
}

const Quat C_Animator::GetPoseToPoseRotation(const uint& tick, const Channel& channel, const Quat& original_rotation) const
{
	if (!channel.HasRotationKeyframes())
	{
		return original_rotation;
	}

	return channel.GetRotationKeyframe(tick)->second;
}

const float3 C_Animator::GetPoseToPoseScale(const uint& tick, const Channel& channel, const float3& original_scale) const
{
	if (!channel.HasScaleKeyframes())
	{
		return original_scale;
	}

	return channel.GetScaleKeyframe(tick)->second;
}

Transform C_Animator::GetBlendedTransform(const double& blended_keyframe, const Channel& blended_channel, const Transform& original_transform) const
{
	const float3& position	= GetBlendedPosition(blended_keyframe, blended_channel, original_transform.position);
	const Quat& rotation	= GetBlendedRotation(blended_keyframe, blended_channel, original_transform.rotation);
	const float3& scale		= GetBlendedScale(blended_keyframe, blended_channel, original_transform.scale);

	return Transform(position, rotation, scale);
}

const float3 C_Animator::GetBlendedPosition(const double& blending_keyframe, const Channel& blending_channel, const float3& original_position) const
{
	if (!blending_channel.HasPositionKeyframes())
	{
		return original_position;
	}

	float3 position			= GetInterpolatedPosition(blending_keyframe, blending_channel, original_position);

	double blend_frame		= blending_keyframe - blending_clip->GetStart();
	float blend_rate		= (float)(blend_frame / blend_frames);
	float3 blended_position	= original_position.Lerp(position, blend_rate);
	
	return blended_position;
}

const Quat C_Animator::GetBlendedRotation(const double& blending_keyframe, const Channel& blending_channel, const Quat& original_rotation) const
{
	if (!blending_channel.HasRotationKeyframes())
	{
		return original_rotation;
	}

	Quat rotation			= GetInterpolatedRotation(blending_keyframe, blending_channel, original_rotation);

	double blend_frame		= blending_keyframe - blending_clip->GetStart();
	float blend_rate		= (float)(blend_frame / blend_frames);
	Quat blended_rotation	= original_rotation.Slerp(rotation, blend_rate);

	return blended_rotation;
}

const float3 C_Animator::GetBlendedScale(const double& blending_keyframe, const Channel& blending_channel, const float3& original_scale) const
{
	if (!blending_channel.HasScaleKeyframes())
	{
		return original_scale;
	}

	float3 scale			= GetInterpolatedScale(blending_keyframe, blending_channel, original_scale);

	double blend_frame		= blending_keyframe - blending_clip->GetStart();
	float blend_rate		= (float)(blend_frame / blend_frames);
	float3 blended_scale	= original_scale.Lerp(scale, blend_rate);

	return blended_scale;
}

void C_Animator::FindAnimationBones(const R_Animation* r_animation)
{
	if (r_animation == nullptr)
	{
		return;
	}
	if (r_animation->channels.empty())
	{
		return;
	}

	std::vector<BoneLink> links;
	bool success = FindBoneLinks(r_animation, links);
	if (success)
	{
		animation_bones.emplace(r_animation->GetUID(), links);

		GameObject* root_bone = FindRootBone(links);
		if (root_bone != nullptr)
		{
			SetRootBone(root_bone);
			UpdateDisplayBones();
		}
	}
}

bool C_Animator::FindBoneLinks(const R_Animation* r_animation, std::vector<BoneLink>& links)
{
	if (r_animation == nullptr)
	{
		LOG("[ERROR] Animator Component: Could not find Bone Links! Error: Given R_Animation* was nullptr.");
		return false;
	}
	if (r_animation->channels.empty())
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

	for (auto channel = r_animation->channels.cbegin(); channel != r_animation->channels.cend(); ++channel)						// Trying out the auto keyword
	{
		auto go_item = childs.find(channel->name);
		if (go_item != childs.end())
		{
			go_item->second->is_bone = true;
			links.push_back(BoneLink((*channel), go_item->second));
		}
	}

	childs.clear();

	return true;
}

GameObject* C_Animator::FindRootBone(const std::vector<BoneLink>& links)
{	
	for (auto link = links.cbegin(); link != links.cend(); ++link)																// Trying out the auto keyword
	{
		if (link->game_object->parent == nullptr)
		{
			continue;
		}

		if (!link->game_object->parent->is_bone)
		{
			return link->game_object;
		}
	}

	return nullptr;
}

void C_Animator::SetRootBone(const GameObject* root_bone)
{
	if (root_bone == nullptr)
	{
		LOG("[ERROR] Animator Component: Could not Set Root Bone! Error: Given GameObject* was nullptr.");
		return;
	}
	
	if (current_root_bone == nullptr)
	{
		current_root_bone = root_bone;
	}
	else
	{
		if (current_root_bone != root_bone)
		{
			LOG("[WARNING] Animator Component: Disparity between root bones detected! A: [%s], B: [%s]", current_root_bone->GetName(), root_bone->GetName());
		}
	}
}

void C_Animator::UpdateDisplayBones()
{
	display_bones.clear();
	
	if (current_root_bone != nullptr)
	{
		GenerateBoneSegments(current_root_bone);
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
	
	C_Transform* bone_transform = bone->GetComponent<C_Transform>();

	for (uint i = 0; i < bone->childs.size(); ++i)
	{
		LineSegment display_bone = { float3::zero, float3::zero };

		display_bone.a = bone_transform->GetWorldPosition();
		display_bone.b = bone->childs[i]->GetComponent<C_Transform>()->GetWorldPosition();

		display_bones.push_back(display_bone);

		GenerateBoneSegments(bone->childs[i]);
	}
}

bool C_Animator::GenerateDefaultClip(const R_Animation* r_animation, AnimatorClip& default_clip)
{
	if (r_animation == nullptr)
	{
		LOG("[ERROR] Animator Component: Could not Generate Default Clip! Error: Given R_Animation* was nullptr.");
		return false;
	}
	
	std::string default_name	= r_animation->GetName() + std::string(" Default");
	default_clip				= AnimatorClip(r_animation, default_name, 0, (uint)r_animation->GetDuration(), false);

	return true;
}

void C_Animator::SortBoneLinksByHierarchy(const std::vector<BoneLink>& bone_links, const GameObject* root_bone, std::vector<BoneLink>& sorted)
{
	if (root_bone == nullptr)
	{
		return;
	}
	
	if (root_bone == current_root_bone)
	{
		for (uint j = 0; j < bone_links.size(); ++j)
		{
			if (bone_links[j].channel.name == root_bone->GetName())
			{
				sorted.push_back(bone_links[j]);
			}
		}
	}

	for (uint i = 0; i < root_bone->childs.size(); ++i)
	{
		for (uint j = 0; j < bone_links.size(); ++j)
		{
			if (bone_links[j].channel.name == root_bone->childs[i]->GetName())
			{
				sorted.push_back(bone_links[j]);
			}
		}
	}

	for (uint i = 0; i < root_bone->childs.size(); ++i)
	{
		SortBoneLinksByHierarchy(bone_links, root_bone->childs[i], sorted);
	}
}

void C_Animator::AddAnimation(R_Animation* r_animation)
{
	if (r_animation == nullptr)
	{
		LOG("[ERROR] Animator Component: Could not Add Animation to %s's Animation Component! Error: Argument R_Animation* was nullptr.", this->GetOwner()->GetName());
		return;
	}

	animations.push_back(r_animation);

	FindAnimationBones(r_animation);

	AnimatorClip default_clip = AnimatorClip();
	bool success = GenerateDefaultClip(r_animation, default_clip);
	if (success)
	{
		clips.emplace(default_clip.GetName(), default_clip);

		if (current_clip == nullptr)
		{
			SetCurrentClip(&clips.find(default_clip.GetName())->second);
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

	if (current_clip == nullptr)
	{
		current_clip = (AnimatorClip*)&clip;
	}

	return true;
}

void C_Animator::PlayClip(const std::string& clip_name, const uint& blend_frames)
{
	auto item = clips.find(clip_name);
	if (item == clips.end())
	{
		LOG("[ERROR] Animator Component: Could not Play Clip! Error: Could not find any clip with the given name!");
		return;
	}
	if (current_clip != nullptr && current_clip->GetName() == clip_name)
	{
		return;
	}
	
	if (current_clip == nullptr || blend_frames == 0 || blend_frames > item->second.GetDuration())
	{
		Stop();
		SetCurrentClip(&item->second);
	}
	else
	{
		SetBlendingClip(&item->second, blend_frames);
	}

	Play();
}

bool C_Animator::Play()
{
	if (current_clip == nullptr)
	{
		return false;
	}
	
	play	= true;

	pause	= false;
	step	= false;
	stop	= false;

	current_clip->playing = true;

	if (BlendingClipExists()) 
	{ 
		blending_clip->playing = true; 
	};

	return play;
}

bool C_Animator::Pause()
{
	if (play)
	{
		pause	= true;
		play	= false;
		step	= false;
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
	stop	= true;

	play	= false;
	pause	= false;
	step	= false;
	
	current_clip->playing = false;
	current_clip->ClearClip();

	if (BlendingClipExists())
	{
		blending_clip->playing = false;
		blending_clip->ClearClip();
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
	if (current_clip == nullptr)
	{
		LOG("[ERROR] Animator Component: Could not Step Animation to Prev Keyframe! Error: Current Clip (AnimatorClip*) was nullptr.");
		return false;
	}

	current_clip->StepClipToPrevKeyframe();

	for (uint i = 0; i < current_bones.size(); ++i)
	{
		const Transform& transform				= Transform(current_bones[i].game_object->GetComponent<C_Transform>()->GetLocalTransform());
		const Transform& interpolated_transform = GetInterpolatedTransform((double)current_clip->GetClipTick(), current_bones[i].channel, transform);

		current_bones[i].game_object->GetComponent<C_Transform>()->ImportTransform(interpolated_transform);
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
	if (current_clip == nullptr)
	{
		LOG("[ERROR] Animator Component: Could not Step Animation to Next Keyframe! Error: Current Clip (AnimatorClip*) was nullptr.");
		return false;
	}
	
	current_clip->StepClipToNextKeyframe();

	for (uint i = 0; i < current_bones.size(); ++i)
	{
		const Transform& transform				= Transform(current_bones[i].game_object->GetComponent<C_Transform>()->GetLocalTransform());
		const Transform& interpolated_transform = GetInterpolatedTransform((double)current_clip->GetClipTick(), current_bones[i].channel, transform);

		current_bones[i].game_object->GetComponent<C_Transform>()->ImportTransform(interpolated_transform);
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
	return current_clip;
}

AnimatorClip* C_Animator::GetBlendingClip() const
{
	return blending_clip;
}

void C_Animator::SetCurrentClip(AnimatorClip* clip)
{
	std::string error_string = "[ERROR] Animator Component: Could not Set Current Clip to { " + std::string(this->GetOwner()->GetName()) + " }'s Animator Component";
	
	if (clip == nullptr)
	{
		LOG("%s! Error: Given AnimatorClip* was nullptr.", error_string.c_str());
		return;
	}
	if (clips.find(clip->GetName()) == clips.end())
	{
		LOG("%s! Error: Could not find the given AnimatorClip* in the clips map.", error_string.c_str());
		return;
	}
	if (clip->GetAnimation() == nullptr)
	{
		LOG("%s! Error: Given AnimatorClip* had no R_Animation* assigned to it.", error_string.c_str());
		return;
	}
	
	auto bones = animation_bones.find(clip->GetAnimation()->GetUID());
	if (bones == animation_bones.end())
	{
		LOG("%s! Error: Could not find the Bones of the Clip's animation (R_Animation*).");
		return;
	}

	current_clip	= clip;
	current_bones	= bones->second;

	//current_clip->ClearClip();
}

void C_Animator::SetBlendingClip(AnimatorClip* clip, uint blend_frames)
{
	std::string error_string = "[ERROR] Animator Component: Could not Set Blending Clip in { " + std::string(this->GetOwner()->GetName()) + " }'s Animator Component";

	if (clip == nullptr)
	{
		LOG("%s! Error: Given AnimatorClip* was nullptr.", error_string.c_str());
		return;
	}
	if (clips.find(clip->GetName()) == clips.end())
	{
		LOG("%s! Error: Could not find the given AnimatorClip* in the clips map.", error_string.c_str());
		return;
	}
	if (clip->GetAnimation() == nullptr)
	{
		LOG("%s! Error: Given AnimatorClip* had no R_Animation* assigned to it.", error_string.c_str());
		return;
	}

	auto bones = animation_bones.find(clip->GetAnimation()->GetUID());
	if (bones == animation_bones.end())
	{
		LOG("%s! Error: Could not find the Bones of the Clip's animation (R_Animation*).");
		return;
	}
	
	blending_clip		= clip;
	blending_bones		= bones->second;
	this->blend_frames	= blend_frames;

	blending_clip->ClearClip();																					// Resetting the clip just in case.
}

void C_Animator::SetCurrentClipByIndex(const uint& index)
{
	if (index >= clips.size())
	{
		LOG("[ERROR] Animator Component: Could not Set Current Clip By Index! Error: Given Index was out of bounds.");
		return;
	}

	std::string error_string = "[ERROR] Animator Component: Could not Set Current Clip in { " + std::string(this->GetOwner()->GetName()) + " }'s Animator Component";

	uint i = 0;
	for (auto item = clips.cbegin(); item != clips.cend(); ++item)
	{
		if (i == index)																										// Dirty way of finding items in a map by index.
		{
			const AnimatorClip& clip = item->second;

			if (animation_bones.find(clip.GetAnimation()->GetUID()) == animation_bones.end())
			{
				LOG("%s! Error: Could not find the Bones of the Clip's animation (R_Animation*).");
				return;
			}

			current_clip	= (AnimatorClip*)&clip;
			current_bones	= animation_bones.find(clip.GetAnimation()->GetUID())->second;

			current_clip->ClearClip();

			return;
		}

		++i;
	}
}

/*void C_Animator::SetBlendingClipByIndex(const uint& index, const uint& blend_frames)
{

}*/

bool C_Animator::CurrentClipExists() const
{
	return (current_clip != nullptr) ? true : false;
}

bool C_Animator::BlendingClipExists() const
{
	return (blending_clip != nullptr) ? true : false;
}

void C_Animator::ClearCurrentClip()
{
	current_clip = nullptr;
	current_bones.clear();
}

void C_Animator::ClearBlendingClip()
{
	blending_clip	= nullptr;
	blend_frames	= 0;
	blending_bones.clear();
}

// --- GET/SET METHODS
std::vector<LineSegment> C_Animator::GetDisplayBones() const
{
	return display_bones;
}

std::vector<std::string> C_Animator::GetClipNamesAsVector() const
{
	std::vector<std::string> clip_names;

	for (auto clip = clips.cbegin(); clip != clips.cend(); ++clip)
	{
		clip_names.push_back(clip->first);
	}

	return clip_names;
}

std::string C_Animator::GetClipNamesAsString() const
{
	std::string clip_names = "";

	for (auto clip = clips.cbegin(); clip != clips.cend(); ++clip)
	{
		clip_names += clip->first.c_str();
		clip_names += '\0';
	}

	return clip_names;
}

std::string C_Animator::GetAnimationNamesAsString() const
{
	std::string animation_names = "";

	for (auto animation = animations.cbegin(); animation != animations.cend(); ++animation)
	{
		animation_names += (*animation)->GetName();
		animation_names += '\0';
	}

	return animation_names;
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
	return playback_speed;
}

bool C_Animator::GetInterpolate() const
{
	return interpolate;
}

bool C_Animator::GetLoopAnimation() const
{
	return loop_animation;
}

bool C_Animator::GetPlayOnStart() const
{
	return play_on_start;
}

bool C_Animator::GetCameraCulling() const
{
	return camera_culling;
}

bool C_Animator::GetShowBones() const
{
	return show_bones;
}

void C_Animator::SetPlaybackSpeed(const float& playback_speed)
{
	this->playback_speed = playback_speed;
}

void C_Animator::SetInterpolate(const bool& set_to)
{
	interpolate = set_to;
}

void C_Animator::SetLoopAnimation(const bool& set_to)
{
	loop_animation = set_to;
}

void C_Animator::SetPlayOnStart(const bool& set_to)
{
	play_on_start = set_to;
}

void C_Animator::SetCameraCulling(const bool& set_to)
{
	camera_culling = set_to;
}

void C_Animator::SetShowBones(const bool& set_to)
{
	show_bones = set_to;
}

// --- BONE LINK METHODS
BoneLink::BoneLink() : 
channel(Channel()),
game_object(nullptr)
{

}

BoneLink::BoneLink(const Channel& channel, GameObject* game_object) : 
channel(channel),
game_object(game_object)
{

}