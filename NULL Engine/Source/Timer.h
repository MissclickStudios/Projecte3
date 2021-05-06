#ifndef __TIMER_H__
#define __TIMER_H__

#include "Hourglass.h"
#include "Macros.h"

typedef unsigned __int32	Uint32;
typedef __int32	int32;

class MISSCLICK_API Timer
{
public:
	Timer();								// Constructor
	~Timer();

public:
	void	Start			();				// Initializes the timer.
	void	Stop			();				// Stops the timer.
	void	Pause			();
	void	Resume			();

	Uint32	Read			() const;		// Will return the registered time in milliseconds.
	float	ReadSec			() const;		// Will return the registered time in seconds.

	bool	IsActive		() const;
	bool	IsPaused		() const;
	void	AddTimeToClock	(int32 ticks);
	void	AddTimeToClock	(float seconds);

private:
	bool		running;						// Will keep track of whether or not the timer is still active.
	int32			startedAt;					// Will keep track of the exact moment at which the timer started running.
	int32			pausedAt;
};

#endif //__TIMER_H__