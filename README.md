# timer

## Exemple:
For this exemple you need to get [freeOnExit][github.co/ox223252/freeOnExit] lib.

```C
void exitS ( void * arg ) 
{
	printf("exit\n");
	exit ( (int)arg );
}

int main ( void )
{
	initFreeOnExit();

	timer ( 7000000, exitS, ( void * )1, true );
	printf ( "    %d\n", __LINE__ );

	intitWatchdog ( 2, true );
	printf ( "    %d\n", __LINE__ );

	startWatchdog ( );
	printf ( "    %d\n", __LINE__ );
	usleep ( 1500000 );
	printf ( "    %d\n", __LINE__ );
	resetWatchdog ( );
	printf ( "    %d\n", __LINE__ );

	while ( 1 )
	{
		sleep ( 1 );
	}
	return ( 0 );
}
```

```Shell
> gcc timer.c ../freeOnExit/freeOnExit.c -lpthread -DTIMER_WITH_FOE -DFOE_WITH_THREAD && ./a.out
```