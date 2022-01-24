#include "composants.h"

#include <sstream>
#include <iostream>

using namespace std;

void print_netlist(ostream& flux,
                    vector<Composant*> composants,
                    vector<Variable*> entrees,
                    vector<Variable*> sorties,
                    Variable* clock,
                    Variable* display,
                    Variable* ram);

int main()
{
    Microprocesseur *mic=new Microprocesseur();
    print_netlist(cout,{mic},{},{mic->get_out(),mic->get_pc()},
        mic->get_registre(15),mic->get_registre(14),mic->get_ram());
    delete mic;

    return 0;
}

void print_netlist(ostream& flux,
                    vector<Composant*> composants,
                    vector<Variable*> entrees,
                    vector<Variable*> sorties,
                    Variable* clock,
                    Variable* display,
                    Variable* ram)
{
    flux<<"INPUT ";
    for(int i=0;i<entrees.size();i++)
    {
        if(i!=0)
            flux<<",";
        flux<<entrees[i]->print();
    }
    flux<<endl;

    flux<<"OUTPUT ";
    for(int i=0;i<sorties.size();i++)
    {
        if(i!=0)
            flux<<", ";
        flux<<sorties[i]->print();
    }
    flux<<endl;

    if(clock) flux<<"CLOCK "<<clock->print()<<endl;

    if(display) flux<<"DISPLAY "<<display->print()<<endl;

    if(ram) flux<<"RAM "<<ram->print()<<endl;

    flux<<"VAR "<<endl;
    for(int i=0;i<composants.size();i++)
    {
        vector<Variable*> var=composants[i]->get_variables();
        for(int j=0;j<var.size();j++)
        {
            if(j!=0)
                flux<<", ";
            flux<<var[j]->print()<<" : "<<var[j]->get_taille();
        }
    }
    if(composants.size()!=0&&entrees.size()!=0)
        flux<<", ";
    for(int j=0;j<entrees.size();j++)
    {
        if(j!=0)
            flux<<", ";
        flux<<entrees[j]->print()<<" : "<<entrees[j]->get_taille();
    }
    flux<<endl;

    flux<<"IN"<<endl;

    for(int i=0;i<composants.size();i++)
    {
        composants[i]->print(flux);        
    }
}