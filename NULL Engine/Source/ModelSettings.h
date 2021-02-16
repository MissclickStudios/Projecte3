#ifndef __MODEL_SETTINGS_H__
#define __MODEL_SETTINGS_H__

#include "ImportSettings.h"

class ParsonNode;

class ModelSettings : public ImportSettings
{
public:
	ModelSettings();
	~ModelSettings();

	bool Save(ParsonNode& settings) const override;
	bool Load(const ParsonNode& settings) override;

private:

};

#endif // !__MODEL_SETTINGS_H__