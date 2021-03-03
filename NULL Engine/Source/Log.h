#ifndef __LOG_H__
#define __LOG_H__

#include "Macros.h"

#define LOG(format, ...) log(__FILE__, __LINE__, format, __VA_ARGS__)

void NULL_API log(const char file[], int line, const char* format, ...);

#endif // !__LOG_H__