#include <stdlib.h>
																
#include "Application.h"

#include "SDL/include/SDL.h"
#include "Profiler.h"
#include "MemoryManager.h"

#pragma comment( lib, "Source/Dependencies/SDL/libx86/SDL2.lib" )
#pragma comment( lib, "Source/Dependencies/SDL/libx86/SDL2main.lib" )
#pragma comment( lib, "Source/Dependencies/Brofiler/libx86/ProfilerCore32.lib" )

enum class MainStatus
{
	CREATION,
	AWAKE,
	START,
	UPDATE,
	FINISH,
	EXIT
};

extern /*Enginenamespace::*/Application* /*EngineNamespace::*/CreateApplication();

/*Enginenamespace::*/Application* mainApp = nullptr;

int main(int argc, char ** argv)
{
	LOG("Starting game '%s'...", TITLE);

	int mainReturn		= EXIT_FAILURE;
	MainStatus state	= MainStatus::CREATION;

	while (state != MainStatus::EXIT)
	{
		BROFILER_FRAME("NULL Engine Profiling Test");

		switch (state)
		{
		case MainStatus::CREATION:

			LOG("-------------- Application Creation --------------");
#ifndef NULL_BUILD_DLL
			mainApp	= CreateApplication();
#endif
			state	= MainStatus::START;
			break;

		case MainStatus::START:

			LOG("-------------- Application Init --------------");
			if (!mainApp->Init())
			{
				LOG("Application Init exits with ERROR");
				state = MainStatus::EXIT;
			}
			else
			{
				state = MainStatus::UPDATE;
				LOG("-------------- Application Update --------------");
			}

			break;

		case MainStatus::UPDATE:
		{
			UpdateStatus updateReturn = App->Update();							//Why App->Update???

			if (updateReturn == UpdateStatus::THROW_ERROR)
			{
				LOG("Application Update exits with ERROR");
				state = MainStatus::EXIT;
			}

			if (updateReturn == UpdateStatus::STOP)
			{
				state = MainStatus::FINISH;
			}
		}
			break;

		case MainStatus::FINISH:

			LOG("-------------- Application CleanUp --------------");
			if (!mainApp->CleanUp())
			{
				LOG("Application CleanUp exits with ERROR");
			}
			else
			{
				mainReturn = EXIT_SUCCESS;
			}

			state = MainStatus::EXIT;

			break;

		}
	}

	RELEASE(mainApp);

	LOG("Exiting game '%s'...\n", TITLE);

	return mainReturn;
}