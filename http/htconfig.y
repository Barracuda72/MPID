%{ 
#include <stdio.h>
#include <string.h>
#include "htconfig.h"

#define YYSTYPE char *
#define YYDEBUG 1

extern FILE * yyin;
extern int yydebug;
htconf *internal_conf;
int res;

char* strncpy_c(char *dest, const char *src, size_t n)
{
  size_t i;

  if (!src || !dest) return 0;

  printf("SRC is %s\n", src);

  for (i = 0 ; i < n && src[i] != '\0'; i++)
    dest[i] = src[i];
    
  for ( ; i < n ; i++)
    dest[i] = '\0';

  if (n > 0)
    dest[n - 1] = '\0';
  
  return dest;
}

void yyerror(const char *str) 
{
  fprintf(stderr, "Error: %s\n", str);
  res = -1;
}

int yywrap()
{
  return 1;
}

int read_config(char *path, htconf *conf)
{
  res = 0;
  conf->extcnt = 0;
  //yydebug = 1;
  //printf("Config: %X, %X\n", (int)internal_conf, (int)conf);
  internal_conf = conf;
  //printf("Config: %X, %X\n", (int)internal_conf, (int)conf);
  yyin = fopen(path, "rb");
  if(yyin == NULL) return -2;
  yyparse();
  fclose(yyin);
  return res;
}

#ifdef __CONFTEST__
int main()
{

  yydebug = 1;
  htconf *cofg = malloc(sizeof(htconf));
  if(read_config("htconfig.cfg", cofg) != 0) printf("Error %d!\n", res);

  printf("ServRoot will be %s\n",cofg->sroot);
  printf("ServName will be %s\n",cofg->sname);
  int i;
  for (i = 0; i<cofg->extcnt; i++) printf("%s will be exec by %s\n", cofg->ext[i], cofg->exe[i]);
  printf("DirIndex is %d\n", cofg->dirind);
  printf("Index files is:\n");
  for (i = 0; i<cofg->indcnt; i++) printf("%s\n", cofg->index[i]);

  free(cofg);
}
#endif

%}

%token TOKFILE TOKPATH TOKNAME TOKCOMMENT TOKROOT TOKWORD TOKINDEX TOKEXEC TOKAT TOKTRUE TOKFALSE TOKDIRIN;

%%

commands: /* empty */ | commands command;
command: servroot | servname | index | exec | dirind | TOKCOMMENT;
servroot: TOKROOT TOKPATH {strncpy_c(internal_conf->sroot, $2, STR_LIMIT);};
servname: TOKNAME TOKWORD {strncpy_c(internal_conf->sname, $2, STR_LIMIT);};
index: TOKINDEX fnames {strncpy_c(internal_conf->index[0], $2, STR_LIMIT);};
fnames: | fnames TOKFILE {strncpy_c(internal_conf->index[internal_conf->indcnt++], $2, STR_LIMIT);};
exec: TOKEXEC TOKWORD handle {strncpy_c(internal_conf->ext[internal_conf->extcnt], $2, 16); strncpy_c(internal_conf->exe[internal_conf->extcnt++], $3, STR_LIMIT);};
handle: TOKPATH | TOKAT { $$ = $1; };
dirind: TOKDIRIN boolean {internal_conf->dirind = (long)$2;};
boolean: TOKTRUE | TOKFALSE {$$ = $1;};
%%
