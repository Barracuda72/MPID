/* System includes */
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>

/* Our own includes */
#include <libLog.h>
//#include <libmime.h>
#include <libproto.h>

/* Debuggin macroses */
#define DEBUG_START() int dbgloghandle = log_open("debug.html")
#define DEBUG_STOP() log_close(dbgloghandle)
#define DEBUG(x) log_write_string(dbgloghandle, x, LOG_WARN)

int sockId, sockIdNew, status, i = 0, clientStructSize = 0;
struct addrinfo *hints, *server;
struct sockaddr_in *client;
char clientIpAddress[16];

int flag = 0;
int running = 1;

int server_umask = 0;
char server_rootdir[] = "/var/www/";

/*
Example of HTTP answer

HTTP/1.1 200 OK
Date: Tue, 17 Aug 2010 17:01:16 GMT
Server: Apache/2.2.12 (Ubuntu)
Last-Modified: Tue, 17 Aug 2010 17:00:34 GMT
ETag: "18dd0ae-b1-48e07e32d661d"
Accept-Ranges: bytes
Content-Length: 177
Vary: Accept-Encoding
Content-Type: text/html
X-Pad: avoid browser bug

Fuck you, %s:%d!


*/

//char *tmpbuf;
//char answer[] = "HTTP/1.1 200 OK\n\nFuck you, %s:%d!\n\n";

//char *path;

void signal_action(int arg)
{
  printf("Signal %d is received!\n", arg);
  switch (arg) {
    case SIGINT:
      running = 0;
      close(sockId);
      printf("Socket closed\n");
      //break; NOT NEEDED!!!
    case SIGIO:
      flag += 1;
      break;

    default:
      break;
  }
  
}

int main()
{
  DEBUG_START();
  pid_t pid, sid;
  
  /*pid = fork();
  
  if(pid < 0) 
  {
    //If we can't fork, exit
    DEBUG("Unable to fork from start process");
    exit(EXIT_FAILURE);
  }
  
  if(pid > 0) exit(EXIT_SUCCESS);	//Parent process should die
    */
  umask(server_umask);
  
  /*sid = setsid();			//Create new process group
  
  if(sid < 0)
  {
    //Can't create new group
    DEBUG("Unable to set SID");
    exit(EXIT_FAILURE);
  }*/
    
  if(chdir(server_rootdir) < 0)
  {
    //Can't chdir to working directory
    DEBUG("Unable to chdir");
    exit(EXIT_FAILURE);
  }
  
  //Close file descriptors
  /*close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);*/
  
  memset(clientIpAddress, '\0', 16);
  clientStructSize = sizeof(struct sockaddr_in);
  client = malloc(clientStructSize);
  
  if(!client)
  {
    DEBUG("Unable to allocate clientStructSize");
    exit(EXIT_FAILURE);
  }
  
  status = (int)signal(SIGIO, (*signal_action));	//For asyncronic input/output
  
  if(status < 0)
  {
    DEBUG("Error setting up signal SIGIO");
    exit(EXIT_FAILURE);
  }
  
  status = (int)signal(SIGINT, (*signal_action));	//Interrupt, Ctrl-C
  
  if(status < 0)
  {
    DEBUG("Error setting up signal SIGINT");
    exit(EXIT_FAILURE);
  }
  
  status = (int)signal(SIGCHLD, SIG_IGN);		//We don't need to wait child death
  
  if(status < 0)
  {
    DEBUG("Error setting up signal SIGCHLD");
    exit(EXIT_FAILURE);
  }
  
  /*status = (int)signal(SIGPROF, SIG_IGN);		//We don't want profiling signals
  
  if(status < 0)
  {
    DEBUG("Error setting up signal SIGPROF");
    exit(EXIT_FAILURE);
  }*/
  
  sockId = socket(AF_INET, SOCK_STREAM, 6);	//TCP
  
  if (sockId == -1)
  {
    DEBUG("Error creating socket");
    exit(EXIT_FAILURE);
  }
  
  status = fcntl(sockId, F_SETOWN, getpid());
  
  if(status != 0)
  {
    DEBUG("Error setting up socket owner");
    exit(EXIT_FAILURE);
  }
  
  //status = fcntl(sockId, F_SETFL, O_ASYNC|O_NONBLOCK);
  
  if(status != 0)
  {
    DEBUG("Error setting up socket mode");
    exit(EXIT_FAILURE);
  }
  
  hints = malloc(sizeof(struct addrinfo));
  hints->ai_family = AF_INET;		// TCP/IPv4
  hints->ai_socktype = SOCK_STREAM;
  hints->ai_protocol = 6;		// TCP
  hints->ai_flags = AI_PASSIVE;		// Receive
  
  status = getaddrinfo("127.0.0.1", "8082", hints, &server);
  
  if (status != 0) 
  {
    DEBUG("Error converting address");
    exit(EXIT_FAILURE);
  }
  
  status = bind(sockId, server->ai_addr, server->ai_addrlen);
  
  if(status < 0)
  {
    DEBUG("Error binding socket");
    exit(EXIT_FAILURE);
  }
  
  status = listen(sockId, 10);
  
  if(status < 0)
  {
    DEBUG("Error listening socket");
    exit(EXIT_FAILURE);
  }
  
  void *module = dlopen("libhttp.so", RTLD_NOW);
  
  if (!module)
  {
    DEBUG("Error during load module: libhttp.so");
  	DEBUG("dlerror output:");
  	DEBUG(dlerror());
  	exit(EXIT_FAILURE);
  }
  dlerror();	//Clear any existing error

  int (*handler)(int, struct sockaddr_in *);
  int (*initmodule)(void);
  int (*finimodule)(void);
  
  *(void **)(&initmodule) = dlsym(module, CONN_INIT_ROUTINE);
  (*initmodule)();
  
  *(void **)(&handler) = dlsym(module, CONN_HANDLE_ROUTINE);
  
  *(void **)(&finimodule) = dlsym(module, CONN_FINI_ROUTINE);

  flag = 0;
  running = 1;
  while (running) 
  {
    //while (flag == 0) sleep(1);
  
    if(!running) break;
  
    sockIdNew = accept(sockId, (struct sockaddr *)client, &clientStructSize);
  
    if(sockIdNew < 0)
    {
      DEBUG("Error waiting socket");
      exit(EXIT_FAILURE);
    }
  
    if(fork() == 0)
    { 
      //We are in child, handle request

      if((*handler)(sockIdNew, client) == CONN_NEED_CLOSE) 
        close(sockIdNew);
      
      exit(0);
    } else {
      //We are in parent process
      //DEBUG("Child started");
      flag -= 1;
      close(sockIdNew);
    }
  }
  
  printf("Exiting...\n");
  close(sockId);
  (*finimodule)();
  dlclose(module);
}

