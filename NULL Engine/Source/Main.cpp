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

Application* App = nullptr;

int main(int argc, char ** argv)
{
	LOG("Starting game '%s'...", TITLE);

	int main_return		= EXIT_FAILURE;
	MAIN_STATUS state	= MAIN_STATUS::CREATION;

	while (state != MAIN_STATUS::EXIT)
	{
		BROFILER_FRAME("NULL Engine Profiling Test");

		switch (state)
		{
		case MAIN_STATUS::CREATION:

			LOG("-------------- Application Creation --------------");
			App		= new Application();
			state	= MAIN_STATUS::START;
			break;

		case MAIN_STATUS::START:

			LOG("-------------- Application Init --------------");
			if (!App->Init())
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
			UPDATE_STATUS update_return = App->Update();							// THIS HERE

			if (update_return == UPDATE_STATUS::THROW_ERROR)
			{
				LOG("Application Update exits with ERROR");
				state = MAIN_STATUS::EXIT;
			}

			if (update_return == UPDATE_STATUS::STOP)
			{
				state = MAIN_STATUS::FINISH;
			}
		}
			break;

		case MAIN_STATUS::FINISH:

			LOG("-------------- Application CleanUp --------------");
			if (!App->CleanUp())
			{
				LOG("Application CleanUp exits with ERROR");
			}
			else
			{
				main_return = EXIT_SUCCESS;
			}

			state = MAIN_STATUS::EXIT;

			break;

		}
	}

	RELEASE(App);

	LOG("Exiting game '%s'...\n", TITLE);

	return main_return;
}