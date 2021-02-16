// ----------------------------------------------------
// Timer.cpp --- CPU Tick Timer class.
// Fast timer with millisecond precision.
// ----------------------------------------------------

#include "SDL/include/SDL_timer.h"

#include "VariableTypedefs.h"

#include "Timer.h"

// ---------------------------------------------
Timer::Timer() : running(false), started_at(0), stopped_at(0)
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
	started_at = SDL_GetTicks();												// Registers the starting time in milliseconds.
}

// ---------------------------------------------
void Timer::Stop()
{
	running = false;															// Stops the timer.
	stopped_at = SDL_GetTicks();												// Registers the stopping time in milliseconds.
}

// ---------------------------------------------
Uint32 Timer::Read() const
{
	if (running)
	{
		return SDL_GetTicks() - started_at;										// Returns the time that has elapsed since the start in milliseconds.
	}
	else
	{
		return stopped_at - started_at;											// Returns the time that has elapsed since the stop in milliseconds.
	}
}

// ---------------------------------------------
float Timer::ReadSec() const
{
	if (running)
	{
		return (float)(SDL_GetTicks() - started_at) / 1000.0f;					// Returns the time that has elapsed since the start in seconds.
	}
	else
	{
		return (float)(stopped_at - started_at);								// Returns the time that has elapsed since the stop in seconds.
	}												
}

bool Timer::IsActive() const
{
	return running;
}