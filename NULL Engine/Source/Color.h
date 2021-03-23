#ifndef __COLOR_H__
#define __COLOR_H__

#include "Macros.h"

namespace math
{
	class float4;
}

struct NULL_API Color
{
	float r, g, b, a;
	
	Color();
	Color(float r, float g, float b, float a = 1.0f);

	// --- GET/SET
	Color	Get				();												// Will Return the Color vector in Range: 0.0f ~ 1.0f. Will be checked and mod. to be within the limits before return.
	Color	GetRGB255		();												// Will Return the Color vector in Range: 0.0f ~ 255.0f. Will be checked & mod. to be within the limits before return.
	float*	C_Array			();

	void	Set				(float r, float g, float b, float a = 1.0f);	// Will set the Color vector with the given RGBA values. Will be checked and mod. to be within limits. 
	void	Set				(const Color& color);							// Both Set() methods expec the RGBA values to be in Range: 0.0f ~ 1.0f.
	void	SetFromRGB255	(const Color& color);							// Will set the Color vector with the given RGBA values in Range: 0.0f ~ 255.0f in the passed Color vector.
	
	void	SetWithinLimits	();												// Will modify this vector to set the RGBA values within Range: 0.0f ~ 1.0f. Used in Gets and Sets.

	// --- OPERATORS
	float*	operator &();													// Returns a float* of the vector.
	 float* operator &() const;													// Returns a float* of the vector.

	Color	operator +(const Color& color);									// col.r = r + color.r, ... .	The values are expected to be within Range: 0.0f ~ 1.0f.
	Color	operator -(const Color& color);									// col.r = r - color.r, ... .	The values are expected to be within Range: 0.0f ~ 1.0f.
	Color	operator *(float scalar);										// col.r = r * color.r, ... .	Remember that the value of the vector has to remain in Range: 0.0f ~ 1.0f after the ops.
	Color	operator /(float scalar);										// col.r = r / color.r, ... .	Remember that the value of the vector has to remain in Range: 0.0f ~ 1.0f after the ops.

	Color	operator +=(const Color& color);								// r += color.r, ... .			The values are expected to be within Range: 0.0f ~ 1.0f.
	Color	operator -=(const Color& color);								// r -= color.r, ... .			The values are expected to be within Range: 0.0f ~ 1.0f.
	Color	operator *=(float scalar);										// r *= color.r, ... .			Remember that the value of the vector has to remain in Range: 0.0f ~ 1.0f after the ops.
	Color	operator /=(float scalar);										// r /= color.t. ... .			Remember that the value of the vector has to remain in Range: 0.0f ~ 1.0f after the ops.

	Color	operator =(const Color& color);									// this = color; Necessary?
	Color	operator =(const math::float4& color);									// this = color; Necessary?
	bool	operator ==(const Color& color);								// if (this.r == color.r && ... && this.a == this.a), then return true.
};

extern NULL_API Color Red;
extern NULL_API Color Green;
extern NULL_API Color Blue;
extern NULL_API Color Cyan;
extern NULL_API Color Magenta;
extern NULL_API Color Yellow;
extern NULL_API Color Orange;
extern NULL_API Color Pink;
extern NULL_API Color Black;
extern NULL_API Color White;

#endif // !__COLOR_H__