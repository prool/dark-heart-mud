/*
 * HELL, formerly known as BTL/Darkheart, by Adam Powell and Chris Mellish,
 *  1996 to 1998.  If you reproduce this code in any way please give credit
 *  to the writers.  Thanks.
 */


#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "hell.h"
#include "recycle.h"
#include "magic.h"

/* command procedures needed */
DECLARE_DO_FUN(do_help          );
DECLARE_DO_FUN(do_look          );
DECLARE_DO_FUN(do_skills        );
DECLARE_DO_FUN(do_unread        );
DECLARE_DO_FUN(do_title         );
BAN_DATA *ban_list;


/*
 * Malloc debugging stuff.
 */
#if defined(sun)
#undef MALLOC_DEBUG
#endif

#if defined(MALLOC_DEBUG)
#include <malloc.h>
extern  int     malloc_debug    args( ( int  ) );
extern  int     malloc_verify   args( ( void ) );
#endif



/*
 * Signal handling.
 * Apollo has a problem with __attribute(atomic) in signal.h,
 *   I dance around it.
 */
#if defined(apollo)
#define __attribute(x)
#endif

#if defined(unix)
#include <signal.h>
#endif

#if defined(apollo)
#undef __attribute
#endif



/*
 * Socket and TCP/IP stuff.
 */
#if     defined(macintosh) || defined(MSDOS)
const   char    echo_off_str    [] = { '\0' };
const   char    echo_on_str     [] = { '\0' };
const   char    go_ahead_str    [] = { '\0' };
#endif

#if     defined(unix)
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "telnet.h"
const   char    echo_off_str    [] = { IAC, WILL, TELOPT_ECHO, '\0' };
const   char    echo_on_str     [] = { IAC, WONT, TELOPT_ECHO, '\0' };
const   char    go_ahead_str    [] = { IAC, GA, '\0' };
const   char 	edit_mode_str	[] = { IAC, WILL,TELOPT_SGA,IAC,WILL,TELOPT_ECHO, IAC, DO, TELOPT_BINARY};
const   char 	norm_mode_str	[] = { IAC, WONT,TELOPT_SGA,IAC,WONT,TELOPT_ECHO, IAC, DONT, TELOPT_BINARY};
const   char 	ctrl_mode_str	[] = { IAC, WONT,TELOPT_SGA,IAC, DONT, TELOPT_BINARY};
#endif

int gm_cntrl;

/*
 * OS-dependent declarations.
 */
#if     defined(_AIX)
#include <sys/select.h>
int     accept          args( ( int s, struct sockaddr *addr, int *addrlen ) );
int     bind            args( ( int s, struct sockaddr *name, int namelen ) );
void    bzero           args( ( char *b, int length ) );
int     getpeername     args( ( int s, struct sockaddr *name, int *namelen ) );
int     getsockname     args( ( int s, struct sockaddr *name, int *namelen ) );
int     gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );
int     listen          args( ( int s, int backlog ) );
int     setsockopt      args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
int     socket          args( ( int domain, int type, int protocol ) );
#endif

#if     defined(apollo)
#include <unistd.h>
void    bzero           args( ( char *b, int length ) );
#endif

#if     defined(__hpux)
int     accept          args( ( int s, void *addr, int *addrlen ) );
int     bind            args( ( int s, const void *addr, int addrlen ) );
void    bzero           args( ( char *b, int length ) );
int     getpeername     args( ( int s, void *addr, int *addrlen ) );
int     getsockname     args( ( int s, void *name, int *addrlen ) );
int     gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );
int     listen          args( ( int s, int backlog ) );
int     setsockopt      args( ( int s, int level, int optname,
				const void *optval, int optlen ) );
int     socket          args( ( int domain, int type, int protocol ) );
#endif

#if     defined(interactive)
#include <net/errno.h>
#include <sys/fnctl.h>
#endif

#if     defined(linux)
/* 
    Linux shouldn't need these. If you have a problem compiling, try
    uncommenting accept and bind.
int     accept          args( ( int s, struct sockaddr *addr, int *addrlen ) );
int     bind            args( ( int s, struct sockaddr *name, int namelen ) );
*/

#if 0 // by prool
int     close           args( ( int fd ) );
//int     getpeername     args( ( int s, struct sockaddr *name, int *namelen ) ); // by prool
//int     getsockname     args( ( int s, struct sockaddr *name, int *namelen ) );
int     gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );
int     listen          args( ( int s, int backlog ) );
int     read            args( ( int fd, char *buf, int nbyte ) );
int     select          args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int     socket          args( ( int domain, int type, int protocol ) );
int     write           args( ( int fd, char *buf, int nbyte ) );
#endif // by prool

#endif

#if     defined(riscbsd)
int     read            args( ( int fd, char *buf, int nbyte ) );
int     close           args( ( int fd ) );
int     select          args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int     write           args( ( int fd, char *buf, int nbyte ) );
#endif

#if     defined(macintosh)
#include <console.h>
#include <fcntl.h>
#include <unix.h>
struct  timeval
{
	time_t  tv_sec;
	time_t  tv_usec;
};
#if     !defined(isascii)
#define isascii(c)              ( (c) < 0200 )
#endif
static  long                    theKeys [4];

int     gettimeofday            args( ( struct timeval *tp, void *tzp ) );
#endif

#if     defined(MIPS_OS)
extern  int             errno;
#endif

#if     defined(MSDOS)
int     gettimeofday    args( ( struct timeval *tp, void *tzp ) );
int     kbhit           args( ( void ) );
#endif

#if     defined(NeXT)
int     close           args( ( int fd ) );
int     fcntl           args( ( int fd, int cmd, int arg ) );
#if     !defined(htons)
u_short htons           args( ( u_short hostshort ) );
#endif
#if     !defined(ntohl)
u_long  ntohl           args( ( u_long hostlong ) );
#endif
int     read            args( ( int fd, char *buf, int nbyte ) );
int     select          args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int     write           args( ( int fd, char *buf, int nbyte ) );
#endif

#if     defined(sequent)
int     accept          args( ( int s, struct sockaddr *addr, int *addrlen ) );
int     bind            args( ( int s, struct sockaddr *name, int namelen ) );
int     close           args( ( int fd ) );
int     fcntl           args( ( int fd, int cmd, int arg ) );
int     getpeername     args( ( int s, struct sockaddr *name, int *namelen ) );
int     getsockname     args( ( int s, struct sockaddr *name, int *namelen ) );
int     gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );
#if     !defined(htons)
u_short htons           args( ( u_short hostshort ) );
#endif
int     listen          args( ( int s, int backlog ) );
#if     !defined(ntohl)
u_long  ntohl           args( ( u_long hostlong ) );
#endif
int     read            args( ( int fd, char *buf, int nbyte ) );
int     select          args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int     setsockopt      args( ( int s, int level, int optname, caddr_t optval,
			    int optlen ) );
int     socket          args( ( int domain, int type, int protocol ) );
int     write           args( ( int fd, char *buf, int nbyte ) );
#endif

/* This includes Solaris Sys V as well */
#if defined(sun)
int     accept          args( ( int s, struct sockaddr *addr, int *addrlen ) );
int     bind            args( ( int s, struct sockaddr *name, int namelen ) );
void    bzero           args( ( char *b, int length ) );
int     close           args( ( int fd ) );
int     getpeername     args( ( int s, struct sockaddr *name, int *namelen ) );
int     getsockname     args( ( int s, struct sockaddr *name, int *namelen ) );
/*int   gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );*/
int     listen          args( ( int s, int backlog ) );
int     read            args( ( int fd, char *buf, int nbyte ) );
int     select          args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
#if defined(SYSV)
int setsockopt          args( ( int s, int level, int optname,
			    const char *optval, int optlen ) );
#else
/*int   setsockopt      args( ( int s, int level, int optname, void *optval,
			    int optlen ) );*/
#endif
int     socket          args( ( int domain, int type, int protocol ) );
int     write           args( ( int fd, char *buf, int nbyte ) );
#endif

#if defined(ultrix)
int     accept          args( ( int s, struct sockaddr *addr, int *addrlen ) );
int     bind            args( ( int s, struct sockaddr *name, int namelen ) );
void    bzero           args( ( char *b, int length ) );
int     close           args( ( int fd ) );
int     getpeername     args( ( int s, struct sockaddr *name, int *namelen ) );
int     getsockname     args( ( int s, struct sockaddr *name, int *namelen ) );
int     gettimeofday    args( ( struct timeval *tp, struct timezone *tzp ) );
int     listen          args( ( int s, int backlog ) );
int     read            args( ( int fd, char *buf, int nbyte ) );
int     select          args( ( int width, fd_set *readfds, fd_set *writefds,
			    fd_set *exceptfds, struct timeval *timeout ) );
int     setsockopt      args( ( int s, int level, int optname, void *optval,
			    int optlen ) );
int     socket          args( ( int domain, int type, int protocol ) );
int     write           args( ( int fd, char *buf, int nbyte ) );
#endif



/*
 * Global variables.
 */
DESCRIPTOR_DATA *   descriptor_list;    /* All open descriptors         */
DESCRIPTOR_DATA *   d_next;             /* Next descriptor in loop      */
FILE *              fpReserve;          /* Reserved file handle         */
bool                god;                /* All new chars are gods!      */
bool                merc_down;          /* Shutdown                     */
bool                wizlock;            /* Game is wizlocked            */
bool                newlock;            /* Game is newlocked            */
char                str_boot_time[MAX_INPUT_LENGTH];
time_t              current_time;       /* time of this pulse */        



/*
 * OS-dependent local functions.
 */
#if defined(macintosh) || defined(MSDOS)
void    game_loop_mac_msdos     args( ( void ) );
bool    read_from_descriptor    args( ( DESCRIPTOR_DATA *d ) );
#endif

#if defined(unix)
void    game_loop_unix          args( ( int control ) );
int     init_socket             args( ( int port ) );
void    init_descriptor         args( ( int control ) );
bool    read_from_descriptor    args( ( DESCRIPTOR_DATA *d ) );
bool    write_to_descriptor     args( ( int desc, char *txt, int length) );
#endif




/*
 * Other local functions (OS-independent).
 */
bool    output_buffer           args( ( DESCRIPTOR_DATA *d ) );
bool    check_parse_name        args( ( char *name ) );
bool    check_reconnect         args( ( DESCRIPTOR_DATA *d, char *name,
				    bool fConn ) );
bool    check_playing           args( ( DESCRIPTOR_DATA *d, char *name ) );
int     main                    args( ( int argc, char **argv ) );
void    nanny                   args( ( DESCRIPTOR_DATA *d, char *argument ) );
bool    process_output          args( ( DESCRIPTOR_DATA *d, bool fPrompt ) );
void    read_from_buffer        args( ( DESCRIPTOR_DATA *d ) );
void    stop_idling             args( ( CHAR_DATA *ch ) );
void    bust_a_prompt           args( ( CHAR_DATA *ch ) );

    int port;
    int control;

int main( int argc, char **argv )
{
    struct timeval now_time;
    bool fCopyOver = FALSE;

#if defined(unix)
    extern int gm_cntrl;
#endif

    /*
     * Memory debugging if needed.
     */
#if defined(MALLOC_DEBUG)
    malloc_debug( 2 );
#endif

    /*
     * Init time.
     */
    gettimeofday( &now_time, NULL );
    current_time        = (time_t) now_time.tv_sec;
    strcpy( str_boot_time, ctime( &current_time ) );

    /*
     * Macintosh console initialization.
     */
#if defined(macintosh)
    console_options.nrows = 31;
    cshow( stdout );
    csetmode( C_RAW, stdin );
    cecho2file( "log file", 1, stderr );
#endif

    /*
     * Reserve one channel for our use.
     */
    if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL )
    {
	perror( NULL_FILE );
	exit( 1 );
    }

    /*
     * Get the port number.
     */
    port = 2000;
    if ( argc > 1 )
    {
	if ( !is_number( argv[1] ) )
	{
	    fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
	    exit( 1 );
	}
	else if ( ( port = atoi( argv[1] ) ) <= 1024 )
	{
	    fprintf( stderr, "Port number must be above 1024.\n" );
	    exit( 1 );
	}
    }
#if 0 // prool
    if (argv[2] && argv[2][0])
    {
	fCopyOver = TRUE;
	control = atoi(argv[3]);
    }
#endif

    /*
     * Run the game.
     */
#if defined(macintosh) || defined(MSDOS)
    boot_db( );
    log_string( "Merc is ready to rock." );
    game_loop_mac_msdos( );
#endif

#if defined(unix)
    if (!fCopyOver)
	control = init_socket( port );
    gm_cntrl = control;
    boot_db( fCopyOver );
    sprintf( log_buf, "%s is ready to kick arse on port %d.", CODE_VER,  port);
    log_string( log_buf );
    game_loop_unix( control );
    close (control);
#endif

    /*
     * That's all, folks.
     */
    log_string( "Normal termination of game." );
    exit( 0 );
    return 0;
}



#if defined(unix)
int init_socket( int port )
{
    static struct sockaddr_in sa_zero;
    struct sockaddr_in sa;
    int x = 1;
    int fd;

    if ( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
    {
	perror( "Init_socket: socket" );
	exit( 1 );
    }

    if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
    (char *) &x, sizeof(x) ) < 0 )
    {
	perror( "Init_socket: SO_REUSEADDR" );
	close(fd);
	exit( 1 );
    }

    sa              = sa_zero;
    sa.sin_family   = AF_INET;
    sa.sin_port     = htons( port );

    if ( bind( fd, (struct sockaddr *) &sa, sizeof(sa) ) < 0 )
    {
	perror("Init socket: bind" );
	close(fd);
	exit(1);
    }


    if ( listen( fd, 3 ) < 0 )
    {
	perror("Init socket: listen");
	close(fd);
	exit(1);
    }

    return fd;
}
#endif



