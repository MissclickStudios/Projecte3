// ----------------------------------------------------
// Interface module for parson. (Wrapper)
// Employed to load/save engine configurations.
// Employed to serialize scenes and game objects.
// ----------------------------------------------------

#include "MathGeoLib/include/Math/float2.h"
#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/float4.h"
#include "MathGeoLib/include/Math/Quat.h"

#include "VariableTypedefs.h"																	// Globals										
#include "Macros.h"																				// 
#include "Log.h"																				// -------

#include "Color.h"																				// Containers

#include "Application.h"																		// Not quite sure whether or not this is a dependency to have here.
#include "M_FileSystem.h"																		// ----------------------------------------------------------------

#include "JSONParser.h"																			// Header file of this cpp file.

#include "MemoryManager.h"

ParsonNode::ParsonNode() : 
rootValue	(nullptr), 
rootNode	(nullptr)
{
	rootValue	= json_value_init_object();														// Creates an empty JSON_Object. Being a JSON_Value*, it needs to be passed to a JSON_Object*
	rootNode	= json_value_get_object(rootValue);											// Gets the previous JSON_Value and sets it as a JSON_Object*. First handle/node of the file.
}

ParsonNode::ParsonNode(const char* buffer) : 
rootValue	(nullptr), 
rootNode	(nullptr)
{	
	rootValue = json_parse_string(buffer);														// Creates a JSON_Object out of a given buffer. Used to load a previous .json file.
	
	if (rootValue != NULL)																		// Using NULL instead of nullptr as parson is a library written in C, not C++.
	{
		rootNode = json_value_get_object(rootValue);
	}
	else
	{
		LOG("[ERROR] Parse operation through string failed!");
	}
}

ParsonNode::ParsonNode(JSON_Object* object) :													// If config is init with an existing JSON_Object*, node will be directly init with it.
rootValue	(nullptr), 
rootNode	(object)
{
	
}

ParsonNode::~ParsonNode()
{
	Release();																					// Will clean up any allocated memory in this specific ParsonNode().
}

uint ParsonNode::SerializeToBuffer(char** buffer)
{
	uint size = (uint)json_serialization_size_pretty(rootValue);
	*buffer = new char[size];

	JSON_Status status = json_serialize_to_buffer_pretty(rootValue, *buffer, size);

	if (status == JSONFailure)
	{
		LOG("[ERROR] Could not serialize the buffer! Error: Parson could not allocate or write the buffer.");
		RELEASE_ARRAY(buffer);
		size = 0;
	}

	return size;
}

uint ParsonNode::SerializeToFile(const char* path, char** buffer)
{
	uint written = 0;
	
	uint size = SerializeToBuffer(buffer);
	if (size == 0)
	{
		LOG("[ERROR] JSON Parser: Could not Serialize Root Node into File! Error: Could not Serialize Root Node into buffer.");
		return 0;
	}

	written = App->fileSystem->Save(path, *buffer, size);
	if (written > 0)
	{
		LOG("[STATUS] JSON Parser: Successfully Serialized Root Node to File! Path: %s", path);
	}
	else
	{
		LOG("[ERROR] JSON Parser: Could not Serialize Root Node into File! Error: File System could not write the File.");
	}

	return written;
}

bool ParsonNode::Release()
{
	if (rootValue != NULL)
	{
		json_value_free(rootValue);															// Frees the memory that parson previously allocated to root_value.
	}

	return true;
}

// --- PARSON NODE METHODS ---
// --- GETTERS AND SETTERS
int ParsonNode::GetInteger(const const char* name)
{
	return json_object_get_number(rootNode, name);
}

double ParsonNode::GetNumber(const char* name) const
{	
	return (NodeHasValueOfType(name, JSONNumber)) ? json_object_get_number(rootNode, name) : INVALID_PARSON_NUMBER;
}

float2 ParsonNode::GetFloat2(const const char* name)
{
	JSON_Array* tempArray = json_object_get_array(rootNode, name);
	float2 floats;
	floats.x = json_array_get_number(tempArray, 0);
	floats.y = json_array_get_number(tempArray, 1);

	return floats;
}

float3 ParsonNode::GetFloat3(const const char* name)
{
	JSON_Array* tempArray = json_object_get_array(rootNode, name);
	float3 floats;
	floats.x = json_array_get_number(tempArray, 0);
	floats.y = json_array_get_number(tempArray, 1);
	floats.z = json_array_get_number(tempArray, 2);

	return floats;
}

