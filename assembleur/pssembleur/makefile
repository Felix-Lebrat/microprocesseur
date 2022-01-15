all : assembleur.exe
	./make-exec && ./pssembly clock/clock_speed.pss
	
assembleur.exe: assembleur.ml
	dune build assembleur.exe

clean : 
	dune clean 
