#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>
#include <functional>
#include <ctime>
#include <cmath>
#include <ncurses.h>
#include <thread>
#include <chrono>

using namespace std;

bool get_bit(char *mem,int no);

void set_bit(char *mem,int no,bool val);

void copy(char *src,char *dest,int src_addr,int dest_addr,int word_size);

void create_mem(char **mem,int addr_size,int word_size);

int to_int(char *mem,int addr,int word_size);

void of_int(char *mem,int addr,int word_size,int i);

void print(char *mem,int addr,int word_size,bool meta,int ligne=0);

bool of_input(char *mem,int size,string mem_name);

void of_string(char *mem,int size,string str);

void init_rom(char *rom,const char* filename,bool hexa,int addr_size,int word_size);

void op_not(char *left,char* right,int size);

void op_and(char *left,char* a,char* b,int size);

void op_nand(char *left,char* a,char* b,int size);

void op_or(char *left,char* a,char* b,int size);

void op_xor(char *left,char* a,char* b,int size);

void op_concat(char *left,char* a,char* b,int a_size,int b_size);

void op_select(char *left,int i,char *a);

void op_slice(char* left,int i1,int i2,char *a);

//l'horloge
struct Coord
{
        Coord():x(0),y(0){};
        Coord(double x,double y):x(x),y(y){};

        double x;
        double y;
};

Coord operator+(Coord const& a,Coord const& b);
Coord operator-(Coord const& a,Coord const& b);
Coord operator*(double const& a,Coord const& b);

void setPixel(Coord pos);

void segment(Coord a,Coord b);

const Coord segments[7][2]=
{
    {Coord(0,1),Coord(0,4)},
    {Coord(1,5),Coord(4,5)},
    {Coord(6,5),Coord(9,5)},
    {Coord(10,4),Coord(10,1)},
    {Coord(9,0),Coord(6,0)},
    {Coord(4,0),Coord(1,0)},
    {Coord(5,1),Coord(5,4)}
};

void afficheur(Coord pos,char val);

void horloge(Coord pos,char* data);

//les classes
class Variable
{
public:
    Variable(){};
    Variable(int addr_size,int word_size)
    :m_addr_size(addr_size),m_word_size(word_size),m_etape(0)
    {
        create_mem(&m_mem,addr_size,word_size);
    }
    char* val()
    {
        if(m_etape!=Variable::etape)
        {
            calculer();
            m_etape=Variable::etape;
        }
        return m_mem;
    };
    virtual void calculer()=0;
    static int etape;
    int get_word_size(){return m_word_size;};
    int get_addr_size(){return m_addr_size;};
    char* get_mem(){return m_mem;};
protected:
    char* m_mem;
    int m_etape;
    int m_addr_size;
    int m_word_size;
};

class Id:public Variable
{
public:
    Id(){};
    Id(int addr_size,int word_size,Variable* var):Variable(addr_size,word_size),m_var(var)
    {};
    void calculer()
    {
        copy(m_var->val(),m_mem,0,0,m_word_size);
    }
private:
    Variable* m_var;
};

class Constante:public Variable
{
public:
    Constante(){};
    Constante(int word_size,string c):Variable(0,word_size)
    {
        of_string(m_mem,word_size,c);
    };
    void calculer(){};
};

class Not:public Variable
{
public:
    Not(){};
    Not(int word_size,Variable* var):Variable(0,word_size),m_var(var)
    {};
    void calculer()
    {
        op_not(m_mem,m_var->val(),m_word_size);
    };
private:
    Variable* m_var;
};

class Reg:public Variable
{
public:
    Reg(){};
    Reg(int word_size,Variable *var):Variable(0,word_size),m_var(var)
    {};
    void calculer(){};
    void actualiser()
    {
        copy(m_var->val(),m_mem,0,0,m_word_size);
    };
    char* get_mem(){return m_mem;};
private:
    Variable* m_var;
};

class Concat:public Variable
{
public:
    Concat(){};
    Concat(Variable *a,Variable* b):Variable(0,a->get_word_size()+b->get_word_size()),
    m_a(a),m_b(b)
    {};
    void calculer()
    {
        op_concat(m_mem,m_a->val(),m_b->val(),m_a->get_word_size(),m_b->get_word_size());
    };
private:
    Variable* m_a;
    Variable* m_b;

};

class Select:public Variable
{
public:
    Select(){};
    Select(Variable* var,int no):Variable(0,1),m_var(var),m_no(no)
    {};
    void calculer()
    {
        op_select(m_mem,m_no,m_var->val());
    }
private:
    Variable* m_var;
    int m_no;
};

