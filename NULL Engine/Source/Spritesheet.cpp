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

	spritesheetPixelWidth = 0;
	spritesheetPixelHeight = 0;
}

Spritesheet::~Spritesheet()
{
}

void Spritesheet::SetSpritesheetSize(int s_rows, int s_columns, int s_pisxelHeight, int s_pixelLenght)
{
	columns = s_columns;
	rows = s_rows;
	spritesheetPixelHeight = s_pisxelHeight;
	spritesheetPixelWidth = s_pixelLenght;
}

Frame* Spritesheet::GetAtlasPosition(int pixelPosX, int pixelPosY, int pixelWidth, int pixelHeight)
{
	Frame* frame = new Frame();

	frame->proportionBeginX = (float)pixelPosX / spritesheetPixelWidth;
	frame->proportionFinalX = ((float)pixelPosX + pixelWidth) / spritesheetPixelWidth;

	frame->proportionBeginY = (float)pixelPosY / spritesheetPixelHeight;
	frame->proportionFinalY = ((float)pixelPosY + pixelHeight) / spritesheetPixelHeight;

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
	int rowN = 1;
	int columnN = 1;
	int count = 0;

	//hmmmmmmmm
	for (int k = 1; k < rows; k++) 
	{
		for (int i = 1; i < columns; i++) 
		{	
			columnN++;
			count++;
			if(count == frameNumber){}
			GetFrameProportions(rowN, columnN);
		}
		rowN++;
		columnN = 1;
		if (count == frameNumber)
		GetFrameProportions(rowN, columnN);
	}
}