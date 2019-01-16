#ifndef __TIMER_H__
#define __TIMER_H__
////////////////////////////////////////////////////////////////////////////////
/// \copiright ox223252, 2017
///
/// This program is free software: you can redistribute it and/or modify it
///     under the terms of the GNU General Public License published by the Free
///     Software Foundation, either version 2 of the License, or (at your
///     option) any later version.
///
/// This program is distributed in the hope that it will be useful, but WITHOUT
///     ANY WARRANTY; without even the implied of MERCHANTABILITY or FITNESS FOR
///     A PARTICULAR PURPOSE. See the GNU General Public License for more
///     details.
///
/// You should have received a copy of the GNU General Public License along with
///     this program. If not, see <http://www.gnu.org/licenses/>
////////////////////////////////////////////////////////////////////////////////

#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>

#if defined ( TIMER_WITH_FOE ) && defined ( FOE_WITH_THREAD )
#include "../freeOnExit/freeOnExit.h"
#endif

////////////////////////////////////////////////////////////////////////////////
/// \file timer.h
/// \brief library to nanage threaded timer and watchdog
/// \author ox223252
/// \date 2018-08
/// \copyright GPLv2
/// \version 1.0
/// \warning v1.0 only for Linux
/// \bug NONE
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// \fn void * startTimer ( const uint32_t time, void (*callback)( void * ), 
///     void * arg, const bool argFree );
/// \param[ in ] time : time in µseconds
/// \param[ in ] callnback : callback function
/// \param[ in ] arg : callback argument
/// \brief set a timer before executin a function
/// \retrun pointer on timer
////////////////////////////////////////////////////////////////////////////////
void * startTimer( const uint32_t time, void (*callback)( void * ), void * arg);

////////////////////////////////////////////////////////////////////////////////
/// \fn int stopTimer ( void * timer );
/// \param timer: pointer provided by startTimer
/// \retrun 0 if OK else see errno for more details
////////////////////////////////////////////////////////////////////////////////
int stopTimer ( void ** timer );

////////////////////////////////////////////////////////////////////////////////
/// only one watchdog at a time
////////////////////////////////////////////////////////////////////////////////
/// \fn int watchdogInit ( const uint32_t time, void ( * fnc )( void * ), void * arg,
///     bool restart );
/// \param[ in ] time : watchdog timer before ending by 10ms step
/// \param[ in ] fnc : function who need watchdog
/// \param[ in ] arg : function args
/// \param[ in ] restartg : restart function if watchdog interrupt occured
/// \brief init watchdog for a particular function but it doesn't start it
/// \retrun 0 if OK else see errno for more details
////////////////////////////////////////////////////////////////////////////////
int watchdogInit ( const uint32_t time, void * ( * fnc )( void * ), void * arg,
	bool restart );

////////////////////////////////////////////////////////////////////////////////
/// \fn void resetWatchdog ( void );
/// \brief reset timing vaue
////////////////////////////////////////////////////////////////////////////////
void watchdogReset ( void );

////////////////////////////////////////////////////////////////////////////////
/// \fn void startWatchdog ( void );
/// \brief start watchdog
/// \retrun 0 if OK else see errno for more details
////////////////////////////////////////////////////////////////////////////////
int watchdogStart ( void );

////////////////////////////////////////////////////////////////////////////////
//// \fn int watchdogCoreActive ( void );
/// \brief return status of thread
/// \return -1 : not started / 0 endded / 1 working
////////////////////////////////////////////////////////////////////////////////
int watchdogCoreActive ( void );

////////////////////////////////////////////////////////////////////////////////
//// \fn int watchdoActive ( void );
/// \brief return status of thread
/// \return -1 : not started / 0 endded / 1 working
////////////////////////////////////////////////////////////////////////////////
int watchdoActive ( void );

////////////////////////////////////////////////////////////////////////////////
/// \fn bool watchdogWait ( bool wait );
/// \param wait : wait status
/// \brief set watchdog status in pause without stoping it
/// \retrun old status
////////////////////////////////////////////////////////////////////////////////
bool watchdogWait ( bool wait );

////////////////////////////////////////////////////////////////////////////////
/// \fn void stopWatchdog ( bool stopCoreFnc );
/// \param[ in ] stopCoreFnc : stop coreFnc
/// \brief stop watchdog
/// \retrun 0 if OK else see errno for more details
////////////////////////////////////////////////////////////////////////////////
int watchdogStop ( bool stopCoreFnc );

#endif