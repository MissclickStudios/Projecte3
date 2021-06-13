#ifndef __COMPONENT_H__
#define __COMPONENT_H__

#include "Macros.h"

class ParsonNode;
class GameObject;

typedef unsigned __int32 uint32;

enum class ComponentType
{
	NONE,
	TRANSFORM,												// --- BASICS
	MESH,													// 
	MATERIAL,												// 
	LIGHT,													// 
	CAMERA,													// ----------

	ANIMATOR,												// --- ANIMATION
	ANIMATION,												// -------------

	RIGIDBODY,												// --- PHYSICS
	BOX_COLLIDER,											//
	SPHERE_COLLIDER,										// 
	CAPSULE_COLLIDER,										// -----------

	PARTICLE_SYSTEM,										// --- PARTICLE SYSTEM

	CANVAS,													// --- UI

	AUDIOSOURCE,											// --- AUDIO
	AUDIOLISTENER,											// ---------

	PLAYER_CONTROLLER,										// --- BEHAVIOURS
	BULLET_BEHAVIOR,										// 
	PROP_BEHAVIOR,											// 
	CAMERA_BEHAVIOR,										// 
	GATE_BEHAVIOR,											// --------------

	UI_IMAGE,												// --- UI
	UI_TEXT,												// ------
	UI_BUTTON,

	SCRIPT,													// --- SCRIPTING

	ANIMATOR2D,												// --- ANIMATIONS2D

	NAVMESH_AGENT,											// --- NAVIGATION

	UI_CHECKBOX,
	UI_SLIDER
};

class MISSCLICK_API Component
{
public:
	Component(GameObject* owner, ComponentType type, bool isActive = true);
	virtual ~Component();

	virtual bool Start();
	virtual bool Update();
	virtual bool CleanUp();

	virtual bool SaveState(ParsonNode& root) const;
	virtual bool LoadState(ParsonNode& root);

	virtual inline ComponentType GetType() const { return type; }						// This is needed to be able to use templeates for functions such as GetComponent<>();

public:
	const char*		GetNameFromType() const;											// Will return a string with the name of the component. Depends on COMPONENT_TYPE.
	const char*		GetTypesAsString() const;
	
	uint32			GetID() const;														// Will return the component's ID.
	void			ResetID();															// Will reset the component's ID. WARNING: All references to this comp. will be lost (serialization).

	bool			IsActive() const;													// 
	virtual void	SetIsActive(bool setTo);											// 

	GameObject*		GetOwner() const;													//

protected:
	bool			isActive;

private:
	uint32			id;																	// 

	ComponentType	type;																//
	GameObject*		owner;																// 
};

#endif // !__COMPONENT_H__