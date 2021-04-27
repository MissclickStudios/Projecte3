#ifndef __PERFECT_TIMER_H__
#define __PERFECT_TIMER_H__

#include "Macros.h"

typedef unsigned __int64 uint64;

class MISSCLICK_API PerfectTimer
{
public:
	PerfectTimer();							// Constructor


public:
	void	Start		();
	void	Stop		();

	double	ReadMs		() const;
	uint64	ReadTicks	() const;

private:
	bool			running;				// Will keep track of whether or not the timer is still active.
	uint64			startedAt;				// Will keep track of the exact moment at which the timer started running.
	uint64			stoppedAt;				// Will keep track of the exact moment at which the timer stopped running.
	static uint64	frequency;				// As the frequency will be the same across all timers, it will be declared as static.
};

#endif // !__PERFECT_TIMER_H__