#if defined(macintosh) || defined(MSDOS)
void game_loop_mac_msdos( void )
{
    struct timeval last_time;
    struct timeval now_time;
    static DESCRIPTOR_DATA dcon;

    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    /*
     * New_descriptor analogue.
     */
    dcon.descriptor     = 0;
    dcon.connected      = CON_GET_NAME;
    dcon.host           = str_dup( "localhost" );
    dcon.outsize        = 2000;
    dcon.outbuf         = alloc_mem( dcon.outsize );
    dcon.next           = descriptor_list;
    dcon.showstr_head   = NULL;
    dcon.showstr_point  = NULL;
    descriptor_list     = &dcon;

    /*
     * Send the greeting.
     */
    {
	extern char * help_greeting;
	if ( help_greeting[0] == '.' )
	    write_to_buffer( &dcon, help_greeting+1, 0 );
	else
	    write_to_buffer( &dcon, help_greeting  , 0 );
    }

    /* Main loop */
    while ( !merc_down )
    {
	DESCRIPTOR_DATA *d;

	/*
	 * Process input.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next      = d->next;
	    d->fcommand = FALSE;

#if defined(MSDOS)
	    if ( kbhit( ) )
#endif
	    {
		if ( d->character != NULL )
		    d->character->timer = 0;
		if ( !read_from_descriptor( d ) )
		{
		    if ( d->character != NULL)
			save_char_obj( d->character );
		    d->outtop   = 0;
		    close_socket( d );
		    continue;
		}
	    }

	    if (d->character != NULL && d->character->daze > 0)
		--d->character->daze;

	    if ( d->character != NULL && d->character->wait > 0 )
	    {
		--d->character->wait;
		continue;
	    }

	    if ( d->incomm[0] == '\0' ) 
	    {
		read_from_buffer( d );
		do_autowalk( d );
	    }
	    if ( d->incomm[0] != '\0' )
	    {
		d->fcommand     = TRUE;
		stop_idling( d->character );

		if ( d->connected == CON_PLAYING )
		{
		    char curcom[MAX_STRING_LENGTH];
		    char *newpos;
   		    int i;

		    newpos = split_semicolon(d->incomm, curcom);
		    bug( curcom, 0 );
		    substitute_alias( d, curcom );
		    for (i = 0; newpos[i]!='\0'; i++)
			d->incomm[i] = newpos[i];
		    d->incomm[i] = '\0';
		    bug( d->incomm, 0 );
		}
		else
		{
		    nanny( d, d->incomm );
		    d->incomm[0]    = '\0';
		}
	    }
	}



	/*
	 * Autonomous game motion.
	 */
	update_handler( );



	/*
	 * Output.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;

	    if ( ( d->fcommand || d->outtop > 0 ) )
	    {
		if ( !process_output( d, TRUE ) )
		{
		    if ( d->character != NULL && d->character->level > 1)
			save_char_obj( d->character );
		    d->outtop   = 0;
		    close_socket( d );
		}
	    }
	}



	/*
	 * Synchronize to a clock.
	 * Busy wait (blargh).
	 */
	now_time = last_time;
	for ( ; ; )
	{
	    int delta;

#if defined(MSDOS)
	    if ( kbhit( ) )
#endif
	    {
		if ( dcon.character != NULL )
		    dcon.character->timer = 0;
		if ( !read_from_descriptor( &dcon ) )
		{
		    if ( dcon.character != NULL && d->character->level > 1)
			save_char_obj( d->character );
		    dcon.outtop = 0;
		    close_socket( &dcon );
		}
#if defined(MSDOS)
		break;
#endif
	    }

	    gettimeofday( &now_time, NULL );
	    delta = ( now_time.tv_sec  - last_time.tv_sec  ) * 1000 * 1000
		  + ( now_time.tv_usec - last_time.tv_usec );
	    if ( delta >= 1000000 / PULSE_PER_SECOND )
		break;
	}
	last_time    = now_time;
	current_time = (time_t) last_time.tv_sec;
    }

    return;
}
#endif



#if defined(unix)
void game_loop_unix( int control )
{
    static struct timeval null_time;
    struct timeval last_time;

    signal( SIGPIPE, SIG_IGN );
    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    /* Main loop */
    while ( !merc_down )
    {
	fd_set in_set;
	fd_set out_set;
	fd_set exc_set;
	DESCRIPTOR_DATA *d;
	int maxdesc;

#if defined(MALLOC_DEBUG)
	if ( malloc_verify( ) != 1 )
	    abort( );
#endif

	/*
	 * Poll all active descriptors.
	 */
	FD_ZERO( &in_set  );
	FD_ZERO( &out_set );
	FD_ZERO( &exc_set );
	FD_SET( control, &in_set );
	maxdesc = control;
	for ( d = descriptor_list; d; d = d->next )
	{
	    maxdesc = UMAX( maxdesc, d->descriptor );
	    FD_SET( d->descriptor, &in_set  );
	    FD_SET( d->descriptor, &out_set );
	    FD_SET( d->descriptor, &exc_set );
	}

	if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
	{
	    perror( "Game_loop: select: poll" );
	    exit( 1 );
	}

	/*
	 * New connection?
	 */
	if ( FD_ISSET( control, &in_set ) )
	    init_descriptor( control );

	set_alarm( 60 );

	/*
	 * Kick out the freaky folks.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;   
	    if ( FD_ISSET( d->descriptor, &exc_set ) )
	    {
		FD_CLR( d->descriptor, &in_set  );
		FD_CLR( d->descriptor, &out_set );
		if ( d->character && d->character->level > 1)
		    save_char_obj( d->character );
		d->outtop       = 0;
		close_socket( d );
	    }
	}

	/*
	 * Process input.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next      = d->next;
	    d->fcommand = FALSE;

	    if ( FD_ISSET( d->descriptor, &in_set ) || d->connected == CON_EDITOR)
	    {
		if ( d->character != NULL )
		    d->character->timer = 0;
		if ( !read_from_descriptor( d ) )
		{
		    FD_CLR( d->descriptor, &out_set );
		    if ( d->character != NULL && d->character->level > 1)
			save_char_obj( d->character );
		    d->outtop   = 0;
		    close_socket( d );
		    continue;
		}
	    }

	    if (d->character != NULL && d->character->daze > 0)
		--d->character->daze;

	    if ( d->character != NULL && d->character->wait > 0 )
	    {
		--d->character->wait;
		continue;
	    }

	    if ( d->incomm[0] == '\0' ) 
	    {
		read_from_buffer( d );
		if (d->character != NULL && d->connected == CON_PLAYING)
		    REMOVE_BIT(d->character->flag4, AFF_AUTOWALKING);
		if (d->incomm[0] != '\0' && d->connected == CON_PLAYING) do_autowalk( d );
	    }

	    if (d->connected == CON_CONSOLE)
	    {
		if (d->incomm[0] != 0)
		{
		    if (!str_cmp(d->incomm, "exit"))
			char_to_game( d->character );
		    else stc("Type 'exit' to exit console mode.\n\r", d->character);
		    d->incomm[0] = 0;
		}
		continue;
	    }

	    if (d->incomm[0] != '\0' && d->value != -1) d->value = 100;

	    if (d->value != -1)
	    {
		send_title( d );
		continue;
	    }

	    if ( d->incomm[0] != '\0' )
	    {
		d->fcommand     = TRUE;
		stop_idling( d->character );

		if (d->showstr_point)
		{
		    show_string(d,d->incomm);
		    d->incomm[0] = 0;
		}
		else if ( d->connected == CON_PLAYING )
		{
		    char curcom[MAX_STRING_LENGTH];
		    char *newpos;
   		    int i;

		    SET_BIT(d->character->flag4, AFF_COMMAND_ENTERED);
		    newpos = split_semicolon(d->incomm, curcom);
		    interpret( d->character, curcom );
		    for (i = 0; newpos[i]!='\0'; i++)
			d->incomm[i] = newpos[i];
		    d->incomm[i] = '\0';
		}
		else if (d->connected == CON_EDITOR)
		{
		    editor( d );
		    d->incomm[0]    = '\0';		    
		}
		else
		{
		    nanny( d, d->incomm );
		    d->incomm[0]    = '\0';
		}

	    }
	}



	/*
	 * Autonomous game motion.
	 */
	update_handler( );



	/*
	 * Output.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;
	    if (++d->sent_pulse > 3)
	    {
		d->sent_pulse = 0;
		d->sent_secs[0] += 1;
		if (d->sent_secs[0] > 9999)
		{
		    d->sent_secs[1] += 1;
		    d->sent_secs[0] -= 10000;
		    if (d->sent_secs[1] > 30000)
		    {
			d->sent_secs[0]  = 0;
			d->sent_secs[1]  = 0;
			d->sent_pulse    = 0;
			d->sent_bytes[0] = 0;
			d->sent_bytes[1] = 0;
			d->sent_bytes[2] = 0;
		    }
		}
	    }
	    if ( ( d->fcommand || d->outtop > 0 )
	    &&   FD_ISSET(d->descriptor, &out_set) )
	    {
	    	if (d->connected == CON_PLAYING && IS_SET(d->character->flag4, AFF_SCREEN_SPLIT))
		{
		    if (IS_SET(d->character->flag4, AFF_COMMAND_ENTERED) && 1 == 0)
		    {
			ansi_restore_cursor( d->character );
			ansi_goto( d->character, 1, 23 );
			ansi_save_cursor( d->character );
			ansi_goto( d->character, 1, 1 );
			ansi_restore_cursor( d-> character );
		    }
		}
		if ( !process_output( d, TRUE ) )
		{
		    if ( d->character != NULL && d->character->level > 1)
			save_char_obj( d->character );
		    d->outtop   = 0;
		    close_socket( d );
		    continue;
		}
	    	if (d->connected == CON_PLAYING && IS_SET(d->character->flag4, AFF_SCREEN_SPLIT)) ansi_goto( d->character, 1, 1 );
	    	if (d->connected == CON_PLAYING && IS_SET(d->character->flag4, AFF_SCREEN_SPLIT)) ansi_save_cursor( d->character );
	    	if (d->connected == CON_PLAYING && IS_SET(d->character->flag4, AFF_SCREEN_SPLIT)) ansi_goto( d->character, 1, 19 );
	    }

	}



	/*
	 * Synchronize to a clock.
	 * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
	 * Careful here of signed versus unsigned arithmetic.
	 */
	{
	    struct timeval now_time;
	    long secDelta;
	    long usecDelta;

	    gettimeofday( &now_time, NULL );
	    usecDelta   = ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
			+ 1000000 / PULSE_PER_SECOND;
	    secDelta    = ((int) last_time.tv_sec ) - ((int) now_time.tv_sec );
	    while ( usecDelta < 0 )
	    {
		usecDelta += 1000000;
		secDelta  -= 1;
	    }

	    while ( usecDelta >= 1000000 )
	    {
		usecDelta -= 1000000;
		secDelta  += 1;
	    }

	    if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
	    {
		struct timeval stall_time;

		stall_time.tv_usec = usecDelta;
		stall_time.tv_sec  = secDelta;
		if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 )
		{
		    perror( "Game_loop: select: stall" );
		    exit( 1 );
		}
	    }
	}

	gettimeofday( &last_time, NULL );
	current_time = (time_t) last_time.tv_sec;
    }

    return;
}
#endif

void boot_screen( int control, int pos, int size )
{
    static struct timeval null_time;

    signal( SIGPIPE, SIG_IGN );

    {
	fd_set in_set;
	fd_set out_set;
	fd_set exc_set;
	DESCRIPTOR_DATA *d;
	int maxdesc, i, e;
	char buf[255];
	extern char * boot_greeting;

	if (boot_greeting == NULL) return;

	e = pos * 56 / ((size <= 0) ? 1 : size);
	if (e > 55) e = 55;

	for (i = 0; i < 56; i++)
	    buf[i] = ' ';
	for (i = 0; i < e; i++)
	    buf[i] = '*';
	buf[55] = 0;

#if defined(MALLOC_DEBUG)
	if ( malloc_verify( ) != 1 )
	    abort( );
#endif

	/*
	 * Poll all active descriptors.
	 */
	FD_ZERO( &in_set  );
	FD_ZERO( &out_set );
	FD_ZERO( &exc_set );
	FD_SET( control, &in_set );
	maxdesc = control;
	for ( d = descriptor_list; d; d = d->next )
	{
	    maxdesc = UMAX( maxdesc, d->descriptor );
	    FD_SET( d->descriptor, &in_set  );
	    FD_SET( d->descriptor, &out_set );
	    FD_SET( d->descriptor, &exc_set );
	}

	if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
	{
	    perror( "Game_loop: select: poll" );
	    exit( 1 );
	}

	/*
	 * New connection?
	 */
	if ( FD_ISSET( control, &in_set ) )
	{
	    init_descriptor( control );
	    /* send boot screen */
	    ansi_cls_desc( descriptor_list );
	    ansi_goto_desc( descriptor_list, 1, 2 );
 	    if ( boot_greeting[0] == '.' )
	    	write_to_descriptor( descriptor_list->descriptor, boot_greeting+1, 0 );
	    else
	    	write_to_descriptor( descriptor_list->descriptor, boot_greeting , 0 );
	}

	/*
	 * Kick out the freaky folks.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;   
	    if ( FD_ISSET( d->descriptor, &exc_set ) )
	    {
		FD_CLR( d->descriptor, &in_set  );
		FD_CLR( d->descriptor, &out_set );
		if ( d->character && d->character->level > 1)
		    save_char_obj( d->character );
		d->outtop       = 0;
		close_socket( d );
	    }
	// Update .'s
	    ansi_goto_desc( d, 12, 18);
	    write_to_descriptor( d->descriptor, buf, 0 );
	}

	/*
	 * Output.
	 */
	for ( d = descriptor_list; d != NULL; d = d_next )
	{
	    d_next = d->next;

	    if ( ( d->fcommand || d->outtop > 0 )
	    &&   FD_ISSET(d->descriptor, &out_set) )
	    {
		if ( !process_output( d, TRUE ) )
		{
		    if ( d->character != NULL && d->character->level > 1)
			save_char_obj( d->character );
		    d->outtop   = 0;
		    close_socket( d );
		}
	    }
	}
    }

    return;
}



