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
	BoneLink(const Channel& channel, GameObject* gameObject);
	
	Channel channel;
	GameObject* gameObject;
};

struct AnimationBones
{
	std::string name;
	std::vector<BoneLink> bones;
};

enum class InterpolationType																								// WIP IDEA
{
	LINEAL,
	CUSTOM
};

class NULL_API C_Animator : public Component																							// In charge of managing Skeletal Animation
{
public:
	C_Animator(GameObject* owner);
	~C_Animator();

	bool Update() override;
	bool CleanUp() override;

	bool SaveState(ParsonNode& root) const override;
	bool LoadState(ParsonNode& root) override;

	static inline ComponentType GetType() { return ComponentType::ANIMATOR; }

public:
	void AddAnimation(R_Animation* rAnimation);
	bool AddClip(const AnimatorClip& clip);
	void PlayClip(const std::string& clipName, const uint& blendFrames);
	
	bool Play();
	bool Pause();
	bool Step();
	bool Stop();

public:																														// --- DEBUG METHODS
	bool StepToPrevKeyframe();
	bool StepToNextKeyframe();
	bool RefreshBoneDisplay();

public:																														// --- CURRENT/BLENDING ANIMATION METHODS
	AnimatorClip* GetCurrentClip() const;
	AnimatorClip* GetBlendingClip() const;

	void SetCurrentClip(AnimatorClip* clip);
	void SetBlendingClip(AnimatorClip* clip, uint blendFrames);

	void SetCurrentClipByIndex(const uint& index);
	void SetBlendingClipByIndex(const uint& index, const uint& blendFrames);

	bool CurrentClipExists() const;
	bool BlendingClipExists() const;

	void ClearCurrentClip();
	void ClearBlendingClip();

public:																														// --- GET/SET METHODS

	std::vector<LineSegment> GetDisplayBones() const;
	std::vector<std::string> GetClipNamesAsVector() const;
	std::string GetClipNamesAsString() const;
	std::string GetAnimationNamesAsString() const;
	R_Animation* GetAnimationByIndex(const uint& index) const;

	float GetPlaybackSpeed() const;
	bool GetInterpolate() const;
	bool GetLoopAnimation() const;
	bool GetPlayOnStart() const;
	bool GetCameraCulling() const;
	bool GetShowBones() const;
		 
	void SetPlaybackSpeed(const float& playbackSpeed);
	void SetInterpolate(const bool& setTo);
	void SetLoopAnimation(const bool& setTo);
	void SetPlayOnStart(const bool& setTo);
	void SetCameraCulling(const bool& setTo);
	void SetShowBones (const bool& setTo);
		 
private: 																									// --- INTERNAL METHODS
	bool StepAnimation				();
	bool StepClips					();
	bool BlendAnimation				();
	bool ValidateCurrentClip		();
		 
	void SwitchBlendingToCurrent	();
	void ResetBones					();
		 
	void AddAnimationsToAdd			();

	Transform GetInterpolatedTransform(const double& keyframe, const Channel& channel, const Transform& originalTransform) const;
	const float3 GetInterpolatedPosition(const double& keyframe, const Channel& channel, const float3& originalPosition) const;
	const Quat GetInterpolatedRotation(const double& keyframe, const Channel& channel, const Quat& originalRotation) const;
	const float3 GetInterpolatedScale(const double& keyframe, const Channel& channel, const float3& originalScale) const;

	Transform GetPoseToPoseTransform(const uint& tick, const Channel& channel, const Transform& originalTransform) const;
	const float3 GetPoseToPosePosition(const uint& tick, const Channel& channel, const float3& originalPosition) const;
	const Quat GetPoseToPoseRotation(const uint& tick, const Channel& channel, const Quat& originalRotation) const;
	const float3 GetPoseToPoseScale(const uint& tick, const Channel& channel, const float3& originalScale) const;

	Transform GetBlendedTransform(const double& blendingKeyframe, const Channel& blendingChannel, const Transform& originalTransform) const;
	const float3 GetBlendedPosition(const double& blendingKeyframe, const Channel& blendingChannel, const float3& originalPosition) const;
	const Quat GetBlendedRotation(const double& blendingKeyframe, const Channel& blendingChannel, const Quat& originalRotation) const;
	const float3 GetBlendedScale(const double& blendingKeyframe, const Channel& blendingChannel, const float3& originalScale) const;

	void FindAnimationBones(const R_Animation* rAnimation);
	bool FindBoneLinks(const R_Animation* rAnimation, std::vector<BoneLink>& links);
	GameObject* FindRootBone(const std::vector<BoneLink>& links);
	void SetRootBone(const GameObject* rootBone);
		 
	void UpdateDisplayBones();
	void GenerateBoneSegments(const GameObject* bone);
		 
	bool GenerateDefaultClip(const R_Animation* rAnimation, AnimatorClip& defaultClip);
		 
	void SortBoneLinksByHierarchy(const std::vector<BoneLink>& boneLinks, const GameObject* rootBone, std::vector<BoneLink>& sorted);

private:

	std::vector<R_Animation*> animations;																// Animation Resources. Contain bone information (transforms...).
	std::map<uint32, std::vector<BoneLink>> animationBones;
	std::unordered_map<std::string, AnimatorClip> clips;																	// Segments of animations. "Idle", "Walk", "Attack"...
	
	std::vector<BoneLink> currentBones;															// Multiple animations will have the same bones.
	std::vector<BoneLink> blendingBones;

	std::vector<LineSegment> displayBones;															// Line Segments between GO bones. For debug purposes.

	std::vector<R_Animation*> animationsToAdd;														// TODO: Change this to something cleaner later.

	AnimatorClip* currentClip;
	AnimatorClip* blendingClip;

	const GameObject* currentRootBone;

private:	
																												// --- FUNCTIONALITY VARIABLES
	uint blendFrames;

	bool play;
	bool pause;
	bool step;
	bool stop;

private:
																												// --- GET/SET VARIABLES	
	float playbackSpeed;
	bool interpolate;
	bool loopAnimation;
	bool playOnStart;
	bool cameraCulling;
	
	bool showBones;
};

#endif // !__C_ANIMATOR_H__