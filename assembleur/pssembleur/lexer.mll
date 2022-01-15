{
open Parser
exception Lexing_error of string 
let uins = Hashtbl.create 20
let bins = Hashtbl.create 20
let jins = Hashtbl.create 20
let registers = Hashtbl.create 12
let () = List.iter (fun (x,y) -> Hashtbl.add jins x y)
    [
      "jz" , ();
      "jnz" , ();
      "jmp", ();
      "jl" , ();
    ]
let ()  = List.iter ( fun (x,y) -> Hashtbl.add uins x y ) 
    [
      "not", ();
      "push", ();
      "pop", ();
    ]
let () = List.iter (fun (x,y)-> Hashtbl.add bins x y )
    [
      "mov", ();
      "xor", ();
      "or", ();
      "and", ();
      "add", ();
      "sub", ();
      "mul", ();
      "lsl", ();
      "lrl", ();
      "cmp", ();
      "test", ();
    ]
let () = List.iter (fun (x,y) -> Hashtbl.add registers x y )
    [
      "rax", ();
      "rbx", ();
      "rcx", ();
      "rdx", ();
      "rsi", ();
      "rdi", ();
      "rbp", ();
      "rsp", ();
      "r8", ();
      "r9", ();
      "r10", ();
      "r11", ();
      "r12", ();
      "r13", ();
      "r14", ();
      "r15", ();
    ]
}


let alpha = ['a'-'z' 'A'-'Z']
let chiffre = ['0'-'9']
let keyword = alpha ('_' |alpha | chiffre)*
let entier = '0' | ['1'-'9'] chiffre*
                  
rule token = parse 
   |[' ' '\t'] {token lexbuf}
   |':' {TPOINTS}
   |'(' {LPAR}
   |')' {RPAR}
   |("ret"|"RET") {RET}
   |("call"|"CALL") {CALL}
   |keyword as s 
     { let s= String.lowercase_ascii s in
       try Hashtbl.find bins s ;
        BINS s
       with Not_found ->
         begin try Hashtbl.find uins s;
         UINS s
         with Not_found ->
           begin try Hashtbl.find registers s;
           REG s
           with Not_found ->
             begin try Hashtbl.find jins s ;
                 JINS s
               with Not_found ->
             LABEL s
             end
          end
       end    

     }
   |entier as x {INT (int_of_string x)}
   |'\n' {Lexing.new_line lexbuf; NL}
   |'#' {comment lexbuf}
   |eof {EOF}
   |_ as s {raise (Lexing_error ("Mot clé inconnu"^(Char.escaped s)))}
and comment = parse
   |'\n' {Lexing.new_line lexbuf; NL}
   |eof {EOF}
   |_ {comment lexbuf}
