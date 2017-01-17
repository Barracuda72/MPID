%{
#include <stdio.h> 
#include "htconfig.y.h"
%}

%%
ServerName return TOKNAME;
ServerRoot return TOKROOT;
Index return TOKINDEX;
DirIndex return TOKDIRIN;
Exec return TOKEXEC;
@ return TOKAT;
[Tt][Rr][Uu][Ee] yylval = 1; return TOKTRUE;
[Ff][Aa][Ll][Ss][Ee] yylval = 0; return TOKFALSE;
[a-zA-Z][a-zA-Z0-9]* yylval = (long)strdup(yytext); return TOKWORD;
\/[^\n#]* yylval = (long)strdup(yytext); return TOKPATH;
[a-zA-Z]*\.[a-zA-Z]* yylval = (long)strdup(yytext); return TOKFILE;
#[^\n]* /* return TOKCOMMENT; */
\n /* Ignore */
[\t]+ /* Ignore */
%%
