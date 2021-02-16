#ifndef __KEYFRAME_H__
#define __KEYFRAME_H__

template <typename T>
struct Keyframe
{
	Keyframe() : time(-1.0f)									{}
	Keyframe(double time, T value) : time(time), value(value)	{}

	double time;
	T value;
};

#endif // !__KEYFRAME_H__
