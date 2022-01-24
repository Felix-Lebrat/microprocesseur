flag=-g

prog:build composants.o main.o alu.o
	g++ $(flag) -o build/$@ build/composants.o build/main.o build/alu.o
	build/prog>../netlist_compiler/microprocesseur.net

build:
	mkdir build

%.o:%.cpp
	g++ $(flag) -o build/$@ -c $<

clean:
	rm -f build/*

save:clean
	tar -czvf ../../generation_netlist_felix.tgz .
	scp ../../generation_netlist_felix.tgz flebrat@sas.eleves.ens.fr:./
	rm -f ../../generation_netlist_felix.tgz
