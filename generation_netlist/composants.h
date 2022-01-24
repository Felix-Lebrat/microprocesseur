#ifndef _COMPOSANTS_H_
#define _COMPOSANTS_H_

#include <ostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include "alu.h"

class Variable
{
public:
    Variable(std::string nom,int taille,bool prefix=true);
    Variable(int val,int taille=-1):
        m_val(val),m_taille(taille),m_est_const(true){};
    std::string print();
    int get_taille(){return m_taille;};
    bool est_const(){return m_est_const;};
private:
    std::string m_nom;
    int m_no;
    int m_taille;
    int m_val;
    bool m_est_const;
    bool m_prefix;
    static std::map<std::string,int> instances;
    static const std::string prefix;
};


class Composant
{
public:
    Composant(){};
    virtual void print(std::ostream& flux)=0;
    virtual std::vector<Variable*> get_variables()=0;
    virtual ~Composant(){};
    static std::vector<Variable*> get_variables(std::vector<Composant*> &composants);
};

class Primitive : public Composant
{
public:
    Primitive(std::string type,int taille,std::vector<Variable*> entrees):
    m_sortie(type,taille),m_entrees(entrees),m_type(type){};
    ~Primitive();
    void print(std::ostream& flux);
    Variable* get_sortie(){return &m_sortie;};
    std::vector<Variable*> get_variables(){return {&m_sortie};};
    void ajouter_entree(Variable* v);
private:
    std::vector<Variable*> m_entrees;
    Variable m_sortie;
    std::string m_type;
};

class Demux:public Composant
{
public:
    //controle et entree doivent avoir la meme taille
    Demux(Variable *entree,Variable* controle);
    void print(std::ostream& flux);
    Variable* get_s0();
    Variable* get_s1();
    std::vector<Variable*> get_variables();
private:
    Variable *m_e;//entree
    Variable* m_c;//controle

    Primitive m_not;
    Primitive m_et_0;
    Primitive m_et_1;
};

class BusOfWire: public Composant
{
public:
    BusOfWire(Variable* fil,int taille_bus);
    ~BusOfWire();
    void print(std::ostream& flux);
    std::vector<Variable*> get_variables();
    Variable* get_bus();
private:
    std::vector<Primitive*> m_intermediaires;
    Variable* m_fil;
    Variable* construire(int n);
};

class Binop:public Composant
{
public:
    Binop(std::vector<Variable*> &entrees,std::string type);
    void print(std::ostream& flux);
    std::vector<Variable*> get_variables();
    Variable* get_sortie();
    ~Binop();
private:
    Binop* m_g;
    Binop* m_d;
    Primitive* m_binop;
    Variable* m_var;
};

class Registre:public Composant
{
public:
    Registre(Variable* write_data,Variable* write_enable);
    void print(std::ostream& flux);
    std::vector<Variable*> get_variables();
    Variable* get_read_data();
private:
    Primitive m_mux;
    Primitive m_reg;
};

class GestionnaireRegistres:public Composant
{
public:
    GestionnaireRegistres(int taille_addr,Variable *reg1,Variable *reg2,Variable* write_enable,Variable* write_data);
    ~GestionnaireRegistres();
    void print(std::ostream& flux);
    std::vector<Variable*> get_variables();
    Variable* get_val1(){return m_val1;};
    Variable* get_val2(){return m_val2;};
    Variable* get_registre(int no);
private:
    Variable* m_val1;
    Variable* m_val2;
    GestionnaireRegistres* m_ss_gest0;
    GestionnaireRegistres* m_ss_gest1;
    Registre* m_reg;
    std::vector<Composant*> m_composants;
};

