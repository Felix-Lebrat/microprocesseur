all : assembleur.exe
	./make-exec && ./pssembly clock/clock_speed.pss && ./pssembly clock/clock.pss
	
assembleur.exe: assembleur.ml
	dune build assembleur.exe

clean : 
	dune clean 
	rm clock/*.rom
