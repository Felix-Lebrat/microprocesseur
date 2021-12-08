%{
#include <stdlib.h>
#include <string.h>
#include <structures.h>
#include <parser.h>
%}



%option noyywrap
whitespace [\n\t]
comma[,]
ID  [a-z][a-z 0-9]
NUMBER (0)|([1-9][0-9]*)
%%
MOV {return MOV}
NOT {return NOT}
XOR {return XOR}
OR {return OR}
AND {return AND}
ADD {return ADD}
SUB {return SUB}
MUL {return MUL}
LSL {return LSL}
PUSH {return PUSH}
POP {return POP}
CMP {return CMP}
JMP {return JMP}
RET {return RET}

ID {yylval.id=; return VAL}



%%
