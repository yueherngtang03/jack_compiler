#include "symbols.h"
#include "string.h"
#include "stdio.h"
#include <stdlib.h>

int quickscan = 0;
int files = 0;
void changeQuickScan(int new) {
    quickscan = new;
}

void changeFiles(int new) {
    files = new;
}


// FIRST ROUND - BASE FILES
int ns_out = -1;
Symbol St[100][1000];
int OuterCount[100];

void NextClass(){
    ns_out++;
    OuterCount[ns_out] = 0;
}

void InsertMethod(char* name, int isMethod)
{
    int ns_in = OuterCount[ns_out];
    strcpy (St[ns_out][ns_in].name , name);
    St[ns_out][ns_in].isMethod = isMethod;
    OuterCount[ns_out]++;
}

int FindMethod(char* class_name, char* name)
{
    if (class_name != NULL){
        for (int i = 0; i < ns_out + 1; i++)
        {
            if (!strcmp(St[i][0].name, class_name))
            {
                for (int j = 0; j < OuterCount[i]; j++){
                    if (!strcmp(St[i][j].name, name))
                    {
                        return j;
                    }
                }
            }
        }
    }
    return -1; //Index was not found
}

int FindIsMethod(char* class_name, char* name)
{
    if (class_name != NULL){
        for (int i = 0; i < ns_out + 1; i++)
        {
            if (!strcmp(St[i][0].name, class_name))
            {
                for (int j = 0; j < OuterCount[i]; j++){
                    if (!strcmp(St[i][j].name, name))
                    {
                        return St[i][j].isMethod;
                    }
                }
            }
        }
    }
    return -1; //Index was not found
}

int FindClass(char* class_name)
{

    for (int i = 0; i < ns_out + 1; i++)
    {

        if (!strcmp(St[i][0].name, class_name))
        {
            return i;
        }
    }

    return -1; //Index was not found
}

void ClearAll(){
    for (int i = 0; i < 100; i++)
    {
        OuterCount[i] = 0;
        for (int j = 0; j < 1000; j++){
            memset(St[i][j].name, 0, sizeof(St[i][j].name));
            memset(St[i][j].type, 0, sizeof(St[i][j].type));
        }
    }

    ns_out = -1;
}

// SECOND ROUND - CLASS VARIABLES
Symbol ClassVar[1000];
int classvar_count = 0;
int static_count = 0;
int field_count = 0;

void InsertClassVar(char* name, char* type, char* call)
{
    strcpy (ClassVar[classvar_count].name , name);
    strcpy (ClassVar[classvar_count].type , type);

    if (!strcmp(call,"static")){
        char str[3];
        char output[128];
        sprintf(str, "%d", static_count);
        strcat(output,"static ");
        strcat(output, str);
        strcpy(ClassVar[classvar_count].index , output);
        static_count++;
    }
    else if (!strcmp(call,"field")){
        char str[3];
        char output[128];
        sprintf(str, "%d", field_count);
        strcat(output,"this ");
        strcat(output, str);
        strcpy(ClassVar[classvar_count].index , output);
        field_count++;
    } else{
        strcmp(ClassVar[classvar_count].index , "This is not a field nor static");
    }

    classvar_count++;
}


int FindClassVar(char* name)
{
    for (int j = 0; j < classvar_count; j++){
        if (!strcmp(ClassVar[j].name, name))
        {
            return j;
        }
    }
    return -1; //Index was not found
}

void ClearClassVar(){
    for (int i = 0; i < 1000; i++) {
        // Reset name to empty string
        memset(ClassVar[i].name, 0, sizeof(ClassVar[i].name));
    }
    classvar_count = 0;
    static_count = 0;
    field_count = 0;

}

char* FindClassVarType(char* name)
{
    for (int j = 0; j < classvar_count; j++){
        if (!strcmp(ClassVar[j].name, name))
        {
            return ClassVar[j].type;
        }
    }
    return ""; //Index was not found
}

int GetClassVarCount()
{
    return classvar_count;
}

// Third Round - METHOD VARIABLES

Symbol MethodVar[1000];
int methodvar_count = 0;
int argument_count = 0;
int local_count = 0;

void addOneArgumentCount(){
    argument_count++;
}

void InsertMethodVar(char* name, char* type, char* call)
{
    strcpy (MethodVar[methodvar_count].name , name);
    strcpy (MethodVar[methodvar_count].type , type);

    if (!strcmp(call,"argument")){
        char str[3];
        char output[128];
        sprintf(str, "%d", argument_count);
        strcat(output,"argument ");
        strcat(output, str);
        strcpy(MethodVar[methodvar_count].index , output);
        argument_count++;
    }
    else if (!strcmp(call,"local")){
        char str[3];
        char output[128];
        sprintf(str, "%d", local_count);
        strcat(output,"local ");
        strcat(output, str);
        strcpy(MethodVar[methodvar_count].index , output);
        local_count++;
    } else{
        strcmp(MethodVar[methodvar_count].index , "This is not an argument nor local");
    }

    methodvar_count++;
}


int FindMethodVar(char* name)
{
    for (int j = 0; j < methodvar_count; j++){
        if (!strcmp(MethodVar[j].name, name))
        {
            return j;
        }
    }
    return -1; //Index was not found
}

void ClearMethodVar(){
    for (int i = 0; i < 1000; i++) {
        // Reset name to empty string
        memset(MethodVar[i].name, 0, sizeof(MethodVar[i].name));
    }
    methodvar_count = 0;
    argument_count = 0;
    local_count = 0;
}

char* FindMethodVarType(char* name)
{
    for (int j = 0; j < methodvar_count; j++){
        if (!strcmp(MethodVar[j].name, name))
        {
            return MethodVar[j].type;
        }
    }
    return ""; //Index was not found
}

char* FindMemoryPosition(char* name){
    for (int j = 0; j < methodvar_count; j++){
        if (!strcmp(MethodVar[j].name, name))
        {
            return MethodVar[j].index;
        }
    }

    for (int j = 0; j < classvar_count; j++){
        if (!strcmp(ClassVar[j].name, name))
        {
            return ClassVar[j].index;
        }
    }
    return "";
}















