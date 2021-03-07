#ifndef __M_UISYSTEM_H__
#define __M_UISYSTEM_H__

#include "Module.h"

#include "Dependencies/FreeType/include/ft2build.h"
#include "Dependencies/FreeType/include/freetype/freetype.h"

class ParsonNode;
typedef unsigned int uint;

class GameObject;

class NULL_API M_UISystem : public Module
{
public:

	M_UISystem(bool isActive = true);
	~M_UISystem();

	bool Init(ParsonNode& config) override;
	UpdateStatus PreUpdate(float dt) override;
	UpdateStatus Update(float dt) override;
	UpdateStatus PostUpdate(float dt) override;
	bool CleanUp() override;
		 
	bool LoadConfiguration(ParsonNode& root) override;
	bool SaveConfiguration(ParsonNode& root) const override;

public:
	//temp
	FT_Error error;
	FT_Library libraryFT;
	FT_Face face;


	bool IsCameraSwap() const;

	void SetIsCameraSwap();

private:

	bool isCameraSwap;
	
};

#endif // !__M_UISYSTEM_H__