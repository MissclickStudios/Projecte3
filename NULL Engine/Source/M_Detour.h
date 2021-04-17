#ifndef __M_DETOUR_H__
#define __M_DETOUR_H__

#include "Module.h"

class M_Detour : public Module
{
public:
	M_Detour(bool isActive = true);
	~M_Detour();

	bool		Init(ParsonNode& root) override;
	bool		Start() override;
	UpdateStatus Update(float dt) override;
	bool		CleanUp() override;

private:

};
#endif // __M_DETOUR_H__