#ifndef __BONE_LINK_H__
#define __BONE_LINK_H__

#include <string>
#include <vector>

#include "Channel.h"

class GameObject;

struct BoneLink
{
	BoneLink() : channel(Channel()), gameObject(nullptr) {}
	BoneLink(const Channel& channel, GameObject* gameObject) : channel(channel), gameObject(gameObject) {}

	Channel channel;
	GameObject* gameObject;
};

#endif // !__BONE_LINK_H__