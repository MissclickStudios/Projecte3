#ifndef __C_LIGHT_H__
#define __C_LIGHT_H__

#include "Component.h"

class ParsonNode;
class GameObject;

class C_Light : public Component
{
public:
	C_Light(GameObject* owner);
	~C_Light();

	bool Update		() override;
	bool CleanUp	() override;

	bool SaveState	(ParsonNode& root) const override;
	bool LoadState	(ParsonNode& root) override;

	static inline COMPONENT_TYPE GetType() { return COMPONENT_TYPE::LIGHT; }			// This is needed to be able to use templeates for functions such as GetComponent<>();

private:

};

#endif // !__C_LIGHT_H__