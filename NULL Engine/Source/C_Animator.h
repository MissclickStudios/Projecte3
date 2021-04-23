#ifndef __C_ANIMATOR_H__
#define __C_ANIMATOR_H__

#include <string>
#include <vector>
#include <map>

#include "MathGeoTransform.h"

#include "BoneLink.h"
#include "AnimatorClip.h"
#include "AnimatorTrack.h"

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

	bool Start		() override;
	bool Update		() override;
	bool CleanUp	() override;

	bool SaveState	(ParsonNode& root) const override;
	bool LoadState	(ParsonNode& root) override;

	static inline ComponentType GetType() { return ComponentType::ANIMATOR; }

public:
	void			AddAnimation							(R_Animation* rAnimation);
	
	bool			AddClip									(const AnimatorClip& clip);
	bool			EditClip								(const char* originalClipName, const AnimatorClip& editedClip);
	bool			DeleteClip								(const char* clipName);
	
	bool			AddTrack								(const AnimatorTrack& newTrack);
	bool			EditTrack								(const char* originalTrackName, const AnimatorTrack& editedTrack);
	bool			DeleteTrack								(const char* trackName);

	AnimatorClip	GetClip									(const char* clipName) const;									// Rets invalid clip if not found. Check with clip->ClipIsValid().
	AnimatorClip*	GetClipAsPtr							(const char* clipName);											// Returns nullptr if clip is not found.

	AnimatorTrack	GetTrack								(const char* trackName) const;
	AnimatorTrack*	GetTrackAsPtr							(const char* trackName);

	void			PlayClip								(const char* clipName, uint blendFrames);
	void			PlayClip								(const char* clipName, float blendTime);
	
	bool			Play									();
	bool			Pause									();
	bool			Step									();
	bool			Stop									();

public:																														// --- DEBUG METHODS
	bool			StepToPrevKeyframe						();
	bool			StepToNextKeyframe						();
	bool			RefreshBoneDisplay						();

public:																														// --- CURRENT/BLENDING ANIMATION METHODS
	AnimatorClip*	GetCurrentClip							() const;														// /* TRACK */ 
	AnimatorClip*	GetBlendingClip							() const;														// /* TRACK */

	void			SetCurrentClip							(AnimatorClip* clip);											// /* TRACK */
	void			SetBlendingClip							(AnimatorClip* clip, uint blendFrames);							// /* TRACK */

	void			SetCurrentClipByIndex					(uint index);													// 
	void			SetBlendingClipByIndex					(uint index, uint blendFrames);									// 

	bool			CurrentClipExists						() const;														// /* TRACK */
	bool			BlendingClipExists						() const;														// /* TRACK */

	void			ClearCurrentClip						();																// /* TRACK */
	void			ClearBlendingClip						();																// /* TRACK */

public:																														// --- GET/SET METHODS
	std::vector<LineSegment> GetDisplayBones				() const;
	std::vector<std::string> GetClipNamesAsVector			() const;

	std::string		GetAnimatorStateAsString				() const;

	std::string		GetClipNamesAsString					() const;
	std::string		GetAnimationNamesAsString				() const;
	R_Animation*	GetAnimationByIndex						(uint index) const;
	int				GetIndexByAnimation						(const R_Animation* rAnimation) const;

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
	bool			StepClips								();																									/* TRACK */
	bool			ValidateCurrentClip						();																									/* TRACK */
					
	void			SwitchBlendingToCurrent					();																									/* TRACK */
	void			ResetBones								();																									/* TRACK */

private:																													// --- BONE/CHANNEL UPDATE METHODS
	void			UpdateChannelTransforms					();																									/* TRACK */
	void			UpdateMeshSkinning						();
	void			UpdateDisplayBones						();
	void			GenerateBoneSegments					(const GameObject* bone);
	
	Transform		GetInterpolatedTransform				(double keyframe, const Channel& channel, C_Transform* originalTransform) const;					/* TRACK */
	const float3	GetInterpolatedPosition					(double keyframe, const Channel& channel) const;													/* TRACK */
	const Quat		GetInterpolatedRotation					(double keyframe, const Channel& channel) const;													/* TRACK */
	const float3	GetInterpolatedScale					(double keyframe, const Channel& channel) const;													/* TRACK */

	Transform		GetPoseToPoseTransform					(uint tick, const Channel& channel, C_Transform* originalTransform) const;							/* TRACK */

	Transform		GetBlendedTransform						(double bKeyframe, const Channel& bChannel, const Transform& originalTransform) const;				/* TRACK */
	const float3	GetBlendedPosition						(double bKeyframe, const Channel& bChannel, float bRate, const float3& originalPosition) const;		/* TRACK */
	const Quat		GetBlendedRotation						(double bKeyframe, const Channel& bChannel, float bRate, const Quat& originalRotation) const;		/* TRACK */
	const float3	GetBlendedScale							(double bKeyframe, const Channel& bChannel, float bRate, const float3& originalScale) const;		/* TRACK */

private:
	std::vector<R_Animation*>						animations;											// Animation Resources. Contain bone information (transforms...).
	std::map<uint32, std::vector<BoneLink>>			animationBones;										// Stores the links between the animation bones and the GOs for each animation.
	
	std::vector<C_Mesh*>							animatedMeshes;										// TMP. Until a better implementation is found;

	std::vector<GameObject*>						bones;												//
	std::vector<BoneLink>*							currentBones;		/* TRACK */								// Multiple animations will have the same bones.
	std::vector<BoneLink>*							blendingBones;		/* TRACK */								//
	std::vector<LineSegment>						displayBones;										// Line Segments between GO bones. For debug purposes.

	std::map<std::string, AnimatorTrack>			tracks;												// Allows to overlap multiple clips with diff. root bones. Ex: "Torso", "Legs", etc.
	std::map<std::string, AnimatorClip>				clips;												// Segments of animations. "Idle", "Walk", "Attack", etc.

	AnimatorClip*				currentClip;		/* TRACK */
	AnimatorClip*				blendingClip;		/* TRACK */

	GameObject*					rootBone;

private:																								// --- FUNCTIONALITY VARIABLES
	AnimatorState	animatorState;

	bool init;

	uint			blendFrames;					/* TRACK */

private:																								// --- GET/SET VARIABLES	
	float			playbackSpeed;
	bool			interpolate;					/* TRACK */
	bool			loopAnimation;					/* DELETE */
	bool			playOnStart;					/* DELETE */
	bool			cameraCulling;
	
	bool			showBones;
};

#endif // !__C_ANIMATOR_H__