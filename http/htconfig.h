/* HTTP header config file */

#define STR_LIMIT 1024

#define HANDLE_INTERNAL "@"

typedef struct _htconf
{
  char sname[STR_LIMIT];
  char sroot[STR_LIMIT];
  char index[16][STR_LIMIT];
  unsigned char indcnt;
  unsigned char extcnt;
//   unsigned char indcnt;
  unsigned char dirind;
  char ext[16][16];
  char exe[16][STR_LIMIT];
} htconf;

int read_config(char *path, htconf *conf);
