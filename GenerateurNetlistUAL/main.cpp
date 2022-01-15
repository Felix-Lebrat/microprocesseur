#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <map>
#include <set>

using namespace std;

const unsigned int nbits = 32, instrSize=8;

//vector<string> inputs = {"ual_e1","ual_e2","ual_instruction"},output = {"ual_s"};
map<string,unsigned int> vars = {{"ual_e1",nbits},{"ual_e2",nbits},{"ual_instr",instrSize},{"ual_s",nbits}};


void generateCases(ostream &out, string prefix)
{
    vector<map<unsigned int,set<string>>> cases(8); // position - size of the value - set of values
    unsigned int i,l;
    for (i=0; i<instrSize; ++i)
    {
        out << prefix << i << "_1 = SELECT " << i << " " << prefix << endl;
	vars.insert({prefix + to_string(i) + "_1",1});
        out << prefix << i << "_0 = NOT " << prefix << i << "_1" << endl;
	vars.insert({prefix + to_string(i) + "_0",1});
        cases[i][1] = {"0","1"};
    }
    for (l=1; l<8; l*=2)
    {
        for (i=0; i<8; i+=2*l)
        {
            for (string case1 : cases[i][l]) for (string case2 : cases[i+l][l])
            {
                out << prefix << i << "_" << case1 << case2 << " = AND " << prefix << i << "_" << case1 << " " << prefix << i+l << "_" << case2 << endl;
		vars.insert({prefix + to_string(i) + "_" + case1 + case2,1});
                cases[i][2*l].insert(case1+case2);
            }
        }
    }
}

void generateConcat(ostream &out, string prefix, unsigned int N, unsigned int offset=0)
{
    if (N<=1) return;
    if (N<=2)
    {
        out << prefix << offset << "_" << N << " = CONCAT " << prefix << offset << " " << prefix << offset+N/2 << endl;
        vars.insert({prefix + to_string(offset) + "_" + to_string(N),2});
	return;
    }
    generateConcat(out, prefix, N/2, offset);
    generateConcat(out, prefix, N-N/2, offset+N/2);
    out << prefix << offset << "_" << N << " = CONCAT " << prefix << offset << "_" << N/2 << " " << prefix << offset+N/2 << "_" << N/2 << endl;
    vars.insert({prefix + to_string(offset) + "_" + to_string(N),N});
}

void generateFulladder(ostream &out, string a, string b, string c, string r, string s)
{
    static unsigned int id = 0;
    out << r << " = OR adder3_" << id << " adder5_" << id << endl;
    out << s << " = XOR adder1_" << id << " " << c << endl;
    out << "adder1_" << id << " = XOR " << a << " " << b << endl;
    out << "adder3_" << id << " = AND " << a << " " << b << endl;
    out << "adder5_" << id << " = AND " << "adder1_" << id << " " << c << endl;
    vars.insert({r,1});
    vars.insert({s,1});
    vars.insert({"adder1_"+to_string(id),1});
    vars.insert({"adder3_"+to_string(id),1});
    vars.insert({"adder5_"+to_string(id),1});
}

void generateAdd(ostream &out, string E1, string E2, string S, string R="", bool decomp=true)
{
    static unsigned int id = 0;
    out << "r0_" << id << " = 0" << endl;
    vars.insert({"r0_" + to_string(id),1});
    for (unsigned int i=0; i<nbits; i++)
    {
        if (decomp)
        {
            out << E1 << "_" << i << " = SELECT " << i << " " << E1 << endl;
            out << E2 << "_" << i << " = SELECT " << i << " " << E2 << endl;
	    vars.insert({E1+"_"+to_string(i),1});
	    vars.insert({E2+"_"+to_string(i),1});
        }
        generateFulladder(out, E1+"_"+to_string(i), E2+"_"+to_string(i), "r"+to_string(i)+"_"+to_string(id), "r"+to_string(i+1)+"_"+to_string(id), S+"_"+to_string(i));
    }
    if (R!="") {out << R << " = r8_" << id << endl; vars.insert({R,1});}
    generateConcat(out, S, nbits);
    out << S << " = " << S << "0_" << nbits << endl;
    vars.insert({S,nbits});
}

void generateSlide(ostream &out, string E, string S, unsigned int d)
{
    unsigned int i;
    for (i=0; i<nbits-d; i++)
    {
        out << E << "_" << i << " = SELECT " << i << " " << E << endl;
        out << S << "_" << i+d << " = " << E << "_" << i << endl;
	vars.insert({E + "_" + to_string(i),1});
	vars.insert({S + "_" + to_string(i+d),1});
    }
    for (i=0; i<d; i++) {out << S << "_" << i+d << " = 0" << endl; vars.insert({S+"_"+to_string(i+d),1});}
    generateConcat(out, S, nbits);
}

