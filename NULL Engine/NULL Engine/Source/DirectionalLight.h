#ifndef __DIRECTIONALLIGHT_H__
#define __DIRECTIONALLIGHT_H__

#include "MathGeoTransform.h"
#include "Light.h"

class DirectionalLight : public Light
{
public:
	DirectionalLight();
	~DirectionalLight();

	void Init() override;
	void SetPos(float x, float y, float z) override;
	void Active(bool isActive) override;
	void Render() override;

	static inline LightType GetType() { return LightType::DIRECTIONAL; }
	inline float3 GetDirection() const { return direction; }

	inline void SetDirection(float3 newDirection) { direction = newDirection; }

private:

	float3 direction;

};

#endif // !__DIRECTIONALLIGHT_H__