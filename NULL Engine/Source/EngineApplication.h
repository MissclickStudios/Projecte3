#ifndef __ENGINEAPPLICATION_H__
#define __ENGINEAPPLICATION_H__

#include "Application.h"

class M_Editor;
class M_Recast;

class EngineApplication : public /*EngineNamespace::*/Application {

public:

	EngineApplication();
	~EngineApplication();

	//void SaveForBuild(const Broken::json& reference, const char* path) const;
	//void GetDefaultGameConfig(Broken::json& config) const;

public:
	class M_Editor* editor;
	class M_Recast* recast;
};

extern EngineApplication* EngineApp;

#endif