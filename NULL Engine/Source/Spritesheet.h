#ifndef __SPRITESHEET_H__
#define __SPRITESHEET_H__

class R_Texture;

struct Frame
{
	float proportionBeginX;
	float proportionBeginY;
	float proportionFinalX;
	float proportionFinalY;
};

class Spritesheet
{
public:
	Spritesheet(R_Texture* spritesheet);
	~Spritesheet();

	void SetSpritesheetSize(int s_rows, int s_columns, int s_pisxelHeight, int s_pixelLenght);
	void SetCurrentFrameLocation(int frameNumber);
	Frame* GetAtlasPosition(int pixelPosX, int pixelPosY, int pixelWidth, int pixelHeight);

	Frame currentFrame;

	R_Texture* spriteSheet;
	
	int rows;
	int columns;
	int animationNumber;

	int spritesheetPixelWidth;
	int spritesheetPixelHeight;
private:

	void GetFrameProportions(int row, int column);
};

#endif // !__SPRITESHEET_H__