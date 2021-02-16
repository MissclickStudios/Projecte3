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
root_value	(nullptr), 
root_node	(nullptr)
{
	root_value	= json_value_init_object();														// Creates an empty JSON_Object. Being a JSON_Value*, it needs to be passed to a JSON_Object*
	root_node	= json_value_get_object(root_value);											// Gets the previous JSON_Value and sets it as a JSON_Object*. First handle/node of the file.
}

ParsonNode::ParsonNode(const char* buffer) : 
root_value	(nullptr), 
root_node	(nullptr)
{	
	root_value = json_parse_string(buffer);														// Creates a JSON_Object out of a given buffer. Used to load a previous .json file.
	
	if (root_value != NULL)																		// Using NULL instead of nullptr as parson is a library written in C, not C++.
	{
		root_node = json_value_get_object(root_value);
	}
	else
	{
		LOG("[ERROR] Parse operation through string failed!");
	}
}

ParsonNode::ParsonNode(JSON_Object* object) :													// If config is init with an existing JSON_Object*, node will be directly init with it.
root_value	(nullptr), 
root_node	(object)
{
	
}

ParsonNode::~ParsonNode()
{
	Release();																					// Will clean up any allocated memory in this specific ParsonNode().
}

uint ParsonNode::SerializeToBuffer(char** buffer)
{
	uint size = (uint)json_serialization_size_pretty(root_value);
	*buffer = new char[size];

	JSON_Status status = json_serialize_to_buffer_pretty(root_value, *buffer, size);

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

	written = App->file_system->Save(path, *buffer, size);
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
	if (root_value != NULL)
	{
		json_value_free(root_value);															// Frees the memory that parson previously allocated to root_value.
	}

	return true;
}

// --- PARSON NODE METHODS ---
// --- GETTERS AND SETTERS
double ParsonNode::GetNumber(const char* name) const
{	
	if (NodeHasValueOfType(name, JSONNumber))													// First whether or not the value in the child node is of the correct type is checked.
	{
		return json_object_get_number(root_node, name);											// If the type matches the function's return type, then the value is read from the .json file.
	}
	else
	{
		const char* node_name = json_object_get_name(root_node, 0);
		LOG("[ERROR] JSON Parser: Node %s's %s did not have a Number variable!", node_name, name);
	}

	return JSONError;
}

