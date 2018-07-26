#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "timer.h"

typedef struct
{
	uint32_t time;
	void (*callback)( void * );
	void * arg;
}
callback_strcut;

static struct
{
	int32_t reset:15,
		time:15,
		on:1,
		exit:1;
}
_timer_watchDog = { 0 };

static void * timerCounter ( void * arg )
{
	callback_strcut data = *(callback_strcut*)arg;

	usleep ( data.time );

	printf ( "end timer\n" );

	data.callback ( data.arg );
	pthread_exit ( NULL );
}

static void * watchDog ( void * arg )
{
	while ( !_timer_watchDog.on || 
		_timer_watchDog.time-- )
	{
		if ( !_timer_watchDog.exit )
		{
			return ( NULL );
		}
		sleep ( 1 );
	}
	printf ( "end watchdog\n" );
	exit ( 0 );
}

pthread_t * timer ( const uint32_t time, void (*callback)( void * ), void * arg, const bool argFree )
{
	pthread_t * ptr = NULL;
	static callback_strcut data;

	data.time = time;
	data.callback = callback;
	data.arg = arg;

	ptr = malloc ( sizeof ( pthread_t ) );

	if ( !ptr )
	{
		return ( NULL );
	}

	if ( pthread_create ( ptr, NULL, timerCounter, ( void * )&data ) )
	{
		free ( ptr );
		return ( NULL );
	}

	#if defined( TIMER_WITH_FOE ) && defined( FOE_WITH_THREAD )
	if ( argFree )
	{
		setThreadKillOnExit ( *ptr );
		setFreeOnExit ( ptr );
	}
	#endif

	return ( ptr );
}

void * intitWatchdog ( const uint16_t time, const bool argFree )
{
	pthread_t * ptr = NULL;

	if ( _timer_watchDog.exit )
	{
		errno = EBUSY;
		return ( NULL );
	}

	_timer_watchDog.reset = time & 0x7f;
	_timer_watchDog.time = time & 0x7f;
	_timer_watchDog.on = 0;
	_timer_watchDog.exit = 1;

	ptr = malloc ( sizeof ( pthread_t ) );

	if ( !ptr )
	{
		return ( NULL );
	}

	if ( pthread_create ( ptr, NULL, watchDog, NULL ) )
	{
		free ( ptr );
		return ( NULL );
	}

	#if defined( TIMER_WITH_FOE ) && defined( FOE_WITH_THREAD )
	if ( argFree )
	{
		setThreadKillOnExit ( *ptr );
		setFreeOnExit ( ptr );
	}
	#endif

	return ( ptr );
}

void resetWatchdog ( void )
{
	_timer_watchDog.time = _timer_watchDog.reset;
}

void startWatchdog ( void )
{
	_timer_watchDog.on = 1;
}

void pauseWatchdog ( void )
{
	_timer_watchDog.on = 0;
}

void stopWatchdog ( void )
{
	_timer_watchDog.exit = 1;
}