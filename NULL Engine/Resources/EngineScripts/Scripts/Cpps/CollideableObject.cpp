#include "CollideableObject.h"

#include "GameObject.h"
#include "C_Script.h"

CollideableObject* GetCollideableScript(GameObject* gameObject, ObjectType baseType)
{
    for (uint i = 0; i < gameObject->components.size(); ++i)
        if (gameObject->components[i]->GetType() == ComponentType::SCRIPT)
        {
            CollideableObject* cObj = (CollideableObject*)((C_Script*)gameObject->components[i])->GetScriptData();
            if (cObj->baseType == baseType)
                return cObj;
        }
    return nullptr;
}
