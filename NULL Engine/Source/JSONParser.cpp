// ----------------------------------------------------
// Interface module for parson. (Wrapper)
// Employed to load/save engine configurations.
// Employed to serialize scenes and game objects.
// ----------------------------------------------------

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
double ParsonNode::GetNumber(const char* name) const
{	
	if (NodeHasValueOfType(name, JSONNumber))													// First whether or not the value in the child node is of the correct type is checked.
	{
		return json_object_get_number(rootNode, name);											// If the type matches the function's return type, then the value is read from the .json file.
	}
	else
	{
		const char* nodeName = json_object_get_name(rootNode, 0);
		LOG("[ERROR] JSON Parser: Node %s's %s did not have a Number variable!", nodeName, name);
	}

	return JSONError;
}

const char* ParsonNode::GetString(const char* name) const
{
	if (NodeHasValueOfType(name, JSONString))
	{
		return json_object_get_string(rootNode, name);
	}
	else
	{
		LOG("[ERROR] JSON Parser: %s did not have a String variable!", name);
	}

	return "NOT FOUND";
}

bool ParsonNode::GetBool(const char* name) const
{
	if (NodeHasValueOfType(name, JSONBoolean))
	{
		return (bool)json_object_get_boolean(rootNode, name);
	}
	else
	{
		LOG("[ERROR] JSON Parser: %s did not have a Bool variable!", name);
	}

	return false;
}

ParsonArray ParsonNode::GetArray(const char* name) const
{
	if (NodeHasValueOfType(name, JSONArray))
	{
		return ParsonArray(json_object_get_array(rootNode, name));
	}
	else
	{
		LOG("[ERROR] JSON Parser: %s did not have a JSON_Array variable!", name);
	}

	return ParsonArray();																		// If an empty config array is returned then check that json_array == nullptr and size == 0.
}

ParsonNode ParsonNode::GetNode(const char* name) const
{
	if (!NodeHasValueOfType(name, JSONObject))
	{
		LOG("[ERROR] JSON Parser: %s did not have a JSON_Object variable!", name);				// Just for display purposes.
	}
	
	return ParsonNode(json_object_get_object(rootNode, name));									// json_object_get_object() returns NULL if no JSON_Object can be found. Remember to check!
}

void ParsonNode::SetNumber(const char* name, double number)
{
	JSON_Status status = json_object_set_number(rootNode, name, number);

	if (status == JSONFailure)
	{
		LOG("[ERROR] JSON Parser: Could not set %s with the given Number!", name);
	}
}

void ParsonNode::SetString(const char* name, const char* string)
{
	JSON_Status status = json_object_set_string(rootNode, name, string);

	if (status == JSONFailure)
	{
		LOG("[ERROR] JSON Parser: Could not set %s with the given String!", name);
	}
}

void ParsonNode::SetBool(const char* name, bool value)
{
	JSON_Status status = json_object_set_boolean(rootNode, name, value);

	if (status == JSONFailure)
	{
		LOG("[ERROR] JSON Parser: Could not set %s with the given Bool!", name);
	}
}

ParsonArray ParsonNode::SetArray(const char* name)
{
	JSON_Status status = json_object_set_value(rootNode, name, json_value_init_array());		// Adding the array to the .json file.

	if (status == JSONFailure)
	{
		LOG("[ERROR] JSON Parser: Could not set %s with an JSON_Array!", name);
	}
	
	return ParsonArray(json_object_get_array(rootNode, name), name);							// Constructing and returning a handle to the created array.
}																								// json_object_get_array() will return NULL if the JSON_Array could not be created.

ParsonNode ParsonNode::SetNode(const char* name)
{
	json_object_set_value(rootNode, name, json_value_init_object());							// Adding the node to the .json file.

	return ParsonNode(json_object_get_object(rootNode, name));									// Constructing and returning a handle to the created node.
}

bool ParsonNode::NodeHasValueOfType(const char* name, JSON_Value_Type value_type) const
{
	return json_object_has_value_of_type(rootNode, name, value_type);
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
		size				= json_array_get_count(jsonArray);
	}
	else
	{
		LOG("[ERROR] The JSON_Array* passed to the constructor was nullptr!");
	}
}

double ParsonArray::GetNumber(const uint& index) const
{
	if (HasValueOfTypeAtIndex(index, JSONNumber))
	{
		return json_array_get_number(jsonArray, index);
	}
	else
	{
		LOG("[ERROR] JSON Parser: Array Index %u did not have a Number variable!", index);
	}

	return JSONError;
}

const char* ParsonArray::GetString(const uint& index) const
{
	if (HasValueOfTypeAtIndex(index, JSONString))
	{
		return json_array_get_string(jsonArray, index);
	}
	else
	{
		LOG("[ERROR] JSON Parser: Array Index %u did not have a String variable!", index);
	}
	
	return "NOT FOUND";
}

