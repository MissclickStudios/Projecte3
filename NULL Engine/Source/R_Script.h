#ifndef __ResourceScript_H__
#define __ResourceScript_H__

#include "Resource.h"

#include <utility>
#include <vector>
#include <string>

typedef unsigned __int64 uint64;

class NULL_API ResourceScript : public Resource {
public:
	ResourceScript();
	virtual ~ResourceScript();

	//bool SaveMeta(ParsonNode& metaRoot) const override;		//Per guardar les dependencies a altres resources
	//bool LoadMeta(const ParsonNode& metaRoot) override;		//De Moment Np utilitzat

	/*bool CreateMetaData(const unsigned int& force_id = 0);
	bool ReadBaseInfo(const char* assets_file_path);
	void ReadLibrary(const char* meta_data);*/

	bool NeedReload() const;
private:

	std::string GetDataStructure(const std::string& line, const std::string& api);

private:

	uint64 lastTimeMod = 0;

public:
	bool reloadCompleted = false;
	std::vector<std::pair<std::string, bool>> dataStructures;

	std::string headerPath;
};
#endif //__ResourceScript_H__