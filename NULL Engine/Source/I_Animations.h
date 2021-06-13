#ifndef __I_ANIMATIONS_H__
#define __I_ANIMATIONS_H__

#include <string>

struct aiAnimation;
struct aiNodeAnim;

struct Channel;
class R_Animation;

typedef unsigned int uint;

namespace Importer
{
	namespace Animations
	{
		void Import	(const aiAnimation* assimpAnimation, R_Animation* rAnimation);											// 
		uint Save	(const R_Animation* rAnimation, char** buffer);															// 
		bool Load	(const char* buffer, R_Animation* rAnimation);															// 

		namespace Utilities
		{
			void GetPositionKeys		(const aiNodeAnim* aiChannel, Channel& rChannel);									// 
			void GetRotationKeys		(const aiNodeAnim* aiChannel, Channel& rChannel);									// 
			void GetScaleKeys			(const aiNodeAnim* aiChannel, Channel& rChannel);									// 

			uint GetChannelsDataSize	(const R_Animation* rAnimation);													// 
			void StoreChannelName		(const Channel& rChannel, char** cursor);											// 
			void StorePositionKeysData	(const Channel& rChannel, char** cursor);											// 
			void StoreRotationKeysData	(const Channel& rChannel, char** cursor);											// 
			void StoreScaleKeysData		(const Channel& rChannel, char** cursor);											// 

			void LoadChannelName		(char** cursor, std::string& channelName);											// 
			void LoadPositionKeysData	(char** cursor, Channel& rChannel);													// 
			void LoadRotationKeysData	(char** cursor, Channel& rChannel);													// 
			void LoadScaleKeysData		(char** cursor, Channel& rChannel);													// 

			void ValidateChannel		(Channel& rChannel);																// Detects and erases any discrepancies with the given channel.
			void FuseChannels			(const Channel& newChannel, Channel& existingChannel);								// Fuses channels with the same name. New are fused into existing.
		}
	}
}

#endif // !__I_ANIMATIONS_H__