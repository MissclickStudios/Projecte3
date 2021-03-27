#ifndef __ANIMATION_CLIP_H__
#define __ANIMATION_CLIP_H__

#include <string>

struct AnimationClip
{
	AnimationClip() : name("[NONE]"), duration(0.0f), start(0.0f), end(0.0f) {}
	AnimationClip(const std::string& name, const float& duration, const float& start, const float& end) : name(name), duration(duration), start(start), end(end) {}

	std::string name;
	float duration;
	float start;
	float end;
};

#endif // !__ANIMATION_CLIP_H__