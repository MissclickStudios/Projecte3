#include <stdlib.h>
																
#include "Application.h"
#include "ConfigurationSettings.h"
#include "Log.h"

#include "SDL/include/SDL.h"
#include "Profiler.h"

#pragma comment( lib, "Source/Dependencies/SDL/libx86/SDL2.lib" )
#pragma comment( lib, "Source/Dependencies/SDL/libx86/SDL2main.lib" )
#ifndef NOPROFILER
#pragma comment( lib, "Source/Dependencies/Optik/OptickCore.lib" ) //TODO: if build configuration not link this
#endif // !NOPROFILER



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
		OPTICK_FRAME("Begin Frame")

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
				LOG("-------------- Application Update --------------");
				/*bool success = App->Start();
				state = (success) ? MainStatus::UPDATE : MainStatus::EXIT;*/

				state = (App->Start()) ? MainStatus::UPDATE : MainStatus::EXIT;
				
				//state = MainStatus::UPDATE;
			}

			break;

		case MainStatus::UPDATE:
		{
			UpdateStatus updateReturn = App->Update();							//Why App->Update???  Because it needs to be called in order to have updates each frame???

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