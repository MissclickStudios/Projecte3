#include "Assimp.h"

#include "FileSystemDefinitions.h"
#include "VariableTypedefs.h"

#include "Channel.h"

#include "Application.h"
#include "M_FileSystem.h"

#include "R_Animation.h"

#include "I_Animations.h"

#include "MemoryManager.h"

#define HEADER_SIZE 5

using namespace Importer::Animations;																	// Not a good thing to do but it will be employed sparsely and only inside this .cpp

void Importer::Animations::Import(const aiAnimation* ai_animation, R_Animation* r_animation)
{
	if (r_animation == nullptr)
	{
		LOG("[ERROR] Importer: Could not Import Animation! Error: R_Animation* was nullptr.");
		return;
	}
	if (ai_animation == nullptr)
	{
		LOG("[ERROR] Importer: Could not Import Animation { %s }! Error: aiAnimation* was nullptr.", r_animation->GetAssetsFile());
		return;
	}

	r_animation->SetName(ai_animation->mName.C_Str());
	r_animation->SetDuration(ai_animation->mDuration);
	r_animation->SetTicksPerSecond(ai_animation->mTicksPerSecond);

	for (uint i = 0; i < ai_animation->mNumChannels; ++i)
	{
		aiNodeAnim* ai_channel	= ai_animation->mChannels[i];
		Channel r_channel		= Channel(ai_channel->mNodeName.C_Str());
		
		Utilities::GetPositionKeys(ai_channel, r_channel);
		Utilities::GetRotationKeys(ai_channel, r_channel);
		Utilities::GetScaleKeys(ai_channel, r_channel);

		r_animation->channels.push_back(r_channel);
	}
}

void Importer::Animations::Utilities::GetPositionKeys(const aiNodeAnim* ai_channel, Channel& r_channel)
{
	for (uint i = 0; i < ai_channel->mNumPositionKeys; ++i)
	{
		aiVectorKey pk	= ai_channel->mPositionKeys[i];

		double time		= pk.mTime;
		float3 position = float3(pk.mValue.x, pk.mValue.y, pk.mValue.z);

		r_channel.position_keyframes.emplace(time, position);
	}
}

void Importer::Animations::Utilities::GetRotationKeys(const aiNodeAnim* ai_channel, Channel& r_channel)
{
	for (uint i = 0; i < ai_channel->mNumRotationKeys; ++i)
	{
		aiQuatKey rk	= ai_channel->mRotationKeys[i];

		double time		= rk.mTime;
		Quat rotation	= Quat(rk.mValue.x, rk.mValue.y, rk.mValue.z, rk.mValue.w);

		r_channel.rotation_keyframes.emplace(time, rotation);
	}
}

void Importer::Animations::Utilities::GetScaleKeys(const aiNodeAnim* ai_channel, Channel& r_channel)
{
	for (uint i = 0; i < ai_channel->mNumScalingKeys; ++i)
	{
		aiVectorKey sk	= ai_channel->mScalingKeys[i];

		double time		= sk.mTime;
		float3 scale	= float3(sk.mValue.x, sk.mValue.y, sk.mValue.z);

		r_channel.scale_keyframes.emplace(time, scale);
	}
}

uint Importer::Animations::Save(const R_Animation* r_animation, char** buffer)
{
	if (r_animation == nullptr)
	{
		LOG("[ERROR] Animations Importer: Could not Save Animation to Library! Error: R_Animation was nullptr.");
		return 0;
	}
	
	uint written	= 0;
	uint size		= 0;

	// --- CALCULATING THE REQUIRED BUFFER SIZE ---
	uint channels_data_size = Utilities::GetChannelsDataSize(r_animation);

	double header_data[HEADER_SIZE] = {
		(double)strlen(r_animation->GetName()),
		r_animation->GetDuration(),
		r_animation->GetTicksPerSecond(),
		(double)r_animation->channels.size(),
		(double)channels_data_size
	};

	uint header_data_size	= sizeof(header_data) /*+ sizeof(double)*/;
	uint name_data_size		= (uint)header_data[0] * sizeof(char);
	size					= header_data_size + name_data_size + channels_data_size;
	if (size == 0)
	{
		LOG("[WARNING] Animation { %s } with UID [%lu] had no data to Save!", r_animation->GetAssetsFile(), r_animation->GetUID());
		return 0;
	}

	// --- STORING THE DATA IN A BUFFER ---
	*buffer			= new char[size];
	char* cursor	= *buffer;
	uint bytes		= 0;

	// --- HEADER DATA
	bytes = header_data_size;
	memcpy_s(cursor, size, (const void*)header_data, bytes);
	cursor += bytes;

	bytes = name_data_size;
	memcpy_s(cursor, size, (const void*)r_animation->GetName(), bytes);
	cursor += bytes;

	// --- CHANNELS DATA
	//uint vec_key_data_size	= sizeof(double) + (sizeof(float) * 3);
	//uint quat_key_data_size = sizeof(double) + (sizeof(float) * 4);

	for (uint i = 0; i < r_animation->channels.size(); ++i)
	{
		Channel r_channel = r_animation->channels[i];

		Utilities::StoreChannelName(r_channel, &cursor);
		Utilities::StorePositionKeysData(r_channel, &cursor);
		Utilities::StoreRotationKeysData(r_channel, &cursor);
		Utilities::StoreScaleKeysData(r_channel, &cursor);
	}

	// --- SAVING THE BUFFER ---
	std::string path	= ANIMATIONS_PATH + std::to_string(r_animation->GetUID()) + ANIMATIONS_EXTENSION;
	written				= App->file_system->Save(path.c_str(), *buffer, size);
	if (written > 0)
	{
		LOG("[STATUS] Animations Importer: Successfully Saved Animation { %s } to Library! Path: %s", r_animation->GetAssetsFile(), path.c_str());
	}
	else
	{
		LOG("[ERROR] Animations Importer: Could not Save Animation { %s } to Library! Error: File System could not Write File.", r_animation->GetAssetsFile());
	}

	return written;
}