void close_socket( DESCRIPTOR_DATA *dclose )
{
    CHAR_DATA *ch;

    if ( dclose->outtop > 0 )
	process_output( dclose, FALSE );

    if ( dclose->snoop_by != NULL )
    {
	write_to_buffer( dclose->snoop_by,
	    "Your victim has left the game.\n\r", 0 );
    }

    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d != NULL; d = d->next )
	{
	    if ( d->snoop_by == dclose )
		d->snoop_by = NULL;
	}
    }

    if ( ( ch = dclose->character ) != NULL )
    {
	sprintf( log_buf, "Closing link to %s.", ch->name );
	log_string( log_buf );
	/* cut down on wiznet spam when rebooting */
	if ( dclose->connected == CON_PLAYING && !merc_down)
	{
	    act( "$n has lost $s link.", ch, NULL, NULL, TO_ROOM );
	    wiznet("Net death has claimed $N.",ch,NULL,WIZ_LINKS,0,0);
	    ch->desc = NULL;
	}
	else
	{
	    free_char(dclose->original ? dclose->original : 
		dclose->character );
	}
    }

    if ( d_next == dclose )
	d_next = d_next->next;   

    if ( dclose == descriptor_list )
    {
	descriptor_list = descriptor_list->next;
    }
    else
    {
	DESCRIPTOR_DATA *d;

	for ( d = descriptor_list; d && d->next != dclose; d = d->next )
	    ;
	if ( d != NULL )
	    d->next = dclose->next;
	else
	    bug( "Close_socket: dclose not found.", 0 );
    }

    close( dclose->descriptor );
    free_descriptor(dclose);
#if defined(MSDOS) || defined(macintosh)
    exit(1);
#endif
    return;
}



bool read_from_descriptor( DESCRIPTOR_DATA *d )
{
    int iStart;

    /* Hold horses if pending command already. */
    if ( d->incomm[0] != '\0' )
	return TRUE;

    /* Check for overflow. */
    iStart = strlen(d->inbuf);
    if ( iStart >= sizeof(d->inbuf) - 10 )
    {
	sprintf( log_buf, "%s input overflow!", d->host );
	log_string( log_buf );
	write_to_descriptor( d->descriptor,
	    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
	return FALSE;
    }

    /* Snarf input. */
#if defined(macintosh)
    for ( ; ; )
    {
	int c;
	c = getc( stdin );
	if ( c == '\0' || c == EOF )
	    break;
	putc( c, stdout );
	if ( c == '\r' )
	    putc( '\n', stdout );
	d->inbuf[iStart++] = c;
	if ( iStart > sizeof(d->inbuf) - 10 )
	    break;
    }
#endif

#if defined(MSDOS) || defined(unix)
    for ( ; ; )
    {
	int nRead;

	nRead = read( d->descriptor, d->inbuf + iStart,
	    sizeof(d->inbuf) - 10 - iStart );
	if ( nRead > 0 )
	{
	    iStart += nRead;
	    if ( d->inbuf[iStart-1] == '\n' || d->inbuf[iStart-1] == '\r' )
		break;
	}
	else if ( nRead == 0 && d->connected == CON_EDITOR)
	{
	    return TRUE;
	}
	else if ( nRead == 0 )
	{
	    log_string( "EOF encountered on read." );
	    return FALSE;
	}
	else if ( errno == EWOULDBLOCK )
	    break;
	else
	{
	    perror( "Read_from_descriptor" );
	    return FALSE;
	}
    }
#endif

    d->inbuf[iStart] = '\0';
    return TRUE;
}



/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA *d )
{
    CHAR_DATA *ch = d->original ? d->original : d->character;
    char lastletter;
    int i, j, k;

    /*
     * Hold horses if pending command already.
     */
    if ( d->incomm[0] != '\0' || d->inbuf[0] == '\0')
	return;


    if (d->connected == CON_EDITOR)
    {
	for (i = 0, k = 0; d->inbuf[i] != '\0'; i++)
	    d->incomm[k++] = d->inbuf[i];
	d->inbuf[0] = '\0';
	d->incomm[k] = '\0';
	return;
    }

    /*
     * Look for at least one new line.
     */

    	for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
	{
	    if ( d->inbuf[i] == '\0' )
	    	return;
    	}

    /*
     * Canonical input processing.
     */
    for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
    {
	if (d->inbuf[i] == '\0' && d->connected == CON_EDITOR) break;
	if ( k >= MAX_INPUT_LENGTH - 2 )
	{
	    write_to_descriptor( d->descriptor, "Line too long.\n\r", 0 );

	    /* skip the rest of the line */
	    for ( ; d->inbuf[i] != '\0'; i++ )
	    {
		if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
		    break;
	    }
	    d->inbuf[i]   = '\n';
	    d->inbuf[i+1] = '\0';
	    break;
	}

	if ( d->inbuf[i] == '\b' && k > 0 )
	    --k;
	else if (d->inbuf[i] == '~')
	{
	    d->incomm[k++] = '#';
	    d->incomm[k++] = '-';
	}
	else if (d->inbuf[i] == '%')
	{
	    d->incomm[k++] = '#';
	    d->incomm[k++] = '+';
	}
	else if (d->connected == CON_EDITOR) d->incomm[k++] = d->inbuf[i];
	else if ( isascii(d->inbuf[i]) && isprint(d->inbuf[i]) )
	    d->incomm[k++] = d->inbuf[i];
    }

    /*
     * Finish off the line.
     */
    if (d->connected == CON_EDITOR) 
    { 
	d->incomm[k] = '\0'; 
	d->inbuf[0] = '\0';
	return;
    }
    if ( k == 0 )
	d->incomm[k++] = ' ';
    d->incomm[k] = '\0';

    /*
     * Deal with bozos with #repeat 1000 ...
     */


    if ( k > 1 || d->incomm[0] == '!' )
    {
	if ( d->incomm[0] != '!' && strcmp( d->incomm, d->inlast[d->inlasti] ) )
	{
	    d->repeat = 0;
	}
	else
	{
	    if (++d->repeat % 10 == 0 && d->character
		&& d->connected == CON_PLAYING)
	    {
		SET_BIT(d->character->flag3, AFF_SPAM_WARNING);
	    }
		

	    if (d->repeat >= 25 && d->character
	    &&  d->connected == CON_PLAYING)
	    {
		SET_BIT(d->character->flag3, AFF_SPAM_VIOLATION);
		sprintf( log_buf, "%s input spamming!", d->host );
		log_string( log_buf );
		wiznet("Spam spam spam $N spam spam spam spam spam!",
		       d->character,NULL,WIZ_SPAM,0,get_trust(d->character));
		if (d->incomm[0] == '!')
		    wiznet(d->inlast[d->inlasti],d->character,NULL,WIZ_SPAM,0,
			get_trust(d->character));
		else
		    wiznet(d->incomm,d->character,NULL,WIZ_SPAM,0,
			get_trust(d->character));
		d->repeat = 0;
/*		write_to_descriptor( d->descriptor,
		    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
		strcpy( d->incomm, "quit" ); */
	    }
	}
    }


    /*
     * Do '!' substitution.
     */

    lastletter = d->incomm[strlen(d->incomm) - 1];

    if ( d->incomm[0] == '!' && d->incomm[1] != '!')
    {
	int i;

	if (isdigit(d->incomm[1]))
	    i = d->incomm[1] - '0';
	else
	    i = 1;

	if (i == 0) i = 10;

	i = d->inlasti - i + 1;
	if (i < 0) i += 10;
	if (i > 10) i = 1;

        if ( d->inlast[i] != NULL )
	   strcpy( d->incomm, d->inlast[i] );
    }
    else if (d->incomm[0] == '+')
    {
	char tempbuf[MAX_STRING_LENGTH];
	strcpy(tempbuf, &d->incomm[1]);
	strcpy(d->incomm, d->inlast[d->inlasti]);
	strcat(d->incomm, tempbuf);
    }
    else if (d->incomm[0] == '<' && d->incomm[1] != '\0')
    {
	char arg[MAX_INPUT_LENGTH];
	char *temp;
	char *arg2;
	int i = 0, j = 0;

	arg2 = &d->incomm[1];
	arg2 = one_argument(arg2, arg);

	if (arg[0] != '\0' && arg2[0] != '\0')
	{
	    stc("Substituting.\n\r",ch);
	    temp = d->inlast[d->inlasti];
	    d->incomm[0] = '\0';
	    while (temp[i] != '\0')
	    {
	    	if (!str_prefix(arg, &temp[i]))
		{
		    i += strlen(arg);
		    d->incomm[j] = '\0';
		    strcat(d->incomm, arg2);
		    j += strlen(arg2) + 1;
		}
		else
		{
		    d->incomm[j++] = temp[i++];
		}
	    }
	    d->incomm[j] = '\0';
	}
	else
	{
	    stc("Format:   < <word> <string>\n\r",ch);
	    d->incomm[0] = '\0';
	}
    }

    if (str_prefix(d->incomm, "!!"))
    {
    	if (++d->inlasti > 9) d->inlasti = 0;
	if (d->incomm[0] != '\0') strcpy( d->inlast[d->inlasti], d->incomm );
    }

    if (lastletter == '$')
    {
	d->incomm[0] = '\0';
	stc("Command skipped.\n\r",ch);
    }

    substitute_alias(d, d->incomm); /* no longer exec's command */

    /*
     * Shift the input buffer.
     */
    while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
	i++;
    for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j] ) != '\0'; j++ )
	;
    return;
}



/*
 * Low level output function.
 */
bool process_output( DESCRIPTOR_DATA *d, bool fPrompt )
{
    extern bool merc_down;

    /*
     * Bust a prompt.
     */
    if (!merc_down && d->showstr_point)
	write_to_buffer(d,"[Hit Return to continue]\n\r",0);
    else if (fPrompt && !merc_down && d->connected == CON_PLAYING)
    {
	CHAR_DATA *ch;

	ch = d->original ? d->original : d->character;
	if (!IS_SET(ch->comm, COMM_COMPACT) && d->connected != CON_CONSOLE)
	    write_to_buffer( d, "\n\r", 2 );


	if ( IS_SET(ch->comm, COMM_PROMPT) )
	    bust_a_prompt( d->character );

	if (IS_SET(ch->comm,COMM_TELNET_GA))
	    write_to_buffer(d,go_ahead_str,0);
    }

    /*
     * Short-circuit if nothing to write.
     */
    if ( d->outtop == 0 )
	return TRUE;

    /*
     * Snoop-o-rama.
     */
    if ( d->snoop_by != NULL )
    {
	if (d->character != NULL)
	    write_to_buffer( d->snoop_by, d->character->name,0);
	write_to_buffer( d->snoop_by, "> ", 2 );
	write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
    }

    /*
     * OS-dependent output.
     */

    if (d==NULL)
	return FALSE;
    if (write_to_descriptor(d->descriptor,d->outbuf,d->outtop))
    {
        d->outtop = 0;
    	return TRUE;
    }

    d->outtop = 0;
    return FALSE;
}

/*
 * Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud
 */
