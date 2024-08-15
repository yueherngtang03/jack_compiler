#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "lexer.h"
#include "parser.h"

extern int quickscan;
extern int files;

typedef struct {
    char name[128];
    char type[128];
    char index[128];
    int isMethod;
} Symbol;


void changeQuickScan(int new);
void changeFiles(int new);

void addOneArgumentCount();
void InsertMethod(char* name, int isMethod);
int FindIsMethod(char* class_name, char* name);
int FindMethod(char* class_name, char* name);
void NextClass();
int FindClass(char* class_name);
void ClearAll();

void InsertClassVar(char* name, char* type,char* call);
int FindClassVar(char* name);
void ClearClassVar();
char* FindClassVarType(char* name);
int GetClassVarCount();

void InsertMethodVar(char* name, char* type, char* call);
int FindMethodVar(char* name);
void ClearMethodVar();
char* FindMethodVarType(char* name);


char* FindMemoryPosition(char* name);



#endif
