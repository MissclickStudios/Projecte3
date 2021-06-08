#include <vector>
#include <string>
#include "ScriptMacros.h"

namespace ScriptCrossDllHelpers 
{
	SCRIPTS_FUNCTION void StringVectorPushBackChar(void*ptr,const char*newString)
	{
		(*(std::vector<std::string>*)ptr).push_back(newString);
	}
	SCRIPTS_FUNCTION void StringVectorEmplaceBackChar(void* ptr, const char* newString)
	{
		(*(std::vector<std::string>*)ptr).emplace_back(newString);
	}
	SCRIPTS_FUNCTION void StringVectorPushBackString(void* ptr, const std::string& newString)
	{
		(*(std::vector<std::string>*)ptr).push_back(newString);
	}
	SCRIPTS_FUNCTION void StringVectorEmplaceBackString(void* ptr, const std::string& newString)
	{
		(*(std::vector<std::string>*)ptr).emplace_back(newString);
	}
	SCRIPTS_FUNCTION void StringVectorReserve(void* ptr, int size)
	{
		(*(std::vector<std::string>*)ptr).reserve(size);
	}
	SCRIPTS_FUNCTION void StringVectorErase(void* ptr, int index)
	{
		(*(std::vector<std::string>*)ptr).erase((*(std::vector<std::string>*)ptr).begin() + index);
	}
	SCRIPTS_FUNCTION void SetString(void* dstptr, const char* source)
	{
		(*(std::string*)dstptr) = source;
	}
}