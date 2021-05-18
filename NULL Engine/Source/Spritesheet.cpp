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

void Spritesheet::GetFrameProportions(int row, int column)
{
	currentFrame.proportionBeginX = ((float)column - 1) / columns;
	currentFrame.proportionFinalX = (float)column / columns;

	currentFrame.proportionBeginY =  ((float)row - 1) / rows;
	currentFrame.proportionFinalY =  (float)row / rows;
}



void Spritesheet::SetCurrentFrameLocation(int frameNumber)
{
	int count = 1;
	int rowsN = 1;
	int columnsN = 1;

	for(int i = 1; i < rows; i++)
	{
		rowsN = i;
		for(int k = 1; k < columns; k++)
		{
			columnsN = k;
			count++;
			if(count == frameNumber)
			GetFrameProportions(rowsN, columnsN);
		}
	}
}