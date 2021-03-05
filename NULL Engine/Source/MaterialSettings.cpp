#include "JSONParser.h"

#include "MaterialSettings.h"

#include "MemoryManager.h"

MaterialSettings::MaterialSettings() : ImportSettings()
{

}

MaterialSettings::~MaterialSettings()
{

}

bool MaterialSettings::Save(ParsonNode& settings) const
{
	bool ret = true;


	
	return ret;
}

bool MaterialSettings::Load(const ParsonNode& settings)
{
	bool ret = true;



	return ret;
}