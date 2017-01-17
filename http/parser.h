/* HTTP Parser for MPID */

#define METHOD_GET 1
#define METHOD_POST 2
#define METHOD_PUT 3

#define HTTP_10 4
#define HTTP_11 5

#define ARGS_LIMIT 1024
#define FNAME_LIMIT 256
#define HOST_LIMIT 64
#define UAGENT_LIMIT 256
#define REFERER_LIMIT 256
#define ACCEPT_LIMIT 1024
#define CONT_LIMIT 64
#define COOKIE_LIMIT 1024
#define CONTT_LIMIT 128

typedef struct _http_info
{
  unsigned char method;
  unsigned char http_ver;
  char fname[FNAME_LIMIT];
  char args[ARGS_LIMIT];
  char host[HOST_LIMIT];
  unsigned short port;
  char uagent[UAGENT_LIMIT];
  char referer[REFERER_LIMIT];
  char accept[ACCEPT_LIMIT];
  char contlen[CONT_LIMIT];
  char cookie[COOKIE_LIMIT];
  char conttype[CONTT_LIMIT];
  char *postdata;
} http_info;

int parse_input(char *data, http_info *i);

