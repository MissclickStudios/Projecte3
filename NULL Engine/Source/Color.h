#ifndef __COLOR_H__
#define __COLOR_H__

#include "Macros.h"

namespace math
{
	class float4;
}

struct MISSCLICK_API Color
{
	float r, g, b, a;
	
	Color();
	Color(float r, float g, float b, float a = 1.0f);
	Color(float* color);

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

extern MISSCLICK_API Color Red;
extern MISSCLICK_API Color Green;
extern MISSCLICK_API Color Blue;
extern MISSCLICK_API Color Cyan;
extern MISSCLICK_API Color Magenta;
extern MISSCLICK_API Color Yellow;
extern MISSCLICK_API Color Orange;
extern MISSCLICK_API Color Pink;
extern MISSCLICK_API Color Purple;
extern MISSCLICK_API Color Black;
extern MISSCLICK_API Color White;

extern MISSCLICK_API Color LightRed;
extern MISSCLICK_API Color LightGreen;
extern MISSCLICK_API Color LightBlue;
extern MISSCLICK_API Color LightCyan;
extern MISSCLICK_API Color LightMagenta;
extern MISSCLICK_API Color LightYellow;

#endif // !__COLOR_H__