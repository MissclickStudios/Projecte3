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
	currentFrame.proportionBeginX = row /(rows - 1);
	currentFrame.proportionFinalX =  (row + 1) / (rows - 1);

	currentFrame.proportionBeginY = column/ (columns - 1);
	currentFrame.proportionBeginY = (column + 1) /(columns - 1);
}



void Spritesheet::SetCurrentFrameLocation(int frameNumber)
{
		GetFrameProportions(1, 1);
}