#ifndef __UIELEMENT_H__
#define __UIELEMENT_H__

class C_Canvas;

enum class UIElementType
{
	NONE,
	BUTTON
};

class UIElement
{
public:

	UIElement(C_Canvas* owner, UIElementType type, bool isActive = true);
	virtual ~UIElement();

	virtual bool Update();
	virtual bool CleanUp();

	virtual UIElementType GetType() const { return type; }	

	C_Canvas* GetOwner() const;

private:

	bool isActive;

	UIElementType type;
	C_Canvas* owner;
};

#endif // !__UIELEMENT_H__