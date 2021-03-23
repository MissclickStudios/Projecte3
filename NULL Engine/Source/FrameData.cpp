#include "Log.h"

#include "FrameData.h"
#include "MemoryManager.h"


FrameData::FrameData()
{
	timeSinceStart		= 0;
	frameCount				= 0;
	framesThisSecond		= 0;
	framesLastSecond		= 0;

	avgFps					= 0.0f;
	msLastFrame			= 0;

	dt						= 0.0f;

	framesThisSecond		= 0;
	millisecondsCounter	= 0;
}

void FrameData::Update(uint ms)
{
	++frameCount;																		// --- DATA SINCE STARTUP
	timeSinceStart		+= ms;															// 
	avgFps				= (float)frameCount / (timeSinceStart / 1000.0f);				// ----------------------

	++framesThisSecond;																	// --- FRAMES LAST SECOND
	millisecondsCounter += ms;															// 
	if (millisecondsCounter > 1000)														// 
	{																					// 
		framesLastSecond	= framesThisSecond;											// 
		framesThisSecond	= 0;														// 
		millisecondsCounter	= 0;														// 
	}																					// ----------------------

	msLastFrame			= ms;															// --- DELTA TIME DATA
	dt					= (float)ms / 1000.0f;											// -------------------
}

void FrameData::ResetData()
{
	timeSinceStart			= 0;
	frameCount				= 0;
	framesThisSecond		= 0;
	framesLastSecond		= 0;

	avgFps					= 0.0f;
	msLastFrame				= 0;

	dt						= 0.0f;

	framesThisSecond		= 0;
	millisecondsCounter		= 0;
}