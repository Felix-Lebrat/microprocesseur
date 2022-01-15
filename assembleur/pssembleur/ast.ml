type param = 
  |Reg of string
  |Int of int
  |Mem of string
type binstruction = 
  {
    binstr : string;
    p1  : param;
    p2  : param 
  }

type uinstruction = 
  {
    uinstr : string;
    p  : param
  }
type jinstruction = 
  {
    jinstr : string;
    p_jmp : string
  }
type instruction = 
  |Ivoid
  |Iret (* L'instruction de retour *)
  |Icall of string
  |Jins of jinstruction  (* Une instruction de saut *)
  |Uins of uinstruction
  |Bins of binstruction
type decl = 
  {
    label : string;
    instr : instruction list;
  }

type fichier  = 
  {
    code : decl list  
  }
