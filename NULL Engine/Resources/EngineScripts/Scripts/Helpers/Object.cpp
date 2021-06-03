#include "Object.h"

#include "GameObject.h"
#include "C_Script.h"

Object* GetObjectScript(GameObject* gameObject, ObjectType baseType)
{
    for (uint i = 0; i < gameObject->components.size(); ++i)
        if (gameObject->components[i]->GetType() == ComponentType::SCRIPT)
        {
            Object* cObj = (Object*)((C_Script*)gameObject->components[i])->GetScriptData();
            if (cObj == nullptr)
                return nullptr;
            if (cObj->baseType == baseType || (baseType == ObjectType::NONE && cObj->baseType != ObjectType::NONE))
                return cObj;
        }
    return nullptr;
}

void Object::Activate()
{
    for (uint i = 0; i < gameObject->components.size(); ++i)
        gameObject->components[i]->SetIsActive(true);
    gameObject->SetIsActive(true);
}

void Object::Deactivate()
{
    for (uint i = 0; i < gameObject->components.size(); ++i)
        gameObject->components[i]->SetIsActive(false);
    gameObject->SetIsActive(false);
}