void bust_a_prompt( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    const char *str;
    const char *i;
    char *point;
    char doors[MAX_INPUT_LENGTH];
    EXIT_DATA *pexit;
    bool found;
    const char *dirname[] = {"N","E","S","W","U","D"};
    int door, per;

    if (IS_SET(ch->flag4, AFF_SCREEN_SPLIT))
    {
	char bar1[10];
	char bar2[10];
	char bar3[10];
	char buf[300];
	int per, i;

	ansi_goto(ch, 1, 21);

	per = (9 * ch->hit) / ch->max_hit;
	for (i = 0; i < 9; i++)
	    if (i <= per) bar1[i] = '*'; else bar1[i] = ' ';
	bar1[9] = 0;

	per = (9 * ch->move) / ch->max_move;
	for (i = 0; i < 9; i++)
	    if (i <= per) bar2[i] = '*'; else bar2[i] = ' ';
	bar2[9] = 0;

	if (ch->fighting != NULL)
	    per = (9 * ch->fighting->hit) / ch->fighting->max_hit;
	else
	    per = 0;
	for (i = 0; i < 9; i++)
	    if (i <= per) bar3[i] = '*'; else bar3[i] = ' ';
	bar3[9] = 0;

// draw the status bar for hp, move, enemy hp, exp,  and exits
// -=[hp]-[*****    ]=--=[mv]-[*****    ]=--=[enemy]-[*****    ]=--=[ex]-[NSEWUD]=-

	sprintf(buf, "#4-#6=[#7hp#6]-[#7%s#6]=#4--#6=[#7mv#6]-[#7%s#6]=#4--#6=[#7enemy#6]-[#7%s#6]=#4--#6=[#7ex#6]-[#7%c%c%c%c%c%c#6]=#4-#n",
	    bar1, bar2, bar3,
	    (ch->in_room != NULL && ch->in_room->exit[DIR_NORTH] != NULL && ch->in_room->exit[DIR_NORTH]->u1.to_room != NULL) ? 'N' : ' ',
	    (ch->in_room != NULL && ch->in_room->exit[DIR_SOUTH] != NULL && ch->in_room->exit[DIR_SOUTH]->u1.to_room != NULL) ? 'S' : ' ',
	    (ch->in_room != NULL && ch->in_room->exit[DIR_EAST] != NULL && ch->in_room->exit[DIR_EAST]->u1.to_room != NULL) ? 'E' : ' ',
	    (ch->in_room != NULL && ch->in_room->exit[DIR_WEST] != NULL && ch->in_room->exit[DIR_WEST]->u1.to_room != NULL) ? 'W' : ' ',
	    (ch->in_room != NULL && ch->in_room->exit[DIR_UP] != NULL && ch->in_room->exit[DIR_UP]->u1.to_room != NULL) ? 'U' : ' ',
	    (ch->in_room != NULL && ch->in_room->exit[DIR_DOWN] != NULL && ch->in_room->exit[DIR_DOWN]->u1.to_room != NULL) ? 'D' : ' '
	    );
	stc(buf, ch);
//	ansi_restore_cursor( ch );
	return;
    }

    if (IS_FLAG2(ch, AFF_EARTH_MELD))
    {
 	stc("[Melded with the Earth] ", ch);
	return;
    }

    if (IS_FLAG4(ch, AFF_TREE_MELD))
    {
 	stc("[Melded with a Tree] ", ch);
	return;
    }

    if (is_mob_switched(ch))
    {
	stcf(ch, "[Switched with %s] ", GET_NAME(ch));
	return;
    }

    if (is_item(ch))
    {
	if (ch->sw_obj->in_obj != NULL)
	    stcf(ch, "[Inside %s] ", ch->sw_obj->in_obj->short_descr);
	else if (ch->sw_obj->on != NULL)
	    stcf(ch, "[On top of %s] ", ch->sw_obj->on->short_descr);
	else if (ch->sw_obj->carried_by != NULL)
	    stcf(ch, "[Carried by %s] ", GET_NAME(ch->sw_obj->carried_by));
	else 
	    stc("[Object Form] ", ch);
	return;
    }


    if (ch->registerred == 0)
    {
	ch->registerred = 1;
//	stc("<UR> ",ch);
    }

    if (IS_SET(ch->comm, COMM_AFK))
    {
	stc("#7<#3==#1-- #7AFK#1 --#3==#7>#n  ", ch);
	return;
    }

    if (ch->pcdata != NULL && ch->prefix != NULL && !str_cmp(ch->prefix, "molc "))
    {
	MOB_INDEX_DATA *mob;

	if ((mob = get_mob_index(ch->pcdata->curr_vnum_medit)) != NULL)
	    stcf(ch, "#6MOB#n:#4 %s #6[#4%d#6]#n : ", mob->short_descr, mob->vnum);
	else
	    stc("#6MOB#n:#4 '#6select#4' a mob#n : ", ch);
	return;
    }

    if (ch->pcdata != NULL && ch->prefix != NULL && !str_cmp(ch->prefix, "oolc "))
    {
	OBJ_INDEX_DATA *obj;

	if ((obj = get_obj_index(ch->pcdata->curr_vnum_oedit)) != NULL)
	    stcf(ch, "#2OBJ#n:#1 %s #2[#1%d#2]#n : ", obj->short_descr, obj->vnum);
	else
	    stc("#2OBJ#n:#1 '#2select#1' an object#n : ", ch);
	return;
    }

    point = buf;
    str = ch->prompt;
    if (ch->position == POS_FIGHTING && ch->fightprompt != NULL)
	str = ch->fightprompt;

    if (ch->stunned > 0)
    {
	int col = 2;
	if (ch->fighting == NULL) ch->stunned = 0;
	if (ch->stunned > 2) col = 3;
	if (ch->stunned > 6) col = 1;
	stcf(ch, "#%d<Stunned> ", col);
    }

    if ((ch->move * 100  / ch->max_move) <= 0)
	stc("#1<Exhausted> ", ch);
    else if ((ch->move * 100/ ch->max_move) < 5)
	stc("#3<Tired> ", ch);


    if (config_info->autohelp != NULL && !IS_NPC(ch) &&
	(ch->tempprompt == NULL || str_cmp(ch->tempprompt, config_info->autohelp)))
    {
	sprintf(buf, "#bType 'help %s' > #n %s", config_info->autohelp, ch->prefix);
	send_to_char(buf, ch);
	return;
    }

    if (str == NULL || str[0] == '\0')
    {
	sprintf( buf, "#b<%dhp %dmv %dpsp> #n %s",
	    ch->hit,ch->move,ch->psp,ch->prefix);
	send_to_char(buf,ch);
	return;
    }

   *point++ = '#'; *point++ = 'b';

   while( *str != '\0' )
   {
      if( *str != '&' && *str != '%')
      {
	 *point++ = *str++;
	 continue;
      }
      ++str;
      switch( *str )
      {
	 default :
	    i = " "; break;
	case 'd': if (ch->fighting == NULL)
		  {
	    		sprintf(buf2,"N/A");
	    		i = buf2; break;
		  }
		  else
		  {
	    		sprintf(buf2,"%d", ch->distance);
	    		i = buf2; break;
		  }
	case 'e':
	    found = FALSE;
	    doors[0] = '\0';
	    for (door = 0; door < 6; door++)
	    {
		if ((pexit = ch->in_room->exit[door]) != NULL
		&&  pexit ->u1.to_room != NULL
		&&  (can_see_room(ch,pexit->u1.to_room)
		||   (IS_AFFECTED(ch,AFF_INFRARED) 
		&&    !IS_AFFECTED(ch,AFF_BLIND)))
		&&  !IS_SET(pexit->exit_info,EX_CLOSED))
		{
		    found = TRUE;
		    strcat(doors,dirname[door]);
		}
	    }
	    if (!found)
		strcat(buf,"none");
	    sprintf(buf2,"%s",doors);
	    i = buf2; break;
	 case 'c' :
	    sprintf(buf2,"%s","\n\r");
	    i = buf2; break;
	 case 'h' :
	    sprintf( buf2, "%d", ch->hit );
	    i = buf2; break;
	 case 'H' :
	    sprintf( buf2, "%d", ch->max_hit );
	    i = buf2; break;
	 case 'v' :
	    sprintf( buf2, "%d", ch->move );
	    i = buf2; break;
	 case 'V' :
	    sprintf( buf2, "%d", ch->max_move );
	    i = buf2; break;
	case 'i' :
	    sprintf( buf2, "{%d %d %d}", ch->initiative[0], ch->initiative[1], ch->initiative[4]);
	    i = buf2; break;
	 case 'x' :
	    sprintf( buf2, "%ld", ch->currentexp );
	    i = buf2; break;
	 case 'b' :
	    sprintf(buf2, "%d", (ch->secondaryrace==3) ? ch->blood : 
				(ch->secondaryrace==1) ? ch->breaths : 0);
	    i = buf2; break;
	 case 'p' :
	    sprintf( buf2, "%d", ch->psp);
	    i = buf2; break;
	 case 'P' :
	    sprintf( buf2, "%d", ch->max_psp);
	    i = buf2; break;
	 case 'g' :
	    sprintf( buf2, "%ld", ch->gold);
	    i = buf2; break;
	 case 's' :
	    sprintf( buf2, "%ld", ch->silver);
	    i = buf2; break;
	 case 'a' :
	    if( ch->level > 9 )
	       sprintf( buf2, "%d", ch->alignment );
	    else
	       sprintf( buf2, "%s", IS_GOOD(ch) ? "good" : IS_EVIL(ch) ?
		"evil" : "neutral" );
	    i = buf2; break;
	 case 'r' :
	    if( ch->in_room != NULL )
	       sprintf( buf2, "%s", 
		((!IS_NPC(ch) && IS_SET(ch->act,PLR_HOLYLIGHT)) ||
		 (!IS_AFFECTED(ch,AFF_BLIND) && !room_is_dark( ch->in_room )))
		? ch->in_room->name : "darkness");
	    else
	       sprintf( buf2, " " );
	    i = buf2; break;
	 case 'R' :
	    if( IS_IMMORTAL( ch ) && ch->in_room != NULL )
	       sprintf( buf2, "%d", ch->in_room->vnum );
	    else
	       sprintf( buf2, " " );
	    i = buf2; break;
	 case 'z' :
	    if( IS_IMMORTAL( ch ) && ch->in_room != NULL )
	       sprintf( buf2, "%s", ch->in_room->area->name );
	    else
	       sprintf( buf2, " " );
	    i = buf2; break;
	 case 'o' :
	    if (ch->fighting == NULL) sprintf(buf2, "[ Not Fighting ]");
	    else
	    {
	        per = ch->fighting->hit * 100 / ch->fighting->max_hit;
	        if (per >= 100) sprintf(buf2,     "[  Excellent  ]");
	        else if (per >= 90) sprintf(buf2, "[   Bruised   ]");
	        else if (per >= 82) sprintf(buf2, "[  Scratched  ]");
	        else if (per >= 75) sprintf(buf2, "[   Grazed    ]");
	        else if (per >= 50) sprintf(buf2, "[   Wounded   ]");
	        else if (per >= 40) sprintf(buf2, "[ Big Wounds  ]");
	        else if (per >= 30) sprintf(buf2, "[    Hurt     ]");
	        else if (per >= 22) sprintf(buf2, "[Gaping Wounds]");
	        else if (per >= 15) sprintf(buf2, "[  Very Hurt  ]");
	        else if (per >= 7) sprintf(buf2, "[    Awful    ]");
	        else if (per >= 0) sprintf(buf2, "[Brink O Death]");
	        else sprintf(buf2, "[ Unconcious  ]");
	    }
	    i = buf2;break;
	 case 'O' :
	    per = ch->hit * 100 / ch->max_hit;
	    if (per >= 100) sprintf(buf2,     "[  Excellent  ]");
	    else if (per >= 90) sprintf(buf2, "[   Bruised   ]");
	    else if (per >= 82) sprintf(buf2, "[  Scratched  ]");
	    else if (per >= 75) sprintf(buf2, "[   Grazed    ]");
	    else if (per >= 50) sprintf(buf2, "[   Wounded   ]");
	    else if (per >= 40) sprintf(buf2, "[ Big Wounds  ]");
	    else if (per >= 30) sprintf(buf2, "[    Hurt     ]");
	    else if (per >= 22) sprintf(buf2, "[Gaping Wounds]");
	    else if (per >= 15) sprintf(buf2, "[  Very Hurt  ]");
	    else if (per >= 7) sprintf(buf2, "[    Awful    ]");
	    else if (per >= 0) sprintf(buf2, "[Brink O Death]");
	    else sprintf(buf2, "[ Unconcious  ]");
	    i = buf2;break;
      }
      ++str;
      while( (*point = *i) != '\0' )
	 ++point, ++i;
   }
   write_to_buffer( ch->desc, buf, point - buf);

   if (ch->prefix[0] != '\0')
	write_to_buffer(ch->desc,ch->prefix, 0);
   return;
}



/*
 * Append onto an output buffer.
 */
