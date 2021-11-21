#ifndef STRUCTURES_H
#define STRUCTURES_H
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>

struct Reg
{	
	Reg() {};
   	Reg(std::string name, std::vector<bool> code) : name(name)
	{

		for(int i = 0; i < 16; i++)
			this->code[i] = code[i];	
	}	
	std::string name;
	bool code[16];		// Un registre est représenté par le code qui lui correspond
};

struct Op
{	
	
	Op() {};
   	Op(std::string name, bool *code) : name(name)
	{
		for(int i = 0; i < 16; i++)
			this->code[i] = *(code+i);	
	}	
	std::string name;
	bool code[16];
};
union Argument
{
	Reg r;
	int i;
	std::string lbl;
};
struct Instr // Une expression est une OP suivie d'un ou deux registres ou d'autres choses
{
	Op op;
	std::vector<Argument> args;
};
struct Decl  // Une déclaration est un label dans lequel on va avoir du code
{
	std::string name;
	std::vector<Instr> instructions;
};
struct Program  // 
{
	std::vector<Decl> decls;
};

/*
 *	Un code assembleur est constitué de labels (q
 *
 *
 *
 *
 */

#endif
