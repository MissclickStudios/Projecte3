// ----------------------------------------------------
// PerfectTimer.cpp --- CPU Tick Timer class.
// Slow timer with microsecond precision.
// ----------------------------------------------------

#include "SDL/include/SDL_timer.h"

#include "VariableTypedefs.h"

#include "PerfectTimer.h"

#include "MemoryManager.h"

uint64 PerfectTimer::frequency = 0;

// ---------------------------------------------
PerfectTimer::PerfectTimer() : running(false), startedAt(0), stoppedAt(0)
{
	if (frequency == 0)
	{
		frequency = SDL_GetPerformanceFrequency();								// Sets the frequency cycles in seconds (coming from microseconds).
	}

	Start();
}

// ---------------------------------------------
void PerfectTimer::Start()
{
	running		= true;																		// Starts the timer.
	startedAt	= SDL_GetPerformanceCounter();												// Registers the starting time in microseconds.
}

// ---------------------------------------------
void PerfectTimer::Stop()
{
	running		= false;																	// Stops the timer.
	stoppedAt	= SDL_GetPerformanceCounter();												// Registers the stopping time in microseconds.
}

// ---------------------------------------------
double PerfectTimer::ReadMs() const
{
	if (running)
	{
		return (SDL_GetPerformanceCounter() - startedAt) / (frequency / 1000.0f);			// Returns the time that has elapsed since the start in milliseconds.
	}
	else
	{
		return (stoppedAt - startedAt) / (frequency / 1000.0f);							// Returns the time that has elapsed since the stop in milliseconds.
	}
}

// ---------------------------------------------
uint64 PerfectTimer::ReadTicks() const
{
	if (running)
	{
		return (SDL_GetPerformanceCounter() - startedAt);									// Returns the time that has elapsed since the start in ticks (or microseconds).
	}
	else
	{
		return (SDL_GetPerformanceCounter() - stoppedAt);									// Returns the time that has elapsed since the stop in ticks (or microseconds).
	}
}