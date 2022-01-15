Spécifications
-
Les instructions sont codées sur 16-bits et doivent etre alignées sur ce nombre de bits

Registres
-

16 registres: \
rax\
rbx\
rcx\
rdx\
rsi\
rdi\
rbp\
rsp\
r8\
r9\
r10\
r11\
r12\
r13\
r14\
r15 





Jeu d'instructions
-
CALL label (sucre syntaxique pour JMP au début du label et push l'adresse a laquelle on est)
RET ( sucre syntaxique pour POP l'adresse d'ou on vient et JMP a cette adresse )
MOV r1 r2 := r2 = r1\
NOT r1    := r1 = ~r1 ( complément bit à bit )\
XOR r1 r2 := r2 = r2 ^ r1\
OR  r1 r2 := r2 = r2 | r1\
AND r1 r2 := r2 = r2 & r1\
ADD r1 r2 := r2 = r2 + r1\
SUB r1 r2 := r2 = r2 - r1\
MUL r1 r2 := r2 = r2 * r1\
LSL r1    :=\
LSL r2    := \
PUSH r1   :=\
POP r1    :=\
CMP s1 s2 := Renvoie les flags correspondant à s2 - s1\
TEST s1 s2 := Renvoie les flags correspondant à s1 & s2\
JMP s := met le compteur du programme à s\
JNZ s := jump si pas à 0\
JZ s := jump si egal à 0 (si le dernier flag est nul )


Codage des instructions
-
les deux premiers bits d'une instructiosn servent a savoir si la premiere / deuxieme instructrons \
correspondent à des registres ( il y a un 1 si c'est un registre et un 0 sinon \
le 3 eme bit sert a determiner si l'instruction correspond à une instruction \
les 13 bits suivant sont inutiles (pour l'instant )\
Les 16 bits suivant servent à décrire l'instruction\


