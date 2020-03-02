#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "timer.h"

typedef struct
{
	uint32_t time;
	pthread_t id;
	void ( * callback )( void * );
	void * arg;
}
_timer_struct_t;

static struct
{
	uint32_t reset;
	uint32_t time;
	struct
	{
		uint8_t on:1,
			wait:1,
			restart:1;
	}
	flag;
	pthread_t watchdogId;
	pthread_t coreId;
	void * ( * fnc )( void * );
	void * arg;
}
_timer_watchdog = { 0 };


//
// timer part
//
static void * timerCounter ( void * arg )
{
	usleep ( ((_timer_struct_t*)arg)->time );

	((_timer_struct_t*)arg)->callback ( ((_timer_struct_t*)arg)->arg );
	pthread_exit ( NULL );
}

void * startTimer ( const uint32_t time, void (*callback)( void * ), void * arg )
{
	_timer_struct_t * timer = NULL;

	timer = malloc ( sizeof ( *timer ) );

	if ( !timer )
	{
		return ( NULL );
	}
	
	timer->time = time;
	timer->callback = callback;
	timer->arg = arg;

	if ( pthread_create ( &timer->id, NULL, timerCounter, ( void * )timer ) )
	{
		free ( timer );
		return ( NULL );
	}

	#if defined( TIMER_WITH_FOE ) && defined( FOE_WITH_THREAD )
		setThreadCancelOnExit ( timer->id );
		setFreeOnExit ( timer );
	#endif

	return ( timer );
}

int stopTimer ( void ** timer )
{
	if ( !timer ||
		!*timer ) 
	{
		errno = EINVAL;
		return ( __LINE__ );
	}

	#if defined( TIMER_WITH_FOE ) && defined( FOE_WITH_THREAD )
		unsetThreadCancelOnExit ( ( *(_timer_struct_t**)timer)->id );
		unsetFreeOnExit ( *(_timer_struct_t**)timer );
	#endif

	pthread_cancel ( ( *(_timer_struct_t**)timer)->id );
	pthread_join ( ( *(_timer_struct_t**)timer)->id, NULL );
	free ( *(_timer_struct_t**)timer );
	*(_timer_struct_t**)timer = NULL;
	return ( 0 );
}


//
// watchdog part
//
#pragma GCC diagnostic ignored "-Wunused-parameter"
static void * watchdog ( void * arg )
{
	if ( pthread_create ( &_timer_watchdog.coreId, NULL, _timer_watchdog.fnc, _timer_watchdog.arg ) )
	{
		return ( NULL );
	}

	#if defined ( TIMER_WITH_FOE ) && defined ( FOE_WITH_THREAD )
		if ( setThreadCancelOnExit ( _timer_watchdog.coreId ) )
		{
			return ( NULL );
		}
	#endif

	do
	{
		usleep ( 10000 );
		

		if ( !_timer_watchdog.flag.wait )
		{
			_timer_watchdog.time--;
		}
		else
		{ // if wath dog wait mode
			continue;
		}
		
		if ( _timer_watchdog.time == 0 )
		{
		}

		if ( pthread_kill ( _timer_watchdog.coreId, 0 ) == ESRCH )
		{ // coreFunction end correctly
			pthread_join ( _timer_watchdog.coreId, NULL );

			#if defined( TIMER_WITH_FOE ) && defined( FOE_WITH_THREAD )
				unsetThreadCancelOnExit ( _timer_watchdog.coreId );
			#endif

			return ( NULL );
		}

		if ( _timer_watchdog.time == 0 )
		{
			pthread_cancel ( _timer_watchdog.coreId );
			pthread_join ( _timer_watchdog.coreId, NULL );

			#if defined( TIMER_WITH_FOE ) && defined( FOE_WITH_THREAD )
				unsetThreadCancelOnExit ( _timer_watchdog.coreId );
			#endif

			if ( _timer_watchdog.flag.restart )
			{
				_timer_watchdog.time = _timer_watchdog.reset;

				if ( pthread_create ( &_timer_watchdog.coreId, NULL, _timer_watchdog.fnc, _timer_watchdog.arg ) )
				{
					return ( NULL );
				}

				#if defined ( TIMER_WITH_FOE ) && defined ( FOE_WITH_THREAD )
					if ( setThreadCancelOnExit ( _timer_watchdog.coreId ) )
					{
						return ( NULL );
					}
				#endif
			}
			else
			{
				return ( NULL );
			}
		}
	}
	while ( 1 );
}
#pragma GCC diagnostic pop

int watchdogInit ( const uint32_t time, void * ( * fnc )( void * ), void * arg, bool restart )
{
	if ( _timer_watchdog.flag.on )
	{
		errno = EBUSY;
		return ( __LINE__ );
	}

	_timer_watchdog.reset = time;
	_timer_watchdog.time = time;
	_timer_watchdog.flag.on = 0;
	_timer_watchdog.flag.wait = 0;
	_timer_watchdog.flag.restart = restart;
	_timer_watchdog.coreId = 0;

	_timer_watchdog.fnc = fnc;
	_timer_watchdog.arg = arg;

	#if defined( TIMER_WITH_FOE ) && defined( FOE_WITH_THREAD )
    	initFreeOnExit ( );
	#endif
	return ( 0 );
}

void watchdogReset ( void )
{
	_timer_watchdog.time = _timer_watchdog.reset;
}

int watchdogStart ( void )
{
	_timer_watchdog.flag.on = 1;


	if ( pthread_create ( &_timer_watchdog.watchdogId, NULL, watchdog, NULL ) )
	{
		return ( __LINE__ );
	}

	#if defined( TIMER_WITH_FOE ) && defined( FOE_WITH_THREAD )
		if ( setThreadCancelOnExit ( _timer_watchdog.watchdogId ) )
		{
			return ( __LINE__ );
		}
	#endif
	return ( 0 );
}

int watchdogCoreActive ( void )
{
	if ( !_timer_watchdog.coreId )
	{
		return ( -1 );
	}
	return ( pthread_kill ( _timer_watchdog.coreId, 0 ) != ESRCH );
}

int watchdoActive ( void )
{
	if ( !_timer_watchdog.watchdogId )
	{
		return ( -1 );
	}
	return ( pthread_kill ( _timer_watchdog.watchdogId, 0 ) != ESRCH );
}

bool watchdogWait ( bool wait )
{
	bool old = _timer_watchdog.flag.wait;
	_timer_watchdog.flag.wait = wait;
	return ( old );
}

int watchdogStop ( bool stopCoreFnc )
{
	_timer_watchdog.flag.on = 0;
	_timer_watchdog.flag.wait = false;


	pthread_cancel ( _timer_watchdog.watchdogId );
	pthread_join ( _timer_watchdog.watchdogId, NULL );
	
	#if defined( TIMER_WITH_FOE ) && defined( FOE_WITH_THREAD )
		unsetThreadCancelOnExit ( _timer_watchdog.watchdogId );
	#endif

	_timer_watchdog.watchdogId = 0;
	
	if ( stopCoreFnc )
	{
		pthread_cancel ( _timer_watchdog.coreId );
		pthread_join ( _timer_watchdog.coreId, NULL );
		
		#if defined( TIMER_WITH_FOE ) && defined( FOE_WITH_THREAD )
			unsetThreadCancelOnExit ( _timer_watchdog.coreId );
		#endif

		_timer_watchdog.coreId = 0;
	}

	return ( 0 );
}