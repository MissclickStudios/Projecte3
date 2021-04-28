#ifndef __ANIMATOR_CLIP_H__
#define __ANIMATOR_CLIP_H__

#include <string>
#include "Macros.h"

class ParsonNode;
class R_Animation;

typedef unsigned int uint;

class MISSCLICK_API AnimatorClip
{
public:
	AnimatorClip();
	AnimatorClip(const R_Animation* animation, const std::string& name, uint start, uint end, float speed, bool loop);

	bool StepClip	(const float& dt);
	void ClearClip	();

	bool SaveState	(ParsonNode& root) const;
	bool LoadState	(const ParsonNode& root);

public:																																// --- CLIP UTILITY/DEBUG METHODS
	void				EditClip					(const R_Animation* newAnimation, const std::string& newName, uint newStart, uint newEnd, float newSpeed, bool newLoop);
	bool				ClipIsValid					() const;
	
	void				StepClipToPrevKeyframe		();
	void				StepClipToNextKeyframe		();

public:																																// --- CLIP TIME DATA GET/SET METHODS
	void				LoopClip					(bool setTo);
	bool				IsLooped					() const;

	float				GetClipTime					() const;
	float				GetClipFrame				() const;
	uint				GetClipTick					() const;

	float				GetAnimationFrame			() const;
	uint				GetAnimationTick			() const;

public:																																// --- GENERAL CLIP DATA GET METHODS
	const R_Animation*	GetAnimation				() const;
	const char*			GetAnimationName			() const;
	float				GetAnimationTicksPerSecond	() const;
	float				GetAnimationDuration		() const;

	const char*			GetName						() const;
	uint				GetStart					() const;
	uint				GetEnd						() const;
	float				GetSpeed					() const;
	float				GetDuration					() const;
	float				GetDurationInSeconds		() const;

	void				SetAnimation				(const R_Animation* rAnimation);
	void				SetSpeed					(float newSpeed);

public:																																// --- TMP VARIABLES THAT MIGHT CHANGE LATER ON
	bool				inNewTick;																									// Tells when a new keyframe/tick/frame has been reached.
	bool				playing;

private:																															// --- CLIP VARIABLES
	const R_Animation*	animation;
	std::string			name;
	uint				start;
	uint				end;
	float				speed;
	float				duration;
	float				durationInSeconds;
	

	float				time;
	float				frame;
	uint				tick;

	bool				loop;
	bool				interruptible;
};

#endif // !__ANIMATOR_CLIP_H__