class Mux:public Variable
{
public:
    Mux(){};
    Mux(Variable* choice,Variable* a,Variable* b):
    Variable(0,a->get_word_size()),m_choice(choice),m_a(a),m_b(b){};
    void calculer()
    {
        if(!get_bit(m_choice->val(),0))
            copy(m_a->val(),m_mem,0,0,m_word_size);
        else
            copy(m_b->val(),m_mem,0,0,m_word_size);
    };
private:
    Variable* m_choice;
    Variable* m_a;
    Variable* m_b;
};

class Slice:public Variable
{
public:
    Slice(){};
    Slice(Variable *var,int i1,int i2)
    :Variable(0,i2-i1+1),m_var(var),m_i1(i1),m_i2(i2)
    {};
    void calculer()
    {
        op_slice(m_mem,m_i1,m_i2,m_var->val());
    };
private:
    Variable* m_var;
    int m_i1;
    int m_i2;
};

class Rom:public Variable
{
public:
    Rom(){};
    Rom(int word_size,string nom,bool hexa,Variable *ra)
    :Variable(ra->get_word_size(),word_size),m_ra(ra)
    {
        create_mem(&m_rom,m_addr_size,word_size);
        init_rom(m_rom,nom.c_str(),hexa,m_addr_size,word_size);
    };
    void calculer()
    {
        copy(m_rom,m_mem,to_int(m_ra->val(),0,m_addr_size),0,m_word_size);
    };
private:
    char* m_rom;
    Variable* m_ra;
};

class Ram:public Variable
{
public:
    Ram(){};
    Ram(Variable* ra,
    Variable* we,Variable* wa,Variable* wd,int word_size):
    Variable(ra->get_word_size(),word_size),m_ra(ra),m_we(we),
    m_wa(wa),m_wd(wd)
    {
        create_mem(&m_ram,m_addr_size,m_word_size);
    };
    void calculer()
    {
        copy(m_ram,m_mem,to_int(m_ra->val(),0,m_addr_size),0,m_word_size);
    };
    void actualiser()
    {
        if(get_bit(m_we->val(),0))
        {
            copy(m_wd->val(),m_ram,0,to_int(m_wa->val(),0,m_addr_size),m_word_size);
        }
    };
    char* get_ram(){return m_ram;};
private:
    char* m_ram;
    Variable* m_ra;
    Variable* m_we;
    Variable* m_wa;
    Variable* m_wd;
};

class Binop:public Variable
{
public:
    Binop(){};
    Binop(Variable* a,Variable* b,char op)
    :Variable(0,a->get_word_size()),m_op(op),m_a(a),m_b(b)
    {};
    void calculer()
    {
        switch (m_op)
        {
        case 'a'://and
            op_and(m_mem,m_a->val(),m_b->val(),m_word_size);
            break;
        case 'o'://or
            op_or(m_mem,m_a->val(),m_b->val(),m_word_size);
            break;
        case 'n'://nand
            op_nand(m_mem,m_a->val(),m_b->val(),m_word_size);
            break;
        case 'x'://xor
            op_xor(m_mem,m_a->val(),m_b->val(),m_word_size);
            break;
        default:
            throw "oups";
            break;
        }
    };
private:
    Variable* m_a;
    Variable* m_b;
    char m_op;

};

class Input:public Variable
{
public:
    Input(){};
    Input(string name,int word_size):
    Variable(0,word_size),m_name(name)
    {};
    void calculer()
    {
        while(!of_input(m_mem,m_word_size,m_name));
    };
private:
    string m_name;
};


int Variable::etape=0;


