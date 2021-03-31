#ifndef __PROFILER_H__
#define __PROFILER_H__

#ifndef NOPROFILER
#include "Dependencies/Optik/include/optick.h"

#else
#define OPTICK_FRAME(FRAME_NAME, ...)
#define OPTICK_CATEGORY(NAME, CATEGORY)
#endif // !NOPROFILER

#endif // !__PROFILER_H__