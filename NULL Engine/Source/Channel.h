#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include <map>

#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/Quat.h"

#include "Macros.h"

typedef std::map<double, float3>::const_iterator	PositionKeyframe;												// Typedefs declared in order to improve code readability,
typedef std::map<double, Quat>::const_iterator		RotationKeyframe;												// PositionKeyframe and ScaleKeyframe are technically the same.
typedef std::map<double, float3>::const_iterator	ScaleKeyframe;													// However, they have been separated for differentiation.

enum class KeyframeType
{
	POSITION,
	ROTATION,
	SCALE
};

struct Channel																										// Channels are the "Bones" of the animation. For distinction between this
{																													// "Bones" and the Mesh's bones they have been re-named to Channels.
	Channel();
	Channel(const char* name);

	bool				HasKeyframes					(KeyframeType type) const;
	bool				HasPositionKeyframes			() const;													// --- There will be no keyframes when:
	bool				HasRotationKeyframes			() const;													// Size == 1	(Initial Position. Always needed regardless)
	bool				HasScaleKeyframes				() const;													// Time == -1	(Time cannot be negative, -1 used as "non-valid" ID)

	PositionKeyframe	GetPositionKeyframe				(double keyframe) const;
	RotationKeyframe	GetRotationKeyframe				(double keyframe) const;
	ScaleKeyframe		GetScaleKeyframe				(double keyframe) const;	

	PositionKeyframe	GetPrevPositionKeyframe			(double keyframe) const;									// --- Returns the Keyframe immediately before the given.
	RotationKeyframe	GetPrevRotationKeyframe			(double keyframe) const;									// Ex: current_keyframe = 5 --> return keyframe 4;
	ScaleKeyframe		GetPrevScaleKeyframe			(double keyframe) const;									// ------------------------------------------------------	

	PositionKeyframe	GetNextPositionKeyframe			(double keyframe) const;									// --- Returns the Keyframe immediately after the given.
	RotationKeyframe	GetNextRotationKeyframe			(double keyframe) const;									// Ex: current_keyframe = 5 --> return keyframe 6;
	ScaleKeyframe		GetNextScaleKeyframe			(double keyframe) const;									// -----------------------------------------------------

	PositionKeyframe	GetClosestPrevPositionKeyframe	(double keyframe) const;									// --- Returns the Keyframe immediately closest before the given.
	RotationKeyframe	GetClosestPrevRotationKeyframe	(double keyframe) const;									// Ex: current_keyframe = 5.5f --> return keyframe 5;
	ScaleKeyframe		GetClosestPrevScaleKeyframe		(double keyframe) const;									// --------------------------------------------------------------

	PositionKeyframe	GetClosestNextPositionKeyframe	(double keyframe) const;									// --- Returns the Keyframe immediately closest after the given.
	RotationKeyframe	GetClosestNextRotationKeyframe	(double keyframe) const;									// Ex: current_keyframe = 5.5f --> return keyframe 6;
	ScaleKeyframe		GetClosestNextScaleKeyframe		(double keyframe) const;									// -------------------------------------------------------------

	std::map<double, float3>	positionKeyframes;																	// Position-related keyframes.
	std::map<double, Quat>		rotationKeyframes;																	// Rotation-related keyframes.
	std::map<double, float3>	scaleKeyframes;																		// Scale-related keyframes.

	std::string					name;																				// Name of the Channel/Bone
};

#endif // !__CHANNEL_H__