float4 ParsonNode::GetFloat4(const const char* name)
{
	JSON_Array* tempArray = json_object_get_array(rootNode, name);
	float4 floats;
	floats.x = json_array_get_number(tempArray, 0);
	floats.y = json_array_get_number(tempArray, 1);
	floats.z = json_array_get_number(tempArray, 2);
	floats.w = json_array_get_number(tempArray, 3);
	return floats;
}

Quat ParsonNode::GetQuat(const const char* name)
{
	JSON_Array* tempArray = json_object_get_array(rootNode, name);
	Quat quat;
	quat.x = json_array_get_number(tempArray, 0);
	quat.y = json_array_get_number(tempArray, 1);
	quat.z = json_array_get_number(tempArray, 2);
	quat.w = json_array_get_number(tempArray, 3);

	return quat;
}

Color ParsonNode::GetColor(const char* name)
{
	JSON_Array* tempArray = json_object_get_array(rootNode, name);
	Color color;
	color.r = json_array_get_number(tempArray, 0);
	color.g = json_array_get_number(tempArray, 1);
	color.b = json_array_get_number(tempArray, 2);
	color.a = json_array_get_number(tempArray, 3);

	return color;
}

const char* ParsonNode::GetString(const char* name) const
{
	return (NodeHasValueOfType(name, JSONString)) ? json_object_get_string(rootNode, name) : "[NOT FOUND]";
}

bool ParsonNode::GetBool(const char* name) const
{
	return (NodeHasValueOfType(name, JSONBoolean)) ? (bool)json_object_get_boolean(rootNode, name) : false;
}

ParsonArray ParsonNode::GetArray(const char* name, bool logErrors) const						// If an invalid ParsonArray is returned, check with ParsonArray::ArrayIsValid().
{
	return (NodeHasValueOfType(name, JSONArray, logErrors)) ? ParsonArray(json_object_get_array(rootNode, name)) : ParsonArray();
}

ParsonNode ParsonNode::GetNode(const char* name, bool logErrors) const							// Returns invalid node upon failure. Remember to check with NodeIsValid()!
{
	return (NodeHasValueOfType(name, JSONObject, logErrors)) ? ParsonNode(json_object_get_object(rootNode, name)) : ParsonNode((JSON_Object*)nullptr);
}

void ParsonNode::SetInteger(const char* name, int number)										// JSONSuccess is just a placeholder to be able to use the ternary operator.
{
	(json_object_set_number(rootNode, name, number) != JSONFailure) ? JSONSuccess : LOG("[ERROR] JSON Parser: Could not set { %s } with the given Integer!", name);
}

void ParsonNode::SetNumber(const char* name, double number)										// JSONSuccess is just a placeholder to be able to use the ternary operator.
{
	(json_object_set_number(rootNode, name, number) != JSONFailure) ? JSONSuccess : LOG("[ERROR] JSON Parser: Could not set { %s } with the given Number!", name);
}

void ParsonNode::SetFloat2(const char* name, const float2& float2)
{
	JSON_Array* tempArray = json_object_get_array(rootNode, name);
	if (tempArray == nullptr)
	{
		JSON_Value* val = json_value_init_array();
		tempArray		= json_value_get_array(val);

		json_object_dotset_value(rootNode, name, val);
	}
	else 
	{
		json_array_clear(tempArray);
	}

	json_array_append_number(tempArray, float2.x);
	json_array_append_number(tempArray, float2.y);
}

void ParsonNode::SetFloat3(const char* name, const float3& float3)
{
	JSON_Array* tempArray = json_object_get_array(rootNode, name);
	if (tempArray == nullptr)
	{
		JSON_Value* val = json_value_init_array();
		tempArray		= json_value_get_array(val);

		json_object_dotset_value(rootNode, name, val);
	}
	else 
	{
		json_array_clear(tempArray);
	}

	json_array_append_number(tempArray, float3.x);
	json_array_append_number(tempArray, float3.y);
	json_array_append_number(tempArray, float3.z);
}

