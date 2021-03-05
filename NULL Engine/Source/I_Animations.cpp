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

void Importer::Animations::Import(const aiAnimation* assimpAnimation, R_Animation* rAnimation)
{
	if (rAnimation == nullptr)
	{
		LOG("[ERROR] Importer: Could not Import Animation! Error: R_Animation* was nullptr.");
		return;
	}
	if (assimpAnimation == nullptr)
	{
		LOG("[ERROR] Importer: Could not Import Animation { %s }! Error: aiAnimation* was nullptr.", rAnimation->GetAssetsFile());
		return;
	}

	rAnimation->SetName(assimpAnimation->mName.C_Str());
	rAnimation->SetDuration(assimpAnimation->mDuration);
	rAnimation->SetTicksPerSecond(assimpAnimation->mTicksPerSecond);

	for (uint i = 0; i < assimpAnimation->mNumChannels; ++i)
	{
		aiNodeAnim* aiChannel	= assimpAnimation->mChannels[i];
		Channel rChannel		= Channel(aiChannel->mNodeName.C_Str());
		
		Utilities::GetPositionKeys(aiChannel, rChannel);
		Utilities::GetRotationKeys(aiChannel, rChannel);
		Utilities::GetScaleKeys(aiChannel, rChannel);

		rAnimation->channels.push_back(rChannel);
	}
}

void Importer::Animations::Utilities::GetPositionKeys(const aiNodeAnim* aiChannel, Channel& rChannel)
{
	for (uint i = 0; i < aiChannel->mNumPositionKeys; ++i)
	{
		aiVectorKey pk	= aiChannel->mPositionKeys[i];

		double time		= pk.mTime;
		float3 position = float3(pk.mValue.x, pk.mValue.y, pk.mValue.z);

		rChannel.positionKeyframes.emplace(time, position);
	}
}

void Importer::Animations::Utilities::GetRotationKeys(const aiNodeAnim* aiChannel, Channel& rChannel)
{
	for (uint i = 0; i < aiChannel->mNumRotationKeys; ++i)
	{
		aiQuatKey rk	= aiChannel->mRotationKeys[i];

		double time		= rk.mTime;
		Quat rotation	= Quat(rk.mValue.x, rk.mValue.y, rk.mValue.z, rk.mValue.w);

		rChannel.rotationKeyframes.emplace(time, rotation);
	}
}

void Importer::Animations::Utilities::GetScaleKeys(const aiNodeAnim* aiChannel, Channel& rChannel)
{
	for (uint i = 0; i < aiChannel->mNumScalingKeys; ++i)
	{
		aiVectorKey sk	= aiChannel->mScalingKeys[i];

		double time		= sk.mTime;
		float3 scale	= float3(sk.mValue.x, sk.mValue.y, sk.mValue.z);

		rChannel.scaleKeyframes.emplace(time, scale);
	}
}

uint Importer::Animations::Save(const R_Animation* rAnimation, char** buffer)
{
	if (rAnimation == nullptr)
	{
		LOG("[ERROR] Animations Importer: Could not Save Animation to Library! Error: R_Animation was nullptr.");
		return 0;
	}
	
	uint written	= 0;
	uint size		= 0;

	// --- CALCULATING THE REQUIRED BUFFER SIZE ---
	uint channelsDataSize = Utilities::GetChannelsDataSize(rAnimation);

	double headerData[HEADER_SIZE] = {
		(double)strlen(rAnimation->GetName()),
		rAnimation->GetDuration(),
		rAnimation->GetTicksPerSecond(),
		(double)rAnimation->channels.size(),
		(double)channelsDataSize
	};

	uint headerDataSize	= sizeof(headerData) /*+ sizeof(double)*/;
	uint nameDataSize		= (uint)headerData[0] * sizeof(char);
	size					= headerDataSize + nameDataSize + channelsDataSize;
	if (size == 0)
	{
		LOG("[WARNING] Animation { %s } with UID [%lu] had no data to Save!", rAnimation->GetAssetsFile(), rAnimation->GetUID());
		return 0;
	}

	// --- STORING THE DATA IN A BUFFER ---
	*buffer			= new char[size];
	char* cursor	= *buffer;
	uint bytes		= 0;

	// --- HEADER DATA
	bytes = headerDataSize;
	memcpy_s(cursor, size, (const void*)headerData, bytes);
	cursor += bytes;

	bytes = nameDataSize;
	memcpy_s(cursor, size, (const void*)rAnimation->GetName(), bytes);
	cursor += bytes;

	// --- CHANNELS DATA
	//uint vec_key_data_size	= sizeof(double) + (sizeof(float) * 3);
	//uint quat_key_data_size = sizeof(double) + (sizeof(float) * 4);

	for (uint i = 0; i < rAnimation->channels.size(); ++i)
	{
		Channel rChannel = rAnimation->channels[i];

		Utilities::StoreChannelName(rChannel, &cursor);
		Utilities::StorePositionKeysData(rChannel, &cursor);
		Utilities::StoreRotationKeysData(rChannel, &cursor);
		Utilities::StoreScaleKeysData(rChannel, &cursor);
	}

	// --- SAVING THE BUFFER ---

	std::string path	= ANIMATIONS_PATH + std::to_string(rAnimation->GetUID()) + ANIMATIONS_EXTENSION;
	written				= App->fileSystem->Save(path.c_str(), *buffer, size);

	if (written > 0)
	{
		LOG("[STATUS] Animations Importer: Successfully Saved Animation { %s } to Library! Path: %s", rAnimation->GetAssetsFile(), path.c_str());
	}
	else
	{
		LOG("[ERROR] Animations Importer: Could not Save Animation { %s } to Library! Error: File System could not Write File.", rAnimation->GetAssetsFile());
	}

	return written;
}