bool ParsonArray::GetBool(const uint& index) const
{
	if (HasValueOfTypeAtIndex(index, JSONBoolean))
	{
		return json_array_get_boolean(jsonArray, index);
	}
	else
	{
		LOG("[ERROR] JSON Parser: Array Index %u did not have a Bool variable!", index);
	}

	return false;
}

void ParsonArray::GetColor(const uint& index, Color& color) const
{
	bool validRed		= HasValueOfTypeAtIndex(index, JSONNumber);															// Safety check
	bool validGreen	= HasValueOfTypeAtIndex(index + 1, JSONNumber);															// Safety check
	bool validBlue		= HasValueOfTypeAtIndex(index + 2, JSONNumber);														// Safety check
	bool validAlpha	= HasValueOfTypeAtIndex(index + 3, JSONNumber);															// Safety check

	if (validRed && validGreen && validBlue && validAlpha)
	{
		color.r = (float)json_array_get_number(jsonArray, index);															// Getting r from the array.
		color.g = (float)json_array_get_number(jsonArray, index + 1);														// Getting g from the array.
		color.b = (float)json_array_get_number(jsonArray, index + 2);														// Getting b from the array.
		color.a = (float)json_array_get_number(jsonArray, index + 3);														// Getting a from the array.
	}
}

void ParsonArray::GetFloat3(const uint& index, float3& vec3) const
{
	bool validX = HasValueOfTypeAtIndex(index, JSONNumber);																	// Safety check
	bool validY = HasValueOfTypeAtIndex(index + 1, JSONNumber);																// Safety check
	bool validZ = HasValueOfTypeAtIndex(index + 2, JSONNumber);																// Safety check

	if (validX && validY && validZ)
	{
		vec3.x = (float)json_array_get_number(jsonArray, index);															// Getting x from the array.
		vec3.y = (float)json_array_get_number(jsonArray, index + 1);														// Getting y from the array.
		vec3.z = (float)json_array_get_number(jsonArray, index + 2);														// Getting z from the array.
	}
}

void ParsonArray::GetFloat4(const uint& index, float4& vec4) const
{
	bool validX = HasValueOfTypeAtIndex(index, JSONNumber);																	// Safety check
	bool validY = HasValueOfTypeAtIndex(index + 1, JSONNumber);																// Safety check
	bool validZ = HasValueOfTypeAtIndex(index + 2, JSONNumber);																// Safety check
	bool validW = HasValueOfTypeAtIndex(index + 3, JSONNumber);																// Safety check

	if (validX && validY && validZ && validW)
	{
		vec4.x = (float)json_array_get_number(jsonArray, index);															// Getting x from the array.
		vec4.y = (float)json_array_get_number(jsonArray, index + 1);														// Getting y from the array.
		vec4.z = (float)json_array_get_number(jsonArray, index + 2);														// Getting z from the array.
		vec4.w = (float)json_array_get_number(jsonArray, index + 3);														// Getting w from the array.
	}
}

void ParsonArray::GetFloat4(const uint& index, Quat& vec4) const
{
	bool validX = HasValueOfTypeAtIndex(index, JSONNumber);																	// Safety check
	bool validY = HasValueOfTypeAtIndex(index + 1, JSONNumber);																// Safety check
	bool validZ = HasValueOfTypeAtIndex(index + 2, JSONNumber);																// Safety check
	bool validW = HasValueOfTypeAtIndex(index + 3, JSONNumber);																// Safety check

	if (validX && validY && validZ && validW)
	{
		vec4.x = (float)json_array_get_number(jsonArray, index);															// Getting x from the array.
		vec4.y = (float)json_array_get_number(jsonArray, index + 1);														// Getting y from the array.
		vec4.z = (float)json_array_get_number(jsonArray, index + 2);														// Getting z from the array.
		vec4.w = (float)json_array_get_number(jsonArray, index + 3);														// Getting w from the array.
	}
}

ParsonNode ParsonArray::GetNode(const uint& index) const
{
	if (!HasValueOfTypeAtIndex(index, JSONObject))
	{
		LOG("[ERROR] JSON Parser: Could not get the Node at %u index in the %s Array!", name);
	}

	return json_array_get_object(jsonArray, index);
}

void ParsonArray::SetNumber(const double& number)
{
	JSON_Status status = json_array_append_number(jsonArray, number);

	if (status == JSONFailure)
	{
		LOG("[ERROR] JSON Parser: Could not append Number to %s Array!", name);
	}
	else
	{
		++size;
	}
}

