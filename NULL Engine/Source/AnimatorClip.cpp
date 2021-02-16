#include "JSONParser.h"

#include "Application.h"
#include "M_ResourceManager.h"

#include "R_Animation.h"

#include "AnimatorClip.h"

AnimatorClip::AnimatorClip() : 
animation			(nullptr), 
name				("[NONE]"), 
start				(0), 
end					(0), 
duration			(0.0f),
duration_in_seconds	(0.0f),
loop				(false),
time				(0.0f),
frame				(0.0f),
tick				(0),
in_new_tick			(false),
playing				(false)
{

}

AnimatorClip::AnimatorClip(const R_Animation* animation, const std::string& name, const uint& start, const uint& end, const bool& loop) : 
animation	(animation), 
name		(name), 
start		(start), 
end			(end),
loop		(loop),
time		(0.0f),
frame		(0.0f),
tick		(0),
in_new_tick	(false),
playing		(false)
{
	duration				= (float)(end - start);
	duration_in_seconds		= (animation != nullptr) ? (duration / animation->GetTicksPerSecond()) : 0.0f;
}

bool AnimatorClip::StepClip(const float& dt)
{
	if (animation == nullptr)
	{
		return false;
	}

	uint prev_tick = tick;

	time	+= dt;
	frame	= time * animation->GetTicksPerSecond();
	tick	= (uint)floor(frame);

	in_new_tick = (tick != prev_tick);

	if (time > duration_in_seconds)
	{
		ClearClip();
		return false;
	}

	return true;
}

void AnimatorClip::ClearClip()
{
	time	= 0.0f;
	frame	= 0.0f;
	tick	= 0;
}

bool AnimatorClip::SaveState(ParsonNode& root) const
{
	bool ret = true;
	
	root.SetNumber("AnimationUID", (double)animation->GetUID());

	root.SetString("Name", name.c_str());
	root.SetNumber("Start", (double)start);
	root.SetNumber("End", (double)end);
	root.SetNumber("Duration", (double)duration);
	root.SetNumber("DurationInSeconds", (double)duration_in_seconds);

	root.SetBool("Loop", loop);

	return ret;
}

bool AnimatorClip::LoadState(const ParsonNode& root)
{
	bool ret = true;
	
	animation				= (R_Animation*)App->resource_manager->RequestResource((uint32)root.GetNumber("AnimationUID"));

	name					= root.GetString("Name");
	start					= (uint)root.GetNumber("Start");
	end						= (uint)root.GetNumber("End");
	duration				= (float)root.GetNumber("Duration");
	duration_in_seconds		= (float)root.GetNumber("DurationInSeconds");

	loop					= root.GetBool("Loop");
	
	return ret;
}

// --- CLIP DEBUG METHODS
void AnimatorClip::StepClipToPrevKeyframe()
{
	tick = (tick != start) ? --tick : end;
}

void AnimatorClip::StepClipToNextKeyframe()
{
	tick = (tick != end) ? ++tick : start;
}

// --- CLIP TIME DATA GET/SET METHODS
void AnimatorClip::LoopClip(const bool& set_to)
{
	loop = set_to;
}

bool AnimatorClip::IsLooped() const
{
	return loop;
}

float AnimatorClip::GetClipTime() const
{
	return time;
}

float AnimatorClip::GetClipFrame() const
{
	return frame;
}

uint AnimatorClip::GetClipTick() const
{
	return tick;
}

float AnimatorClip::GetAnimationFrame() const
{
	return ((float)start + frame);
}

uint AnimatorClip::GetAnimationTick() const
{
	return (start + tick);
}

// --- GENERAL CLIP DATA GET METHODS
const R_Animation* AnimatorClip::GetAnimation() const
{
	return animation;
}

const char* AnimatorClip::GetAnimationName() const
{
	return ((animation != nullptr) ? animation->GetName() : "[NONE]");
}

float AnimatorClip::GetAnimationTicksPerSecond() const
{
	return ((animation != nullptr) ? animation->GetTicksPerSecond() : 0.0f);
}

float AnimatorClip::GetAnimationDuration() const
{
	return ((animation != nullptr) ? animation->GetDuration() : 0.0f);
}

const char* AnimatorClip::GetName() const
{
	return name.c_str();
}

uint AnimatorClip::GetStart() const
{
	return start;
}

uint AnimatorClip::GetEnd() const
{
	return end;
}

float AnimatorClip::GetDuration() const
{
	return duration;
}

float AnimatorClip::GetDurationInSeconds() const
{
	return duration_in_seconds;
}