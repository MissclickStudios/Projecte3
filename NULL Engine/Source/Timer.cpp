// ----------------------------------------------------
// Timer.cpp --- CPU Tick Timer class.
// Fast timer with millisecond precision.
// ----------------------------------------------------

#include "SDL/include/SDL_timer.h"

#include "VariableTypedefs.h"

#include "Timer.h"

#include "MemoryManager.h"

// ---------------------------------------------
Timer::Timer() : running(false), startedAt(0u), pausedAt(0u)
{
	Start();
}

Timer::~Timer()
{

}

// ---------------------------------------------
void Timer::Start()
{
	running = true;																// Starts the timer.
	startedAt = SDL_GetTicks();													// Registers the starting time in milliseconds.
	pausedAt = 0;
}

// ---------------------------------------------
void Timer::Stop()
{
	running = false;															// Stops the timer.
	startedAt = 0;
	pausedAt = 0;
}

void Timer::Pause()
{
	if (!running)
		return;

	if (pausedAt == 0)
		pausedAt = SDL_GetTicks();
}

void Timer::Resume()
{
	if (!running)
		return;

	if (pausedAt != 0)
	{
		startedAt = SDL_GetTicks() - (pausedAt - startedAt);
		pausedAt = 0;
	}
}

// ---------------------------------------------
Uint32 Timer::Read() const
{
	if (running)
	{
		if (pausedAt == 0)
			return SDL_GetTicks() - startedAt;								// Returns the time that has elapsed since the start in milliseconds.
		else
			return pausedAt - startedAt;									// Returns the time that has elapsed untill the pause in milliseconds.
	}
	else
		return 0;															
}

// ---------------------------------------------
float Timer::ReadSec() const
{
	if (running)
	{
		if (pausedAt == 0)
		{
			return (float)(SDL_GetTicks() - startedAt) / 1000.0f;			// Returns the time that has elapsed since the start in seconds.
		}
		else
			return (float)(pausedAt - startedAt) / 1000.0f;					// Returns the time that has elapsed untill the pause in seconds.
	}
	else
		return 0;							
}

bool Timer::IsActive() const
{
	return running;
}

bool Timer::IsPaused() const
{
	if (!running || pausedAt == 0)
		return false;
	return true;
}

void Timer::AddTimeToClock(int32 ticks)
{
	if (!running)
		return;

	startedAt -= ticks;
}

void Timer::AddTimeToClock(float seconds)
{
	if (!running)
		return;

	startedAt -= (int)(seconds * 1000.0f);
}
