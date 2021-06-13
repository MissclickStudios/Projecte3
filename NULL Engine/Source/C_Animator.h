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
class C_Mesh;

struct Channel;

typedef unsigned int		uint;
typedef unsigned __int32	uint32;

enum class InterpolationType																										// WIP IDEA
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

class MISSCLICK_API C_Animator : public Component																					// In charge of managing Skeletal Animation
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

public:																																// --- C_ANIMATOR MANAGEMENT METHODS
	bool			Play(bool applyToTracks = false);
	bool			Pause(bool applyToTracks = true);
	bool			Step(bool applyToTracks = true);
	bool			Stop(bool applyToTracks = true);

	void			AddAnimation(R_Animation* rAnimation);
	
	bool			AddClip(const AnimatorClip& clip);
	bool			EditClip(const char* originalClipName, const AnimatorClip& editedClip);
	bool			DeleteClip(const char* clipName);
	
	bool			AddTrack(const AnimatorTrack& newTrack);
	bool			EditTrack(const char* originalTrackName, const AnimatorTrack& editedTrack);
	bool			DeleteTrack(const char* trackName);

	void			PlayClip(const char* trackName, const char* clipName, uint blendFrames);
	void			PlayClip(const char* trackName, const char* clipName, float blendTime);
	void			PlayClip(AnimatorTrack* track, AnimatorClip* clip, uint blendFrames);
	void			PlayClip(AnimatorTrack* track, AnimatorClip* clip, float blendTime);

	void			SetTrackWithClip(const char* trackName, const char* clipName);
	void			SetTrackWithClip(AnimatorTrack* track, AnimatorClip* clip);

public:																																// --- GET/SET METHODS
	GameObject*		GetRootBone() const;
	
	std::string		GetAnimatorStateAsString() const;
	std::vector<LineSegment> GetDisplayBones() const;
	
	AnimatorClip	GetClip(const char* clipName) const;																			// Rets invalid clip if not found. Check ClipIsValid().
	AnimatorClip*	GetClipAsPtr(const char* clipName);																				// Returns nullptr if clip is not found.

	AnimatorTrack	GetTrack(const char* trackName) const;
	AnimatorTrack*	GetTrackAsPtr(const char* trackName);

	std::vector<R_Animation*>*				GetAnimationsAsPtr();
	std::vector<GameObject*>*				GetBonesAsPtr();
	std::map<std::string, AnimatorClip>*	GetClipsAsPtr();
	std::map<std::string, AnimatorTrack>*	GetTracksAsPtr();

	float			GetPlaybackSpeed() const;
	bool			GetCameraCulling() const;
	bool			GetShowBones() const;

	void			SetPlaybackSpeed(float playbackSpeed);
	void			SetCameraCulling(bool setTo);
	void			SetShowBones(bool setTo);

private: 																															// --- C_ANIMATOR INITIALIZATION METHODS
	void			GetAnimatedMeshes();
	void			FindRootBone(GameObject* child);
	void			SetRootBone(GameObject* rootBone);

	void			FindBones();																									// Finds the root bone by it. the childs recursively.
	void			FindBoneLinks();
	void			CrossCheckBonesWithMeshBoneMapping();																			
	std::vector<BoneLink>*	GetAnimationBoneLinks(uint32 UID);

	void			GenerateDefaultClips();																	
	bool			GenerateDefaultClip(const R_Animation* rAnimation, AnimatorClip& defaultClip);			

	void			FindTracksRootBones();
	void			GeneratePreviewTrack();

private:																															// --- ANIMATION/CLIP REPRODUCTION METHODS
	void			CheckGameState();
	bool			StepAnimation();

	void			UpdateMeshSkinning();
	void			UpdateDisplayBones();
	void			GenerateBoneSegments(const GameObject* bone);

private:																								// --- FUNCTIONALITY VARIABLES
	AnimatorState	animatorState;
	bool			init;

private:																								// --- GET/SET VARIABLES	
	float			playbackSpeed;
	bool			cameraCulling;
	bool			showBones;

private:																								// --- ANIMATION/TRACK/CLIP STRUCTURES
	std::vector<R_Animation*>				animations;													// Animation Resources. Contain bone information (transforms...).
	std::map<uint32, std::vector<BoneLink>>	animationBones;												// Stores the links between the animation bones and the GOs for each animation.
	
	std::vector<C_Mesh*>					animatedMeshes;												// TMP. Until a better implementation is found;

	std::vector<GameObject*>				bones;														//
	std::vector<LineSegment>				displayBones;												// Line Segments between GO bones. For debug purposes.

	std::map<std::string, AnimatorTrack>	tracks;														// Allows to overlap multiple clips with diff. root bones. Ex: "Torso", "Legs", etc.
	std::map<std::string, AnimatorClip>		clips;														// Segments of animations. "Idle", "Walk", "Attack", etc.

	GameObject*								rootBone;
};

#endif // !__C_ANIMATOR_H__