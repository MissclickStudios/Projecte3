#include <stdlib.h>
																
#include "Application.h"																	// ATTENTION: Already included in Application.h.

#include "SDL/include/SDL.h"
#include "Profiler.h"
#include "MemoryManager.h"

#pragma comment( lib, "Source/Dependencies/SDL/libx86/SDL2.lib" )
#pragma comment( lib, "Source/Dependencies/SDL/libx86/SDL2main.lib" )
#pragma comment( lib, "Source/Dependencies/Brofiler/libx86/ProfilerCore32.lib" )

enum class MAIN_STATUS
{
	CREATION,
	AWAKE,
	START,
	UPDATE,
	FINISH,
	EXIT
};

Application* app = nullptr;

int main(int argc, char ** argv)
{
	LOG("Starting game '%s'...", TITLE);

	int mainReturn		= EXIT_FAILURE;
	MAIN_STATUS state	= MAIN_STATUS::CREATION;

	while (state != MAIN_STATUS::EXIT)
	{
		BROFILER_FRAME("NULL Engine Profiling Test");

		switch (state)
		{
		case MAIN_STATUS::CREATION:

			LOG("-------------- Application Creation --------------");
			app		= new Application();
			state	= MAIN_STATUS::START;
			break;

		case MAIN_STATUS::START:

			LOG("-------------- Application Init --------------");
			if (!app->Init())
			{
				LOG("Application Init exits with ERROR");
				state = MAIN_STATUS::EXIT;
			}
			else
			{
				state = MAIN_STATUS::UPDATE;
				LOG("-------------- Application Update --------------");
			}

			break;

		case MAIN_STATUS::UPDATE:
		{
			UPDATE_STATUS updateReturn = app->Update();							// THIS HERE

			if (updateReturn == UPDATE_STATUS::THROW_ERROR)
			{
				LOG("Application Update exits with ERROR");
				state = MAIN_STATUS::EXIT;
			}

			if (updateReturn == UPDATE_STATUS::STOP)
			{
				state = MAIN_STATUS::FINISH;
			}
		}
			break;

		case MAIN_STATUS::FINISH:

			LOG("-------------- Application CleanUp --------------");
			if (!app->CleanUp())
			{
				LOG("Application CleanUp exits with ERROR");
			}
			else
			{
				mainReturn = EXIT_SUCCESS;
			}

			state = MAIN_STATUS::EXIT;

			break;

		}
	}

	RELEASE(app);

	LOG("Exiting game '%s'...\n", TITLE);

	return mainReturn;
}