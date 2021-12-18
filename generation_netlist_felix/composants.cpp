#include "composants.h"
#include <iostream>

using namespace std;

const string Variable::prefix="fe";
map<string,int> Variable::instances=map<string,int>();

Variable::Variable(std::string nom,int taille):
m_nom(nom),m_taille(taille),m_est_const(false)
{
    if(instances.find(m_nom)==instances.end())
    {
        instances[m_nom]=0;
    }
    m_no=instances[m_nom];
    instances[m_nom]+=1;
}

string Variable::print()
{
    stringstream sstream;
    if(m_est_const)
        sstream<<m_val;
    else
        sstream<<prefix<<"_"<<m_nom<<m_no;
    
    return sstream.str();
}

Primitive::~Primitive()
{
    for(int i=0;i<m_entrees.size();i++)
    {
        if(m_entrees[i]->est_const())
            delete m_entrees[i];
    }
}

void Primitive::print(ostream& flux)
{
    flux<<m_sortie.print()<<"="<<m_type<<" ";
    for(int i=0;i<m_entrees.size();i++)
    {
        flux<<m_entrees[i]->print()<<" ";
    }
    flux<<endl;
}

void Primitive::ajouter_entree(Variable* v)
{
    m_entrees.push_back(v);
}

Demux::Demux(Variable *entree,Variable* controle)
:m_e(entree),m_c(controle),
m_not("NOT",controle->get_taille(),{controle}),
m_et_0("AND",entree->get_taille(),{entree,m_not.get_sortie()}),
m_et_1("AND",entree->get_taille(),{entree,controle})
{}

Variable* Demux::get_s0()
{
    return m_et_0.get_sortie();
}

Variable* Demux::get_s1()
{
    return m_et_1.get_sortie();
}

void Demux::print(ostream& flux)
{
    m_not.print(flux);
    m_et_0.print(flux);
    m_et_1.print(flux);
}

vector<Variable*> Demux::get_variables()
{
    return {get_s0(),get_s1(),m_not.get_sortie()};
}

Variable* BusOfWire::construire(int n)
{
    if(n==1)
    {
        return m_fil;        
    }
    else
    {
        Variable* v=construire(n/2);
        Primitive* p;
        p=new Primitive("CONCAT",2*int(n/2)*m_fil->get_taille(),{v,v});
        m_intermediaires.push_back(p);
        if(n%2==1)
        {
            p=new Primitive("CONCAT",n*m_fil->get_taille(),{p->get_sortie(),m_fil});
            m_intermediaires.push_back(p);
        }
        return p->get_sortie();
    }

}

BusOfWire::BusOfWire(Variable* fil,int taille_bus)
:m_fil(fil)
{
    construire(taille_bus);
}

BusOfWire::~BusOfWire()
{
    for(int i=0;i<m_intermediaires.size();i++)
    {
        delete m_intermediaires[i];
    }
}

void BusOfWire::print(std::ostream& flux)
{
    for(int i=0;i<m_intermediaires.size();i++)
    {
        m_intermediaires[i]->print(flux);
    }
}

std::vector<Variable*> BusOfWire::get_variables()
{
    vector<Variable*> ret;
    for(int i=0;i<m_intermediaires.size();i++)
    {
        ret.push_back(m_intermediaires[i]->get_sortie());
    }
    return ret;
}

Variable* BusOfWire::get_bus()
{
    return m_intermediaires[m_intermediaires.size()-1]->get_sortie();
}

Registre::Registre(Variable* write_data,Variable* write_enable):
m_reg("REG",write_data->get_taille(),{}),
m_mux("MUX",write_data->get_taille(),{write_enable,m_reg.get_sortie(),write_data})
{
    m_reg.ajouter_entree(m_mux.get_sortie());
}

void Registre::print(std::ostream& flux)
{
    m_mux.print(flux);
    m_reg.print(flux);
}

std::vector<Variable*> Registre::get_variables()
{
    return {m_mux.get_sortie(),m_reg.get_sortie()};    
}

Variable* Registre::get_read_data()
{
    return m_reg.get_sortie();
}


GestionnaireRegistres::GestionnaireRegistres(int taille_addr,Variable *reg1,Variable *reg2,Variable* write_enable,Variable* write_data)
:m_val1(0),m_val2(0)
{
    if(taille_addr==0)
    {
        Registre* reg=new Registre(write_data,write_enable);

        m_val1=reg->get_read_data();
        m_val2=reg->get_read_data();

        m_composants.push_back(reg);
    }
    else
    {
        Primitive* select_reg1=new Primitive("SELECT",1,{new Variable(taille_addr-1),reg1});
        Primitive* select_reg2=new Primitive("SELECT",1,{new Variable(taille_addr-1),reg2});
        Demux* demux=new Demux(write_enable,select_reg2->get_sortie());
        GestionnaireRegistres* ss_gest0=new GestionnaireRegistres(taille_addr-1,reg1,reg2,demux->get_s0(),write_data);
        GestionnaireRegistres* ss_gest1=new GestionnaireRegistres(taille_addr-1,reg1,reg2,demux->get_s1(),write_data);
        Primitive* mux_val1=new Primitive("MUX",write_data->get_taille(),{select_reg1->get_sortie(),ss_gest0->m_val1,ss_gest1->m_val1});
        Primitive* mux_val2=new Primitive("MUX",write_data->get_taille(),{select_reg2->get_sortie(),ss_gest0->m_val1,ss_gest1->m_val1});

        m_val1=mux_val1->get_sortie();
        m_val2=mux_val2->get_sortie();

        m_composants.push_back(select_reg1);
        m_composants.push_back(select_reg2);
        m_composants.push_back(demux);
        m_composants.push_back(ss_gest0);
        m_composants.push_back(ss_gest1);
        m_composants.push_back(mux_val1);
        m_composants.push_back(mux_val2);
    }
}

GestionnaireRegistres::~GestionnaireRegistres()
{
    for(int i=0;i<m_composants.size();i++)
    {
        delete m_composants[i];
    }
}

void GestionnaireRegistres::print(std::ostream& flux)
{
    for(int i=0;i<m_composants.size();i++)
    {
        m_composants[i]->print(flux);
    }
}

vector<Variable*> GestionnaireRegistres::get_variables()
{
    vector<Variable*> ret;
    vector<Variable*> temp;
    for(int i=0;i<m_composants.size();i++)
    {
        temp=m_composants[i]->get_variables();
        ret.insert(ret.end(),temp.begin(),temp.end());
    }
    return ret;
}