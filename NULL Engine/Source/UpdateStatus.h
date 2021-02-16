#ifndef __UPDATE_STATUS_H__
#define __UPDATE_STATUS_H__

enum class UPDATE_STATUS						// Defines all the states that the application can be at when updating.
{
	CONTINUE = 1,
	STOP,
	THROW_ERROR
};

#endif // !__UPDATE_STATUS_H__