/* libproto.h - common file for protocol libraries */

/* Defines */

/* Connection was processed and may be closed */
#define CONN_NEED_CLOSE	1

/* Connection was not fully processed and need to still exists */
#define CONN_DONT_CLOSE	2

/* There was error during connection processing */
#define CONN_WAS_ERROR	3

/* Protocol handling name */

#define CONN_HANDLE_ROUTINE	"handle_connection"

#define CONN_INIT_ROUTINE	"init_module"

#define CONN_FINI_ROUTINE	"fini_module"
/* Functions */

/* Initialization routine of protocol library */

int init_module(void);

/* Connection processing routine */

int handle_connection(int sockfd, struct sockaddr_in* cli);

int fini_module(void);
