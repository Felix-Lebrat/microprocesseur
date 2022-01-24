all:
	make --directory=assembleur/pssembleur
	cp assembleur/pssembleur/clock/*.rom ./
	cp netlist_compiler/out ./microprocesseur

clock:
	cp clock.pss.rom fe_ROM0

clock_speed:
	cp clock_speed.pss.rom fe_ROM0
	
clean:
	make --directory=assembleur/pssembleur clean
	make --directory=generation_netlist clean
	make --directory=netlist_compiler clean
	rm fe_ROM0 clock.pss.rom clock_speed.pss.rom netlist_compiler/microprocesseur.net microprocesseur
