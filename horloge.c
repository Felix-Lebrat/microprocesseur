#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
	system("make");
	if (argc > 1){
		if(argv[1][0] == '-' && argv[1][1] == 'f'){
			system("cp clock_speed.pss.rom fe_ROM0");
		}
		else{
			printf("Argument invalide : il faut faire -f pour le mode rapide");
		}
	}
	else if (argc == 1){
			system("cp clock.pss.rom fe_ROM0");
	}
	
	system("bin/microprocesseur -a");
}