int main(int argc,char **argv)
{

    int step=-1;
    bool hexa=false;
    int clock=0;
    char clock_data_7[14];
    int clock_data[6];
    char clock_codage[10]={
        0b0111111,
        0b0000110,
        0b1011011,
        0b1001111,
        0b1100110,
        0b1101101,
        0b1111101,
        0b0000111,
        0b1111111,
        0b1101111
    };
    bool affichage=true;
    bool debug=false;

    for(int i=1;i<argc;i++)
    {
        if(argv[i]==string("-h"))
        {
            hexa=true;
            continue;
        }
        if(argv[i]==string("-s"))
        {
            i++;
            if(i<argc)
            {
                step=atoi(argv[i]);
            }
            continue;
        }
        if(argv[i]==string("-a"))
            affichage=false;
        if(argv[i]==string("-d"))
            debug=true;
    }

//déclaration des variables
$

//initialisation des variables
$

    if(meta)
    {
        initscr();
        start_color();
        init_color(COLOR_YELLOW, 1000, 890, 0);
        init_pair(1,COLOR_MAGENTA,COLOR_MAGENTA);
        init_pair(2,COLOR_YELLOW,COLOR_YELLOW);
    }
    while(step!=0)
    {
        Variable::etape++;
        step--;

        if(affichage)
        {

        //outputs
$

        }

        //actualisation des rams
$


        //actualisation des registres
$


        if(meta)
        {
            if(clock!=time(0))
            {
                clock=time(0);
                of_int(clock_registre->get_mem(),0,clock_registre->get_word_size(),clock);
            }

            int disp=to_int(display->get_mem(),0,display->get_word_size());
            if(disp!=0)
            {
                for(int i=0;i<6;i++)
                {   
                    clock_data[i]=
                        to_int(ram->get_ram(),
                        (disp&((1<<ram->get_addr_size())-1))+i,//on selectionne les derniers bits pour l'adresse
                        ram->get_word_size());
                }
                for(int i=0;i<5;i++)
                {
                    clock_data_7[2*i+1]=clock_codage[clock_data[i]%10];
                    clock_data[i]/=10;
                    clock_data_7[2*i]=clock_codage[clock_data[i]%10];
                }
                for(int i=0;i<4;i++)
                {
                    clock_data_7[13-i]=clock_codage[clock_data[5]%10];
                    clock_data[5]/=10;
                }
                clear();
                horloge(Coord(9,9),clock_data_7);
                refresh();
                of_int(display->get_mem(),0,display->get_word_size(),0);
            }
        }

        if(meta)
        {
            if(affichage) refresh();
            if(debug)
                getch();
        }

    }

    if(meta) endwin();
}

bool get_bit(char *mem,int no)
{
    int i=no/8;
    int k=no%8;
    char mask=1<<k;
    return mem[i] & mask;
}

void set_bit(char *mem,int no,bool val)
{
    int i=no/8;
    int k=no%8;
    char mask0=~(1<<k);
    mem[i]=mem[i]&mask0;
    char mask=val<<k;
    mem[i]=mem[i]|mask;
}

void copy(char *src,char *dest,int src_addr,int dest_addr,int word_size)
{
    for(int i=0;i<word_size;i++)
    {
        set_bit(dest,dest_addr*word_size+i,get_bit(src,src_addr*word_size+i));
    }
}

void create_mem(char **mem,int addr_size,int word_size)
{
    int size=((1<<addr_size)*word_size+7)/8;
    *mem=new char[size];
    for(int i=0;i<size;i++)
    {
        (*mem)[i]=0;
    }
}

int to_int(char *mem,int addr,int word_size)
{
    int ret=0;
    for(int i=0;i<word_size;i++)
    {
        ret*=2;
        ret+=get_bit(mem,addr*word_size+i);
    }
    return ret;
}

void of_int(char *mem,int addr,int word_size,int i)
{
    for(int k=word_size-1;k>=0;k--)
    {
        set_bit(mem,addr*word_size+k,i%2);
        i/=2;
    }
}

void print(char *mem,int addr,int word_size,bool meta,int ligne)
{
    if(meta) move(ligne,0);
    for(int i=0;i<word_size;i++)
    {
        if(meta)
            printw("%d",get_bit(mem,addr*word_size+i));
        else
            cout<<get_bit(mem,addr*word_size+i);
    }
    if(!meta) cout<<endl;
}

bool of_input(char *mem,int size,string mem_name)
{
    cout<<mem_name<<"?"<<endl;
    string str;
    cin>>str;
    if(size!=str.size())
    {
        cerr<<"mauvais format"<<endl;
        return false;
    }            
    for(int i=0;i<size;i++)
    {
        if(str[i]=='0'||str[i]=='1')
        {
            str[i]-='0';
            set_bit(mem,i,str[i]);
        }
        else
        {
            cerr<<"mauvais format"<<endl;
            return false;
        }
    }
    return true;
}

void of_string(char *mem,int size,string str)
{
    for(int i=0;i<size;i++)
    {
        str[i]-='0';
        set_bit(mem,i,str[i]);
    }
}

