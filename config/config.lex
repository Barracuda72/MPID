%{
#include <stdio.h> 
#include "config.y.h"
%}

%%
Modules return TOKMODULES;
Module return TOKMODULE;
address return TOKADDRESS;
port return TOKPORT;
[a-zA-Z][a-zA-Z0-9_]* yylval = strdup(yytext); return TOKWORD;
[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3} yylval = strdup(yytext); return TOKIPADDR;
[0-9]+ yylval = strdup(yytext); return TOKNUM;
= return TOKEQUAL;
[a-zA-Z0-9_\/.-]+ return TOKFILE;
@include[ \t]+ return TOKINCL;
\" return TOKQUOTE;
\{ return TOKOBRACE;
\} return TOKEBRACE;
; return TOKSEMICOLON;
#[^\n]* /* ignore return TOKCOMMENT; */
\n /* Ignore */
[\t]+ /* Ignore */
, return TOKZAPYATAYA;
.	/* Ignore */
%%
