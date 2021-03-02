#ifndef __UIELEMENT_H__
#define __UIELEMENT_H__

#include "C_Canvas.h" //For canvas and for rect

enum class UIElementType
{
	NONE,
	TEXT,
	IMAGE
};

enum class UIElementState
{
	NONE,
	HOVERED,
	PRESSED,
	RELEASED,
};

class UIElement
{
public:

	UIElement(C_Canvas* canvas, UIElementType type, Rect rect, bool isActive = true);
	virtual ~UIElement();

	virtual bool Update();
	virtual bool CleanUp();

	virtual UIElementType GetType() const { return type; }	

public:

	const char* GetNameFromType() const;

	C_Canvas* GetCanvas() const;
	Rect GetRect() const;
	float GetX() const;
	float GetY() const;
	float GetW() const;
	float GetH() const;
	bool IsActive() const;

	
	void SetCanvas(C_Canvas* canvas);
	void SetRect(const Rect& rect);
	void SetX(const float x);
	void SetY(const float y);
	void SetW(const float w);
	void SetH(const float h);
	void SetIsActive(const bool& setTo);

private:

	bool isActive;

	UIElementType type;
	C_Canvas* canvas;
	bool isDraggable;
	Rect rect;

};

#endif // !__UIELEMENT_H__