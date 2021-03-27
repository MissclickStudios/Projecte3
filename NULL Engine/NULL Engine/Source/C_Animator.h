#ifndef __C_ANIMATOR_H__
#define __C_ANIMATOR_H__

#include <string>
#include <vector>
#include <unordered_map>

#include "MathGeoTransform.h"

#include "BoneLink.h"
#include "AnimatorClip.h"

#include "Component.h"

namespace math
{
	class LineSegment;
}

class ParsonNode;

class R_Animation;

class GameObject;
class C_Transform;

struct Channel;

typedef unsigned int		uint;
typedef unsigned __int32	uint32;

struct AnimatorBones
{
	std::string name;
	std::vector<BoneLink> bones;
};

enum class InterpolationType																								// WIP IDEA
{
	LINEAL,
	CUSTOM
};

enum class AnimatorState
{
	PLAY,
	PAUSE,
	STEP,
	STOP
};

class NULL_API C_Animator : public Component																				// In charge of managing Skeletal Animation
{
public:
	C_Animator(GameObject* owner);
	~C_Animator();

	bool Update		() override;
	bool CleanUp	() override;

	bool SaveState	(ParsonNode& root) const override;
	bool LoadState	(ParsonNode& root) override;

	static inline ComponentType GetType() { return ComponentType::ANIMATOR; }

public:
	void			AddAnimation							(R_Animation* rAnimation);
	bool			AddClip									(const AnimatorClip& clip);
	void			PlayClip								(const std::string& clipName, uint blendFrames);
	
	bool			Play									();
	bool			Pause									();
	bool			Step									();
	bool			Stop									();

public:																														// --- DEBUG METHODS
	bool			StepToPrevKeyframe						();
	bool			StepToNextKeyframe						();
	bool			RefreshBoneDisplay						();

public:																														// --- CURRENT/BLENDING ANIMATION METHODS
	AnimatorClip*	GetCurrentClip							() const;
	AnimatorClip*	GetBlendingClip							() const;

	void			SetCurrentClip							(AnimatorClip* clip);
	void			SetBlendingClip							(AnimatorClip* clip, uint blendFrames);

	void			SetCurrentClipByIndex					(uint index);
	void			SetBlendingClipByIndex					(uint index, uint blendFrames);

	bool			CurrentClipExists						() const;
	bool			BlendingClipExists						() const;

	void			ClearCurrentClip						();
	void			ClearBlendingClip						();

public:																														// --- GET/SET METHODS
	std::vector<LineSegment> GetDisplayBones				() const;
	std::vector<std::string> GetClipNamesAsVector			() const;

	std::string		GetAnimatorStateAsString				() const;

	std::string		GetClipNamesAsString					() const;
	std::string		GetAnimationNamesAsString				() const;
	R_Animation*	GetAnimationByIndex						(uint index) const;

	float			GetPlaybackSpeed						() const;
	bool			GetInterpolate							() const;
	bool			GetLoopAnimation						() const;
	bool			GetPlayOnStart							() const;
	bool			GetCameraCulling						() const;
	bool			GetShowBones							() const;

	void			SetPlaybackSpeed						(float playbackSpeed);
	void			SetInterpolate							(bool setTo);
	void			SetLoopAnimation						(bool setTo);
	void			SetPlayOnStart							(bool setTo);
	void			SetCameraCulling						(bool setTo);
	void			SetShowBones							(bool setTo);
		 
private: 																													// --- C_ANIMATOR INITIALIZATION METHODS
	void			GetAnimatedMeshes						();
	void			FindBones								();
	void			FindBoneLinks							();
	void			GenerateDefaultClips					();
	
	void			FindRootBone							();
	void			SetRootBone								(GameObject* rootBone);
	GameObject*		GetRootBone								() const;
	void			CrossCheckBonesWithMeshBoneMapping		();
	
	bool			GenerateDefaultClip						(const R_Animation* rAnimation, AnimatorClip& defaultClip);

private:																													// --- ANIMATION/CLIP REPRODUCTION METHODS
	void			CheckGameState							();
	
	bool			StepAnimation							();
	bool			StepClips								();
	bool			BlendAnimation							();
	bool			ValidateCurrentClip						();
					
	void			SwitchBlendingToCurrent					();
	void			ResetBones								();

private:																													// --- BONE/CHANNEL UPDATE METHODS
	void			UpdateChannelTransforms					();
	void			UpdateMeshSkinning						();
	void			UpdateDisplayBones						();
	void			GenerateBoneSegments					(const GameObject* bone);
	
	//Transform		GetInterpolatedTransform				(double keyframe, const Channel& channel, const Transform& originalTransform) const;
	Transform		GetInterpolatedTransform				(double keyframe, const Channel& channel, C_Transform* originalTransform) const;
	const float3	GetInterpolatedPosition					(double keyframe, const Channel& channel) const;
	const Quat		GetInterpolatedRotation					(double keyframe, const Channel& channel) const;
	const float3	GetInterpolatedScale					(double keyframe, const Channel& channel) const;

	Transform		GetPoseToPoseTransform					(uint tick, const Channel& channel, const Transform& originalTransform) const;

	Transform		GetBlendedTransform						(double blendingKeyframe, const Channel& blendingChannel, const Transform& originalTransform) const;
	const float3	GetBlendedPosition						(double blendingKeyframe, const Channel& blendingChannel, const float3& originalPosition) const;
	const Quat		GetBlendedRotation						(double blendingKeyframe, const Channel& blendingChannel, const Quat& originalRotation) const;
	const float3	GetBlendedScale							(double blendingKeyframe, const Channel& blendingChannel, const float3& originalScale) const;

private:
	std::vector<R_Animation*>						animations;											// Animation Resources. Contain bone information (transforms...).
	std::map<uint32, std::vector<BoneLink>>			animationBones;
	
	std::vector<C_Mesh*>							animatedMeshes;										// TMP. Until a better implementation is found;

	std::vector<GameObject*>						bones;												//
	std::vector<BoneLink>							currentBones;										// Multiple animations will have the same bones.
	std::vector<BoneLink>							blendingBones;										//
	std::vector<LineSegment>						displayBones;										// Line Segments between GO bones. For debug purposes.

	std::unordered_map<std::string, AnimatorClip>	clips;												// Segments of animations. "Idle", "Walk", "Attack"...

	AnimatorClip*				currentClip;
	AnimatorClip*				blendingClip;

	GameObject*					rootBone;

private:																								// --- FUNCTIONALITY VARIABLES
	AnimatorState	animatorState;

	uint			blendFrames;

	bool			needsInit;

private:																								// --- GET/SET VARIABLES	
	float			playbackSpeed;
	bool			interpolate;
	bool			loopAnimation;
	bool			playOnStart;
	bool			cameraCulling;
	
	bool			showBones;
};

#endif // !__C_ANIMATOR_H__