class Decodeur:public Composant
{
public:
    Decodeur(Variable *instr);
    void print(std::ostream& flux);
    std::vector<Variable*> get_variables();
    Variable* get_mux_op1(){return m_mux_op1->get_sortie();};
    Variable* get_op1(){return m_op1->get_sortie();};
    Variable* get_alu_instr(){return m_alu_instr->get_sortie();};
    Variable* get_ret_instr(int i){return m_r[i];};
    Variable* get_reg1(){return m_reg1->get_sortie();};
    Variable* get_reg2(){return m_reg2->get_sortie();};
    Variable* get_reg_we(){return m_reg_we->get_sortie();};
    Variable* get_ram_we(){return m_ram_we->get_sortie();};
    Variable* get_jmp(){return m_jmp;};
    std::vector<Variable*>& get_addr_mode(){return m_addr_mode;};
    ~Decodeur();
private:
    std::vector<Variable*> decode(Variable* opcode);
    Primitive *m_reg1;
    Primitive *m_reg2;
    Primitive *m_op1;//aussi dans le mux final
    Primitive *m_mux_op1;
    Primitive *m_alu_instr;
    Primitive* m_reg_we;
    Primitive* m_ram_we;
    std::vector<Variable*> m_r;//instruction de retour : 
                    //0 : reg1, 1 : ram, 2 : decodeur, sinon alu
    std::vector<Variable*> m_addr_mode;

    std::vector<Composant*> m_intermediaires;
    Variable* m_jmp;
};


class Flags:public Composant
{
public:
    Flags(std::vector<Variable*> &addr_mode,Variable* write_data);//write data : 0 : zero, 1 : negatif
    void print(std::ostream& flux);
    std::vector<Variable*> get_variables();
    Variable* get_rd(){return m_rd;};
    ~Flags();
private:
    Variable* m_rd;//read_data
    std::vector<Composant*> m_intermediaires;
};

class OBIncr:public Composant//one bit incr
{
public:
    OBIncr(Variable* i,Variable *c_i);//in, carry in
    OBIncr(Variable* i);//in, carry in = 1
    void print(std::ostream& flux);
    Variable* get_c_o(){return m_c_o->get_sortie();};
    Variable* get_o(){return m_o->get_sortie();};
    std::vector<Variable*> get_variables();
    ~OBIncr(){delete m_o;delete m_c_o;};
private:
    Primitive *m_o;//out
    Primitive *m_c_o;//carry out
};

class Increment:public Composant
{
public:
    Increment(Variable* i);//in
    void print(std::ostream& flux);
    std::vector<Variable*> get_variables();
    Variable* get_sortie(){return m_sortie->get_sortie();};
    ~Increment();
private:
    Binop* m_sortie;
    std::vector<Composant*> m_intermediaires;
};

class PC:public Composant
{
public:
    PC(Variable* flag,Variable* addr,Variable* jmp);
    void print(std::ostream& flux);
    std::vector<Variable*> get_variables();
    Variable* get_val(){return m_val;};
    ~PC();
private:
    Variable* m_val;
    std::vector<Composant*> m_intermediaires;
};

class Alu:public Composant
{
public:
    Alu(Variable* instr,Variable* op1,Variable* op2);
    void print(std::ostream& flux);
    std::vector<Variable*> get_variables();
    Variable* get_sortie(){return m_sortie;};
    Variable* get_flags(){return m_flags->get_sortie();};
    ~Alu();
private:
    Variable* reverse(Variable* var);
    Variable* m_op1;
    Variable* m_op2;
    Variable* m_instr;
    Variable* m_sortie;
    Primitive* m_flags;
    std::vector<Composant*> m_intermediaires;
    std::vector<Variable*> m_variables;
    std::vector<Variable*> m_binop_temp;
};

class Microprocesseur:public Composant
{
public:
    Microprocesseur();
    void print(std::ostream& flux);
    Variable* get_registre(int no){return m_gest->get_registre(no);};
    Variable* get_ram(){return m_ram;};
    Variable* get_pc(){return m_pc;};
    Variable* get_ram_ra(){return m_ram_ra;};
    Variable* get_ram_wa(){return m_ram_wa;};
    Variable* get_out(){return m_out;};
    Alu* get_alu(){return m_alu;};
    std::vector<Variable*> get_variables();
    ~Microprocesseur();
private:
    std::vector<Composant*> m_intermediaires;
    GestionnaireRegistres* m_gest;
    Variable* m_ram;
    Variable* m_ram_ra;
    Variable* m_ram_wa;
    Variable* m_out;
    Variable *m_pc;
    Alu* m_alu;
};


#endif