#include "Color.h"

Color::Color() : r(0.0f), g(0.0f), b(0.0f), a(1.0f)
{

}

Color::Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a)
{

}

// --- GET/SET METHODS
Color Color::Get()
{
	SetWithinLimits();															// Safety check so the elements inside the Color vector are within the correct range.

	return *this;
}

Color Color::GetRGB255()
{
	SetWithinLimits();

	Color rgb255;

	rgb255.r = r * 255.0f;
	rgb255.g = g * 255.0f;
	rgb255.b = b * 255.0f;
	rgb255.a = a * 255.0f;

	return rgb255;
}

float* Color::C_Array()
{
	return &r;
}

void Color::Set(float r, float g, float b, float a)
{
	if (r > 1.0f || g > 1.0f || b > 1.0f || a > 1.0f)
	{
		SetFromRGB255(Color(r, g, b, a));
		return;
	}
	
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;

	SetWithinLimits();
}

void Color::Set(const Color& color)
{
	if (color.r > 1.0f || color.g > 1.0f || color.b > 1.0f || color.a > 1.0f)
	{
		SetFromRGB255(color);
		return;
	}
	
	r = color.r;
	g = color.g;
	b = color.b;
	a = color.a;

	SetWithinLimits();
}

void Color::SetFromRGB255(const Color& color)
{
	r = color.r / 255.0f;
	g = color.g / 255.0f;
	b = color.b / 255.0f;
	a = color.a / 255.0f;

	SetWithinLimits();
}

void Color::SetWithinLimits()
{	
	r = (r < 0.0f ? 0.0f : r);								// ----------------------------------------
	g = (g < 0.0f ? 0.0f : g);								// If value is less than 0.0f, set to 0.0f.
	b = (b < 0.0f ? 0.0f : b);								// In case rgba exceeds the lowest value.
	a = (a < 0.0f ? 0.0f : a);								// ----------------------------------------

	r = (r > 1.0f ? 1.0f : r);								// ----------------------------------------
	g = (g > 1.0f ? 1.0f : g);								// If value is more than 1.0f, set to 1.0f.
	b = (b > 1.0f ? 1.0f : b);								// In case rgba exceeds the lowest value.
	a = (a > 1.0f ? 1.0f : a);								// ----------------------------------------
}

// --- OPERATOR METHODS
float* Color::operator &()
{
	return (float*)this;
}

Color Color::operator +(const Color& color)
{	
	Color col;

	col.r = r + color.r;
	col.g = g + color.g;
	col.b = b + color.b;
	col.a = a + color.a;

	return col;
}

Color Color::operator -(const Color& color)
{
	Color col;
	
	col.r = r - color.r;
	col.g = g - color.g;
	col.b = b - color.b;
	col.a = a - color.a;

	return col;
}

Color Color::operator *(const float& scalar)
{
	Color col;
	
	col.r = r * scalar;
	col.g = g * scalar;
	col.b = b * scalar;
	col.a = a * scalar;

	return col;
}

Color Color::operator /(const float& scalar)
{
	Color col;

	col.r = r / scalar;
	col.g = g / scalar;
	col.b = b / scalar;
	col.a = a / scalar;

	return col;
}

Color Color::operator +=(const Color& color)
{
	r += color.r;
	g += color.g;
	b += color.b;
	a += color.a;

	return *this;
}

Color Color::operator -=(const Color& color)
{
	r -= color.r;
	g -= color.g;
	b -= color.b;
	a -= color.a;

	return *this;
}

Color Color::operator *=(const float& scalar)
{
	r *= scalar;
	g *= scalar;
	b *= scalar;
	a *= scalar;

	return *this;
}

Color Color::operator /=(const float& scalar)
{
	r /= scalar;
	g /= scalar;
	b /= scalar;
	a /= scalar;

	return *this;
}

Color Color::operator =(const Color& color)
{
	r = color.r;
	g = color.g;
	b = color.b;
	a = color.a;

	return *this;
}

bool Color::operator ==(const Color& color)
{
	return (r == color.r && g == color.g && b == color.b && a == color.a);
}

Color Red		= Color(1.0f, 0.0f, 0.0f);
Color Green		= Color(0.0f, 1.0f, 0.0f);
Color Blue		= Color(0.0f, 0.0f, 1.0f);
Color Cyan		= Color(0.0f, 1.0f, 1.0f);
Color Magenta	= Color(1.0f, 0.0f, 1.0f);
Color Yellow	= Color(1.0f, 1.0f, 0.0f);
Color Orange	= Color(1.0f, 0.3f, 0.0f);
Color Pink		= Color(1.0f, 0.7f, 0.7f);
Color Black		= Color(0.0f, 0.0f, 0.0f);
Color White		= Color(1.0f, 1.0f, 1.0f);