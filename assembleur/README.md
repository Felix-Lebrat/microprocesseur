#Spécifications

Les instructions sont codées sur 16-bits et doivent etre alignées sur ce nombre de bits


#Registres

16 registres: 

rax

rbx

rcx

rdx

rsi

rdi

rbp

rsp

r8

r9

r10

r11

r12

r13

r14

r15 





#Jeu d'instructions
CALL label (sucre syntaxique pour JMP au début du label )

MOV r1 r2 := r2 = r1

NOT r1    := r1 = ~r1 ( complément bit à bit )

XOR r1 r2 := r2 = r2 ^ r1

OR  r1 r2 := r2 = r2 | r1

AND r1 r2 := r2 = r2 & r1

ADD r1 r2 := r2 = r2 + r1

SUB r1 r2 := r2 = r2 - r1

MUL r1 r2 := r2 = r2 * r1

LSL r1    :=

LSR r2    := 

PUSH r1   :=

POP r1    :=

CMP s1 s2 := Renvoie les flags correspondant à s2 - s1

JMP s := met le compteur du programme à s

JZ s := || si le drapeau null est levé

JN s := || si le drapeau neg est levé
