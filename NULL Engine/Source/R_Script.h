#ifndef __R_SCRIPT_H__
#define __R_SCRIPT_H__

#include "Resource.h"

#include <utility>
#include <vector>
#include <string>

typedef unsigned __int64 uint64;

class NULL_API R_Script : public Resource {
public:
	R_Script();
	virtual ~R_Script();

	bool SaveMeta(ParsonNode& metaRoot) const override;		//Per guardar les dependencies a altres resources
	bool LoadMeta(const ParsonNode& metaRoot) override;		//De Moment Np utilitzat

	/*bool CreateMetaData(const unsigned int& force_id = 0);
	bool ReadBaseInfo(const char* assets_file_path);
	void ReadLibrary(const char* meta_data);*/

	bool NeedReload() const;

	bool CleanUp() override;

private:

	std::string GetDataStructure(const std::string& line, const std::string& api);

public:
	uint64 lastTimeMod = 0;
	bool reloadCompleted = false;
	std::vector<std::pair<std::string, bool>> dataStructures;

	std::string headerPath;
};
#endif //__R_SCRIPT_H__