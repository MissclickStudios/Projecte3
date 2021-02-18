#include "JSONParser.h"

#include "R_Scene.h"

R_Scene::R_Scene() : Resource(ResourceType::SCENE)
{

}

R_Scene::~R_Scene()
{

}

bool R_Scene::CleanUp()
{
	bool ret = true;



	return ret;
}

bool R_Scene::SaveMeta(ParsonNode& metaRoot) const
{
	bool ret = true;

	ParsonArray contained_array = metaRoot.SetArray("ContainedResources");

	return ret;
}

bool R_Scene::LoadMeta(const ParsonNode& metaRoot)
{
	bool ret = true;



	return ret;
}