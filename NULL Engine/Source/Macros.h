#ifndef __MACROS_H__
#define __MACROS_H__

// Performance macros
#define TIMER_START(timer) timer.Start();																				// 
#define TIMER_PEEK(timer) LOG("%s took %f s", __FUNCTION__, timer.ReadSec())											// 
#define PERF_TIMER_START(perf_timer) perf_timer.Start()																	// 
#define PERF_TIMER_PEEK(perf_timer) LOG("%s took %f ms", __FUNCTION__, perf_timer.ReadMs())								// 

// Utility macros
#define IN_RANGE( value, minimum, maximum )	( ((value) >= (minimum) && (value) <= (maximum)) ? 1 : 0 )					// 

#define CAP( n )			( (n <= 0.0f)		?	n=0.0f : (n >= 1.0f) ? n=1.0f : n=n )								// 
#define MIN( a, b )			( ((a) < (b))		?	(a) : (b) )															// 
#define MAX( a, b )			( ((a) > (b))		?	(a) : (b) )															// 
#define TO_BOOL( a )		( (a != 0)			?	true : false )														// 
//#define ARRAY_SIZE( a )		( (a != nullptr)	?	(*(&a + 1) - a) : 0 )												// 
#define ARRAY_SIZE(a)		( (a != nullptr)	?	(sizeof(a) / sizeof(a[0])) : 0 )									// 

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
