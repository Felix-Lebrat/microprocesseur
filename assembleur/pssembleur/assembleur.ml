open Format
open Lexing


let parse_only = ref false 
let ifile = ref ""
let set_file f s = f := s

let options = ["--parse-only", Arg.Set parse_only, "Pour faire uniquement la phase de parsing"
              ]
let usage = "usage : ./pssembly [options] file"

let localisation pos = 
  let l = pos.pos_lnum in
  let c = pos.pos_cnum - pos.pos_bol +1 in 
  eprintf "File \"%s\", line %d, characters %d-%d:\n" !ifile l (c-1) c

let main () = 
  Arg.parse options (set_file ifile) usage;
  if !ifile = "" then begin 
    eprintf "Aucun fichier à compiler \n@?";
    exit 1;
  end;
  let f = open_in !ifile in
  let buf = Lexing.from_channel f in
  try 
    let p = Parser.fichier Lexer.token buf in
    close_in f;
    if !parse_only then exit 0;
    Compile.program p (!ifile^".rom") ;
  with 
  |Lexer.Lexing_error c ->
    localisation (Lexing.lexeme_start_p buf);
    eprintf "Erreur lexicale : %s @." c;
    exit 1;
  |Parser.Error -> 
    localisation (Lexing.lexeme_start_p buf);
    eprintf "Erreur Syntaxique au mot %s@." (Lexing.lexeme buf);
    exit 1

let () = main ()
