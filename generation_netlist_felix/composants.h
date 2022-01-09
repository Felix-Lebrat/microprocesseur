#ifndef _COMPOSANTS_H_
#define _COMPOSANTS_H_

#include <ostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>

class Variable
{
public:
    Variable(std::string nom,int taille);
    Variable(int val,int taille=-1):m_val(val),m_taille(taille),m_est_const(true){};
    std::string print();
    int get_taille(){return m_taille;};
    bool est_const(){return m_est_const;};
private:
    std::string m_nom;
    int m_no;
    int m_taille;
    int m_val;
    bool m_est_const;
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
private:
    Variable* m_val1;
    Variable* m_val2;
    std::vector<Composant*> m_composants;
};

class Decodeur:public Composant
{
public:
    Decodeur(Variable *instr){};
    void print(std::ostream& flux)=0;
    std::vector<Variable*> get_variables();
    ~Decodeur();
private:
    Variable* m_instr;
    Primitive *m_reg1;
    Primitive *m_reg2;
    Primitive *m_op1;
    Primitive *m_op2;
    std::vector<Composant*> m_intermediaires;
};

#endif