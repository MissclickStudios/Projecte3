#include "Spritesheet.h"


Spritesheet::Spritesheet(R_Texture* spritesheet)
{
	animationNumber = 0;
	currentFrame.proportionBeginX = 0;
	currentFrame.proportionBeginY = 0;
	currentFrame.proportionFinalX = 0;
	currentFrame.proportionFinalY = 0;

	spriteSheet = spritesheet;
	columns = 0;
	rows = 0;
	pixelHeight = 0;
	pixelLenght = 0;
}

Spritesheet::~Spritesheet()
{
}

void Spritesheet::SetSpritesheetSize(int s_rows, int s_columns, int s_pisxelHeight, int s_pixelLenght)
{
	columns = s_columns;
	rows = s_rows;
	pixelHeight = s_pisxelHeight;
	pixelLenght = s_pixelLenght;
}

Frame* Spritesheet::GetAtlasPosition(int pixelPosX, int pixelPosY, int pixelWidth, int pixelHeight)
{
	Frame* frame = new Frame();

	return frame;
}

void Spritesheet::GetFrameProportions(int row, int column)
{
	currentFrame.proportionBeginX = (float)column / columns;
	currentFrame.proportionFinalX = ((float)column + 1) / columns;

	currentFrame.proportionBeginY =  (float)row / rows;
	currentFrame.proportionFinalY =  ((float)row + 1) / rows;
}



void Spritesheet::SetCurrentFrameLocation(int frameNumber)
{
		GetFrameProportions(2, 2);
}