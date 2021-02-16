#ifndef __JSON_PARSER_H__
#define __JSON_PARSER_H__

#include "parson/include/parson.h"

class ParsonArray;																		// To avoid having to forward-declare this, put Configuration_Array before Configuration. 

struct Color;

namespace math
{
	class float3;
	class float4;
	class Quat;
}

typedef unsigned int uint;

class ParsonNode
{
public:
	ParsonNode();
	ParsonNode(const char* buffer);
	ParsonNode(JSON_Object* object);
	~ParsonNode();

	uint SerializeToBuffer	(char** buffer);															//
	uint SerializeToFile	(const char* path, char** buffer);											// 
	bool Release			();																			// Will free the memory allocated to the parson elements.

public:																									// --- GETTERS, SETTERS & UTILITIES
	double					GetNumber			(const char* name) const;								//
	const char*				GetString			(const char* name) const;								//
	bool					GetBool				(const char* name) const;								//
	ParsonArray				GetArray			(const char* name) const;								//
	ParsonNode				GetNode				(const char* name) const;								//

	void					SetNumber			(const char* name, double number);						//
	void					SetString			(const char* name, const char* string);					//
	void					SetBool				(const char* name, bool value);							//
	ParsonArray				SetArray			(const char* name);										//
	ParsonNode				SetNode				(const char* name);										//

public:
	bool					NodeHasValueOfType	(const char* name, JSON_Value_Type value_type) const;	//
	bool					NodeIsValid			();														//
	JSON_Value*				FindValue			(const char* name, int index);							//

private:
	JSON_Value*			root_value;																		// First value of a given parsed file. The first JSON Object will be derived from this value.
	JSON_Object*		root_node;																		// A JSON Object is the same as an XML Node. Main node from which the rest will be derived.
};

class ParsonArray
{
public:
	ParsonArray();
	ParsonArray(JSON_Array* json_array, const char* name = "Array");

public:																										// --- GETTERS, SETTERS & UTILITIES
	double					GetNumber				(const uint& index) const;								//
	const char*				GetString				(const uint& index) const;								//
	bool					GetBool					(const uint& index) const;								//
	void					GetColor				(const uint& index, Color& color) const;
	void					GetFloat3				(const uint& index, math::float3& vec3) const;
	void					GetFloat4				(const uint& index, math::float4& vec4) const;
	void					GetFloat4				(const uint& index, math::Quat& vec4) const;
	ParsonNode				GetNode					(const uint& index) const;								//

	void					SetNumber				(const double& number);									//
	void					SetString				(const char* string);									//
	void					SetBool					(const bool& value);									//
	void					SetColor				(const Color& color);
	void					SetFloat3				(const math::float3& vec3);
	void					SetFloat4				(const math::float4& vec4);
	ParsonNode				SetNode					(const char* name);										//

public:
	uint					GetSize					() const;												//
	JSON_Value_Type			GetTypeAtIndex			(const uint& index) const;								//
	bool					HasValueOfTypeAtIndex	(const uint& index, JSON_Value_Type value_type) const;	//

	bool					ArrayIsValid			() const;

public:
	JSON_Array*			json_array;																			//
	uint				size;																				//

	const char*			name;
};
#endif // !__JSON_PARSER_H__