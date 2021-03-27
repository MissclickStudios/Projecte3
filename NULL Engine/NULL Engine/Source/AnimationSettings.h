#ifndef __ANIMATION_SETTINGS_H__
#define __ANIMATION_SETTINGS_H__

#include "ImportSettings.h"

class ParsonNode;

class AnimationSettings : public ImportSettings
{
public:
	AnimationSettings();
	~AnimationSettings();

	bool Save(ParsonNode& settings) const override;
	bool Load(const ParsonNode& settings) override;

private:

};

#endif // !__ANIMATION_SETTINGS_H__