void write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, int length )
{
    char tb[MAX_STRING_LENGTH], ccode;
    int i,j;
    bool ansi, grfx = FALSE;

    if (d != NULL && d->as != NULL) d = d->as;

    /*
     * Find length in case caller didn't.
     */
    if ( length <= 0 )
	length = strlen(txt);

    if (d->character == NULL ||
	(d->connected != CON_PLAYING && d->connected != CON_EDITOR))
	ansi = FALSE;
    else ansi = (IS_SET(d->character->comm, COMM_ANSI)) ? TRUE : FALSE;

    /*
     * Initial \n\r if needed.
     */
    if ( d->outtop == 0 && !d->fcommand )
    {
	d->outbuf[0]    = '\n';
	d->outbuf[1]    = '\r';
	d->outtop       = 2;
    }

    /*
     * Expand d->outbuf for ansi info
     */

    j = 0;
    ccode = '3';
    tb[0] = '\0';
    for (i = 0; i < length ; i++)
    {
	if (txt[i] == '\n')
	{
// added to fix stupid PC telnet incompatibility with the rest of the world
		    tb[j++] = 27; tb[j++] = '['; tb[j++] = '1'; tb[j++] = ';';
		    tb[j++] = ccode; tb[j++] = '7'; tb[j++] = 'm';
// end.
		    tb[j++] = 27; tb[j++] = '['; tb[j++] = '0'; tb[j++] = 'm';
	}
	if (txt[i] == '#' && !(d->connected == CON_EDITOR && txt[i+1] != 'I' && txt[i+1] != 'N'))
	{
	    if (txt[++i] == 'B')
	    {
// removed
//		ccode = '4';
		ccode = '3';
		i++;
	    }
	    else ccode = '3';

	    switch (txt[i])
	    {
		default : break;
		case '#': tb[j++] = '#'; break;
		case '-': tb[j++] = '~'; break;
		case '+': tb[j++] = '%'; break;
		case 'I':
		case 'i': tb[j++] = 27; tb[j++] = '['; tb[j++] = '7';
			  tb[j++] = 'm'; break;
		case 'b': if (!ansi) continue;
/*		    tb[j++] = 27; tb[j++] = '['; tb[j++] = '5'; tb[j++] = 'm'; */
/* bold just selects colour white for now */
		    tb[j++] = 27; tb[j++] = '['; tb[j++] = '1'; tb[j++] = ';';
		    tb[j++] = '3'; tb[j++] = '7'; tb[j++] = 'm';
		    break;
		case 'G': if (!ansi) continue;
		    tb[j++] = 27; tb[j++]='('; tb[j++]='0'; grfx = TRUE;
		    break;
		case 'g': if (!ansi) continue;
		    tb[j++] = 27; tb[j++]='('; tb[j++]='B'; grfx = TRUE;
		    break;
		case 'N':
		case 'n':
// added to fix stupid PC telnet incompatibility with the rest of the world
		    tb[j++] = 27; tb[j++] = '['; tb[j++] = '1'; tb[j++] = ';';
		    tb[j++] = ccode; tb[j++] = '7'; tb[j++] = 'm';
// end.
		    tb[j++] = 27; tb[j++] = '['; tb[j++] = '0'; tb[j++] = 'm';
		    break;
		case '0': if (!ansi) continue;
		    tb[j++] = 27; tb[j++] = '['; tb[j++] = '1'; tb[j++] = ';';
		    tb[j++] = ccode; tb[j++] = '0'; tb[j++] = 'm';
		    break;
		case '1': if (!ansi) continue;
		    tb[j++] = 27; tb[j++] = '['; tb[j++] = '1'; tb[j++] = ';';
		    tb[j++] = ccode; tb[j++] = '1'; tb[j++] = 'm';
		    break;
		case '2': if (!ansi) continue;
		    tb[j++] = 27; tb[j++] = '['; tb[j++] = '1'; tb[j++] = ';';
		    tb[j++] = ccode; tb[j++] = '2'; tb[j++] = 'm';
		    break;
		case '3': if (!ansi) continue;
		    tb[j++] = 27; tb[j++] = '['; tb[j++] = '1'; tb[j++] = ';';
		    tb[j++] = ccode; tb[j++] = '3'; tb[j++] = 'm';
		    break;
		case '4': if (!ansi) continue;
		    tb[j++] = 27; tb[j++] = '['; tb[j++] = '1'; tb[j++] = ';';
		    tb[j++] = ccode; tb[j++] = '4'; tb[j++] = 'm';
		    break;
		case '5': if (!ansi) continue;
		    tb[j++] = 27; tb[j++] = '['; tb[j++] = '1'; tb[j++] = ';';
		    tb[j++] = ccode; tb[j++] = '5'; tb[j++] = 'm';
		    break;
		case '6': if (!ansi) continue;
		    tb[j++] = 27; tb[j++] = '['; tb[j++] = '1'; tb[j++] = ';';
		    tb[j++] = ccode; tb[j++] = '6'; tb[j++] = 'm';
		    break;
		case '7': if (!ansi) continue;
		    tb[j++] = 27; tb[j++] = '['; tb[j++] = '1'; tb[j++] = ';';
		    tb[j++] = ccode; tb[j++] = '7'; tb[j++] = 'm';
		    break;
	    }
	}
	else tb[j++] = txt[i];
    }

    if (d->connected != CON_EDITOR)
    {
    	tb[j++] = 27;
    	tb[j++] = '[';
    	tb[j++] = '0';
    	tb[j++] = 'm';
	if (grfx)
	{
		tb[j++] = 27;
		tb[j++]='('; 
		tb[j++]='B';
	}
    }
    tb[j]   = '\0';

    length = j;

    /*
     * Expand the buffer as needed.
     */
    while ( d->outtop + length >= d->outsize )
    {
	char *outbuf;

	if (d->outsize >= 32000)
	{
	    bug("Buffer overflow. Closing.\n\r",0);
	    close_socket(d);
	    return;
	}
	outbuf      = alloc_mem( 2 * d->outsize );
	strncpy( outbuf, d->outbuf, d->outtop );
	free_mem( d->outbuf, d->outsize );
	d->outbuf   = outbuf;
	d->outsize *= 2;
    }

    /*
     * Copy.
     */
    strcpy( d->outbuf + d->outtop, tb );
    d->outtop += length;
    return;
}



/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool write_to_descriptor( int desc, char *txt, int length )
{
    int iStart;
    int nWrite;
    int nBlock;
//    DESCRIPTOR_DATA *d;

#if defined(macintosh) || defined(MSDOS)
    if ( desc == 0 )
	desc = 1;
#endif

    if ( length <= 0 )
	length = strlen(txt);

    for ( iStart = 0; iStart < length; iStart += nWrite )
    {
	nBlock = UMIN( length - iStart, 4096 );
	if ( ( nWrite = write( desc, txt + iStart, nBlock ) ) < 0 )
	    { perror( "Write_to_descriptor" ); return FALSE; }
    } 

/*    for (d = descriptor_list; d != NULL; d = d->next)
    {
	if (d->descriptor == desc)
	{
	    d->sent_bytes[0] += length;
	    if (d->sent_bytes[0] > 9999)
	    {
		d->sent_bytes[0] -= 10000;
		d->sent_bytes[1] += 1;
		if (d->sent_bytes[1] > 9999)
		{
		    d->sent_bytes[1] -= 10000;
		    d->sent_bytes[2] += 1;
		    if (d->sent_bytes[3] > 30000)
		    {
			d->sent_secs[0]  = 0;
			d->sent_secs[1]  = 0;
			d->sent_pulse    = 0;
			d->sent_bytes[0] = 0;
			d->sent_bytes[1] = 0;
			d->sent_bytes[2] = 0;
		    }	    
		}
	    }
	}
    } */

    return TRUE;
}