void ParsonArray::SetString(const char* string)
{
	JSON_Status status = json_array_append_string(jsonArray, string);

	if (status == JSONFailure)
	{
		LOG("[ERROR] JSON Parser: Could not append String to %s Array!", name);
	}
	else
	{
		++size;
	}
}

void ParsonArray::SetBool(const bool& value)
{
	JSON_Status status = json_array_append_boolean(jsonArray, value);

	if (status == JSONFailure)
	{
		LOG("[ERROR] JSON Parser: Could not append Boolean to %s Array!", name);
	}
	else
	{
		++size;
	}
}

void ParsonArray::SetColor(const Color& color)
{
	JSON_Status statusR = json_array_append_number(jsonArray, color.r);													// Adding the RGBA variables to the JSON_Array.
	JSON_Status statusG = json_array_append_number(jsonArray, color.g);													// JSON_Status will be used to check whether or
	JSON_Status statusB = json_array_append_number(jsonArray, color.b);													// not the operation was a success.
	JSON_Status statusA = json_array_append_number(jsonArray, color.a);													// --------------------------------------------

	(statusR == JSONFailure) ? LOG("[ERROR] JSON Parser: Could not append Red to %s Array!", name)		: ++size;			// If an operation was not successful then an ERROR is sent
	(statusG == JSONFailure) ? LOG("[ERROR] JSON Parser: Could not append Green to %s Array!", name)	: ++size;			// to the console.
	(statusB == JSONFailure) ? LOG("[ERROR] JSON Parser: Could not append Blue to %s Array!", name)	: ++size;			// On success the size variable will be updated.
	(statusA == JSONFailure) ? LOG("[ERROR] JSON Parser: Could not append Alpha to %s Array!", name)	: ++size;			// ---------------------------------------------------------
}

void ParsonArray::SetFloat3(const math::float3& vec3)
{
	JSON_Status statusX = json_array_append_number(jsonArray, vec3.x);													// Adding the XYZ variables to the JSON_Array.
	JSON_Status statusY = json_array_append_number(jsonArray, vec3.y);													//
	JSON_Status statusZ = json_array_append_number(jsonArray, vec3.z);													// -------------------------------------------

	(statusX == JSONFailure) ? LOG("[ERROR] JSON Parser: Could not append X to %s Array!", name) : ++size;					// If an operation was not successful then an ERROR is sent
	(statusY == JSONFailure) ? LOG("[ERROR] JSON Parser: Could not append Y to %s Array!", name) : ++size;					// to the console. On success the size var. will be updated.
	(statusZ == JSONFailure) ? LOG("[ERROR] JSON Parser: Could not append Z to %s Array!", name) : ++size;					// ---------------------------------------------------------
}

void ParsonArray::SetFloat4(const math::float4& vec4)
{
	JSON_Status statusX = json_array_append_number(jsonArray, vec4.x);													// Adding the XYZW variables to the JSON_Array.
	JSON_Status statusY = json_array_append_number(jsonArray, vec4.y);													//
	JSON_Status statusZ = json_array_append_number(jsonArray, vec4.z);													//
	JSON_Status statusW = json_array_append_number(jsonArray, vec4.w);													// --------------------------------------------

	(statusX == JSONFailure) ? LOG("[ERROR] JSON Parser: Could not append X to %s Array!", name) : ++size;					// If an operation was not successful then an ERROR is sent
	(statusY == JSONFailure) ? LOG("[ERROR] JSON Parser: Could not append Y to %s Array!", name) : ++size;					// to the console.
	(statusZ == JSONFailure) ? LOG("[ERROR] JSON Parser: Could not append Z to %s Array!", name) : ++size;					// On success the size variable will be updated.
	(statusW == JSONFailure) ? LOG("[ERROR] JSON Parser: Could not append W to %s Array!", name) : ++size;					// --------------------------------------------------------
}

ParsonNode ParsonArray::SetNode(const char* name)
{	
	JSON_Status status = json_array_append_value(jsonArray, json_value_init_object());

	(status == JSONFailure) ? LOG("[ERROR] JSON Parser: Could not append Node to %s Array!", name) : ++size;

	return ParsonNode(json_array_get_object(jsonArray, size - 1));															// As the object was just appended, it will be located at the end.
}

JSON_Value_Type ParsonArray::GetTypeAtIndex(const uint& index) const
{
	return json_value_get_type(json_array_get_value(jsonArray, index));
}

bool ParsonArray::HasValueOfTypeAtIndex(const uint& index, JSON_Value_Type value_type) const
{
	if (GetTypeAtIndex(index) == value_type)
	{
		return true;
	}
	
	return false;
}

bool ParsonArray::ArrayIsValid() const
{
	return (jsonArray != NULL);
}