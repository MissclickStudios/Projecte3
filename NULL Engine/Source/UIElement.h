#ifndef __UIELEMENT_H__
#define __UIELEMENT_H__

class C_Canvas;

enum class UIElementType
{
	NONE,
	BUTTON,
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

	UIElement(C_Canvas* canvas, UIElementType type, bool isActive = true);
	virtual ~UIElement();

	virtual bool Update();
	virtual bool CleanUp();

	virtual UIElementType GetType() const { return type; }	

	C_Canvas* GetCanvas() const;
	
	void SetCanvas(C_Canvas* canvas);

	//Temp
	int uiLayer = 0;
	
private:

	bool isActive;

	UIElementType type;
	C_Canvas* canvas;
	bool isDraggable;

};

#endif // !__UIELEMENT_H__