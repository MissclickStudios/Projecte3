#include "JSONParser.h"

#include "R_Scene.h"

R_Scene::R_Scene() : Resource(RESOURCE_TYPE::SCENE)
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

bool R_Scene::SaveMeta(ParsonNode& meta_root) const
{
	bool ret = true;

	ParsonArray contained_array = meta_root.SetArray("ContainedResources");

	return ret;
}

bool R_Scene::LoadMeta(const ParsonNode& meta_root)
{
	bool ret = true;



	return ret;
}