#ifndef __TEXTURE_SETTINGS_H__
#define __TEXTURE_SETTINGS_H__

#include "ImportSettings.h"

class ParsonNode;

class TextureSettings : public ImportSettings
{
public:
	TextureSettings();
	~TextureSettings();

	bool Save(ParsonNode& settings) const override;
	bool Load(const ParsonNode& settings) override;

private:

};

#endif // !__TEXTURE_SETTINGS_H__