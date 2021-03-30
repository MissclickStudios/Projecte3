#ifndef __R_SCRIPT_H__
#define __R_SCRIPT_H__

#include "Resource.h"

#include <utility>
#include <vector>
#include <string>

class NULL_API R_Script : public Resource {
public:
	R_Script();
	virtual ~R_Script();

	bool SaveMeta(ParsonNode& metaRoot) const override;		//Per guardar les dependencies a altres resources
	bool LoadMeta(const ParsonNode& metaRoot) override;		//De Moment Np utilitzat

public:
	std::vector<std::pair<std::string, bool>> dataStructures;
};
#endif //__R_SCRIPT_H__