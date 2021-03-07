#include <map>

#include "Profiler.h"

#include "VariableTypedefs.h"

#include "Color.h"

#include "EngineApplication.h"
#include "M_Editor.h"

#include "Resource.h"

#include "E_Resources.h"

E_Resources::E_Resources() : EditorPanel("Resources")
{

}

E_Resources::~E_Resources()
{

}

bool E_Resources::Draw(ImGuiIO& io)
{
	BROFILER_CATEGORY("Editor Resources Draw", Profiler::Color::DarkSlateBlue);

	ImGui::Begin("References");
	
	uint models		= 0;
	uint meshes		= 0;
	uint materials	= 0;
	uint textures	= 0;
	uint animations = 0;
	uint shaders	= 0;

	std::multimap<uint, Resource*> sorted;

	const std::map<uint32, Resource*>* resources = EngineApp->editor->GetResourcesThroughEditor();

	std::map<uint32, Resource*>::const_iterator item;
	for (item = resources->cbegin(); item != resources->cend(); ++item)
	{
		if (item->second == nullptr)
			continue;

		switch (item->second->GetType())
		{
		case ResourceType::MODEL:		{ ++models; }		break;
		case ResourceType::MESH:		{ ++meshes; }		break;
		case ResourceType::MATERIAL:	{ ++materials; }	break;
		case ResourceType::TEXTURE:		{ ++textures; }		break;
		case ResourceType::ANIMATION:	{ ++animations; }	break;
		case ResourceType::SHADER:		{ ++shaders; }		break;
		}

		sorted.emplace((uint)item->second->GetType(), item->second);
	}

	std::multimap<uint, Resource*>::iterator rItem;
	for (rItem = sorted.begin(); rItem != sorted.end(); ++rItem)
	{
		ImGui::TextColored(&Cyan, "%s", rItem->second->GetAssetsFile());

		ImGui::Text("UID:");		ImGui::SameLine();	ImGui::TextColored(&Yellow, "       %lu",	rItem->second->GetUID());
		ImGui::Text("Type:");		ImGui::SameLine();	ImGui::TextColored(&Yellow, "      %s",		rItem->second->GetTypeAsString());
		ImGui::Text("References:");	ImGui::SameLine();	ImGui::TextColored(&Yellow, "%u",			rItem->second->GetReferences());

		ImGui::Separator();
	}

	sorted.clear();

	ImGui::Separator();

	ImGui::Text("Num Models:");		ImGui::SameLine();	ImGui::TextColored(&Yellow, "    %u",	models);
	ImGui::Text("Num Meshes:");		ImGui::SameLine();	ImGui::TextColored(&Yellow, "    %u",	meshes);
	ImGui::Text("Num Materials:");	ImGui::SameLine();	ImGui::TextColored(&Yellow, " %u",		materials);
	ImGui::Text("Num Textures:");	ImGui::SameLine();	ImGui::TextColored(&Yellow, "  %u",		textures);
	ImGui::Text("Num Animations:");	ImGui::SameLine();	ImGui::TextColored(&Yellow, "%u",		animations);
	ImGui::Text("Num Shaders:");	ImGui::SameLine();	ImGui::TextColored(&Yellow, "   %u",	shaders);
	
	ImGui::End();
	
	return true;
}

bool E_Resources::CleanUp()
{
	bool ret = true;



	return ret;
}