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

void Spritesheet::GetFrameProportions(int row, int column)
{
	currentFrame.proportionBeginX = (pixelLenght * row) / pixelLenght * (rows - 1);
	currentFrame.proportionFinalX = (pixelLenght * (row + 1)) / pixelLenght * (rows - 1);

	currentFrame.proportionBeginY = (pixelHeight * column) / pixelHeight * (columns - 1);
	currentFrame.proportionBeginY = (pixelHeight * column + 1) / pixelHeight * (columns - 1);
}

void Spritesheet::SetCurrentFrameLocation(int frameNumber)
{
	int rowN = 0;
	int columnN = 0;

	for (int i = 0; i <= rows; i++)
	{
		for (int k = 0; k <= rows; k++)
		{
			if (frameNumber == 0)
			{
				GetFrameProportions(rowN, columnN);
			}
			frameNumber--;
			columnN++;
		}
		columnN = 0;
		rowN++;
	}
}