%{ 
#include <stdio.h>
#include <string.h>

#define YYSTYPE char *

extern FILE * yyin;

void yyerror(const char *str) 
{
  fprintf(stderr, "Error: %s\n", str);
}

int yywrap()
{
  return 1;
}

main()
{
  yyin = fopen("config", "rb");
  yyparse();
  fclose(yyin);
}

%}

%token TOKWORD TOKIPADDR TOKNUM TOKEQUAL TOKFILE TOKINCL TOKQUOTE TOKOBRACE TOKEBRACE TOKSEMICOLON TOKNEWLINE TOKCOMMENT TOKMODULES TOKMODULE TOKADDRESS TOKPORT TOKZAPYATAYA

%%

commands: /* empty */ | commands command ;
command: modules | module ;
zpt: | TOKZAPYATAYA ;

/*quotedwords: | quotedwords quotedword ;*/
quotedword: TOKQUOTE TOKWORD TOKQUOTE { $$ = $2; };
quotedip: TOKQUOTE TOKIPADDR TOKQUOTE { $$ = $2; printf(" ip is '%s'\n", $$);};
quotedport: TOKQUOTE TOKNUM TOKQUOTE { $$ = $2; printf(" port is '%s'\n", $$);};

modules: {printf("Found modules: \n");} TOKMODULES TOKEQUAL modulescontent TOKSEMICOLON  ;
module: {printf("Module descr: ");} TOKMODULE modulename modulecontent TOKSEMICOLON ;

modulename: quotedword { $$ = $1; printf("'%s'\n", $1);};
modulesnames: | modulesnames zpt modulename ;
modulescontent: TOKOBRACE modulesnames TOKEBRACE ;
modulecontent: TOKOBRACE TOKADDRESS TOKEQUAL quotedip TOKSEMICOLON TOKPORT TOKEQUAL quotedport TOKSEMICOLON TOKEBRACE /*{printf("%s\n", $4);} */;
%%