const char* ParsonNode::GetString(const char* name) const
{
	if (NodeHasValueOfType(name, JSONString))
	{
		return json_object_get_string(root_node, name);
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
		return (bool)json_object_get_boolean(root_node, name);
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
		return ParsonArray(json_object_get_array(root_node, name));
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
	
	return ParsonNode(json_object_get_object(root_node, name));									// json_object_get_object() returns NULL if no JSON_Object can be found. Remember to check!
}

void ParsonNode::SetNumber(const char* name, double number)
{
	JSON_Status status = json_object_set_number(root_node, name, number);

	if (status == JSONFailure)
	{
		LOG("[ERROR] JSON Parser: Could not set %s with the given Number!", name);
	}
}

void ParsonNode::SetString(const char* name, const char* string)
{
	JSON_Status status = json_object_set_string(root_node, name, string);

	if (status == JSONFailure)
	{
		LOG("[ERROR] JSON Parser: Could not set %s with the given String!", name);
	}
}

void ParsonNode::SetBool(const char* name, bool value)
{
	JSON_Status status = json_object_set_boolean(root_node, name, value);

	if (status == JSONFailure)
	{
		LOG("[ERROR] JSON Parser: Could not set %s with the given Bool!", name);
	}
}

ParsonArray ParsonNode::SetArray(const char* name)
{
	JSON_Status status = json_object_set_value(root_node, name, json_value_init_array());		// Adding the array to the .json file.

	if (status == JSONFailure)
	{
		LOG("[ERROR] JSON Parser: Could not set %s with an JSON_Array!", name);
	}
	
	return ParsonArray(json_object_get_array(root_node, name), name);							// Constructing and returning a handle to the created array.
}																								// json_object_get_array() will return NULL if the JSON_Array could not be created.

ParsonNode ParsonNode::SetNode(const char* name)
{
	json_object_set_value(root_node, name, json_value_init_object());							// Adding the node to the .json file.

	return ParsonNode(json_object_get_object(root_node, name));									// Constructing and returning a handle to the created node.
}

bool ParsonNode::NodeHasValueOfType(const char* name, JSON_Value_Type value_type) const
{
	return json_object_has_value_of_type(root_node, name, value_type);
}

bool ParsonNode::NodeIsValid()
{
	return (root_node != NULL);
}

JSON_Value* ParsonNode::FindValue(const char* name, int index)
{
	if (index < 0)
	{
		return json_object_get_value(root_node, name);
	}
	else
	{
		JSON_Array* json_array = json_object_get_array(root_node, name);

		if (json_array != nullptr)
		{
			return json_array_get_value(json_array, index);
		}
	}

	return nullptr;
}

// ------------------ PARSON ARRAY METHODS ------------------
ParsonArray::ParsonArray()
{
	json_array	= nullptr;
	size		= 0;
	name		= "NONE";
}

ParsonArray::ParsonArray(JSON_Array* json_array, const char* name) : 
json_array	(nullptr), 
size		(0),
name		(name)
{
	if (json_array != nullptr)
	{
		this->json_array	= json_array;
		size				= json_array_get_count(json_array);
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
		return json_array_get_number(json_array, index);
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
		return json_array_get_string(json_array, index);
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
		return json_array_get_boolean(json_array, index);
	}
	else
	{
		LOG("[ERROR] JSON Parser: Array Index %u did not have a Bool variable!", index);
	}

	return false;
}

void ParsonArray::GetColor(const uint& index, Color& color) const
{
	bool valid_red		= HasValueOfTypeAtIndex(index, JSONNumber);															// Safety check
	bool valid_green	= HasValueOfTypeAtIndex(index + 1, JSONNumber);														// Safety check
	bool valid_blue		= HasValueOfTypeAtIndex(index + 2, JSONNumber);														// Safety check
	bool valid_alpha	= HasValueOfTypeAtIndex(index + 3, JSONNumber);														// Safety check

	if (valid_red && valid_green && valid_blue && valid_alpha)
	{
		color.r = (float)json_array_get_number(json_array, index);															// Getting r from the array.
		color.g = (float)json_array_get_number(json_array, index + 1);														// Getting g from the array.
		color.b = (float)json_array_get_number(json_array, index + 2);														// Getting b from the array.
		color.a = (float)json_array_get_number(json_array, index + 3);														// Getting a from the array.
	}
}

void ParsonArray::GetFloat3(const uint& index, float3& vec3) const
{
	bool valid_x = HasValueOfTypeAtIndex(index, JSONNumber);																// Safety check
	bool valid_y = HasValueOfTypeAtIndex(index + 1, JSONNumber);															// Safety check
	bool valid_z = HasValueOfTypeAtIndex(index + 2, JSONNumber);															// Safety check

	if (valid_x && valid_y && valid_z)
	{
		vec3.x = (float)json_array_get_number(json_array, index);															// Getting x from the array.
		vec3.y = (float)json_array_get_number(json_array, index + 1);														// Getting y from the array.
		vec3.z = (float)json_array_get_number(json_array, index + 2);														// Getting z from the array.
	}
}

void ParsonArray::GetFloat4(const uint& index, float4& vec4) const
{
	bool valid_x = HasValueOfTypeAtIndex(index, JSONNumber);																// Safety check
	bool valid_y = HasValueOfTypeAtIndex(index + 1, JSONNumber);															// Safety check
	bool valid_z = HasValueOfTypeAtIndex(index + 2, JSONNumber);															// Safety check
	bool valid_w = HasValueOfTypeAtIndex(index + 3, JSONNumber);															// Safety check

	if (valid_x && valid_y && valid_z && valid_w)
	{
		vec4.x = (float)json_array_get_number(json_array, index);															// Getting x from the array.
		vec4.y = (float)json_array_get_number(json_array, index + 1);														// Getting y from the array.
		vec4.z = (float)json_array_get_number(json_array, index + 2);														// Getting z from the array.
		vec4.w = (float)json_array_get_number(json_array, index + 3);														// Getting w from the array.
	}
}

void ParsonArray::GetFloat4(const uint& index, Quat& vec4) const
{
	bool valid_x = HasValueOfTypeAtIndex(index, JSONNumber);																// Safety check
	bool valid_y = HasValueOfTypeAtIndex(index + 1, JSONNumber);															// Safety check
	bool valid_z = HasValueOfTypeAtIndex(index + 2, JSONNumber);															// Safety check
	bool valid_w = HasValueOfTypeAtIndex(index + 3, JSONNumber);															// Safety check

	if (valid_x && valid_y && valid_z && valid_w)
	{
		vec4.x = (float)json_array_get_number(json_array, index);															// Getting x from the array.
		vec4.y = (float)json_array_get_number(json_array, index + 1);														// Getting y from the array.
		vec4.z = (float)json_array_get_number(json_array, index + 2);														// Getting z from the array.
		vec4.w = (float)json_array_get_number(json_array, index + 3);														// Getting w from the array.
	}
}

ParsonNode ParsonArray::GetNode(const uint& index) const
{
	if (!HasValueOfTypeAtIndex(index, JSONObject))
	{
		LOG("[ERROR] JSON Parser: Could not get the Node at %u index in the %s Array!", name);
	}

	return json_array_get_object(json_array, index);
}

void ParsonArray::SetNumber(const double& number)
{
	JSON_Status status = json_array_append_number(json_array, number);

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
	JSON_Status status = json_array_append_string(json_array, string);

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
	JSON_Status status = json_array_append_boolean(json_array, value);

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
	JSON_Status status_r = json_array_append_number(json_array, color.r);													// Adding the RGBA variables to the JSON_Array.
	JSON_Status status_g = json_array_append_number(json_array, color.g);													// JSON_Status will be used to check whether or
	JSON_Status status_b = json_array_append_number(json_array, color.b);													// not the operation was a success.
	JSON_Status status_a = json_array_append_number(json_array, color.a);													// --------------------------------------------

	(status_r == JSONFailure) ? LOG("[ERROR] JSON Parser: Could not append Red to %s Array!", name)		: ++size;			// If an operation was not successful then an ERROR is sent
	(status_g == JSONFailure) ? LOG("[ERROR] JSON Parser: Could not append Green to %s Array!", name)	: ++size;			// to the console.
	(status_b == JSONFailure) ? LOG("[ERROR] JSON Parser: Could not append Blue to %s Array!", name)	: ++size;			// On success the size variable will be updated.
	(status_a == JSONFailure) ? LOG("[ERROR] JSON Parser: Could not append Alpha to %s Array!", name)	: ++size;			// ---------------------------------------------------------
}

void ParsonArray::SetFloat3(const math::float3& vec3)
{
	JSON_Status status_x = json_array_append_number(json_array, vec3.x);													// Adding the XYZ variables to the JSON_Array.
	JSON_Status status_y = json_array_append_number(json_array, vec3.y);													//
	JSON_Status status_z = json_array_append_number(json_array, vec3.z);													// -------------------------------------------

	(status_x == JSONFailure) ? LOG("[ERROR] JSON Parser: Could not append X to %s Array!", name) : ++size;					// If an operation was not successful then an ERROR is sent
	(status_y == JSONFailure) ? LOG("[ERROR] JSON Parser: Could not append Y to %s Array!", name) : ++size;					// to the console. On success the size var. will be updated.
	(status_z == JSONFailure) ? LOG("[ERROR] JSON Parser: Could not append Z to %s Array!", name) : ++size;					// ---------------------------------------------------------
}

void ParsonArray::SetFloat4(const math::float4& vec4)
{
	JSON_Status status_x = json_array_append_number(json_array, vec4.x);													// Adding the XYZW variables to the JSON_Array.
	JSON_Status status_y = json_array_append_number(json_array, vec4.y);													//
	JSON_Status status_z = json_array_append_number(json_array, vec4.z);													//
	JSON_Status status_w = json_array_append_number(json_array, vec4.w);													// --------------------------------------------

	(status_x == JSONFailure) ? LOG("[ERROR] JSON Parser: Could not append X to %s Array!", name) : ++size;					// If an operation was not successful then an ERROR is sent
	(status_y == JSONFailure) ? LOG("[ERROR] JSON Parser: Could not append Y to %s Array!", name) : ++size;					// to the console.
	(status_z == JSONFailure) ? LOG("[ERROR] JSON Parser: Could not append Z to %s Array!", name) : ++size;					// On success the size variable will be updated.
	(status_w == JSONFailure) ? LOG("[ERROR] JSON Parser: Could not append W to %s Array!", name) : ++size;					// --------------------------------------------------------
}

ParsonNode ParsonArray::SetNode(const char* name)
{	
	JSON_Status status = json_array_append_value(json_array, json_value_init_object());

	(status == JSONFailure) ? LOG("[ERROR] JSON Parser: Could not append Node to %s Array!", name) : ++size;

	return ParsonNode(json_array_get_object(json_array, size - 1));															// As the object was just appended, it will be located at the end.
}

JSON_Value_Type ParsonArray::GetTypeAtIndex(const uint& index) const
{
	return json_value_get_type(json_array_get_value(json_array, index));
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
	return (json_array != NULL);
}