void ParsonNode::SetFloat4(const char* name, const float4& float4)
{
	JSON_Array* tempArray = json_object_get_array(rootNode, name);
	if (tempArray == nullptr)
	{
		JSON_Value* val = json_value_init_array();
		tempArray		= json_value_get_array(val);

		json_object_dotset_value(rootNode, name, val);
	}
	else
	{
		json_array_clear(tempArray);
	}

	json_array_append_number(tempArray, float4.x);
	json_array_append_number(tempArray, float4.y);
	json_array_append_number(tempArray, float4.z);
	json_array_append_number(tempArray, float4.w);
}

void ParsonNode::SetColor(const char* name, const Color& color)
{
	JSON_Array* tempArray = json_object_get_array(rootNode, name);
	if (tempArray == nullptr)
	{
		JSON_Value* val = json_value_init_array();
		tempArray		= json_value_get_array(val);

		json_object_dotset_value(rootNode, name, val);
	}
	else
	{
		json_array_clear(tempArray);
	}

	json_array_append_number(tempArray, color.r);
	json_array_append_number(tempArray, color.g);
	json_array_append_number(tempArray, color.b);
	json_array_append_number(tempArray, color.a);
}

void ParsonNode::SetQuat(const char* name, const Quat& quat)
{
	JSON_Array* tempArray = json_object_get_array(rootNode, name);
	if (tempArray == nullptr)
	{
		JSON_Value* val = json_value_init_array();
		tempArray		= json_value_get_array(val);

		json_object_dotset_value(rootNode, name, val);
	}
	else
	{
		json_array_clear(tempArray);
	}

	json_array_append_number(tempArray, quat.x);
	json_array_append_number(tempArray, quat.y);
	json_array_append_number(tempArray, quat.z);
	json_array_append_number(tempArray, quat.w);
}

void ParsonNode::SetString(const char* name, const char* string)
{
	JSON_Status status = json_object_set_string(rootNode, name, string);

	if (status == JSONFailure)
	{
		LOG("[ERROR] JSON Parser: Could not set { %s } with the given String!", name);
	}
}

void ParsonNode::SetBool(const char* name, bool value)
{
	JSON_Status status = json_object_set_boolean(rootNode, name, value);

	if (status == JSONFailure)
	{
		LOG("[ERROR] JSON Parser: Could not set { %s } with the given Bool!", name);
	}
}

ParsonArray ParsonNode::SetArray(const char* name)
{
	JSON_Status status = json_object_set_value(rootNode, name, json_value_init_array());									// Adding the array to the .json file.
	if (status == JSONFailure)
	{
		LOG("[ERROR] JSON Parser: Could not set { %s } with a JSON_Array!", name);
	}
	
	return ParsonArray(json_object_get_array(rootNode, name), name);														// Constructing and returning a handle to the created array.
}																															// json_object_get_array() will return NULL upon failure.

ParsonNode ParsonNode::SetNode(const char* name)
{
	json_object_set_value(rootNode, name, json_value_init_object());														// Adding the node to the .json file.

	return ParsonNode(json_object_get_object(rootNode, name));																// Constructing and returning a handle to the created node.
}

bool ParsonNode::NodeHasValueOfType(const char* name, JSON_Value_Type valueType, bool logErrors) const
{
	if (name == nullptr)
	{
		LOG("[ERROR] JSON Parser: Could not check the Node's Value Type! Error: Given name string was nullptr.");
		return false;
	}
	
	if (!json_object_has_value_of_type(rootNode, name, valueType))
	{
		if (logErrors)
		{
			switch (valueType)
			{
			case JSONNull:		{ LOG("[ERROR] JSON Parser: Node { %s } did not have a Null variable!", name); }	break;
			case JSONString:	{ LOG("[ERROR] JSON Parser: Node { %s } did not have a String variable!", name); }	break;
			case JSONNumber:	{ LOG("[ERROR] JSON Parser: Node { %s } did not have a Number variable!", name); }	break;
			case JSONObject:	{ LOG("[ERROR] JSON Parser: Node { %s } did not have an Object variable!", name); }	break;
			case JSONArray:		{ LOG("[ERROR] JSON Parser: Node { %s } did not have an Array variable!", name); }	break;
			case JSONBoolean:	{ LOG("[ERROR] JSON Parser: Node { %s } did not have a Boolean variable!", name); }	break;
			case JSONError:		{ LOG("[ERROR] JSON Parser: Node { %s } did not have an Error variable!", name); }	break;
			}
		}

		return false;
	}
	
	return true;
}