/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny( DESCRIPTOR_DATA *d, char *argument )
{
    DESCRIPTOR_DATA *d_old, *d_next;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *ch;
    char *pwdnew;
    char *p;
    int race,i, plcnt;
    bool fOld;

    while ( isspace(*argument) )
	argument++;

    ch = d->character;

    switch ( d->connected )
    {

    default:
	bug( "Nanny: bad d->connected %d.", d->connected );
	close_socket( d );
	return;

    case CON_GET_NAME:
	if ( argument[0] == '\0' )
	{
	    close_socket( d );
	    return;
	}

        if ( !check_parse_name( argument ) )
	{
	    write_to_buffer( d, "Illegal name, try another.\n\rName: ", 0 );
	    return;
	}
	
	argument[0] = UPPER(argument[0]);
        if (!str_cmp(argument, "Jester"))
	    sprintf(argument, "Gurgle");
	else if (!str_cmp(argument, "Friend"))
	    sprintf(argument, "Mitzrah");
        else if (!str_cmp(argument, "George"))
	    sprintf(argument, "Whois");

	fOld = load_char_obj( d, argument );
	ch   = d->character;

	crash("loaded the char in comm.c");

	if (IS_SET(ch->act, PLR_DENY))
	{
	    crash("A player has been denied... that could crash it!");
	    sprintf( log_buf, "Denying access to %s@%s.", argument, d->host );
	    log_string( log_buf );
	    write_to_buffer( d, "You are denied access.\n\r", 0 );
	    close_socket( d );
	    return;
	}
  
	if (check_ban(d->host,BAN_PERMIT) && !IS_SET(ch->act,PLR_PERMIT))
	{
	    write_to_buffer(d,"Your site has been banned from this mud.\n\r",0);
	    close_socket(d);
	    return;
	}

	crash("loaded the char, now check_reconnect");

	if ( check_reconnect( d, argument, FALSE ) )
	{
	    fOld = TRUE;
	}
	else
	{
	    if ( wizlock && !IS_IMMORTAL(ch)) 
	    {
		write_to_buffer( d, "The game is wizlocked.\n\r", 0 );
		close_socket( d );
		return;
	    }
	}

	crash("loaded the char, now last login"); 

	if ( fOld )
	{
	    /* Old player */
	    crash("this bit means old players!  weird man pizza shit");
	    if (ch->pcdata != NULL)
	    {
		sprintf(buf, "\n\r>>>>---- Last login from: %s on %s ---->\r\n\n\r",
		    (ch->pcdata->last_login_from != NULL) ? ch->pcdata->last_login_from : "(Unknown)",
		    (ch->pcdata->last_login_time != NULL) ? ch->pcdata->last_login_time : "(Unknown)");
		write_to_buffer( d, buf, 0 );
	    }
	    write_to_buffer( d, "Password: ", 0 );
	    write_to_buffer( d, echo_off_str, 0 );
	    d->connected = CON_GET_OLD_PASSWORD;
	    return;
	}
	else
	{
	    /* New player */
	    if (newlock)
	    {
		crash("Well its fucking newlocked isnt it");
		write_to_buffer( d, "The game is newlocked.\n\r", 0 );
		close_socket( d );
		return;
	    }
	    crash("newbies?  fuck off!");

	    if (check_ban(d->host,BAN_NEWBIES))
	    {
		write_to_buffer(d,
		    "New players are not allowed from your site.\n\r",0);
		close_socket(d);
		return;
	    }
	    write_to_buffer( d, "-=[**]=-=[**]=-=[**]=-=[**]=-=[**]=-=[**]=-=[**]=-=[**]=-=[**]=-=[**]=-=[**]=-\n\r\n\r", 0);
	    write_to_buffer( d, "Your character name has not been recognised.  If you wish to create a new\r\n", 0 );
	    write_to_buffer( d, "charcter with this name, please make sure that this is a suitable name.\r\n", 0 );
	    write_to_buffer( d, "Nonsensical names like Ugawangogo just aren't fashionable, and phrases\n\r", 0 );
	    write_to_buffer( d, "shortened into names like Cizmyass aren't in either.  If you want to be\n\r", 0 );
	    write_to_buffer( d, "really fashionable then please choose a cool name, like Strom, or Hairball\r\n", 0 );
	    write_to_buffer( d, "(guess who wrote this), and then the imms will let you keep it too..\r\n\n\r", 0 );
	    write_to_buffer( d, "-=[**]=-=[**]=-=[**]=-=[**]=-=[**]=-=[**]=-=[**]=-=[**]=-=[**]=-=[**]=-=[**]=-\n\r\n\r", 0);
	    sprintf( buf, "Did I get that right, %s (Y/N)? ", argument );
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_CONFIRM_NEW_NAME;
	    return;
	}
	crash("loaded the char after everything"); 

	break;

    case CON_GET_OLD_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	write_to_buffer( d, echo_on_str, 0 );
	crash("Password entered.");
	if (ch->version < 8)
	{
	    int i;

	    ch->secondaryrace = 0;
	    ch->tertiaryrace = 0;
	    for (i = 0; i < MAX_DISCIPLINES; i++)
		ch->power[i] = -2;
	    ch->qp = 0;
	    ch->max_hit = ch->max_hit / 2;
	    ch->pcdata->perm_hit = ch->pcdata->perm_hit / 2; 
	    if (ch->max_hit < 100) ch->max_hit = 100;
	    ch->hit = ch->max_hit;
	    for (i = 0; i < MAX_SKILL; i++)
	    {
		if (skill_table[i].spell_fun == spell_null)
		    ch->pcdata->learned[i] = 0;
		else
		    ch->pcdata->learned[i] = -1;
	    }
	    ch->psp = 0;
	    ch->max_psp = 0;
	    ch->faith = 0;
	    ch->body_form = 0;
	    ch->beast = 0;
	    ch->blood = 100;
	    ch->generation = 0;
	    ch->dragtype = 0;
	    ch->aura = 0;
	    ch->breaths = 0;
	    ch->pkills = 0;
	    ch->pkilled = 0;
	    ch->fire = 0;
	    ch->water = 0;
	    ch->air = 0;
	    ch->earth = 0;
	    ch->entropy = 0;
	    ch->lifemagic = 0;
	    ch->primary = 0;
	    ch->true_faith = 0;
	    for ( i = 0; i < NUM_PSI_SLOTS; i++ )
		ch->psi_learnt[i] = -1;
	    ch->psi_level = 0;
	    ch->clan = CLAN_NONE;
	    ch->level = 1;
	    set_form( ch, 0 );
	    ch->version = 8;
	    ch->gold = 0;
	    ch->silver = 0;
	    ch->sapphires = 0;
 	    ch->emeralds = 0;
	    ch->rage = 0;
	    ch->xhitroll = 0;
	    ch->xdamroll = 0;
	    ch->currentexp = 0;
	    ch->exp = 0;
	
	    stc("Player file updated to version 8.\n\r", ch);
	    save_char_obj(ch);

	}
	crash("Done version check for version 7.");
	if (ch->version < 6)
	{
	    char *newpwd;
	    crash("Version < 6 updates");
	    if ( strcmp( argument, ch->pcdata->pwd )
		&& ch->pcdata->pwd != "getin" )
	    {
		bug("Wrong password.",0);
		bug(argument, 0);
		bug(ch->pcdata->pwd,0);
		write_to_buffer( d, "Wrong password.\n\r",0);
		close_socket( d );
		return;
	    }

	    write_to_buffer( d, "                **** OLD CHARACTER FILE ****\n\r", 0);
	    write_to_buffer( d, "Due to the vast number of changes made to the game you will need to\n\r", 0);
	    write_to_buffer( d, "recreate this character and start again.  The character deletion will allow\n\r", 0);
	    write_to_buffer( d, "you to become more powerful in the long run as otherwise many of the new\n\r", 0);
	    write_to_buffer( d, "features wouldn't be available.  In addition to this, there are now\n\r", 0);
	    write_to_buffer( d, "more security measures to stop hackers being able to gain access to your\n\r", 0);
	    write_to_buffer( d, "characters, and knowing your password.\n\r", 0);
	    write_to_buffer( d, "\n\r", 0);

	    newpwd = str_dup( crypt( ch->pcdata->pwd, ch->name ) );
	    free_string( ch->pcdata->pwd );
	    ch->pcdata->pwd = newpwd;
	    write_to_buffer(d,"You can be:\n\r",0);
	    for ( race = 1; race_table[race].name != NULL; race++ )
	    {
	    	if (!race_table[race].pc_race)
		    break;
	    	write_to_buffer(d,race_table[race].name,0);
	    	write_to_buffer(d," ",1);
	    }
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,"What is your race (help for more information)? ",0);
	    d->connected = CON_GET_NEW_RACE;
	    return;
	}
/* this code is dangerous.. and never needed for anything here. -- whois
	else if (ch->pcdata != NULL && ch->pcdata->switchname != NULL
		&& (!str_cmp(GET_PC_NAME(ch), "Noone") ||
                    !str_cmp(GET_PC_NAME(ch), "Someone")))
	{
	    if (strcmp(argument, "flabdo"))
	    {
		write_to_buffer( d, "You are a fraud.  Attempted login recorded.\n\r", 0);
		close_socket( d );
		return;
	    }
	}
*/ 
	else if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ))
	{
	    write_to_buffer( d, "Wrong password.\n\r", 0 );
	    close_socket( d );
	    return;
	}

	crash("Password verified.");

	REMOVE_BIT(ch->flag4, AFF_BEING_SUCKED);
	ch->sucking = NULL;

	if (check_playing(d,GET_PC_NAME(ch)))
	    return;

	if ( check_reconnect( d, ch->name, TRUE ) )
	    return;

	sprintf( log_buf, "%s@%s has connected.", GET_PROPER_NAME(ch), d->host );
	log_string( log_buf );
	wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));

	look_name(ch);
	ch->mob_spec = 0;

 	/* Olc current editing stuff */
	ch->pcdata->curr_vnum_medit = -1;
	ch->pcdata->curr_vnum_oedit = -1;

        ch->riding = NULL;
        ch->blinkykill = NULL;
	ch->riden_by = NULL;
        ch->shadow_victim = NULL;
	ch->shadow_master = NULL;
 	REMOVE_BIT(ch->flag4, AFF_SHADOW_HIDE);
	REMOVE_BIT(ch->flag4, AFF_QUITTING);
	REMOVE_BIT(ch->flag5, AFF5_SWITCHED);
	if (!IS_FLAG5(ch, AFF5_NO_PROMPT_FOR_ANSI))
	{
	    write_to_buffer(d, "Do you want ANSI Colour [y/n]? ", 0);
            d->connected = CON_WANT_ANSI;
	    break;
 	}

	if (ch->pcdata->plan != NULL
		&& ch->pcdata->plan[0] == 'D'
		&& ch->pcdata->plan[1] == 'I'
		&& ch->pcdata->plan[2] == 'S'
		&& ch->pcdata->plan[3] == 'A'
		&& ch->pcdata->plan[4] == 'B'
		&& ch->pcdata->plan[5] == 'L'
		&& ch->pcdata->plan[6] == 'E'
		&& ch->pcdata->plan[7] == 'D'
		&& ch->pcdata->plan[8] == ':')
	{
	    stc("\n\n\n\n\r", ch);
	    stc("Character File Disabled\n\r========= ==== ========\n\n\n\r", ch);
	    stc("Reason: \n\n\r", ch);
	    if (ch->pcdata->plan[9] == 0)
		stc("Unknown.\n\r", ch);
	    else
		stc(&ch->pcdata->plan[9], ch);
	    stc("\n\n\n\n\r", ch);
	    close_socket(d);
	}



	if ( IS_IMMORTAL(ch) )
	{
	    do_help( ch, "imotd" );
	    d->connected = CON_READ_IMOTD;
	}
	else
	{
	    do_help( ch, "motd" );
	    d->connected = CON_READ_MOTD;
	}
	break;

    case CON_WANT_ANSI:
	switch ( *argument )
	{
	case 'y': case 'Y':
	    SET_BIT(ch->comm, COMM_ANSI);
	    d->connected = CON_PLAYING;
	    write_to_buffer( d, "Welcome to ANSI #1C#2o#3l#4o#5u#6r#n.\n\r", 0 );

	if ( IS_IMMORTAL(ch) )
	{
	    do_help( ch, "imotd" );
	    d->connected = CON_READ_IMOTD;
	}
	else
	{
	    do_help( ch, "motd" );
	    d->connected = CON_READ_MOTD;
	}
	    break;

	case 'n': case 'N':
	    REMOVE_BIT(ch->comm, COMM_ANSI);
	    write_to_buffer( d, "ANSI Colour disabled.\n\r", 0 );
	if ( IS_IMMORTAL(ch) )
	{
	    do_help( ch, "imotd" );
	    d->connected = CON_READ_IMOTD;
	}
	else
	{
	    do_help( ch, "motd" );
	    d->connected = CON_READ_MOTD;
	}
	    break;

	default:
	    write_to_buffer( d, "Please type Yes or No? ", 0 );
	    d->connected = CON_WANT_ANSI;
	    break;
	}

	break;

 
    case CON_BREAK_CONNECT:
	switch( *argument )
	{
	case 'y' : case 'Y':
	    for ( d_old = descriptor_list; d_old != NULL; d_old = d_next )
	    {
		d_next = d_old->next;
		if (d_old == d || d_old->character == NULL)
		    continue;

		if (str_cmp(ch->name,d_old->original ?
		    d_old->original->name : d_old->character->name))
		    continue;

		close_socket(d_old);
	    }
	    if (check_reconnect(d,ch->name,TRUE))
		return;
	    write_to_buffer(d,"Reconnect attempt failed.\n\rName: ",0);
	    if ( d->character != NULL )
	    {
		free_char( d->character );
		d->character = NULL;
	    }
	    d->connected = CON_GET_NAME;
	    break;

	case 'n' : case 'N':
	    write_to_buffer(d,"Name: ",0);
	    if ( d->character != NULL )
	    {
		free_char( d->character );
		d->character = NULL;
	    }
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer(d,"Please type Y or N? ",0);
	    break;
	}
	break;

    case CON_CONFIRM_NEW_NAME:
	switch ( *argument )
	{
	case 'y': case 'Y':
	    sprintf( buf, "Welcome new character!\n\rGive me a password for %s: %s",
		ch->name, echo_off_str );
	    write_to_buffer( d, buf, 0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    break;

	case 'n': case 'N':
	    write_to_buffer( d, "Ok, what IS it, then? ", 0 );
	    free_char( d->character );
	    d->character = NULL;
	    d->connected = CON_GET_NAME;
	    break;

	default:
	    write_to_buffer( d, "Please type Yes or No? ", 0 );
	    break;
	}
	break;

    case CON_GET_NEW_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strlen(argument) < 5 )
	{
	    write_to_buffer( d,
		"Password must be at least five characters long.\n\rPassword: ",
		0 );
	    return;
	}

	pwdnew = crypt( argument, ch->name );
	for ( p = pwdnew; *p != '\0'; p++ )
	{
	    if ( *p == '~' )
	    {
		write_to_buffer( d,
		    "New password not acceptable, try again.\n\rPassword: ",
		    0 );
		return;
	    }
	}

	free_string( ch->pcdata->pwd );
	ch->pcdata->pwd = str_dup( pwdnew );
	write_to_buffer( d, "Please retype password: ", 0 );
	d->connected = CON_CONFIRM_NEW_PASSWORD;
	break;

    case CON_CONFIRM_NEW_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

	if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
	{
	    write_to_buffer( d, "Passwords don't match.\n\rRetype password: ",
		0 );
	    d->connected = CON_GET_NEW_PASSWORD;
	    return;
	}

	write_to_buffer( d, echo_on_str, 0 );
	write_to_buffer(d,"The following races are available:\n\r  ",0);
	for ( race = 1; race_table[race].name != NULL; race++ )
	{
	    if (!race_table[race].pc_race)
		break;
	    write_to_buffer(d,race_table[race].name,0);
	    write_to_buffer(d," ",1);
	}
	write_to_buffer(d,"\n\r",0);
	write_to_buffer(d,"What is your race (help for more information)? ",0);
	d->connected = CON_GET_NEW_RACE;
	break;

    case CON_GET_NEW_RACE:
	one_argument(argument,arg);

	if (!strcmp(arg,"help"))
	{
	    argument = one_argument(argument,arg);
	    if (argument[0] == '\0')
		do_help(ch,"race help");
	    else
		do_help(ch,argument);
	    write_to_buffer(d,
		"What is your race (help for more information)? ",0);
	    break;
	}

	race = race_lookup(argument);

	if (race == 0 || !race_table[race].pc_race)
	{
	    write_to_buffer(d,"That is not a valid race.\n\r",0);
	    write_to_buffer(d,"The following races are available:\n\r  ",0);
	    for ( race = 1; race_table[race].name != NULL; race++ )
	    {
		if (!race_table[race].pc_race)
		    break;
		write_to_buffer(d,race_table[race].name,0);
		write_to_buffer(d," ",1);
	    }
	    write_to_buffer(d,"\n\r",0);
	    write_to_buffer(d,
		"What is your race? (help for more information) ",0);
	    break;
	}

	ch->race = race;
	/* initialize stats */
	for (i = 0; i < MAX_STATS; i++)
	    ch->perm_stat[i] = pc_race_table[race].stats[i];
	ch->affected_by = ch->affected_by|race_table[race].aff;
	ch->imm_flags   = ch->imm_flags|race_table[race].imm;
	ch->res_flags   = ch->res_flags|race_table[race].res;
	ch->vuln_flags  = ch->vuln_flags|race_table[race].vuln;
	ch->form        = race_table[race].form;
	ch->parts       = race_table[race].parts;

	/* add skills */
	for (i = 0; i < 5; i++)
	{
	    if (pc_race_table[race].skills[i] == NULL)
		break;
	    group_add(ch,pc_race_table[race].skills[i],FALSE);
	}
	/* add cost */
	ch->pcdata->points = pc_race_table[race].points;
	ch->size = pc_race_table[race].size;

	write_to_buffer( d, "What is your sex (M/F)? ", 0 );
	d->connected = CON_GET_NEW_SEX;
	break;
	

    case CON_GET_NEW_SEX:
	switch ( argument[0] )
	{
	case 'm': case 'M': ch->sex = SEX_MALE;    
			    ch->pcdata->true_sex = SEX_MALE;
			    break;
	case 'f': case 'F': ch->sex = SEX_FEMALE; 
			    ch->pcdata->true_sex = SEX_FEMALE;
			    break;
	default:
	    write_to_buffer( d, "That's not a sex.\n\rWhat IS your sex? ", 0 );
	    return;
	}

	sprintf( log_buf, "%s@%s new player.", ch->name, d->host );
	log_string( log_buf );
	wiznet("Newbie alert!  $N sighted.",ch,NULL,WIZ_NEWBIE,0,0);
	wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));

	ch->alignment = 750;
	
	/* This bit gives the player all the skills in the game at 0 */
	for (i = 0 ; i < MAX_SKILL ; i++)
 	{
	    if (skill_table[i].spell_fun == spell_null)
		ch->pcdata->learned[i] = 0;
	}

	/* This bit sets all the tree to zero... very helpful */
	for (i = 0 ; i < MAX_SKILL_TREE ; i++)
	    ch->pcdata->treelearn[i] = 0;


	write_to_buffer( d, "\n\r", 2 );

	look_name(ch);

	for (i = 0; weapon_table[i].name != NULL; i++)
		ch->pcdata->learned[*weapon_table[i].gsn] = 0;

	write_to_buffer(d,"\n\r",2);
	write_to_buffer(d, "Do you want ANSI Colour [y/n]? ", 0);
        d->connected = CON_WANT_ANSI;
	break;

    /* case CON_MANUAL:
	if (!str_cmp(argument,"play"))
	    d->connected = CON_PLAYING;
	do_manual(ch, argument);
	break;*/

    case CON_READ_IMOTD:
	write_to_buffer(d,"\n\r",2);
	do_help( ch, "motd" );
	d->connected = CON_READ_MOTD;
	break;

    case CON_READ_MOTD:
	if ( ch->pcdata == NULL || ch->pcdata->pwd[0] == '\0')
	{
	    write_to_buffer( d, "Warning! Null password!\n\r",0 );
	    write_to_buffer( d, "Please report old password with bug.\n\r",0);
	    write_to_buffer( d,
		"Type 'password null <new password>' to fix.\n\r",0);
	}
	write_to_buffer( d, 
    "\n\rWelcome to BTL 3.0  Please do not feed the implementors.\n\r\n\r",
	    0 );
	ch->next        = char_list;
	char_list       = ch;
	d->connected    = CON_PLAYING;
	for (i = 0; i < 10; i++)
	    d->inlast[i][0] = '\0';
	d->inlasti = 0;
	reset_char(ch);

	if ( ch->level == 0 || ch->version < 6)
	{
	    int p;
	    for (p = 1; p < 9; p++)
		ch->elder[p] = -1;

	    for (p = 1; p < MAX_DISCIPLINES ; p++)
		ch->power[p] = -2;

	    for (p = 1; p < 14; p++)
		ch->advpower[p] = -1;

	    for (p = 0; p < 8; p++)
		ch->pcdata->colours[p] = 3;
	    ch->perm_stat[0] += dice(1,5) - 3;
	    ch->perm_stat[1] += dice(1,5) - 3;
	    ch->perm_stat[2] += dice(1,5) - 3;
	    ch->perm_stat[3] += dice(1,5) - 3;
	    ch->perm_stat[4] += dice(1,5) - 3;
	    ch->perm_stat[5] += dice(1,5) - 3;
	    ch->psi_potential  = (ch->perm_stat[STAT_WIS] + ch->perm_stat[STAT_CON]/2 + ch->perm_stat[STAT_INT]/2 + dice(1,4))/2;
	    ch->value[4] = 25;
	    ch->level   = 1;
	    ch->secondaryrace = 0;
	    ch->version = 8;
	    ch->auto_look_line = -1;
	    ch->tertiaryrace = 0;
	    ch->pcdata->damcap = 40;
	    ch->exp     = 1000;
	    ch->pcdata->voodoo_points = 0;
	    ch->best_mob_killed = 0;
	    ch->pcdata->mob_kills = 0;
	    ch->pcdata->mob_killed = 0;
	    ch->currentexp = 1000;
	    ch->hit     = ch->max_hit;
	    ch->xhitroll = 0;
	    ch->xdamroll = 0;
	    ch->move    = ch->max_move;
	    ch->max_psp = 0;
	    ch->psp     = 0;
	    ch->emeralds = 0;
	    ch->diamonds = 0;
	    ch->sapphires = 0;
	    ch->gold = dice(1, 5);
	    ch->silver = dice(1, 10);
	    ch->copper = dice(1, 20);
	    ch->flag5 = 0;
	    ch->flag4 = 0;
	    ch->flag3 = 0;
	    ch->mob_spec = 0;
	    ch->style_skill[0] = 0;
	    ch->style_skill[1] = 0;
	    ch->style_skill[2] = 0;
	    ch->style_skill[3] = 0;
	    ch->style_skill[4] = 0;
	    ch->style_skill[5] = 0;
	    ch->style_skill[6] = 0;
	    ch->style_skill[7] = 0;
	    ch->pcdata->souls = 0;
	    ch->dragtype = 0;
	    ch->aura = 0;
	    ch->curlang = 1;
	    ch->spikes = 0;
	    ch->lifemagic = 0;
	    ch->pcdata->title_prefix = NULL;
	    ch->pcdata->homepage = NULL;
	    ch->pcdata->email	= NULL;
	    ch->pcdata->real_name = NULL;
	    ch->pcdata->plan	= NULL;
	    ch->married_to = NULL;
	    ch->blinkykill = NULL;
	    ch->riding		= NULL;
	    ch->pcdata->quests_completed = 0;
	    ch->pcdata->quest_status = 0;
	    ch->pcdata->conditioned_by	= NULL;
	    ch->recall_room     = 0;
	    ch->pcdata->treelearn[SKILL_WEAPON] = dice(1, 3) + 2;
	    ch->pcdata->treelearn[SKILL_COMBAT] = dice(1, 3);
	    ch->pcdata->treelearn[SKILL_WIZARD] = dice(1, 2);
	    ch->pcdata->treelearn[SKILL_PRIEST] = dice(1, 2);
	    ch->clan		= 0;
	    ch->pcdata->disc_points	= 0;
	    ch->pcdata->disc_research	= -1;
	    ch->breaths		= 0;
	    REMOVE_BIT(ch->flag4, AFF_BEING_SUCKED);
	    ch->sucking = NULL;
	    ch->shadow_master	= NULL;
	    ch->shadow_victim	= NULL;
	    ch->riden_by 	= NULL;
	    ch->altitude 	= 0;
	    ch->primary 	= 0;
	    ch->blood		= 100;
	    ch->beast		= 0;
	    ch->realm		= prime_realm;
	    SET_TIMER(ch, TIMER_NEWBIE_IMM, 30);
	    SET_BIT(ch->act, PLR_AUTOEXIT);

	    for (p = 0 ; p < 11 ; p++)
	    {
		ch->pcdata->max_memorize[p] = 0;
		for (i = 0 ; i < 11 ; i++)
		{
		    ch->pcdata->memorize[p][i] = -1;
		    ch->pcdata->last_memorized[p][i] = -1;
		}
	    }

	    set_switchname(ch, ch->name);
	    if (get_room_index(prime_realm, 3054) != NULL)
	        char_to_room( ch, get_room_index( prime_realm, 3054 ),0 );
	    else char_to_room( ch, get_room_index( prime_realm, 1 ),0 );
	    send_to_char("\n\r",ch);
	    if (ch->version < 6) do_help(ch, "changes");
	    else do_help(ch,"NEWBIE");
	    send_to_char("\n\r",ch);
	    outfit(ch);
	    if (ch->race == RACE_ELF && ch->pcdata->true_sex == SEX_FEMALE)
	        do_title(ch, "the female elf");
	    else if (ch->race == RACE_ELF && ch->pcdata->true_sex == SEX_MALE)
	        do_title(ch, "the male elf");
	    else if (ch->race == RACE_GNOME && ch->pcdata->true_sex == SEX_FEMALE)
	        do_title(ch, "the female gnome");
	    else if (ch->race == RACE_GNOME && ch->pcdata->true_sex == SEX_MALE)
	        do_title(ch, "the male gnome");
	    else if (ch->race == RACE_HALF_ELF && ch->pcdata->true_sex == SEX_FEMALE)
	        do_title(ch, "the female half-elf");
	    else if (ch->race == RACE_HALF_ELF && ch->pcdata->true_sex == SEX_MALE)
	        do_title(ch, "the male half-elf");
	    else if (ch->race == RACE_HUMAN && ch->pcdata->true_sex == SEX_FEMALE)
	        do_title(ch, "the female human");
	    else if (ch->race == RACE_HUMAN && ch->pcdata->true_sex == SEX_MALE)
	        do_title(ch, "the male human");
	}
	else if ( ch->in_room != NULL )
	{
	    char_to_room( ch, ch->in_room, ch->altitude );
	}
	else if ( IS_IMMORTAL(ch) )
	{
	    char_to_room( ch, get_room_index( prime_realm, ROOM_VNUM_START ),0 );
	}
	else
	{
	    char_to_room( ch, get_room_index( prime_realm, ROOM_VNUM_START ),0 );
	}
	

	sprintf(log_buf, "$n has enter the world of %s.", SHORT_MUD_NAME);
        act( log_buf, ch, NULL, NULL, TO_ROOM );
	do_look( ch, "auto" );
	ch->sucking = NULL;
	REMOVE_BIT(ch->flag4, AFF_BEING_SUCKED);
	
	if (ch->pet != NULL)
	{
	    char_from_room( ch->pet );
	    char_to_room(ch->pet,ch->in_room,ch->altitude);
	    act( log_buf,ch->pet,NULL,NULL,TO_ROOM);
	}

	sprintf(log_buf, "$N has enter the world of %s.", SHORT_MUD_NAME);
	wiznet(log_buf,ch,NULL, WIZ_LOGINS,WIZ_SITES,get_trust(ch));

	if (ch->pcdata->last_login_time != NULL)
	    free_string(ch->pcdata->last_login_time);
	sprintf(buf, "%s", (char *) ctime( &current_time ));
	buf[strlen(buf) - 1] = '\0';
	ch->pcdata->last_login_time = str_dup( buf );

	if (ch->pcdata->last_login_from != NULL)
	    free_string(ch->pcdata->last_login_from);
	ch->pcdata->last_login_from = str_dup( d->host );

	plcnt = 0;

	for (d = descriptor_list; d != NULL; d = d->next)
	{
	    if (d->connected == CON_PLAYING)
		plcnt++;
	}

	if (plcnt > config_info->max_players)
	{
	    config_info->max_players = plcnt;
	    save_config();
	}

	do_unread(ch,"");
	break;
    }

    return;
}



