#include "JSONParser.h"

#include "Module.h"

Module::Module(const char* name, bool is_active) : is_active(is_active)
{
	strcpy_s(this->name, MAX_MODULE_NAME_LENGTH, name);							// As one character occupies one byte, we can request exactly as many bytes we need.
}

Module::~Module()
{

}

bool Module::Init(ParsonNode& config)
{
	return true;
}

bool Module::Start()
{
	return true;
}

UPDATE_STATUS Module::PreUpdate(float dt)
{
	return UPDATE_STATUS::CONTINUE;
}

UPDATE_STATUS Module::Update(float dt)
{
	return UPDATE_STATUS::CONTINUE;
}

UPDATE_STATUS Module::PostUpdate(float dt)
{
	return UPDATE_STATUS::CONTINUE;
}

bool Module::CleanUp()
{
	return true;
}

bool Module::LoadConfiguration(ParsonNode& file)
{
	return true;
}

bool Module::SaveConfiguration(ParsonNode& file) const
{
	return true;
}

// ------------ MODULE METHODS ------------
bool Module::IsActive() const
{
	return is_active;
}

bool Module::SetModuleState(bool is_active)
{
	this->is_active = is_active;
	
	if (is_active)
	{
		Start();
	}
	else
	{
		CleanUp();
	}

	return this->is_active;
}

const char* Module::GetName() const
{
	return name;
}