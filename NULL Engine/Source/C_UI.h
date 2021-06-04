#ifndef __C_UI_H__
#define __C_UI_H__

#include "Component.h"
#include "Spritesheet.h"

struct MISSCLICK_API Rect2D
{
	float x, y, w, h;
};

class MISSCLICK_API C_UI : public Component
{
public:

	C_UI(GameObject* owner, ComponentType type, bool interactuable = false, Rect2D rect = { 0,0,50,50 });

	virtual ~C_UI();

	virtual bool Update() override = 0;
	virtual bool CleanUp() override = 0;

	virtual bool SaveState(ParsonNode& root) const override = 0;
	virtual bool LoadState(ParsonNode& root) override = 0;

	//static inline ComponentType GetType() { return type; }
	virtual void HandleInput(C_UI** selectedUi) = 0;
	virtual void Draw2D() = 0;
	virtual void Draw3D() = 0;

	bool Interactuable()const;

public:

	Rect2D GetRect() const;
	void SetRect(const Rect2D& rect);
	void SetX(const float x);
	void SetY(const float y);
	void SetW(const float w);
	void SetH(const float h);

protected:
	Frame GetTexturePosition(int pixelPosX, int pixelPosY, int pixelWidth, int pixelHeight);
	Frame GetTexturePosition(int pixelPosX, int pixelPosY, int pixelWidth, int pixelHeight, int textW, int textH);
protected:
	Rect2D rect = { 0,0,1,1 };
	short childOrder = -1;
	virtual void ResetInput() {}
	bool interactuable = false;

	friend class M_UISystem;
	friend class GameObject;
	friend class C_Canvas;
};

#endif // !__C_UI_H__