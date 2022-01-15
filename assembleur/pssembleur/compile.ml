open Format
open Ast

let labels = Hashtbl.create 20 
let registers = Hashtbl.create 20 
let taille_instruction = 32
let taille_entier = 13


let debug_mode =  "|" 
let debug_line = "\n"

(* Le nombre de ligne que ça prend *)
let taille_bins = 1
let taille_uins = 1
let taille_jins = 1
let taille_call = 2
let taille_ret =  2


let corres = Hashtbl.create 20
(* Il y a 4 modes d'adressage *) 
(* M R
   R M
   R R
   I R

*)


let mov_instr_code =   "0000"
let not_instr_code =   "0001"
let xor_instr_code =   "0010"
let or_instr_code =    "0011"
let and_instr_code =   "0100"
let add_instr_code =   "0101"
let sub_instr_code =   "0110"
let mul_instr_code =   "0111"
let lsl_instr_code =   "1000"
let lrl_instr_code =   "1001"
let push_instr_code =  "1010"
let pop_instr_code =   "1011"
let cmp_instr_code =   "1100"
let test_instr_code =  "1101"
(* Les jumps vont etre ddifferencies par leu mode d'adressage*)
let jmp_instr_code =   "1110"
(*
let jnz_instr_code =   ""
let jz_instr_code =    ""
*)

let () = List.iter (fun (x,y) -> Hashtbl.add registers x y )
    [
      "rax", "0000000000000";
      "rbx", "0000000000001";
      "rcx", "0000000000010";
      "rdx", "0000000000011";
      "rsi", "0000000000100";
      "rdi", "0000000000101";
      "rbp", "0000000000110";
      "rsp", "0000000000111";
      "r8",  "0000000001000";
      "r9",  "0000000001001";
      "r10", "0000000001010";
      "r11", "0000000001011";
      "r12", "0000000001100";
      "r13", "0000000001101";
      "r14", "0000000001110";
      "r15", "0000000001111";
    ]
let () = List.iter  (fun (x,y) -> Hashtbl.add corres x y )
    [
      "mov", mov_instr_code;
      "not", not_instr_code;
      "xor", xor_instr_code;
      "or",   or_instr_code;
      "and", and_instr_code;
      "add", add_instr_code;
      "sub", sub_instr_code;
      "mul", mul_instr_code;
      "lsl", lsl_instr_code;
      "lrl", lrl_instr_code;
      "push", push_instr_code;
      "pop", pop_instr_code;
      "cmp", cmp_instr_code;
      "test", test_instr_code;
      "jmp", jmp_instr_code;
    ]

let write_instr_node t = (Hashtbl.find corres t) ^(debug_mode)

let int32_str i = 
  let taille = ref 0 in 
  let final = ref "" in
  let i_r = ref i in
  while !i_r != 0 do 
    let r = !i_r mod 2 in
    i_r := !i_r /2;
    final := (if r = 1 then "1" else "0")^ !final;
    incr taille ;
  done;
  if !taille < taille_entier then begin
    for _ = !taille to taille_entier-1 do
      final := "0"^ !final;
    done;
  end
  else if !taille > taille_entier then begin
    failwith "Les entiers doivent être sur 13 bits au maximum"
  end;
  !final

let code_mem_bi p1 p2 = match (p1, p2) with
  |(Mem _, Reg _) -> "00"^(debug_mode)
  |(Reg _, Mem _) -> "01"^(debug_mode)
  |(Reg _, Reg _) -> "10"^(debug_mode)
  |(Int _, Reg _) -> "11"^(debug_mode)
  |_ -> failwith "Adressage non compatible"

let write_param_node fmt t= match t with
  |Reg s -> fprintf fmt "%s%s" (Hashtbl.find registers s) (debug_mode)
  |Int i -> fprintf fmt "%s%s" (int32_str i) (debug_mode)
  |Mem s -> fprintf fmt "%s%s" (Hashtbl.find registers s) (debug_mode)
  

let write_param_jmp fmt t= fprintf fmt "%s%s" (int32_str (Hashtbl.find labels t)) debug_mode
  

let p_reg_writer p = match p with
  |Reg _ -> "1"
  |_ -> "0"

let treize_zeros = "0000000000000"^debug_mode

let get_jmp_code = function 
  |"jmp" -> "00"^(debug_mode)
  |"jnz" -> "01"^(debug_mode)
  |"jz"  -> "10"^(debug_mode)
  |"jl"  -> "11"^(debug_mode)
  |_ -> failwith "Pas une fonction de saut"


let write_instr fmt t= match t with
  |Ivoid -> fprintf fmt ""
  |Iret ->  fprintf fmt ""
  |Icall s -> fprintf fmt ""
  |Jins ji -> fprintf fmt "%s%s%a%s%s" (get_jmp_code ji.jinstr)  (jmp_instr_code^ debug_mode) write_param_jmp ji.p_jmp treize_zeros (debug_line)
  |Uins ui -> fprintf fmt "%s%s%a%s%s" "00" (write_instr_node ui.uinstr)  write_param_node ui.p treize_zeros (debug_line)
  |Bins bi -> fprintf fmt "%s%s%a%a%s" (code_mem_bi bi.p1 bi.p2) (write_instr_node bi.binstr) write_param_node bi.p1 write_param_node bi.p2 (debug_line)


let write_decl fmt d= 
  let rec write_decl_list fmt l = match l with
    |[] -> fprintf fmt ""
    |t::q -> fprintf fmt "%a%a" write_instr t write_decl_list q 
  in write_decl_list fmt d.instr

let write_prog  fmt p= 
  let rec write_prog_list fmt l = match l with
    |[] -> fprintf fmt ""
    |t::q -> fprintf fmt "%a%a" write_decl t write_prog_list q 
  in write_prog_list fmt p.code

let program p ofile =
    (* La premiere passe du programme
     * On va enregistrer en mémoire les addresses
     * des différents labels *)
  let p_counter = ref 0 in
  List.iter (
      (* On va chaque fois faire le compte des choses qu'on ajoute *)
      fun d -> 
    Hashtbl.add labels d.label !p_counter;
        List.iter (function 
        |Iret -> (* On va mettre un pop et un jmp *)
          p_counter := !p_counter + taille_ret
        |Icall _ -> 
          p_counter := !p_counter + taille_call 
        |Uins _ -> p_counter := !p_counter + taille_uins
        |Bins _ -> p_counter := !p_counter + taille_bins
        |Jins _ -> p_counter := !p_counter + taille_jins
        |Ivoid -> ()
      ) d.instr;
  ) p.code;
  let f = open_out ofile in
  let fmt = formatter_of_out_channel f in 
  
  fprintf fmt "%a@?" write_prog p

  