bool Importer::Animations::Load(const char* buffer, R_Animation* rAnimation)
{
	bool ret = true;
	
	if (rAnimation == nullptr)
	{
		LOG("[ERROR] Importer: Could not Load Animation from Library! Error: R_Animation* was nullptr.");
		return false;
	}
	
	if (buffer == nullptr)
	{
		LOG("[ERROR] Importer: Could not load Animation { %s } from Library! Error: Buffer was nullptr", rAnimation->GetAssetsFile());
		return false;
	}

	char* cursor	= (char*)buffer;
	uint bytes		= 0;

	// --- HEADER DATA ---
	double headerData[HEADER_SIZE];

	bytes = sizeof(headerData);
	memcpy(headerData, cursor, bytes);
	cursor += bytes;

	std::string name = "";
	name.resize((uint)headerData[0]);
	bytes = (uint)headerData[0] * sizeof(char);
	memcpy(&name[0], cursor, bytes);
	cursor += bytes;

	rAnimation->SetName(name.c_str());
	rAnimation->SetDuration(headerData[1]);
	rAnimation->SetTicksPerSecond(headerData[2]);

	// --- CHANNELS DATA ---
	rAnimation->channels.resize((uint)headerData[3]);
	for (uint i = 0; i < rAnimation->channels.size(); ++i)
	{
		std::string channelName = "";
		Utilities::LoadChannelName(&cursor, channelName);

		Channel rChannel = Channel(channelName.c_str());
		
		Utilities::LoadPositionKeysData(&cursor, rChannel);
		Utilities::LoadRotationKeysData(&cursor, rChannel);
		Utilities::LoadScaleKeysData(&cursor, rChannel);

		rAnimation->channels[i] = rChannel;
	}

	return ret;
}

uint Importer::Animations::Utilities::GetChannelsDataSize(const R_Animation* rAnimation)
{
	uint channelsSize = 0;																									// Will contain the total size of all the animation's channels.
	
	if (rAnimation == nullptr)
	{
		LOG("[ERROR] Animations Importer: Could not get Channels Size Of Data! Error: Argument R_Animation* was nullptr");
		return 0;
	}

	uint vecKeySize	= sizeof(double) + (sizeof(float) * 3);																// float3 is composed by 3 floats.
	uint quatKeySize	= sizeof(double) + (sizeof(float) * 4);																// Quat is composed by 4 floats.
	for (uint i = 0; i < rAnimation->channels.size(); ++i)
	{
		const Channel& rChannel = rAnimation->channels[i];
		uint channelSize = 0;

		channelSize += sizeof(uint) * 4;																					// Length of the name and sizes of the keys maps.
		channelSize += (strlen(rChannel.name.c_str()) * sizeof(char));
		channelSize += rChannel.positionKeyframes.size() * vecKeySize;
		channelSize += rChannel.rotationKeyframes.size() * quatKeySize;
		channelSize += rChannel.scaleKeyframes.size() * vecKeySize;

		channelsSize += channelSize;
	}

	return channelsSize;
}

void Importer::Animations::Utilities::StoreChannelName(const Channel& rChannel, char** cursor)
{
	uint bytes		= 0;
	uint nameLength	= rChannel.name.length();

	bytes = sizeof(uint);
	memcpy(*cursor, (const void*)&nameLength, bytes);
	*cursor += bytes;
	
	bytes	= nameLength * sizeof(char);
	memcpy(*cursor, (const void*)rChannel.name.c_str(), bytes);
	*cursor	+= bytes;
}

void Importer::Animations::Utilities::StorePositionKeysData(const Channel& rChannel, char** cursor)
{
	uint bytes		= 0;
	uint pkSize	= rChannel.positionKeyframes.size();

	bytes = sizeof(uint);
	memcpy(*cursor, (const void*)&pkSize, bytes);
	*cursor += bytes;

	bytes = sizeof(double) + (sizeof(float) * 3);																			// Data Size of a Position Keyframe.
	std::map<double, float3>::const_iterator item;
	for (item = rChannel.positionKeyframes.begin(); item != rChannel.positionKeyframes.end(); ++item)
	{
		memcpy(*cursor, (const void*)&item->first, sizeof(double));
		*cursor += sizeof(double);
		
		memcpy(*cursor, (const void*)&item->second, sizeof(float) * 3);
		*cursor += sizeof(float) * 3;
	}
}

