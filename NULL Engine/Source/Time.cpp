#include "SDL/include/SDL_timer.h"

#include "VariableTypedefs.h"
#include "Log.h"

#include "Time.h"

using namespace Time::Real::Utilities;																	// Not the cleanest but prefer to avoid having to write these
using namespace Time::Game::Utilities;																	// strings every time i need a Utilities variable.

void Time::Sleep(uint ms)
{
	SDL_Delay(ms);
}

// --- REAL CLOCK METHODS ---
void Time::Real::InitRealClock()
{
	//startup_timer.Start();
	frameTimer.Start();
}

void Time::Real::Update()
{
	uint ms = frameTimer.Read();

	clock.Update(ms);
	frameData.Update(ms);

	frameTimer.Start();
}

void Time::Real::DelayUntilFrameCap(uint frameCap)
{
	if (frameCap == 0)														// If frame_cap = 0, then no delay will be applied.
	{
		return;
	}
	
	uint32 currentFrameMs = Utilities::frameTimer.Read();
	uint32 frameCapMs		= 1000 / frameCap;

	if (currentFrameMs < frameCapMs)
	{
		//precise_delay_timer.Start();

		uint requiredDelay = frameCapMs - currentFrameMs;

		Sleep(requiredDelay);
	}
}

void Time::Real::StartPerfTimer()
{
	perfTimer.Start();
}

void Time::Real::StopPerfTimer()
{
	perfTimer.Stop();
}

float Time::Real::PeekPerfTimer()
{
	return perfTimer.ReadMs();
}

Hourglass Time::Real::GetClock()
{
	return clock;
}

FrameData Time::Real::GetFrameData()
{
	return frameData;
}

uint64 Time::Real::GetFrameCount()
{
	return frameData.frameCount;
}

float Time::Real::GetTimeSinceStart()
{
	return frameData.timeSinceStart;
}

uint32 Time::Real::GetFramesLastSecond()
{
	return frameData.framesLastSecond;
}

float Time::Real::GetAverageFPS()
{
	return frameData.avgFps;
}

uint32 Time::Real::GetMsLastFrame()
{
	return frameData.msLastFrame;
}

float Time::Real::GetDT()
{
	return frameData.dt;
}

// --- GAME CLOCK METHODS ---
void Time::Game::Update()
{
	uint ms = (uint)((float)gameFrameTimer.Read() * timeScale);

	gameClock.Update(ms);
	gameFrameData.Update(ms);

	gameFrameTimer.Start();
}

float Time::Game::GetTimeScale()
{
	return timeScale;
}

void Time::Game::SetTimeScale(float newTimeScale)
{
	if (newTimeScale < 0.25f || newTimeScale > 4.0f)
	{
		LOG("[ERROR] Time: Cannot set Game Time Scale below 0.25 or above 4.00!");
		return;
	}
	
	timeScale = newTimeScale;
}

void Time::Game::Play()
{
	gameFrameTimer.Start();
}

void Time::Game::Pause()
{
	gameFrameTimer.Stop();
}

void Time::Game::Step(uint numSteps)
{

}

void Time::Game::Stop()
{
	gameFrameTimer.Stop();
	gameClock.ResetClock();
	gameFrameData.ResetData();
}

Hourglass Time::Game::GetClock()
{
	return gameClock;
}

FrameData Time::Game::GetFrameData()
{
	return gameFrameData;
}

uint64 Time::Game::GetFrameCount()
{
	return gameFrameData.frameCount;
}

float Time::Game::GetTimeSinceStart()
{
	return gameFrameData.timeSinceStart;
}

uint32 Time::Game::GetFramesLastSecond()
{
	return gameFrameData.framesLastSecond;
}

float Time::Game::GetAverageFPS()
{
	return gameFrameData.avgFps;
}

uint32 Time::Game::GetMsLastFrame()
{
	return gameFrameData.msLastFrame;
}

float Time::Game::GetDT()
{
	return gameFrameData.dt;
}