/*
 * Parse a name for acceptability.
 */
bool check_parse_name( char *name )
{
    bool news = FALSE;
    int i; 

    /*
     * Reserved words.
     */
    if ( is_name( name, 
	"all auto immortal self someone something the you demise balance circle loner honor shit fuck wank bastard clinton elf human gnome half half-elf dalamar restrict allow mitzrah whois gurgle") )
	return FALSE;
	

    for (i=0;i<strlen(name);i++)
    {
	if (name[i] != 'n'
	    && name[i] != 's'
	    && name[i] != 'u'
	    && name[i] != 'd'
	    && name[i] != 'e'
	    && name[i] != 'w') news = TRUE;
    }
    if (!news) return FALSE;

    /*
     * Length restrictions.
     */
     
    if ( strlen(name) <  2 )
	return FALSE;

#if defined(MSDOS)
    if ( strlen(name) >  8 )
	return FALSE;
#endif

#if defined(macintosh) || defined(unix)
    if ( strlen(name) > 12 )
	return FALSE;
#endif

    /*
     * Alphanumerics only.
     * Lock out IllIll twits.
     */
    {
	char *pc;
	bool fIll,adjcaps = FALSE,cleancaps = FALSE;
	int total_caps = 0;

	fIll = TRUE;
	for ( pc = name; *pc != '\0'; pc++ )
	{
	    if ( !isalpha(*pc) )
		return FALSE;

	    if ( isupper(*pc)) /* ugly anti-caps hack */
	    {
		if (adjcaps)
		    cleancaps = TRUE;
		total_caps++;
		adjcaps = TRUE;
	    }
	    else
		adjcaps = FALSE;

	    if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
		fIll = FALSE;
	}

	if ( fIll )
	    return FALSE;

	if (cleancaps || (total_caps > (strlen(name)) / 2 && strlen(name) < 3))
	    return FALSE;
    }

    /*
     * Prevent players from naming themselves after mobs.
     *
    {
	extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
	MOB_INDEX_DATA *pMobIndex;
	int iHash;

	for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
	{
	    for ( pMobIndex  = mob_index_hash[iHash];
		  pMobIndex != NULL;
		  pMobIndex  = pMobIndex->next )
	    {
		if ( is_name( name, pMobIndex->player_name ) )
		    return FALSE;
	    }
	}
    }*/

    return TRUE;
}



/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect( DESCRIPTOR_DATA *d, char *name, bool fConn )
{
    CHAR_DATA *ch;

    for ( ch = char_list; ch != NULL; ch = ch->next )
    {
	if ( !IS_NPC(ch)
	&&   (!fConn || ch->desc == NULL)
	&&   !str_cmp( GET_PC_NAME(d->character), GET_PC_NAME(ch) ) )
	{
	    if ( fConn == FALSE )
	    {
		free_string( d->character->pcdata->pwd );
		d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
	    }
	    else
	    {
		free_char( d->character );
		d->character = ch;
		ch->desc         = d;
		ch->timer        = 0;
		if (d->connected == CON_EDITOR) char_to_game( d->character );
		send_to_char(
		    "Reconnecting. Type replay to see missed tells.\n\r", ch );
		act( "$n has reconnected.", ch, NULL, NULL, TO_ROOM );

		sprintf( log_buf, "%s@%s reconnected.", ch->name, d->host );
		log_string( log_buf );
		wiznet("$N groks the fullness of $S link.",
		    ch,NULL,WIZ_LINKS,0,0);
		d->connected = CON_PLAYING;
	    }
	    return TRUE;
	}
    }

    return FALSE;
}



/*
 * Check if already playing.
 */
bool check_playing( DESCRIPTOR_DATA *d, char *name )
{
    DESCRIPTOR_DATA *dold;

    for ( dold = descriptor_list; dold; dold = dold->next )
    {
	if ( dold != d
	&&   dold->character != NULL
	&&   dold->connected != CON_GET_NAME
	&&   dold->connected != CON_GET_OLD_PASSWORD
	&&   !str_cmp( name, dold->original
		 ? GET_PC_NAME(dold->original) : GET_PC_NAME(dold->character) ) )
	{
	    write_to_buffer( d, "That character is already playing.\n\r",0);
	    write_to_buffer( d, "Do you wish to connect anyway (Y/N)?",0);
	    d->connected = CON_BREAK_CONNECT;
	    return TRUE;
	}
    }

    return FALSE;
}



void stop_idling( CHAR_DATA *ch )
{
    if ( ch == NULL
    ||   ch->desc == NULL
    ||   ch->desc->connected != CON_PLAYING
    ||   ch->was_in_room == NULL 
    ||   ch->in_room != get_room_index(prime_realm, ROOM_VNUM_LIMBO))
	return;

    ch->timer = 0;
    char_from_room( ch );
    char_to_room( ch, ch->was_in_room, ch->altitude );
    ch->was_in_room     = NULL;
    act( "$n falls out of the sky with a bang.", ch, NULL, NULL, TO_ROOM );
    return;
}



/*
 * Write to one char.
 */
void send_to_char( const char *txt, CHAR_DATA *ch )
{
    if ( txt != NULL && ch->desc != NULL )
	write_to_buffer( ch->desc, txt, strlen(txt) );
    return;
}

void stc( const char *txt, CHAR_DATA *ch )
{
    if ( txt != NULL && ch->desc != NULL )
	write_to_buffer( ch->desc, txt, strlen(txt) );
    return;
}

int col_str_len(char *txt)
{
    int pos, len;

    len = 0;
    for (pos = 0; txt[pos] != '\0'; pos++)
    {
	if (txt[pos] != '#')
	{
	    len++;
	    continue;
	}

	pos++;
	if (txt[pos] == '\0') return len;
	if (txt[pos] == '#' || txt[pos] == '-') len++;
    }

    return len;
}

void ctc(char *txt, CHAR_DATA *ch)
{
    cent_to_char(txt, ch);
}

void cent_to_buf(char *txt, BUFFER *out)
{
    int len,pos;
    char buf[MAX_STRING_LENGTH];

    len = (80 - col_str_len(txt)) / 2;
    for (pos = 0; pos < len; pos++)
    {
	buf[pos] = ' ';
    }
    buf[pos]= '\0';
    add_buf(out, buf);
    add_buf(out, txt);
    add_buf(out, "\n\r");
}

void cent_to_char(char *txt, CHAR_DATA *ch)
{
    int len,pos;
    char buf[MAX_STRING_LENGTH];

    len = (80 - col_str_len(txt)) / 2;
    for (pos = 0; pos < len; pos++)
    {
	buf[pos] = ' ';
    }
    buf[pos]= '\0';
    send_to_char(buf, ch);
    send_to_char(txt, ch);
    send_to_char("\n\r",ch);
}

/*
 * Send a page to one char.
 */
void page_to_char( const char *txt, CHAR_DATA *ch )
{
    /* Crappy bugfix by Jander */
    if ( txt == NULL || ch->desc == NULL) return;

    if (ch->lines == 0 )
    {
	send_to_char(txt,ch);
	return;
    }
	
#if defined(macintosh)
	send_to_char(txt,ch);
#else
    ch->desc->showstr_head = alloc_mem(strlen(txt) + 1);
    strcpy(ch->desc->showstr_head,txt);
    ch->desc->showstr_point = ch->desc->showstr_head;
    show_string(ch->desc,"");
#endif
}


