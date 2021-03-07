#ifndef __BONE_LINK_H__
#define __BONE_LINK_H__

#include <string>
#include <vector>

#include "Channel.h"

class GameObject;

struct BoneLink
{
	BoneLink();
	BoneLink(const Channel& channel, GameObject* gameObject);

	Channel channel;
	GameObject* gameObject;
};

#endif // !__BONE_LINK_H__