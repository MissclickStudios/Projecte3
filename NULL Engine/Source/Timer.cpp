// ----------------------------------------------------
// Timer.cpp --- CPU Tick Timer class.
// Fast timer with millisecond precision.
// ----------------------------------------------------

#include "SDL/include/SDL_timer.h"

#include "VariableTypedefs.h"

#include "Timer.h"

#include "MemoryManager.h"

// ---------------------------------------------
Timer::Timer() : running(false), startedAt(0), stoppedAt(0)
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
	startedAt = SDL_GetTicks();												// Registers the starting time in milliseconds.
}

// ---------------------------------------------
void Timer::Stop()
{
	running = false;															// Stops the timer.
	stoppedAt = SDL_GetTicks();												// Registers the stopping time in milliseconds.
}

// ---------------------------------------------
Uint32 Timer::Read() const
{
	if (running)
	{
		return SDL_GetTicks() - startedAt;										// Returns the time that has elapsed since the start in milliseconds.
	}
	else
	{
		return stoppedAt - startedAt;											// Returns the time that has elapsed since the stop in milliseconds.
	}
}

// ---------------------------------------------
float Timer::ReadSec() const
{
	if (running)
	{
		return (float)(SDL_GetTicks() - startedAt) / 1000.0f;					// Returns the time that has elapsed since the start in seconds.
	}
	else
	{
		return (float)(stoppedAt - startedAt);								// Returns the time that has elapsed since the stop in seconds.
	}												
}

bool Timer::IsActive() const
{
	return running;
}