void init_rom(char *rom,const char* filename,bool hexa,int addr_size,int word_size)
{
    ifstream file(filename);
    if(!file)
    {
        cerr<<"erreur lors de l'ouverture du fichiers "<<filename<<endl;
        exit(EXIT_FAILURE);
    }

    char buff;
    int nb_bits;
    int size=(1<<addr_size)*word_size;
    if(hexa)
    {
        nb_bits=4;
        if(size%4!=0)
        {
            cerr<<"format hexadecimal impossible car la taille de la rom n'est pas multiple de 4"<<endl;
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        nb_bits=1;
    }
    int pos=0;
    while(file.get(buff)&&pos<size)
    {
        if(hexa)
        {
            if('0'<=buff && buff <= '9')
                buff-='0';
            else if ('A'<=buff && buff <='F')
                buff+=10-'A';
            else if ('a'<=buff && buff <= 'f')
                buff+=10-'a';
            else
                continue;
        }
        else
        {
            if(buff=='0' || buff=='1')
                buff-='0';
            else
                continue;
        }
        pos+=nb_bits;
        for(int i=1;i<=nb_bits;i++)
        {
            set_bit(rom,pos-i,buff%2);
            buff=buff>>1;
        }
    }
    file.close();
}

void op_not(char *left,char* right,int size)
{
    for(int i=0;i<(size+7)/8;i++)
    {
        left[i]=~right[i];
    }
}

void op_and(char *left,char* a,char* b,int size)
{
    for(int i=0;i<(size+7)/8;i++)
    {
        left[i]=a[i]&b[i];
    }
}

void op_nand(char *left,char* a,char* b,int size)
{
    for(int i=0;i<(size+7)/8;i++)
    {
        left[i]=~(a[i]&b[i]);
    }
}

void op_or(char *left,char* a,char* b,int size)
{
    for(int i=0;i<(size+7)/8;i++)
    {
        left[i]=(a[i]|b[i]);
    }
}

void op_xor(char *left,char* a,char* b,int size)
{
    for(int i=0;i<(size+7)/8;i++)
    {
        left[i]=(a[i]^b[i]);
    }
}

void op_concat(char *left,char* a,char* b,int a_size,int b_size)
{
    for(int i=0;i<a_size;i++)
    {
        set_bit(left,i,get_bit(a,i));
    }

    for(int i=0;i<b_size;i++)
    {
        set_bit(left,i+a_size,get_bit(b,i));
    }
}

void op_select(char *left,int i,char *a)
{
    set_bit(left,0,get_bit(a,i));
}


void op_slice(char* left,int i1,int i2,char *a)
{
    for(int i=0;i<i2-i1+1;i++)
    {
        set_bit(left,i,get_bit(a,i+i1));
    }
}

//l'horloge
Coord operator+(Coord const& a,Coord const& b)
{
    return Coord(a.x+b.x,a.y+b.y);
}

Coord operator*(double const& a,Coord const& b)
{
    return Coord(a*b.x,a*b.y);
}

Coord operator-(Coord const& a,Coord const& b)
{
    return a+(-1)*b;
}

void setPixel(Coord pos,int color)
{
    mvaddch(int(pos.x),int(2*pos.y),' '|COLOR_PAIR(color));    
    mvaddch(int(pos.x),int(2*pos.y+1),' '|COLOR_PAIR(color));    
}

void segment(Coord a,Coord b,int color)
{
    Coord vect;
    Coord dir(b.x-a.x,b.y-a.y);
    Coord pos=a;
    double norm(sqrt(dir.x*dir.x+dir.y*dir.y));
    dir.x/=norm;
    dir.y/=norm;
    while(norm>0.9)
    {
        setPixel(pos,color);

        pos=pos+dir;

        vect=b-pos;

        norm=vect.x*vect.x+vect.y*vect.y;
    }
    setPixel(pos,color);
}

void afficheur(Coord pos,char val,int color)
{
    for(int i=0;i<7;i++)
    {
        if(val%2)
        {
            segment(segments[i][0]+pos,segments[i][1]+pos,color);
        }
        val/=2;
    }
}

void horloge(Coord pos,char* data)
{
    Coord curr_pos=pos+Coord(0,10);

    for(int i=0;i<3;i++)
    {
        afficheur(curr_pos,data[2*i],1);
        curr_pos=curr_pos+Coord(0,7);
        afficheur(curr_pos,data[2*i+1],1);
        curr_pos=curr_pos+Coord(0,7);
        if(i!=2)
        {
            setPixel(curr_pos+Coord(3,0),2);
            setPixel(curr_pos+Coord(7,0),2);
            curr_pos=curr_pos+Coord(0,2);
        }
    }

    curr_pos=pos+Coord(13,0);
    for(int i=3;i<6;i++)
    {
        afficheur(curr_pos,data[2*i],1);
        curr_pos=curr_pos+Coord(0,7);
        afficheur(curr_pos,data[2*i+1],1);
        curr_pos=curr_pos+Coord(0,7);
        if(i==5)
        {
            i++;
            afficheur(curr_pos,data[2*i],1);
            curr_pos=curr_pos+Coord(0,7);
            afficheur(curr_pos,data[2*i+1],1);
            curr_pos=curr_pos+Coord(0,7);
        }
        else
        {
            segment(curr_pos+Coord(9,0),curr_pos+Coord(1,3),2);
            curr_pos=curr_pos+Coord(0,5);
        }
    }
    mvaddch(LINES-1,COLS-1,' ');
}