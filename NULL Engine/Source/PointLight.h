#ifndef __POINTLIGHT_H__
#define __POINTLIGHT_H__

#include "MathGeoTransform.h"
#include "Light.h"

class PointLight : public Light
{
public:
	PointLight();
	~PointLight();

	void Init() override;
	void SetPos(float x, float y, float z) override;
	void Active(bool isActive) override;
	void Render() override;

	static inline LightType GetType() { return LightType::POINTLIGHT; }

	inline float3 GetPosition() const { return position; }
	inline void SetPosition(float3 newPosition) { position = newPosition; }

	inline float GetConstant() const { return constant; }
	inline void SetConstant(float newConstant) { constant = newConstant; }

	inline float GetLinear() const { return linear; }
	inline void SetLinear(float newLinear) { linear = newLinear; }

	inline float GetQuadratic() const { return quadratic; }
	inline void SetQuadratic(float newQuadratic) { quadratic = newQuadratic; }


private:

	float3 position;

	float constant;
	float linear;
	float quadratic;

};

#endif // !__POINTLIGHT_H__
