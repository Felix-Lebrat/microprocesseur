#ifndef _ALU_H_
#define _ALU_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <map>
#include <set>
#include <assert.h>

std::map<std::string,unsigned int>& variables_alu();

void print_alu(std::ostream &flux);

#endif