void generateAddTree(ostream &out, string prefix, unsigned int N=nbits, unsigned int offset=0)
{
    if (N<=1) return;
    if (N<=2)
    {
        generateAdd(out, prefix+to_string(offset), prefix+to_string(offset+1), prefix+to_string(offset)+"_2");
        return;
    }
    generateAddTree(out, prefix, N/2, offset);
    generateAddTree(out, prefix, N-N/2, offset+N/2);
    generateAdd(out, prefix+to_string(offset)+"_"+to_string(N/2), prefix+to_string(offset+N/2)+"_"+to_string(N/2), prefix+to_string(offset)+"_"+to_string(N));
}

void netlistBody(ostream &out)
{
    out << "bit32_0 = 00000000000000000000000000000000" << endl;
    vars.insert({"bit32_0",32});

    // Decode instruction
    string prefix="ual_instr";
    generateCases(out,prefix);

    // NOT
    out << "ual_not_out = NOT ual_e2" << endl;
    out << "ual_s_1 = MUX ual_instr0_00000001 bit32_0 ual_not_out" << endl;
    vars.insert({"ual_not_out",nbits});
    vars.insert({"ual_s_1",nbits});

    // AND
    out << "ual_and_out = AND ual_e1 ual_e2" << endl;
    out << "ual_s_2 = MUX ual_instr0_00000010 bit32_0 ual_and_out" << endl;
    vars.insert({"ual_and_out",nbits});
    vars.insert({"ual_s_2",nbits});

    // OR
    out << "ual_or_out = OR ual_e1 ual_e2" << endl;
    out << "ual_s_3 = MUX ual_instr0_00000011 bit32_0 ual_or_out" << endl;
    vars.insert({"ual_or_out",nbits});
    vars.insert({"ual_s_3",nbits});

    // XOR
    out << "ual_xor_out = XOR ual_e1 ual_e2" << endl;
    out << "ual_s_4 = MUX ual_instr0_00000100 bit32_0 ual_xor_out" << endl;
    vars.insert({"ual_xor_out",nbits});
    vars.insert({"ual_s_4",nbits});

    // ADD/SUB
    for (unsigned int i=0; i<nbits; ++i) {
    	vars.insert({"nbits1"+to_string(i),1});
        if (i==0) out << "nbits10 = 1" << endl;
        else out << "nbits1" << i << " = 0" << endl;}
    generateConcat(out, "nbits1", nbits);
    generateAdd(out, "ual_not_out", "nbits1", "ual_2min", "ual_Rmin");
    out << "ual_e2mod = MUX ual_instr8_1 ual_e2 ual_e2min" << endl;
    vars.insert({"ual_e2mod",nbits});
    generateAdd(out, "ual_e1", "ual_e2mod", "ual_add_out", "ual_addR");
    out << "ual_instr_add_sub = AND ual_instr0_00000111 ual_instr0_00000110" << endl;
    vars.insert({"ual_instr_add_sub",1});
    out << "ual_s_5 = MUX ual_instr_add_sub bit32_0 ual_add_out" << endl;
    vars.insert({"ual_s_5",nbits});

    // MUL
    for (unsigned int i=0; i<nbits-1; ++i)
    {
        generateSlide(out, "ual_e1", "ual_e1Slide_"+to_string(i), i);
        out << "ual_mult_term_" << i << " = AND E2_" << i << " ual_e1Slide_" << i << endl;
    	vars.insert({"ual_mult_term_" + to_string(i),nbits});
    }
    generateAddTree(out, "ual_mult_term_");
    out << "ual_s_6 = MUX ual_instr0_00000101 bit32_0 ual_mult_term_0_32" << endl;
    vars.insert({"ual_s_6",nbits});

    out << "ual_s_12 = OR ual_s_1 ual_s_2" << endl;
    vars.insert({"ual_s_12",nbits});
    out << "ual_s_34 = OR ual_s_3 ual_s_4" << endl;
    vars.insert({"ual_s_34",nbits});
    out << "ual_s_56 = OR ual_s_5 ual_s_6" << endl;
    vars.insert({"ual_s_56",nbits});
    out << "ual_s_1234 = OR ual_s_12 ual_s_34" << endl;
    vars.insert({"ual_s_1234",nbits});
    out << "ual_s = OR ual_s_1234 ual_s_56" << endl;
}

void generateHead(ostream &out)
{
    out << "INPUT ual_e1, ual_e2, ual_instr" << endl;
    out << "OUTPUT ual_s" << endl;
    bool notFirst=false;
    out << "VAR ";
    for (pair<string,unsigned int> var : vars) {if (notFirst) out << ", "; out << var.first << ":" << var.second; notFirst=true;}
    out << endl << "IN" << endl;
}

int main()
{
    ofstream netlist("netlist.txt");
    stringstream body;
    netlistBody(body);
    generateHead(netlist);
    netlist << body.str();
    return 0;
}
