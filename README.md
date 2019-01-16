# timer
Create to manage threaded timer and watchdog.

## Get:
```Shell
git clone https://github.com/ox223252/timer.git
```

## Exemples:
### With Signal management:
For this exemple you need to get [freeOnExit](github.co/ox223252/freeOnExit) and [signalHandler](github.co/ox223252/signalHandler) lib.

```C
git clone https://github.com/ox223252/freeOnExit.git
git clone https://github.com/ox223252/signalHandler.git
```

You should use the `FOE_WITH_THREAD` and `TIMER_WITH_FOE` defines.

The freeOnExit and signalHandler libs are used only for manage signals like `Ctrl^C`, but it's not mandatory.

```C
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>


#include "freeOnExit/freeOnExit.h"
#include "signalHandler/signalHandler.h"
#include "timer/timer.h"

static void onCrtlC ( void * arg )
{
    printf ( "%s", ( char * )arg );
    exit ( 0 );
}

static char * activeText ( int s )
{
    static char *status[] = { "not started", "stopped", "active" };
    return ( status[ s + 1 ] );
}

void exitS ( void * arg ) 
{
    time_t rawtime;
    time ( &rawtime );

    printf("timed function at %s", asctime ( localtime ( &rawtime ) ) );
}

void * coreFunction ( void * arg )
{
    time_t rawtime;
    time ( &rawtime );

    printf ( " - start core function at : %s", asctime ( localtime ( &rawtime ) ) );
    
    // a very long work
    sleep ( 5 );

    printf ( " - stop core function at : %s", asctime ( localtime ( &rawtime ) ) );

    watchdogStop ( false );

    return ( NULL );
}

int main ( void )
{
    int i = 0;
    time_t rawtime;
    time ( &rawtime );
    
    initFreeOnExit ( );

    signalHandling signal =
    {
        .flag =
        {
            .Int = 1
        },
        .Int =
        {
            .func = onCrtlC,
            .arg = "Int case\n"
        }
    };

    signalHandlerInit ( &signal );
  
    srand ( time ( NULL ) );
    i = rand()%5 * 1000000;
    
    printf ( "start wait %ds at %s", i / 1000000, asctime ( localtime ( &rawtime ) ) );
    startTimer ( i, exitS, "from timer" );

    sleep ( 5 );

    // coreFunction restarted by watch dog here
    watchdogInit ( 300, coreFunction, NULL, true );
    watchdogStart ( );

    printf ( "WD : %s\n", activeText ( watchdoActive ( ) ) );
    printf ( "Core : %s\n", activeText ( watchdogCoreActive ( ) ) );
    sleep ( 1 );

    printf ( "WD : %s\n", activeText ( watchdoActive ( ) ) );
    printf ( "Core : %s\n", activeText ( watchdogCoreActive ( ) ) );
    sleep ( 10 );

    watchdogStop ( true );
    printf ( "WD : %s\n", activeText ( watchdoActive ( ) ) );
    printf ( "Core : %s\n", activeText ( watchdogCoreActive ( ) ) );

    printf ( "\nMake job correctly\n" );

    // coreFunction restarted by watch dog here
    watchdogInit ( 600, coreFunction, NULL, true );
    watchdogStart ( );

    while ( watchdoActive ( ) > 0 )
    {
        sleep ( 1 );
    }

    return ( 0 );
}

```

```Shell
> gcc main.c -pthread -g timer/timer.c freeOnExit/freeOnExit.c signalHandler/signalHandler.c -DTIMER_WITH_FOE -DFOE_WITH_THREAD && valgrind --leak-check=full ./a.out
```

### Without signals management:
```C
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>


// #include "freeOnExit/freeOnExit.h"
// #include "signalHandler/signalHandler.h"
#include "timer/timer.h"

static char * activeText ( int s )
{
    static char *status[] = { "not started", "stopped", "active" };
    return ( status[ s + 1 ] );
}

void exitS ( void * arg ) 
{
    time_t rawtime;
    time ( &rawtime );

    printf("timed function at %s", asctime ( localtime ( &rawtime ) ) );
}

void * coreFunction ( void * arg )
{
    time_t rawtime;
    time ( &rawtime );

    printf ( " - start core function at : %s", asctime ( localtime ( &rawtime ) ) );
    
    // a very long work
    sleep ( 5 );

    printf ( " - stop core function at : %s", asctime ( localtime ( &rawtime ) ) );

    watchdogStop ( false );

    return ( NULL );
}

int main ( void )
{
    int i = 0;
    time_t rawtime;
    time ( &rawtime );
    void * timer = NULL;
  
    srand ( time ( NULL ) );
    i = rand()%5 * 1000000;
    
    printf ( "start wait %ds at %s", i / 1000000, asctime ( localtime ( &rawtime ) ) );
    timer = startTimer ( i, exitS, "from timer" );

    sleep ( 5 );

    stopTimer( &timer );

    // coreFunction restarted by watch dog here
    watchdogInit ( 300, coreFunction, NULL, true );
    watchdogStart ( );

    printf ( "WD : %s\n", activeText ( watchdoActive ( ) ) );
    printf ( "Core : %s\n", activeText ( watchdogCoreActive ( ) ) );
    sleep ( 1 );

    printf ( "WD : %s\n", activeText ( watchdoActive ( ) ) );
    printf ( "Core : %s\n", activeText ( watchdogCoreActive ( ) ) );
    sleep ( 10 );

    watchdogStop ( true );
    printf ( "WD : %s\n", activeText ( watchdoActive ( ) ) );
    printf ( "Core : %s\n", activeText ( watchdogCoreActive ( ) ) );

    printf ( "\nMake job correctly\n" );

    // coreFunction restarted by watch dog here
    watchdogInit ( 600, coreFunction, NULL, true );
    watchdogStart ( );

    while ( watchdoActive ( ) > 0 )
    {
        sleep ( 1 );
    }

    watchdogStop( true );

    return ( 0 );
}
```

```Shell
> gcc main.c -pthread -g timer/timer.c && valgrind --leak-check=full ./a.out
```

Be care the two function work identicly if you let it endded normaly, but if you use a Ctrl^C the second one will not manage it and you will have memory leaks.

## functions:
```C
void * startTimer( const uint32_t time, void (*callback)( void * ), void * arg);
```

 - **time**: time before starting callback (µs),
 - **callback**: callback function,
 - **arg**: pointer to callback argument

 - **return**: pointer to timer struct use in stopTimer function.

```C
int stopTimer ( void ** timer );
```

 - **timer**: pointer provided by startTimer,

```C
int watchdogInit ( const uint32_t time, void * ( * fnc )( void * ), void * arg, bool restart );
```

 - **time**: watch dog delay before restart core function,
 - **fnc**: core function managed by watchdog,
 - **arg**: core function arg
 - **restart**: flag to determine if core function should be restarted if watchdog interrupt occur

```C
void watchdogReset ( void );
```

```C
int watchdogStart ( void );
```

```C
int watchdogCoreActive ( void );
```

 - **return**: 
   - *-1*: core function not started
   - *0*: core function stopped
   - *1*: core function running

```C
int watchdoActive ( void );
```

 - **return**: 
   - *-1*: watchdog not started
   - *0*: watchdog stopped
   - *1*: watchdog running (but maybe in wait mode).

```C
bool watchdogWait ( bool wait );
```

 - **wait**: flag to set

```C
int watchdogStop ( bool stopCoreFnc );
```

 - **stopCoreFnc**: flag to deactivate or not core function at the same time of watchdog

## Note:
v1.0 work only for Linux