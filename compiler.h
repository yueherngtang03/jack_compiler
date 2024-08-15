#ifndef COMPILER_H
#define COMPILER_H

//#define TEST_COMPILER

#include "parser.h"
#include "symbols.h"

int InitCompiler ();
ParserInfo compile (char* dir_name);
int StopCompiler();

#endif