bool Importer::Animations::Load(const char* buffer, R_Animation* r_animation)
{
	bool ret = true;
	
	if (r_animation == nullptr)
	{
		LOG("[ERROR] Importer: Could not Load Animation from Library! Error: R_Animation* was nullptr.");
		return false;
	}
	
	if (buffer == nullptr)
	{
		LOG("[ERROR] Importer: Could not load Animation { %s } from Library! Error: Buffer was nullptr", r_animation->GetAssetsFile());
		return false;
	}

	char* cursor	= (char*)buffer;
	uint bytes		= 0;

	// --- HEADER DATA ---
	double header_data[HEADER_SIZE];

	bytes = sizeof(header_data);
	memcpy(header_data, cursor, bytes);
	cursor += bytes;

	std::string name = "";
	name.resize((uint)header_data[0]);
	bytes = (uint)header_data[0] * sizeof(char);
	memcpy(&name[0], cursor, bytes);
	cursor += bytes;

	r_animation->SetName(name.c_str());
	r_animation->SetDuration(header_data[1]);
	r_animation->SetTicksPerSecond(header_data[2]);

	// --- CHANNELS DATA ---
	r_animation->channels.resize((uint)header_data[3]);
	for (uint i = 0; i < r_animation->channels.size(); ++i)
	{
		std::string channel_name = "";
		Utilities::LoadChannelName(&cursor, channel_name);

		Channel r_channel = Channel(channel_name.c_str());
		
		Utilities::LoadPositionKeysData(&cursor, r_channel);
		Utilities::LoadRotationKeysData(&cursor, r_channel);
		Utilities::LoadScaleKeysData(&cursor, r_channel);

		r_animation->channels[i] = r_channel;
	}

	return ret;
}

uint Importer::Animations::Utilities::GetChannelsDataSize(const R_Animation* r_animation)
{
	uint channels_size = 0;																									// Will contain the total size of all the animation's channels.
	
	if (r_animation == nullptr)
	{
		LOG("[ERROR] Animations Importer: Could not get Channels Size Of Data! Error: Argument R_Animation* was nullptr");
		return 0;
	}

	uint vec_key_size	= sizeof(double) + (sizeof(float) * 3);																// float3 is composed by 3 floats.
	uint quat_key_size	= sizeof(double) + (sizeof(float) * 4);																// Quat is composed by 4 floats.
	for (uint i = 0; i < r_animation->channels.size(); ++i)
	{
		const Channel& r_channel = r_animation->channels[i];
		uint channel_size = 0;

		channel_size += sizeof(uint) * 4;																					// Length of the name and sizes of the keys maps.
		channel_size += (strlen(r_channel.name.c_str()) * sizeof(char));
		channel_size += r_channel.position_keyframes.size() * vec_key_size;
		channel_size += r_channel.rotation_keyframes.size() * quat_key_size;
		channel_size += r_channel.scale_keyframes.size() * vec_key_size;

		channels_size += channel_size;
	}

	return channels_size;
}

void Importer::Animations::Utilities::StoreChannelName(const Channel& r_channel, char** cursor)
{
	uint bytes			= 0;
	uint name_length	= r_channel.name.length();

	bytes = sizeof(uint);
	memcpy(*cursor, (const void*)&name_length, bytes);
	*cursor += bytes;
	
	bytes	= name_length * sizeof(char);
	memcpy(*cursor, (const void*)r_channel.name.c_str(), bytes);
	*cursor	+= bytes;
}

void Importer::Animations::Utilities::StorePositionKeysData(const Channel& r_channel, char** cursor)
{
	uint bytes		= 0;
	uint pk_size	= r_channel.position_keyframes.size();

	bytes = sizeof(uint);
	memcpy(*cursor, (const void*)&pk_size, bytes);
	*cursor += bytes;

	bytes = sizeof(double) + (sizeof(float) * 3);																			// Data Size of a Position Keyframe.
	std::map<double, float3>::const_iterator item;
	for (item = r_channel.position_keyframes.begin(); item != r_channel.position_keyframes.end(); ++item)
	{
		memcpy(*cursor, (const void*)&item->first, sizeof(double));
		*cursor += sizeof(double);
		
		memcpy(*cursor, (const void*)&item->second, sizeof(float) * 3);
		*cursor += sizeof(float) * 3;
	}
}

