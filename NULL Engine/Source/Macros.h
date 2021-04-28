#ifndef __MACROS_H__
#define __MACROS_H__

//Import/export define
#ifdef NULL_BUILD_DLL
#define MISSCLICK_API __declspec(dllexport)
#else
#define MISSCLICK_API __declspec(dllimport)
#endif

// Performance macros
#define TIMER_START(timer)				timer.Start();																	// Switches on the given Timer.
#define TIMER_PEEK(timer)				LOG("%s took %f s", __FUNCTION__, timer.ReadSec())								// Outputs a LOG displaying a peek to the Timer.
#define PERF_TIMER_START(perf_timer)	perf_timer.Start()																// Switches on the given Performance Timer
#define PERF_TIMER_PEEK(perf_timer)		LOG("%s took %f ms", __FUNCTION__, perf_timer.ReadMs())							// Outputs a LOG displaying a peek to the Performance Timer.

// Utility macros
#define IN_RANGE( value, minimum, maximum )	( ((value) >= (minimum) && (value) <= (maximum)) ? 1 : 0 )					// Will return true of the given value is within the given range.

#define CAP( n )			( (n <= 0.0f)		?	n=0.0f : (n >= 1.0f) ? n=1.0f : n=n )								// Will cut the value of the given variable to be within [0.0f, 1.0f]
#define MIN( a, b )			( ((a) < (b))		?	(a) : (b) )															// Returns the variable with the lower value.
#define MAX( a, b )			( ((a) > (b))		?	(a) : (b) )															// Returns the variable with the higher value.
#define TO_BOOL( a )		( (a != 0)			?	true : false )														// Converts any given atomic variable and converts it to bool.
#define ARRAY_SIZE( a )		( (a != nullptr)	?	((int)(sizeof(a) / sizeof(*(a)))) : 0 )								// Only works with { float a[n] } style arrays, not { float* b } ones.

// Memory Managing Macros
// Deletes a buffer
#define RELEASE( x )			\
	{							\
		if ( x != nullptr )		\
		{						\
			delete x;			\
			x = nullptr;		\
		}						\
	}

// Deletes an array of buffers
#define RELEASE_ARRAY( x )		\
	{							\
		if (x != nullptr)		\
		{						\
			delete[] x;			\
			x = nullptr;		\
		}						\
	}

#endif // !__MACROS_H__