/* string pager */
void show_string(struct descriptor_data *d, char *input)
{
    char buffer[4*MAX_STRING_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    register char *scan, *chk;
    int lines = 0, toggle = 1;
    int show_lines;

    one_argument(input,buf);
    if (buf[0] != '\0')
    {
	if (d->showstr_head)
	{
	    free_mem(d->showstr_head,strlen(d->showstr_head));
	    d->showstr_head = 0;
	}
	d->showstr_point  = 0;
	return;
    }

    if (d->character)
	show_lines = d->character->lines;
    else
	show_lines = 0;

    for (scan = buffer; ; scan++, d->showstr_point++)
    {
	if (((*scan = *d->showstr_point) == '\n' || *scan == '\r')
	    && (toggle = -toggle) < 0)
	    lines++;

	else if (!*scan || (show_lines > 0 && lines >= show_lines))
	{
	    *scan = '\0';
	    write_to_buffer(d,buffer,strlen(buffer));
	    for (chk = d->showstr_point; isspace(*chk); chk++);
	    {
		if (!*chk)
		{
		    if (d->showstr_head)
		    {
			free_mem(d->showstr_head,strlen(d->showstr_head));
			d->showstr_head = 0;
		    }
		    d->showstr_point  = 0;
		}
	    }
	    return;
	}
    }
    return;
}
	

/* quick sex fixer */
void fix_sex(CHAR_DATA *ch)
{
    if (ch->sex < 0 || ch->sex > 2)
	ch->sex = IS_NPC(ch) ? 0 : ch->pcdata->true_sex;
}

void act (const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2,
	  int type)
{
    /* to be compatible with older code */
    act_new(format,ch,arg1,arg2,type,POS_RESTING);
}

void act_new( const char *format, CHAR_DATA *ch, const void *arg1, 
	      const void *arg2, int type, int min_pos)
{
    static char * const he_she  [] = { "it",  "he",  "she" };
    static char * const him_her [] = { "it",  "him", "her" };
    static char * const his_her [] = { "its", "his", "her" };
 
    char buf[MAX_STRING_LENGTH];
    char fname[MAX_INPUT_LENGTH];
    CHAR_DATA *to;
    CHAR_DATA *vch = (CHAR_DATA *) arg2;
    OBJ_DATA *obj1 = (OBJ_DATA  *) arg1;
    OBJ_DATA *obj2 = (OBJ_DATA  *) arg2;
    const char *str;
    const char *i;
    char *point;
    int alt;
 
    /*
     * Discard null and zero-length messages.
     */
    if ( format == NULL || format[0] == '\0' )
	return;

    /* discard null rooms and chars */
    if (ch == NULL || ch->in_room == NULL)
	return;

    to = ch->in_room->people;
    alt = ch->altitude;
    if ( type == TO_VICT )
    {
	if ( vch == NULL )
	{
	    bug( "Act: null vch with TO_VICT.", 0 );
	    return;
	}

	if (vch->in_room == NULL)
	    return;

	to = vch->in_room->people;
	alt = vch->altitude;
    }
 
    for ( ; to != NULL; to = to->next_in_room )
    {
	if (to->altitude != alt) continue;

	if ( to->desc == NULL || (to->position < min_pos && !is_item(to)))
	    continue;
 
	if ( (type == TO_CHAR) && to != ch )
	    continue;
	if ( type == TO_VICT && ( to != vch || to == ch ) )
	    continue;
	if ( type == TO_ROOM && to == ch )
	    continue;
	if ( type == TO_NOTVICT && (to == ch || to == vch) )
	    continue;
 
	point   = buf;
	str     = format;
	while ( *str != '\0' )
	{
	    if ( *str != '$' )
	    {
		*point++ = *str++;
		continue;
	    }
	    ++str;
 
	    if ( arg2 == NULL && *str >= 'A' && *str <= 'Z' )
	    {
		bug( "Act: missing arg2 for code %d.", *str );
		i = " <@@@> ";
	    }
	    else
	    {
		switch ( *str )
		{
		default:  bug( "Act: bad code %d.", *str );
			  i = " <@@@> ";                                break;
		/* Thx alex for 't' idea */
		case 't': i = (char *) arg1;                            break;
		case 'T': i = (char *) arg2;                            break;
		case 'n': i = person( ch,  to  );			break;
		case 'N': i = person( vch, to  );			break;
		case 'e': i = he_she  [URANGE(0, ch  ->sex, 2)];        break;
		case 'E': i = he_she  [URANGE(0, vch ->sex, 2)];        break;
		case 'm': i = him_her [URANGE(0, ch  ->sex, 2)];        break;
		case 'M': i = him_her [URANGE(0, vch ->sex, 2)];        break;
		case 's': i = his_her [URANGE(0, ch  ->sex, 2)];        break;
		case 'S': i = his_her [URANGE(0, vch ->sex, 2)];        break;
 
		case 'p':
		    if (to->sw_obj != NULL && to->sw_obj == obj1)
			i = "you";
		    else if (!can_see_obj(to, obj1) )
			i = "something";
		    else i = obj1->short_descr;

		    break;
 
		case 'P':
		    if (to->sw_obj != NULL && to->sw_obj == obj2)
			i = "you";
		    else if (!can_see_obj(to, obj2) )
			i = "something";
		    else i = obj2->short_descr;

		    break;

		case 'd':
		    if ( arg2 == NULL || ((char *) arg2)[0] == '\0' )
		    {
			i = "door";
		    }
		    else
		    {
			one_argument( (char *) arg2, fname );
			i = fname;
		    }
		    break;
		}
	    }
 
	    ++str;
	    while ( ( *point = *i ) != '\0' )
		++point, ++i;
	}
 
	*point++ = '\n';
	*point++ = '\r';
	buf[0]   = UPPER(buf[0]);
	write_to_buffer( to->desc, buf, point - buf );
    }
 
    return;
}



void save_bans(void)
{
    BAN_DATA *pban;
    FILE *fp;
    bool found = FALSE;

    fclose( fpReserve ); 
    if ( ( fp = fopen( BAN_FILE, "w" ) ) == NULL )
    {
        perror( BAN_FILE );
    }

    for (pban = ban_list; pban != NULL; pban = pban->next)
    {
	if (IS_SET(pban->ban_flags,BAN_PERMANENT))
	{
	    found = TRUE;
	    fprintf(fp,"%-20s %-2d %s\n",pban->name,pban->level,
		print_flags(pban->ban_flags));
	}
     }

     fclose(fp);
     fpReserve = fopen( NULL_FILE, "r" );
     if (!found)
	unlink(BAN_FILE);
}

void load_bans(void)
{
    FILE *fp;
    BAN_DATA *ban_last;
 
    if ( ( fp = fopen( BAN_FILE, "r" ) ) == NULL )
        return;

    ban_last = NULL;
    for ( ; ; )
    {
        BAN_DATA *pban;
        if ( feof(fp) )
        {
            fclose( fp );
            return;
        }
 
        pban = new_ban();
 
        pban->name = str_dup(fread_word(fp));
	pban->level = fread_number(fp);
	pban->ban_flags = fread_flag(fp);
	fread_to_eol(fp);

        if (ban_list == NULL)
	    ban_list = pban;
	else
	    ban_last->next = pban;
	ban_last = pban;
    }
}

bool check_ban(char *site,int type)
{
    BAN_DATA *pban;
    char host[MAX_STRING_LENGTH];
   
    strcpy(host, capitalize(site));
    host[0] = LOWER(host[0]);

    for ( pban = ban_list; pban != NULL; pban = pban->next ) 
    {
	if(!IS_SET(pban->ban_flags,type))
	    continue;
        
	if (IS_SET(pban->ban_flags,BAN_PREFIX)
	&&  IS_SET(pban->ban_flags,BAN_SUFFIX)
	&&  strstr(pban->name,host) != NULL)
	    return TRUE;
        
	if (IS_SET(pban->ban_flags,BAN_PREFIX)
	&&  !str_suffix(pban->name,host))
	    return TRUE;

	if (IS_SET(pban->ban_flags,BAN_SUFFIX)
	&&  !str_prefix(pban->name,host))
	    return TRUE;
    }
   
    return FALSE;
}

void ban_site(CHAR_DATA *ch, char *argument, bool fPerm)
{
    char buf[MAX_STRING_LENGTH],buf2[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    char *name;
    BUFFER *buffer;
    BAN_DATA *pban, *prev;
    bool prefix = FALSE,suffix = FALSE;
    int type;

    argument = one_argument(argument,arg1);
    argument = one_argument(argument,arg2);

    if ( arg1[0] == '\0' )
    {
	if (ban_list == NULL)
	{
	    send_to_char("No sites banned at this time.\n\r",ch);
	    return;
  	}
	buffer = new_buf();

        add_buf(buffer,"Banned sites  level  type     status\n\r");
        for (pban = ban_list;pban != NULL;pban = pban->next)
        {
	    sprintf(buf2,"%s%s%s",
		IS_SET(pban->ban_flags,BAN_PREFIX) ? "*" : "",
		pban->name,
		IS_SET(pban->ban_flags,BAN_SUFFIX) ? "*" : "");
	    sprintf(buf,"%-12s    %-3d  %-7s  %s\n\r",
		buf2, pban->level,
		IS_SET(pban->ban_flags,BAN_NEWBIES) ? "newbies" :
		IS_SET(pban->ban_flags,BAN_PERMIT)  ? "permit"  :
		IS_SET(pban->ban_flags,BAN_ALL)     ? "all"	: "",
	    	IS_SET(pban->ban_flags,BAN_PERMANENT) ? "perm" : "temp");
	    add_buf(buffer,buf);
        }

        page_to_char( buf_string(buffer), ch );
	free_buf(buffer);
        return;
    }

    /* find out what type of ban */
    if (arg2[0] == '\0' || !str_prefix(arg2,"all"))
	type = BAN_ALL;
    else if (!str_prefix(arg2,"newbies"))
	type = BAN_NEWBIES;
    else if (!str_prefix(arg2,"permit"))
	type = BAN_PERMIT;
    else
    {
	send_to_char("Acceptable ban types are all, newbies, and permit.\n\r",
	    ch); 
	return;
    }

    name = arg1;

    if (name[0] == '*')
    {
	prefix = TRUE;
	name++;
    }

    if (name[strlen(name) - 1] == '*')
    {
	suffix = TRUE;
	name[strlen(name) - 1] = '\0';
    }

    if (strlen(name) == 0)
    {
	send_to_char("You have to ban SOMETHING.\n\r",ch);
	return;
    }

    prev = NULL;
    for ( pban = ban_list; pban != NULL; prev = pban, pban = pban->next )
    {
        if (!str_cmp(name,pban->name))
        {
	    if (pban->level > get_trust(ch))
	    {
            	send_to_char( "That ban was set by a higher power.\n\r", ch );
            	return;
	    }
	    else
	    {
		if (prev == NULL)
		    ban_list = pban->next;
		else
		    prev->next = pban->next;
		free_ban(pban);
	    }
        }
    }

    pban = new_ban();
    pban->name = str_dup(name);
    pban->level = get_trust(ch);

    /* set ban type */
    pban->ban_flags = type;

    if (prefix)
	SET_BIT(pban->ban_flags,BAN_PREFIX);
    if (suffix)
	SET_BIT(pban->ban_flags,BAN_SUFFIX);
    if (fPerm)
	SET_BIT(pban->ban_flags,BAN_PERMANENT);

    pban->next  = ban_list;
    ban_list    = pban;
    save_bans();
    sprintf(buf,"%s has been banned.\n\r",pban->name);
    send_to_char( buf, ch );
    return;
}

void do_ban(CHAR_DATA *ch, char *argument)
{
    ban_site(ch,argument,FALSE);
}

void do_permban(CHAR_DATA *ch, char *argument)
{
    ban_site(ch,argument,TRUE);
}

void do_allow( CHAR_DATA *ch, char *argument ) 
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    BAN_DATA *prev;
    BAN_DATA *curr;

    if (!IS_GRANTED(ch, CMD_SITES) && ch->level!=120)
    {
	send_to_char("You do not have access to this command.\n\r",ch);
	return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Remove which site from the ban list?\n\r", ch );
        return;
    }

    prev = NULL;
    for ( curr = ban_list; curr != NULL; prev = curr, curr = curr->next )
    {
        if ( !str_cmp( arg, curr->name ) )
        {
	    if (curr->level > get_trust(ch))
	    {
		send_to_char(
		   "You are not powerful enough to lift that ban.\n\r",ch);
		return;
	    }
            if ( prev == NULL )
                ban_list   = ban_list->next;
            else
                prev->next = curr->next;

            free_ban(curr);
	    sprintf(buf,"Ban on %s lifted.\n\r",arg);
            send_to_char( buf, ch );
	    save_bans();
            return;
        }
    }

    send_to_char( "Site is not banned.\n\r", ch );
    return;
}

/*
 * Macintosh support functions.
 */
#if defined(macintosh)
int gettimeofday( struct timeval *tp, void *tzp )
{
    tp->tv_sec  = time( NULL );
    tp->tv_usec = 0;
}
#endif

void echo_off_to_char( CHAR_DATA *ch )
{
    stc(echo_off_str, ch);
}

void echo_on_to_char( CHAR_DATA *ch )
{
    stc(echo_on_str, ch);
}

void edit_mode_to_char( CHAR_DATA *ch ) 
{
    stc(edit_mode_str, ch);
}

void norm_mode_to_char( CHAR_DATA *ch ) 
{
    stc(norm_mode_str, ch);
}

void ctrl_mode_to_char( CHAR_DATA *ch ) 
{
    stc(ctrl_mode_str, ch);
}


