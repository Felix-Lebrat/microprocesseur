
%{
open Ast
%}

/* Déclaration des tokens */
%token <string> LABEL
%token RET 
%token EOF
%token CALL
%token NL /* Pour déclarer une nouvelle ligne */
%token <string> UINS /* Les instructions a un argument */
%token <string> BINS /* Les instructions a deux arguements*/
%token <string> JINS
%token <int> INT
%token <string> REG
%token TPOINTS 
%token LPAR RPAR
%start fichier 
%type <Ast.fichier> fichier


%%


/* Regles de grammaire */
fichier : 
        l = decl* EOF {{code = l}}
   ;
decl :
  s = LABEL; TPOINTS; NL ; l = instruction *; {{label = s; instr = l}}
;
instruction:
  |NL {Ivoid }
  |RET ; NL{Iret}
  |CALL ; s = LABEL; NL {Icall s}
  |s = JINS; l = LABEL ; NL {Jins {jinstr = s; p_jmp = l}}
  |s = UINS; p = param ; NL{Uins {uinstr = s; p = p}}
  |s = BINS; p1 = param; p2 = param; NL {Bins {binstr = s; p1 = p1; p2 = p2}}
;  
param:
  |LPAR; s = REG; RPAR {Mem s}
  |s = REG {Reg s}
  |i = INT {Int i}
;
