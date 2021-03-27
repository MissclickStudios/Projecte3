#ifndef __R_FOLDER_H__
#define __R_FOLDER_H__

#include "Resource.h"

class ParsonNode;

typedef unsigned __int32 uint32;

class NULL_API R_Folder : public Resource
{
public:
	R_Folder();
	~R_Folder();

	bool CleanUp() override;

	bool SaveMeta(ParsonNode& metaRoot) const override;
	bool LoadMeta(const ParsonNode& metaRoot) override;

public:
	std::vector<uint32> GetContainedResources() const;

	bool IsContained(uint32 UID) const;

public:
	std::vector<uint32> containedResources;

private:
	// NO IMPORT SETTINGS
};

#endif // !_R_FOLDER_H__