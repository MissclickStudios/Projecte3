#ifndef __SCRIPT_H__
#define __SCRIPT_H__

//Import/export define
#ifdef NULL_BUILD_DLL
#define NULL_API __declspec(dllexport)
#else
#define NULL_API __declspec(dllimport)
#endif

#ifndef GAMEBUILD //TODO: When this gets included on the scripts project the macro never gets expanded
#include <string>
#endif // !GAMEBUILD

class GameObject;
class C_Transform;

class NULL_API Script {

public:
	Script() {}
	virtual ~Script() {}
	virtual void Awake() {}
	virtual void Start() {}

	virtual void PreUpdate() {}
	virtual void Update() {}
	virtual void PostUpdate() {}

	virtual void CleanUp() {}

	virtual void OnDisable() {}
	virtual void OnEnable() {}
	bool IsScriptEnabled() const;
	void SetScriptEnable(const bool& enable);

public:
	GameObject* gameObject = nullptr;
	C_Transform* transform = nullptr;
	bool* enabled = nullptr;
#ifndef GAMEBUILD
	//Don't use these variable on the script !!!!!
	const std::string* dataName;
#endif // !GAMEBUILD
};

#endif // !__SCRIPT_H__
