#include "Object.h"

#include "GameObject.h"
#include "C_Script.h"

Object* GetObjectScript(GameObject* gameObject, ObjectType baseType)
{
    for (uint i = 0; i < gameObject->components.size(); ++i)
        if (gameObject->components[i]->GetType() == ComponentType::SCRIPT)
        {
            Object* cObj = (Object*)((C_Script*)gameObject->components[i])->GetScriptData();
            if (cObj->baseType == baseType)
                return cObj;
        }
    return nullptr;
}
