#ifndef __AUDIO_H__
#define __AUDIO_H__

#include "Module.h"

class M_Audio : public Module
{
public:
	M_Audio(bool isActive = true);
	~M_Audio();

	bool Start() override;
	UpdateStatus Update(float dt) override;
	bool CleanUp() override;

	bool InitSoundEngine();

	void TermSoundEngine();

private:

};

#endif //__AUDIO_H__