void Importer::Animations::Utilities::StoreRotationKeysData(const Channel& rChannel, char** cursor)
{
	uint bytes		= 0;
	uint rkSize	= rChannel.rotationKeyframes.size();

	bytes = sizeof(uint);
	memcpy(*cursor, (const void*)&rkSize, bytes);
	*cursor += bytes;
	
	bytes = sizeof(double) + (sizeof(float) * 4);																			// Data Size of a Rotation Keyframe.
	std::map<double, Quat>::const_iterator item;
	for (item = rChannel.rotationKeyframes.begin(); item != rChannel.rotationKeyframes.end(); ++item)
	{
		memcpy(*cursor, (const void*)&item->first, sizeof(double));
		*cursor += sizeof(double);

		memcpy(*cursor, (const void*)&item->second, sizeof(float) * 4);
		*cursor += sizeof(float) * 4;
	}
}

void Importer::Animations::Utilities::StoreScaleKeysData(const Channel& rChannel, char** cursor)
{
	uint bytes = 0;
	uint skSize = rChannel.scaleKeyframes.size();

	bytes = sizeof(uint);
	memcpy(*cursor, (const void*)&skSize, bytes);
	*cursor += bytes;
	
	bytes = sizeof(double) + (sizeof(float) * 3);

	std::map<double, float3>::const_iterator item;
	for (item = rChannel.scaleKeyframes.begin(); item != rChannel.scaleKeyframes.end(); ++item)
	{
		memcpy(*cursor, (const void*)&item->first, sizeof(double));
		*cursor += sizeof(double);

		memcpy(*cursor, (const void*)&item->second, sizeof(float) * 3);
		*cursor += sizeof(float) * 3;
	}
}

void Importer::Animations::Utilities::LoadChannelName(char** cursor, std::string& channelName)
{
	uint bytes		= 0;
	uint nameLength	= 0;

	bytes = sizeof(uint);
	memcpy(&nameLength, (const void*)(*cursor), bytes);
	*cursor += bytes;
	
	channelName.resize(nameLength);
	bytes = nameLength * sizeof(char);
	memcpy(&channelName[0], *cursor, bytes);
	*cursor += bytes;
}

void Importer::Animations::Utilities::LoadPositionKeysData(char** cursor, Channel& rChannel)
{
	uint bytes		= 0;
	uint pkSize	= 0;

	bytes = sizeof(uint);
	memcpy(&pkSize, (const void*)(*cursor), bytes);
	*cursor += bytes;

	uint timeBytes		= sizeof(double);
	uint positionBytes = (sizeof(float) * 3);
	for (uint i = 0; i < pkSize; ++i)
	{
		double time			= 0.0;
		float3 position		= float3::zero;
		
		memcpy(&time, (const void*)(*cursor), timeBytes);
		*cursor += timeBytes;

		memcpy(&position, (const void*)(*cursor), positionBytes);
		*cursor += positionBytes;

		rChannel.positionKeyframes.emplace(time, position);
	}
}

void Importer::Animations::Utilities::LoadRotationKeysData(char** cursor, Channel& rChannel)
{
	uint bytes		= 0;
	uint rkSize	= 0;

	bytes = sizeof(uint);
	memcpy(&rkSize, (const void*)(*cursor), bytes);
	*cursor += bytes;

	uint timeBytes			= sizeof(double);
	uint rotationBytes		= (sizeof(float) * 4);
	for (uint i = 0; i < rkSize; ++i)
	{
		double time		= 0.0;
		Quat rotation	= Quat::identity;

		memcpy(&time, (const void*)(*cursor), timeBytes);
		*cursor += timeBytes;

		memcpy(&rotation, (const void*)(*cursor), rotationBytes);
		*cursor += rotationBytes;

		rChannel.rotationKeyframes.emplace(time, rotation);
	}
}

void Importer::Animations::Utilities::LoadScaleKeysData(char** cursor, Channel& rChannel)
{
	uint bytes		= 0;
	uint skSize	= 0;

	bytes = sizeof(uint);
	memcpy(&skSize, (const void*)(*cursor), bytes);
	*cursor += bytes;

	uint timeBytes		= sizeof(double);
	uint scaleBytes	= (sizeof(float) * 3);
	for (uint i = 0; i < skSize; ++i)
	{
		double time		= 0.0;
		float3 scale	= float3::one;

		memcpy(&time, (const void*)(*cursor), timeBytes);
		*cursor += timeBytes;

		memcpy(&scale, (const void*)(*cursor), scaleBytes);
		*cursor += scaleBytes;

		rChannel.scaleKeyframes.emplace(time, scale);
	}
}