bool ParsonNode::NodeIsValid()
{
	return (rootNode != NULL);
}

JSON_Value* ParsonNode::FindValue(const char* name, int index)
{
	if (index < 0)
	{
		return json_object_get_value(rootNode, name);
	}
	else
	{
		JSON_Array* jsonArray = json_object_get_array(rootNode, name);

		if (jsonArray != nullptr)
		{
			return json_array_get_value(jsonArray, index);
		}
	}

	return nullptr;
}

// ------------------ PARSON ARRAY METHODS ------------------
ParsonArray::ParsonArray()
{
	jsonArray	= nullptr;
	size		= 0;
	name		= "NONE";
}

ParsonArray::ParsonArray(JSON_Array* jsonArray, const char* name) : 
jsonArray	(nullptr), 
size		(0),
name		(name)
{
	if (jsonArray != nullptr)
	{
		this->jsonArray	= jsonArray;
		size			= json_array_get_count(jsonArray);
	}
	else
	{
		LOG("[ERROR] The JSON_Array* passed to the constructor was nullptr!");
	}
}

double ParsonArray::GetNumber(uint index) const
{
	if (index >= size)
	{
		LOG("[ERROR] JSON Parser: Could not Get Number at Index [%u] of Array { %s }! Error: { index >= size }.", index, name);
		return INVALID_PARSON_NUMBER;
	}
	
	return (HasValueOfTypeAtIndex(index, JSONNumber)) ? json_array_get_number(jsonArray, index) : INVALID_PARSON_NUMBER;
}

const char* ParsonArray::GetString(uint index) const
{
	if (index >= size)
	{
		LOG("[ERROR] JSON Parser: Could not Get String at Index [%u] of Array { %s }! Error: { index >= size }.", index, name);
		return "[NOT FOUND]";
	}
	
	return (HasValueOfTypeAtIndex(index, JSONString)) ? json_array_get_string(jsonArray, index) : "[NOT FOUND]";
}

bool ParsonArray::GetBool(uint index) const
{
	if (index >= size)
	{
		LOG("[ERROR] JSON Parser: Could not Get Bool at Index [%u] of Array { %s }! Error: { index >= size }.", index, name);
		return false;
	}
	
	return (HasValueOfTypeAtIndex(index, JSONBoolean)) ? json_array_get_boolean(jsonArray, index) : false;
}

void ParsonArray::GetColor(uint index, Color& color) const																	// The "+ 0" in color.r is unnecessary but aesthetic. 
{
	if (index >= size)
	{
		LOG("[ERROR] JSON Parser: Could not Get Color at Index [%u] of Array { %s }! Error: { index >= size }.", index, name);
		return;
	}
	
	color.r = (HasValueOfTypeAtIndex(index + 0, JSONNumber)) ? (float)json_array_get_number(jsonArray, index + 0) : 1.0f;	// Safety check + Getting r from the array.
	color.g = (HasValueOfTypeAtIndex(index + 1, JSONNumber)) ? (float)json_array_get_number(jsonArray, index + 1) : 1.0f;	// Safety check + Getting g from the array.
	color.b = (HasValueOfTypeAtIndex(index + 2, JSONNumber)) ? (float)json_array_get_number(jsonArray, index + 2) : 1.0f;	// Safety check + Getting b from the array.
	color.a = (HasValueOfTypeAtIndex(index + 3, JSONNumber)) ? (float)json_array_get_number(jsonArray, index + 3) : 1.0f;	// Safety check + Getting a from the array.
}

void ParsonArray::GetFloat3(uint index, float3& vec3) const																	// The "+ 0" in vec3.x is unnecessary but aesthetic. 
{
	if (index >= size)
	{
		LOG("[ERROR] JSON Parser: Could not Get Float3 at Index [%u] of Array { %s }! Error: { index >= size }.", index, name);
		return;
	}
	
	vec3.x = (HasValueOfTypeAtIndex(index + 0, JSONNumber)) ? (float)json_array_get_number(jsonArray, index + 0) : 0.0f;	// Safety check + Getting x from the array.
	vec3.y = (HasValueOfTypeAtIndex(index + 1, JSONNumber)) ? (float)json_array_get_number(jsonArray, index + 1) : 0.0f;	// Safety check + Getting y from the array.
	vec3.z = (HasValueOfTypeAtIndex(index + 2, JSONNumber)) ? (float)json_array_get_number(jsonArray, index + 2) : 0.0f;	// Safety check + Getting z from the array.
}

