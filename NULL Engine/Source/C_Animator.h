#ifndef __C_ANIMATOR_H__
#define __C_ANIMATOR_H__

#include <string>
#include <vector>
#include <unordered_map>

#include "MathGeoTransform.h"

#include "Channel.h"
#include "AnimatorClip.h"

#include "Component.h"

namespace math
{
	class LineSegment;
}

class ParsonNode;

class GameObject;
class R_Animation;

typedef unsigned int uint;
typedef unsigned __int32 uint32;

struct BoneLink
{
	BoneLink();
	BoneLink(const Channel& channel, GameObject* game_object);
	
	Channel channel;
	GameObject* game_object;
};

struct AnimationBones
{
	std::string name;
	std::vector<BoneLink> bones;
};

enum class INTERPOLATION_TYPE																								// WIP IDEA
{
	LINEAL,
	CUSTOM
};

class C_Animator : public Component																							// In charge of managing Skeletal Animation
{
public:
	C_Animator(GameObject* owner);
	~C_Animator();

	bool Update		() override;
	bool CleanUp	() override;

	bool SaveState	(ParsonNode& root) const override;
	bool LoadState	(ParsonNode& root) override;

	static inline COMPONENT_TYPE GetType() { return COMPONENT_TYPE::ANIMATOR; }

public:
	void				AddAnimation				(R_Animation* r_animation);
	bool				AddClip						(const AnimatorClip& clip);
	void				PlayClip					(const std::string& clip_name, const uint& blend_frames);
	
	bool				Play						();
	bool				Pause						();
	bool				Step						();
	bool				Stop						();

public:																														// --- DEBUG METHODS
	bool				StepToPrevKeyframe			();
	bool				StepToNextKeyframe			();
	bool				RefreshBoneDisplay			();

public:																														// --- CURRENT/BLENDING ANIMATION METHODS
	AnimatorClip*		GetCurrentClip				() const;
	AnimatorClip*		GetBlendingClip				() const;

	void				SetCurrentClip				(AnimatorClip* clip);
	void				SetBlendingClip				(AnimatorClip* clip, uint blend_frames);

	void				SetCurrentClipByIndex		(const uint& index);
	void				SetBlendingClipByIndex		(const uint& index, const uint& blend_frames);

	bool				CurrentClipExists			() const;
	bool				BlendingClipExists			() const;

	void				ClearCurrentClip			();
	void				ClearBlendingClip			();

public:																														// --- GET/SET METHODS
	std::vector<LineSegment> GetDisplayBones		() const;
	std::vector<std::string> GetClipNamesAsVector	() const;
	std::string			GetClipNamesAsString		() const;
	std::string			GetAnimationNamesAsString	() const;
	R_Animation*		GetAnimationByIndex			(const uint& index) const;

	float				GetPlaybackSpeed			() const;
	bool				GetInterpolate				() const;
	bool				GetLoopAnimation			() const;
	bool				GetPlayOnStart				() const;
	bool				GetCameraCulling			() const;
	bool				GetShowBones				() const;

	void				SetPlaybackSpeed			(const float& playback_speed);
	void				SetInterpolate				(const bool& set_to);
	void				SetLoopAnimation			(const bool& set_to);
	void				SetPlayOnStart				(const bool& set_to);
	void				SetCameraCulling			(const bool& set_to);
	void				SetShowBones				(const bool& set_to);

private:																													// --- INTERNAL METHODS
	bool				StepAnimation				();
	bool				StepClips					();
	bool				BlendAnimation				();
	bool				ValidateCurrentClip			();

	void				SwitchBlendingToCurrent		();
	void				ResetBones					();

	void				AddAnimationsToAdd			();

	Transform			GetInterpolatedTransform	(const double& keyframe, const Channel& channel, const Transform& original_transform) const;
	const float3		GetInterpolatedPosition		(const double& keyframe, const Channel& channel, const float3& original_position) const;
	const Quat			GetInterpolatedRotation		(const double& keyframe, const Channel& channel, const Quat& original_rotation) const;
	const float3		GetInterpolatedScale		(const double& keyframe, const Channel& channel, const float3& original_scale) const;

	Transform			GetPoseToPoseTransform		(const uint& tick, const Channel& channel, const Transform& original_transform) const;
	const float3		GetPoseToPosePosition		(const uint& tick, const Channel& channel, const float3& original_position) const;
	const Quat			GetPoseToPoseRotation		(const uint& tick, const Channel& channel, const Quat& original_rotation) const;
	const float3		GetPoseToPoseScale			(const uint& tick, const Channel& channel, const float3& original_scale) const;

	Transform			GetBlendedTransform			(const double& blending_keyframe, const Channel& blending_channel, const Transform& original_transform) const;
	const float3		GetBlendedPosition			(const double& blending_keyframe, const Channel& blending_channel, const float3& original_position) const;
	const Quat			GetBlendedRotation			(const double& blending_keyframe, const Channel& blending_channel, const Quat& original_rotation) const;
	const float3		GetBlendedScale				(const double& blending_keyframe, const Channel& blending_channel, const float3& original_scale) const;

	void				FindAnimationBones			(const R_Animation* r_animation);
	bool				FindBoneLinks				(const R_Animation* r_animation, std::vector<BoneLink>& links);
	GameObject*			FindRootBone				(const std::vector<BoneLink>& links);
	void				SetRootBone					(const GameObject* root_bone);

	void				UpdateDisplayBones			();
	void				GenerateBoneSegments		(const GameObject* bone);
	
	bool				GenerateDefaultClip			(const R_Animation* r_animation, AnimatorClip& default_clip);

	void				SortBoneLinksByHierarchy	(const std::vector<BoneLink>& bone_links, const GameObject* root_bone, std::vector<BoneLink>& sorted);

private:
	std::vector<R_Animation*>						animations;																// Animation Resources. Contain bone information (transforms...).
	std::map<uint32, std::vector<BoneLink>>			animation_bones;
	std::unordered_map<std::string, AnimatorClip>	clips;																	// Segments of animations. "Idle", "Walk", "Attack"...
	
	std::vector<BoneLink>							current_bones;															// Multiple animations will have the same bones.
	std::vector<BoneLink>							blending_bones;

	std::vector<LineSegment>						display_bones;															// Line Segments between GO bones. For debug purposes.

	std::vector<R_Animation*>						animations_to_add;														// TODO: Change this to something cleaner later.

	AnimatorClip*									current_clip;
	AnimatorClip*									blending_clip;

	const GameObject*								current_root_bone;

private:																													// --- FUNCTIONALITY VARIABLES
	uint			blend_frames;

	bool			play;
	bool			pause;
	bool			step;
	bool			stop;

private:																													// --- GET/SET VARIABLES	
	float			playback_speed;
	bool			interpolate;
	bool			loop_animation;
	bool			play_on_start;
	bool			camera_culling;
	
	bool			show_bones;
};

#endif // !__C_ANIMATOR_H__