void Importer::Animations::Utilities::StoreRotationKeysData(const Channel& r_channel, char** cursor)
{
	uint bytes		= 0;
	uint rk_size	= r_channel.rotation_keyframes.size();

	bytes = sizeof(uint);
	memcpy(*cursor, (const void*)&rk_size, bytes);
	*cursor += bytes;
	
	bytes = sizeof(double) + (sizeof(float) * 4);																			// Data Size of a Rotation Keyframe.
	std::map<double, Quat>::const_iterator item;
	for (item = r_channel.rotation_keyframes.begin(); item != r_channel.rotation_keyframes.end(); ++item)
	{
		memcpy(*cursor, (const void*)&item->first, sizeof(double));
		*cursor += sizeof(double);

		memcpy(*cursor, (const void*)&item->second, sizeof(float) * 4);
		*cursor += sizeof(float) * 4;
	}
}

void Importer::Animations::Utilities::StoreScaleKeysData(const Channel& r_channel, char** cursor)
{
	uint bytes = 0;
	uint sk_size = r_channel.scale_keyframes.size();

	bytes = sizeof(uint);
	memcpy(*cursor, (const void*)&sk_size, bytes);
	*cursor += bytes;
	
	bytes = sizeof(double) + (sizeof(float) * 3);

	std::map<double, float3>::const_iterator item;
	for (item = r_channel.scale_keyframes.begin(); item != r_channel.scale_keyframes.end(); ++item)
	{
		memcpy(*cursor, (const void*)&item->first, sizeof(double));
		*cursor += sizeof(double);

		memcpy(*cursor, (const void*)&item->second, sizeof(float) * 3);
		*cursor += sizeof(float) * 3;
	}
}

void Importer::Animations::Utilities::LoadChannelName(char** cursor, std::string& channel_name)
{
	uint bytes			= 0;
	uint name_length	= 0;

	bytes = sizeof(uint);
	memcpy(&name_length, (const void*)(*cursor), bytes);
	*cursor += bytes;
	
	channel_name.resize(name_length);
	bytes	= name_length * sizeof(char);
	memcpy(&channel_name[0], *cursor, bytes);
	*cursor += bytes;
}

void Importer::Animations::Utilities::LoadPositionKeysData(char** cursor, Channel& r_channel)
{
	uint bytes		= 0;
	uint pk_size	= 0;

	bytes = sizeof(uint);
	memcpy(&pk_size, (const void*)(*cursor), bytes);
	*cursor += bytes;

	uint time_bytes		= sizeof(double);
	uint position_bytes = (sizeof(float) * 3);
	for (uint i = 0; i < pk_size; ++i)
	{
		double time			= 0.0;
		float3 position		= float3::zero;
		
		memcpy(&time, (const void*)(*cursor), time_bytes);
		*cursor += time_bytes;

		memcpy(&position, (const void*)(*cursor), position_bytes);
		*cursor += position_bytes;

		r_channel.position_keyframes.emplace(time, position);
	}
}

void Importer::Animations::Utilities::LoadRotationKeysData(char** cursor, Channel& r_channel)
{
	uint bytes		= 0;
	uint rk_size	= 0;

	bytes = sizeof(uint);
	memcpy(&rk_size, (const void*)(*cursor), bytes);
	*cursor += bytes;

	uint time_bytes			= sizeof(double);
	uint rotation_bytes		= (sizeof(float) * 4);
	for (uint i = 0; i < rk_size; ++i)
	{
		double time		= 0.0;
		Quat rotation	= Quat::identity;

		memcpy(&time, (const void*)(*cursor), time_bytes);
		*cursor += time_bytes;

		memcpy(&rotation, (const void*)(*cursor), rotation_bytes);
		*cursor += rotation_bytes;

		r_channel.rotation_keyframes.emplace(time, rotation);
	}
}

void Importer::Animations::Utilities::LoadScaleKeysData(char** cursor, Channel& r_channel)
{
	uint bytes		= 0;
	uint sk_size	= 0;

	bytes = sizeof(uint);
	memcpy(&sk_size, (const void*)(*cursor), bytes);
	*cursor += bytes;

	uint time_bytes		= sizeof(double);
	uint scale_bytes	= (sizeof(float) * 3);
	for (uint i = 0; i < sk_size; ++i)
	{
		double time		= 0.0;
		float3 scale	= float3::one;

		memcpy(&time, (const void*)(*cursor), time_bytes);
		*cursor += time_bytes;

		memcpy(&scale, (const void*)(*cursor), scale_bytes);
		*cursor += scale_bytes;

		r_channel.scale_keyframes.emplace(time, scale);
	}
}