void ParsonArray::GetFloat4(uint index, float4& vec4) const																	// The "+ 0" in vec4.x is unnecessary but aesthetic. 
{
	if (index >= size)
	{
		LOG("[ERROR] JSON Parser: Could not Get Float4 at Index [%u] of Array { %s }! Error: { index >= size }.", index, name);
		return;
	}
	
	vec4.x = (HasValueOfTypeAtIndex(index + 0, JSONNumber)) ? (float)json_array_get_number(jsonArray, index + 0) : 0.0f;	// Safety check + Getting x from the array.
	vec4.y = (HasValueOfTypeAtIndex(index + 1, JSONNumber)) ? (float)json_array_get_number(jsonArray, index + 1) : 0.0f;	// Safety check + Getting y from the array.
	vec4.z = (HasValueOfTypeAtIndex(index + 2, JSONNumber)) ? (float)json_array_get_number(jsonArray, index + 2) : 0.0f;	// Safety check + Getting z from the array.
	vec4.w = (HasValueOfTypeAtIndex(index + 3, JSONNumber)) ? (float)json_array_get_number(jsonArray, index + 3) : 0.0f;	// Safety check + Getting w from the array.
}

void ParsonArray::GetQuat(uint index, Quat& quat) const																		// The "+ 0" in quat.x is unnecessary but aesthetic. 
{
	if (index >= size)
	{
		LOG("[ERROR] JSON Parser: Could not Get Quat at Index [%u] of Array { %s }! Error: { index >= size }.", index, name);
		return;
	}
	
	quat.x = (HasValueOfTypeAtIndex(index + 0, JSONNumber)) ? (float)json_array_get_number(jsonArray, index + 0) : 0.0f;	// Safety check + Getting x from the array.
	quat.y = (HasValueOfTypeAtIndex(index + 1, JSONNumber)) ? (float)json_array_get_number(jsonArray, index + 1) : 0.0f;	// Safety check + Getting y from the array.
	quat.z = (HasValueOfTypeAtIndex(index + 2, JSONNumber)) ? (float)json_array_get_number(jsonArray, index + 2) : 0.0f;	// Safety check + Getting z from the array.
	quat.w = (HasValueOfTypeAtIndex(index + 3, JSONNumber)) ? (float)json_array_get_number(jsonArray, index + 3) : 0.0f;	// Safety check + Getting w from the array.
}

ParsonNode ParsonArray::GetNode(uint index) const
{
	if (index >= size)
	{
		LOG("[ERROR] JSON Parser: Could not Get Node at Index [%u] of Array { %s }! Error: { index >= size }.", index, name);
		return json_array_get_object(NULL, 0);
	}
	
	return (HasValueOfTypeAtIndex(index, JSONObject)) ? json_array_get_object(jsonArray, index) : json_array_get_object(NULL, 0);
}

void ParsonArray::SetNumber(double number)
{
	(json_array_append_number(jsonArray, number) != JSONFailure) ? ++size : LOG("[ERROR] JSON Parser: Could not append Number to { %s } Array!", name);
}

void ParsonArray::SetString(const char* string)
{
	(json_array_append_string(jsonArray, string) != JSONFailure) ? ++size : LOG("[ERROR] JSON Parser: Could not append String to { %s } Array!", name);
}

void ParsonArray::SetBool(bool value)
{
	(json_array_append_boolean(jsonArray, value) != JSONFailure) ? ++size : LOG("[ERROR] JSON Parser: Could not append Boolean to { %s } Array!", name);
}

void ParsonArray::SetColor(const Color& color)
{
	(json_array_append_number(jsonArray, (double)color.r) != JSONFailure) ? ++size : LOG("[ERROR] JSON Parser: Could not append R to { %s } Array!", name);
	(json_array_append_number(jsonArray, (double)color.g) != JSONFailure) ? ++size : LOG("[ERROR] JSON Parser: Could not append G to { %s } Array!", name);
	(json_array_append_number(jsonArray, (double)color.b) != JSONFailure) ? ++size : LOG("[ERROR] JSON Parser: Could not append B to { %s } Array!", name);
	(json_array_append_number(jsonArray, (double)color.a) != JSONFailure) ? ++size : LOG("[ERROR] JSON Parser: Could not append A to { %s } Array!", name);
}

