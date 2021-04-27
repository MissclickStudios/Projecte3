#ifndef __LOG_H__
#define __LOG_H__

#include "Macros.h"

#define LOG(format, ...) log(__FILE__, __LINE__, format, __VA_ARGS__)

void MISSCLICK_API log(const char file[], int line, const char* format, ...);

#endif // !__LOG_H__

//TODO !!!!!! La engine necessita els logs sempre !!! -> necessito una altre macro NOLOG
/*#ifndef GAMEBUILD
#include "Macros.h"
#define LOG(format, ...) log(__FILE__, __LINE__, format, __VA_ARGS__)
void MISSCLICK_API log(const char file[], int line, const char* format, ...);
#else
#define LOG(format, ...) //0 //Poso un 0 per els ternaris?
#endif // !GAMEBUILD*/