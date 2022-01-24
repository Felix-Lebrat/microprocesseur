"Vim syntax file

syntax keyword xInstr mov not xor or and add sub mul lsl lrl cmp test jmp jnz jz
syntax keyword xRegis rax rbx rcx rdx rsi rdi rbp rsp r8 r9 r10 r11 r12 r13 r14 r15
syntax match xComment /#.*/
syntax match xNumber /\d/

hi xInstr 	guifg=#5F09A2
hi xRegis		guifg=#FFE300
hi xComment guifg=#75715E
hi xNumber  guifg=#B4A7D6