void ParsonArray::SetFloat3(const math::float3& vec3)
{
	(json_array_append_number(jsonArray, (double)vec3.x) != JSONFailure) ? ++size : LOG("[ERROR] JSON Parser: Could not append X to { %s } Array!", name);
	(json_array_append_number(jsonArray, (double)vec3.y) != JSONFailure) ? ++size : LOG("[ERROR] JSON Parser: Could not append Y to { %s } Array!", name);
	(json_array_append_number(jsonArray, (double)vec3.z) != JSONFailure) ? ++size : LOG("[ERROR] JSON Parser: Could not append Z to { %s } Array!", name);
}

void ParsonArray::SetFloat4(const math::float4& vec4)
{
	(json_array_append_number(jsonArray, (double)vec4.x) != JSONFailure) ? ++size : LOG("[ERROR] JSON Parser: Could not append X to { %s } Array!", name);
	(json_array_append_number(jsonArray, (double)vec4.y) != JSONFailure) ? ++size : LOG("[ERROR] JSON Parser: Could not append Y to { %s } Array!", name);
	(json_array_append_number(jsonArray, (double)vec4.z) != JSONFailure) ? ++size : LOG("[ERROR] JSON Parser: Could not append Z to { %s } Array!", name);
	(json_array_append_number(jsonArray, (double)vec4.w) != JSONFailure) ? ++size : LOG("[ERROR] JSON Parser: Could not append W to { %s } Array!", name);
}

ParsonNode ParsonArray::SetNode(const char* name)
{	
	(json_array_append_value(jsonArray, json_value_init_object()) != JSONFailure) ? ++size : LOG("[ERROR] JSON Parser: Could not append Node to { %s } Array!", this->name);

	return ParsonNode(json_array_get_object(jsonArray, size - 1));											// As the object was just appended, it will be located at the end.
}

bool ParsonArray::ArrayIsValid() const
{
	return (jsonArray != NULL);
}

uint ParsonArray::GetSize() const
{
	return size;
}

bool ParsonArray::IndexIsWithinRange(uint index)
{
	return (index >= size);
}

JSON_Value_Type ParsonArray::GetTypeAtIndex(uint index) const
{	
	if (index >= size)
	{
		LOG("[ERROR] JSON Parser: Could not Get Type of Index [%u] of Array { %s }! Error: { index >= size }.", index, name);
		return JSONError;
	}
	
	return json_value_get_type(json_array_get_value(jsonArray, index));
}

bool ParsonArray::HasValueOfTypeAtIndex(uint index, JSON_Value_Type valueType) const
{
	if (index >= size)
	{
		LOG("[ERROR] JSON Parser: Could not get Value Type of Index [%u] at Array { %s }! Error: { index >= size }.", index, name);
		return false;
	}
	
	JSON_Value_Type indexValueType = GetTypeAtIndex(index);
	if (indexValueType != valueType)
	{
		switch (valueType)
		{
		case JSONNull:		{ LOG("[ERROR] JSON Parser: Index [%u] at Array { %s } did not have a Null variable!", index, name); }		break;
		case JSONString:	{ LOG("[ERROR] JSON Parser: Index [%u] at Array { %s } did not have a String variable!", index, name); }	break;
		case JSONNumber:	{ LOG("[ERROR] JSON Parser: Index [%u] at Array { %s } did not have a Number variable!", index, name); }	break;
		case JSONObject:	{ LOG("[ERROR] JSON Parser: Index [%u] at Array { %s } did not have an Object variable!", index, name); }	break;
		case JSONArray:		{ LOG("[ERROR] JSON Parser: Index [%u] at Array { %s } did not have an Array variable!", index, name); }	break;
		case JSONBoolean:	{ LOG("[ERROR] JSON Parser: Index [%u] at Array { %s } did not have a Boolean variable!", index, name); }	break;
		case JSONError:		{ LOG("[ERROR] JSON Parser: Index [%u] at Array { %s } did not have an Error variable!", index, name); }	break;
		}

		return false;
	}
	
	return true;
}