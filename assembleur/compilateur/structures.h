#ifndef STRUCTURES_H
#define STRUCTURES_H
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>

struct Reg
{	
	Reg(int i) : nReg(i){};
	int nReg;
};

enum class Op
{	
	MOV, NOT, XOR, OR, AND, ADD, SUB, MUL, LSL, PUSH, POP, CMP, JMP
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
union Line // Une ligne est soit une instruction, soit la déclaration d'un label
{
	std::string lbl;
	Instr inst;
};
struct Decl  // Une déclaration est un label dans lequel on va avoir du code
{
	Decl(std::string name) : name (name) {}
	std::string name;
	std::vector<Instr> instruction;
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
