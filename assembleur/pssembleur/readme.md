Spécifications
-
Les instructions sont codées sur 32 bits

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
JL s := jump si strictement inferieur a 


Codage des instructions
-
Les deux premiers bits servent à connaitre le mode d'adressage
Les deux suivants servent à connaitre le code de l'instruction
Les 26 suivants servent aux deux variables

