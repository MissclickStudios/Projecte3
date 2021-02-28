#include <map>

#include "VariableTypedefs.h"

#include "Application.h"
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
	bool ret = true;

	ImGui::Begin("References");
	
	uint models		= 0;
	uint meshes		= 0;
	uint materials	= 0;
	uint textures	= 0;
	uint animations = 0;

	std::multimap<uint, Resource*> sorted;

	std::map<uint32, Resource*> resources;
	App->editor->GetResourcesThroughEditor(resources);

	std::map<uint32, Resource*>::iterator item;
	for (item = resources.begin(); item != resources.end(); ++item)
	{
		if (item->second == nullptr)
		{
			continue;
		}

		switch (item->second->GetType())
		{
		case ResourceType::MODEL:		{ ++models; }		break;
		case ResourceType::MESH:		{ ++meshes; }		break;
		case ResourceType::MATERIAL:	{ ++materials; }	break;
		case ResourceType::TEXTURE:		{ ++textures; }		break;
		case ResourceType::ANIMATION:	{ ++animations; }	break;
		}

		sorted.emplace((uint)item->second->GetType(), item->second);
	}

	std::multimap<uint, Resource*>::iterator multiItem;
	for (multiItem = sorted.begin(); multiItem != sorted.end(); ++multiItem)
	{
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "%s", multiItem->second->GetAssetsFile());

		ImGui::Text("UID:");		ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "       %lu",	multiItem->second->GetUID());
		ImGui::Text("Type:");		ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "      %s",		multiItem->second->GetTypeAsString());
		ImGui::Text("References:");	ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%u",			multiItem->second->GetReferences());

		ImGui::Separator();
	}

	sorted.clear();

	ImGui::Text("Num Models:");		ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "    %u",	models);
	ImGui::Text("Num Meshes:");		ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "    %u",	meshes);
	ImGui::Text("Num Materials:");	ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), " %u",		materials);
	ImGui::Text("Num Textures:");	ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "  %u",		textures);
	ImGui::Text("Num Animations:");	ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%u",		animations);

	ImGui::End();

	return ret;
}

bool E_Resources::CleanUp()
{
	bool ret = true;



	return ret;
}