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
		void Import	(const aiAnimation* ai_animation, R_Animation* r_animation);
		uint Save	(const R_Animation* r_animation, char** buffer);
		bool Load	(const char* buffer, R_Animation* r_animation);

		namespace Utilities
		{
			void GetPositionKeys		(const aiNodeAnim* ai_channel, Channel& r_channel);
			void GetRotationKeys		(const aiNodeAnim* ai_channel, Channel& r_channel);
			void GetScaleKeys			(const aiNodeAnim* ai_channel, Channel& r_channel);

			uint GetChannelsDataSize	(const R_Animation* r_animation);
			void StoreChannelName		(const Channel& r_channel, char** cursor);
			void StorePositionKeysData	(const Channel& r_channel, char** cursor);
			void StoreRotationKeysData	(const Channel& r_channel, char** cursor);
			void StoreScaleKeysData		(const Channel& r_channel, char** cursor);

			void LoadChannelName		(char** cursor, std::string& channel_name);
			void LoadPositionKeysData	(char** cursor, Channel& r_channel);
			void LoadRotationKeysData	(char** cursor, Channel& r_channel);
			void LoadScaleKeysData		(char** cursor, Channel& r_channel);
		}
	}
}

#endif // !__I_ANIMATIONS_H__