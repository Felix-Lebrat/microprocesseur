#include "composants.h"
#include <iostream>

using namespace std;

const string Variable::prefix="fe";
map<string,int> Variable::instances=map<string,int>();

Variable::Variable(std::string nom,int taille,bool prefix):
m_nom(nom),m_taille(taille),m_est_const(false),m_prefix(prefix)
{
    if(prefix)
    {
        if(instances.find(m_nom)==instances.end())
        {
            instances[m_nom]=0;
        }
        m_no=instances[m_nom];
        instances[m_nom]+=1;
    }
}

string Variable::print()
{
    stringstream sstream;
    if(m_est_const)
    {
        if(m_taille==-1)
            sstream<<m_val;
        else
        {
            for(int i=m_taille-1;i>=0;i--)
            {
                sstream<<int(((1<<i)&m_val)!=0);
            }
        }
    }
    else
    {
        if(m_prefix)
            sstream<<prefix<<"_"<<m_nom<<m_no;
        else
            sstream<<m_nom;
    }
    
    return sstream.str();
}

vector<Variable*> Composant::get_variables(vector<Composant*> &composants)
{
    vector<Variable*> ret;
    vector<Variable*> temp;
    for(int i=0;i<composants.size();i++)
    {
        temp=composants[i]->get_variables();
        ret.insert(ret.end(),temp.begin(),temp.end());
    }

    return ret;
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

Binop::Binop(vector<Variable*> &entrees,string type)
:m_g(0),m_d(0),m_binop(0),m_var(0)
{
    if(entrees.size()==1)
    {
        m_var=entrees[0];
    }
    else
    {
        vector<Variable*> gauche;
        gauche.insert(gauche.begin(),
            entrees.begin(),
            entrees.begin()+entrees.size()/2);
        m_g=new Binop(gauche,type);
        Variable* var_g=m_g->get_sortie();

        vector<Variable*> droite;
        droite.insert(droite.begin(),
            entrees.begin()+entrees.size()/2,
            entrees.end());
        m_d=new Binop(droite,type);
        Variable* var_d=m_d->get_sortie();

        int taille=var_g->get_taille();
        if(type=="CONCAT")
            taille+=var_d->get_taille();

        m_binop=new Primitive(type,taille,
            {var_g,var_d});
    }
}

void Binop::print(ostream& flux)
{
    if(m_var==0)
    {
        m_g->print(flux);
        m_d->print(flux);
        m_binop->print(flux);
    }
}

vector<Variable*> Binop::get_variables()
{
    if(m_var!=0)
        return {};

    vector<Variable*> ret;
    ret.push_back(m_binop->get_sortie());

    vector<Variable*> temp;

    temp=m_d->get_variables();
    ret.insert(ret.end(),temp.begin(),temp.end());

    temp=m_g->get_variables();
    ret.insert(ret.end(),temp.begin(),temp.end());

    return ret;
}

Variable* Binop::get_sortie()
{
    if(m_var==0)
        return m_binop->get_sortie();
    return m_var;
}

Binop::~Binop()
{
    if(m_g) delete m_g;
    if(m_d) delete m_d;
    if(m_binop) delete m_binop;
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
:m_val1(0),m_val2(0),m_ss_gest0(0),m_ss_gest1(0),m_reg(0)
{
    if(taille_addr==0)
    {
        Registre* reg=new Registre(write_data,write_enable);

        m_val1=reg->get_read_data();
        m_val2=reg->get_read_data();

        m_composants.push_back(reg);
        m_reg=reg;
    }
    else
    {
        Primitive* select_reg1=new Primitive("SELECT",1,{new Variable(taille_addr-1),reg1});
        Primitive* select_reg2=new Primitive("SELECT",1,{new Variable(taille_addr-1),reg2});
        Demux* demux=new Demux(write_enable,select_reg2->get_sortie());
        GestionnaireRegistres* ss_gest0=new GestionnaireRegistres(taille_addr-1,reg1,reg2,demux->get_s0(),write_data);
        GestionnaireRegistres* ss_gest1=new GestionnaireRegistres(taille_addr-1,reg1,reg2,demux->get_s1(),write_data);
        Primitive* mux_val1=new Primitive("MUX",write_data->get_taille(),{select_reg1->get_sortie(),ss_gest0->m_val1,ss_gest1->m_val1});
        Primitive* mux_val2=new Primitive("MUX",write_data->get_taille(),{select_reg2->get_sortie(),ss_gest0->m_val2,ss_gest1->m_val2});

        m_val1=mux_val1->get_sortie();
        m_val2=mux_val2->get_sortie();

        m_composants.push_back(select_reg1);
        m_composants.push_back(select_reg2);
        m_composants.push_back(demux);
        m_composants.push_back(ss_gest0);
        m_composants.push_back(ss_gest1);
        m_composants.push_back(mux_val1);
        m_composants.push_back(mux_val2);

        m_ss_gest0=ss_gest0;
        m_ss_gest1=ss_gest1;
    }
}

Variable* GestionnaireRegistres::get_registre(int no)
{
    if(m_reg)
        return m_reg->get_read_data();
    if(no%2==0)
        return m_ss_gest0->get_registre(no/2);
    return m_ss_gest1->get_registre(no/2);
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
    flux<<"//gestionnaire de registres\n";
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

vector<Variable*> Decodeur::decode(Variable* opcode)
{
    Primitive* temp;

    if(opcode->get_taille()==1)
    {
        vector<Variable*> retour;
        temp=new Primitive("NOT",1,{opcode});
        m_intermediaires.push_back(temp);
        retour.push_back(temp->get_sortie());
        retour.push_back(opcode);
        return retour;
    }

    int n=opcode->get_taille();

    temp=new Primitive("SLICE",n/2,{new Variable(0),new Variable(n/2-1),opcode});
    m_intermediaires.push_back(temp);
    vector<Variable*> gauche=decode(temp->get_sortie());

    temp=new Primitive("SLICE",(n+1)/2,{new Variable(n/2),new Variable(n-1),opcode});
    m_intermediaires.push_back(temp);
    vector<Variable*> droite=decode(temp->get_sortie());

    vector<Variable*> retour;
    for(int i=0;i<gauche.size();i++)
    {
        for(int j=0;j<droite.size();j++)
        {
            temp=new Primitive("AND",1,{gauche[i],droite[j]});
            m_intermediaires.push_back(temp);
            retour.push_back(temp->get_sortie());
        }
    }

    return retour;
}

Decodeur::Decodeur(Variable* instr)
{
    Primitive *temp;
    Binop *tempb;
    vector<Variable*> binop_temp;

    //decodage du mode d'adressage
    temp=new Primitive("SLICE",2,{new Variable(0),new Variable(1),instr});
    m_intermediaires.push_back(temp);
    m_addr_mode=decode(temp->get_sortie());
    Variable *rm=m_addr_mode[0b01];//(mov registre->memoire)

    //decodage de l'opcode
    temp=new Primitive("SLICE",4,{new Variable(2),new Variable(5),instr});
    m_intermediaires.push_back(temp);

    vector<Variable*> opcode=decode(temp->get_sortie());
    map<string,int> codes_instr=
    {                  
        {"mov",0b0000},
        {"not",0b0001},
        {"xor",0b0010},
        {"or",0b0011},
        {"and",0b0100},
        {"add",0b0101},
        {"sub",0b0110},
        {"mul",0b0111},
        {"lsl",0b1000},
        {"lsr",0b1001},
        {"push",0b1010},//non implémenté
        {"pop",0b1011},//non implémenté
        {"cmp",0b1100},
        {"test",0b1101},
        {"jmp",0b1110}
    };
    m_jmp=opcode[codes_instr["jmp"]];

    m_alu_instr=new Primitive("CONCAT",8,{new Variable(0,4),temp->get_sortie()});
    m_intermediaires.push_back(m_alu_instr);
    m_alu_instr=new Primitive("MUX",8,{
        opcode[codes_instr["cmp"]],
        m_alu_instr->get_sortie(),
        new Variable(0b110,8)        
    });
    m_intermediaires.push_back(m_alu_instr);
    m_alu_instr=new Primitive("MUX",8,{
        opcode[codes_instr["test"]],
        m_alu_instr->get_sortie(),
        new Variable(0b100,8)        
    });
    m_intermediaires.push_back(m_alu_instr);

    //adresses des registres
    m_reg1=new Primitive("SLICE",4,{new Variable(15),new Variable(18),instr});
    m_intermediaires.push_back(m_reg1);
    temp=new Primitive("SLICE",4,{new Variable(28),new Variable(31),instr});
    m_intermediaires.push_back(temp);
    m_reg2=new Primitive("MUX",4,{opcode[codes_instr["not"]],
        temp->get_sortie(),
        m_reg1->get_sortie()//instruction "not" : on écrit dans reg1
    });
    m_intermediaires.push_back(m_reg2);

    //write enable pour le gestionnaire de registres
    //not || !(cmp||test||jmp||rm)
    binop_temp.clear();

    binop_temp.push_back(rm);
    binop_temp.push_back(opcode[codes_instr["jmp"]]);
    binop_temp.push_back(opcode[codes_instr["test"]]);
    binop_temp.push_back(opcode[codes_instr["cmp"]]);

    tempb=new Binop(binop_temp,"OR");
    m_intermediaires.push_back(tempb);

    temp=new Primitive("NOT",1,{tempb->get_sortie()});
    m_intermediaires.push_back(temp);

    m_reg_we=new Primitive("OR",1,{opcode[codes_instr["not"]],temp->get_sortie()});
    m_intermediaires.push_back(m_reg_we);

    //write enable pour la ram
    //rm&&mov
    m_ram_we=new Primitive("AND",1,{rm,opcode[codes_instr["mov"]]});
    m_intermediaires.push_back(m_ram_we);

    //op1
    temp=new Primitive("SLICE",13,{new Variable(6),new Variable(18),instr});
    m_intermediaires.push_back(temp);
    m_op1=new Primitive("CONCAT",32,{new Variable(0,19),temp->get_sortie()});
    m_intermediaires.push_back(m_op1);

    //mux op1 : not||rr
    m_mux_op1=new Primitive("OR",1,{opcode[codes_instr["not"]],m_addr_mode[0b10]});
    m_intermediaires.push_back(m_mux_op1);


    //instruction retour
    temp=new Primitive("OR",1,{m_addr_mode[0b01],m_addr_mode[0b10]});
    m_intermediaires.push_back(temp);
    temp=new Primitive("AND",1,{temp->get_sortie(),opcode[codes_instr["mov"]]});
    m_intermediaires.push_back(temp);
    m_r.push_back(temp->get_sortie());//reg1 : mov&&(rm||rr)
    
    temp=new Primitive("AND",1,{opcode[codes_instr["mov"]],m_addr_mode[0b00]});
    m_intermediaires.push_back(temp);
    m_r.push_back(temp->get_sortie());//ram : mov&&mr

    temp=new Primitive("AND",1,{opcode[codes_instr["mov"]],m_addr_mode[0b11]});
    m_intermediaires.push_back(temp);
    temp=new Primitive("OR",1,{temp->get_sortie(),opcode[codes_instr["jmp"]]});
    m_intermediaires.push_back(temp);
    m_r.push_back(temp->get_sortie());//decodeur : jmp||(mov&&cr)
}

vector<Variable*> Decodeur::get_variables()
{
    return Composant::get_variables(m_intermediaires);
}

void Decodeur::print(ostream& flux)
{
    flux<<"//decodeur\n";
    for(int i=0;i<m_intermediaires.size();i++)
    {
        m_intermediaires[i]->print(flux);
    }
}

Decodeur::~Decodeur()
{
    for(int i=0;i<m_intermediaires.size();i++) delete m_intermediaires[i];
}


Flags::Flags(vector<Variable*> &addr_mode,Variable* write_data)
{
    //00 : jmp
    //01 : jnz
    //10 : jz
    //11 : jl

    Primitive* temp;
    Primitive* reg=new Primitive("REG",2,{write_data});//0:zero 1:negatif
    m_intermediaires.push_back(reg);
    Primitive* z=new Primitive("SELECT",1,{new Variable(1),reg->get_sortie()});
    m_intermediaires.push_back(z);
    Primitive* n=new Primitive("SELECT",1,{new Variable(1),reg->get_sortie()});
    m_intermediaires.push_back(n);

    vector<Variable*> binop_temp;
    temp=new Primitive("NOT",1,{z->get_sortie()});
    m_intermediaires.push_back(temp);
    temp=new Primitive("AND",1,{temp->get_sortie(),addr_mode[0b01]});
    m_intermediaires.push_back(temp);
    binop_temp.push_back(temp->get_sortie());

    temp=new Primitive("AND",1,{z->get_sortie(),addr_mode[0b10]});
    m_intermediaires.push_back(temp);
    binop_temp.push_back(temp->get_sortie());

    temp=new Primitive("AND",1,{n->get_sortie(),addr_mode[0b11]});
    m_intermediaires.push_back(temp);
    binop_temp.push_back(temp->get_sortie());

    binop_temp.push_back(addr_mode[0b00]);

    Binop* binop=new Binop(binop_temp,"OR");//sortie = jmp||(jnz&&!z)||(jz&&z)||(jl&&n)
    m_intermediaires.push_back(binop);

    m_rd=binop->get_sortie();
}

void Flags::print(ostream& flux)
{
    flux<<"//flags\n";
    for(int i=0;i<m_intermediaires.size();i++)
    {
        m_intermediaires[i]->print(flux);
    }
}

vector<Variable*> Flags::get_variables()
{
    return Composant::get_variables(m_intermediaires);
}

Flags::~Flags()
{
    for(int i=0;i<m_intermediaires.size();i++)
    {
        delete m_intermediaires[i];
    }
}

OBIncr::OBIncr(Variable* i,Variable *c_i)
{
    m_o=new Primitive("XOR",1,{i,c_i});
    m_c_o=new Primitive("AND",1,{i,c_i});
}

OBIncr::OBIncr(Variable* i)
{
    m_o=new Primitive("XOR",1,{i,new Variable(1)});
    m_c_o=new Primitive("AND",1,{i,new Variable(1)});
}

void OBIncr::print(ostream& flux)
{
    m_o->print(flux);
    m_c_o->print(flux);
}

vector<Variable*> OBIncr::get_variables()
{
    return {m_o->get_sortie(),m_c_o->get_sortie()};
}

Increment::Increment(Variable* i)
{
    Primitive* prim;
    OBIncr* obincr(0);
    vector<Variable*> binop_temp;
    binop_temp.resize(i->get_taille(),0);
    for(int k=i->get_taille()-1;k>=0;k--)
    {
        prim=new Primitive("SELECT",1,{new Variable(k),i});
        m_intermediaires.push_back(prim);
        if(obincr)
        {
            obincr=new OBIncr(prim->get_sortie(),obincr->get_c_o());
        }
        else
        {
            obincr=new OBIncr(prim->get_sortie());
        }
        m_intermediaires.push_back(obincr);
        binop_temp[k]=obincr->get_o();
    }
    m_sortie=new Binop(binop_temp,"CONCAT");
    m_intermediaires.push_back(m_sortie);
}

void Increment::print(ostream& flux)
{
    flux<<"//increment\n";
    for(int i=0;i<m_intermediaires.size();i++) m_intermediaires[i]->print(flux);
}

vector<Variable*> Increment::get_variables()
{
    return Composant::get_variables(m_intermediaires);
}

Increment::~Increment()
{
    for(int i=0;i<m_intermediaires.size();i++)
    {
        delete m_intermediaires[i];
    }
}

PC::PC(Variable* flag,Variable* addr,Variable *jmp)
{
    Primitive *et=new Primitive("AND",1,{jmp,flag});
    Primitive *mux=new Primitive("MUX",addr->get_taille(),{et->get_sortie()});
    Primitive* reg=new Primitive("REG",addr->get_taille(),{mux->get_sortie()});
    Increment* incr=new Increment(reg->get_sortie());
    mux->ajouter_entree(incr->get_sortie());
    mux->ajouter_entree(addr);
    m_val=reg->get_sortie();

    m_intermediaires.push_back(mux);
    m_intermediaires.push_back(reg);
    m_intermediaires.push_back(incr);
    m_intermediaires.push_back(et);
}

void PC::print(ostream& flux)
{
    flux<<"//PC\n";
    for(int i=0;i<m_intermediaires.size();i++)
    {
        m_intermediaires[i]->print(flux);
    }
}

vector<Variable*> PC::get_variables()
{
    return Composant::get_variables(m_intermediaires);
}

PC::~PC()
{
    for(int i=0;i<m_intermediaires.size();i++)
    {
        delete m_intermediaires[i];
    }
}

Variable* Alu::reverse(Variable* var)
{
    Primitive* temp;
    m_binop_temp.clear();
    for(int i=var->get_taille()-1;i>=0;i--)
    {
        temp=new Primitive("SELECT",1,{new Variable(i),var});
        m_binop_temp.push_back(temp->get_sortie());
        m_intermediaires.push_back(temp);
    }
    Binop* binop=new Binop(m_binop_temp,"CONCAT");
    m_intermediaires.push_back(binop);

    return binop->get_sortie();
}

Alu::Alu(Variable* instr,Variable* op1,Variable* op2)
:m_instr(instr)
{
    m_op1=reverse(op1);
    m_op2=reverse(op2);
    m_sortie=reverse(new Variable("ual_s",32,false));

    Binop *nz=new Binop(m_binop_temp,"OR");//non zero
    m_intermediaires.push_back(nz);
    Primitive* z=new Primitive("NOT",1,{nz->get_sortie()});
    m_intermediaires.push_back(z);
    
    Primitive* n=new Primitive("SELECT",1,{new Variable(0),m_sortie});
    m_intermediaires.push_back(n);

    m_flags=new Primitive("CONCAT",2,{n->get_sortie(),z->get_sortie()});
    m_intermediaires.push_back(m_flags);

    ofstream fichier_alu("alu.net");

    print_alu(fichier_alu);
    fichier_alu.close();

    map<string,unsigned int> vars=variables_alu();
    for(auto it=vars.begin();it!=vars.end();it++)
    {
        m_variables.push_back(new Variable(it->first,it->second,false));
    }
}

void Alu::print(ostream& flux)
{
    flux<<"\n//alu\n";
    ifstream fichier_alu("alu.net");

    if(!fichier_alu.good())
        throw "erreur lors de l'ouverture de \"alu.net\"";

    char temp;
    while(fichier_alu.get(temp))
    {
        flux<<temp;
    }
    fichier_alu.close();

    flux<<"ual_e1="<<m_op1->print()<<"\n";
    flux<<"ual_e2="<<m_op2->print()<<"\n";
    flux<<"ual_instr="<<m_instr->print()<<"\n";
    
    for(int i=0;i<m_intermediaires.size();i++)
    {
        m_intermediaires[i]->print(flux);
    }
    flux<<"\n";
}

vector<Variable*> Alu::get_variables()
{
    vector<Variable*> ret=Composant::get_variables(m_intermediaires);
    ret.insert(ret.end(),m_variables.begin(),m_variables.end());
    return ret;
}

Alu::~Alu()
{
    for(int i=0;i<m_intermediaires.size();i++) delete m_intermediaires[i];
    for(int i=0;i<m_variables.size();i++) delete m_variables[i];
};


Microprocesseur::Microprocesseur()
{
    //rom
    Primitive* rom=new Primitive("ROM",32,{new Variable(13),new Variable(32)});
    m_intermediaires.push_back(rom);

    //decodeur
    Decodeur* decodeur=new Decodeur(rom->get_sortie());
    m_intermediaires.push_back(decodeur);

    //alu
    Primitive *op1=new Primitive("MUX",32,{decodeur->get_mux_op1(),
        decodeur->get_op1()});
    m_intermediaires.push_back(op1);
    Primitive *op2=new Primitive("",32,{});
    m_intermediaires.push_back(op2);
    Alu *alu=new Alu(decodeur->get_alu_instr(),op1->get_sortie(),op2->get_sortie());
    m_intermediaires.push_back(alu);

    //mux final
    Primitive* mux_ret1=new Primitive("MUX",32,{
        decodeur->get_ret_instr(0),
        alu->get_sortie()
    });
    m_intermediaires.push_back(mux_ret1);
    Primitive* mux_ret2=new Primitive("MUX",32,{
        decodeur->get_ret_instr(1),
        mux_ret1->get_sortie()
    });
    m_intermediaires.push_back(mux_ret2);
    Primitive* mux_ret3=new Primitive("MUX",32,{//ceci est le résultat final
        decodeur->get_ret_instr(2),
        mux_ret2->get_sortie(),
        decodeur->get_op1()
    });
    m_intermediaires.push_back(mux_ret3);
    m_out=mux_ret3->get_sortie();

    //gestionnaire de registres
    GestionnaireRegistres *gest=new GestionnaireRegistres(4,
        decodeur->get_reg1(),decodeur->get_reg2(),decodeur->get_reg_we(),
        mux_ret3->get_sortie()
    );
    m_intermediaires.push_back(gest);
    m_gest=gest;

    //ram
    Primitive* ra_ram=new Primitive("SLICE",13,{
        new Variable(19),
        new Variable(31),
        gest->get_val1()
    });
    m_ram_ra=ra_ram->get_sortie();
    m_intermediaires.push_back(ra_ram);
    Primitive* wa_ram=new Primitive("SLICE",13,{
        new Variable(19),
        new Variable(31),
        gest->get_val2()
    });
    m_ram_wa=wa_ram->get_sortie();
    m_intermediaires.push_back(wa_ram);
    Primitive* ram=new Primitive("RAM",32,{
        new Variable(13),//addr size
        new Variable(32),//word size
        ra_ram->get_sortie(),//read addr
        decodeur->get_ram_we(),//write enabl
        wa_ram->get_sortie(),//write addr
        mux_ret3->get_sortie()//write data
    });
    m_intermediaires.push_back(ram);
    m_ram=ram->get_sortie();


    //flags
    Flags *flags=new Flags(decodeur->get_addr_mode(),alu->get_flags());
    m_intermediaires.push_back(flags);

    //program counter
    Primitive* jmp_addr=new Primitive("SLICE",13,{
        new Variable(19),
        new Variable(31),
        mux_ret3->get_sortie()
    });
    m_intermediaires.push_back(jmp_addr);
    PC *pc=new PC(flags->get_rd(),jmp_addr->get_sortie(),decodeur->get_jmp());
    m_intermediaires.push_back(pc);
    m_pc=pc->get_val();

    op1->ajouter_entree(gest->get_val1());
    op2->ajouter_entree(gest->get_val2());
    mux_ret1->ajouter_entree(gest->get_val1());
    mux_ret2->ajouter_entree(ram->get_sortie());
    rom->ajouter_entree(pc->get_val());

}

void Microprocesseur::print(ostream& flux)
{
    for(int i=0;i<m_intermediaires.size();i++)
    {
        m_intermediaires[i]->print(flux);
    }
}

vector<Variable*> Microprocesseur::get_variables()
{
    return Composant::get_variables(m_intermediaires);
}

Microprocesseur::~Microprocesseur()
{
    for(int i=0;i<m_intermediaires.size();i++)
    {
        delete m_intermediaires[i];
    }
}