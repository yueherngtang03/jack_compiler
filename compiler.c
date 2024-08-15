#include "compiler.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>

#include "symbols.h"

int InitCompiler ()
{
    ClearAll();
    ClearMethodVar();
    ClearClassVar();
	return 1;
}

ParserInfo compile (char* dir_name)
{
	ParserInfo p;
    p.er = none;

    DIR *directory;
    struct dirent *entry;

    for (int i = 0; i < 2; i++){
        changeQuickScan(i);

        directory = opendir(".");
        if (directory == NULL) {
            perror("Unable to open directory");
            exit(EXIT_FAILURE);
        }

        while ((entry = readdir(directory)) != NULL) {
            char *extension = strrchr(entry->d_name, '.');
            if (extension != NULL && strcmp(extension, ".jack") == 0) {

                InitParser(entry->d_name);
                p = Parse();
                if (p.er != none){
                    return p;
                }

            }
        }

        directory = opendir(dir_name);

        if (quickscan == 1){
            changeFiles(1);
        }

        if (directory == NULL) {
            perror("Unable to open directory");
            exit(EXIT_FAILURE);
        }

        while ((entry = readdir(directory)) != NULL) {
            char *extension = strrchr(entry->d_name, '.');
            if (extension != NULL && strcmp(extension, ".jack") == 0) {
                char folder[128] = "";
                strcat(folder,dir_name);
                strcat(folder,"/");
                strcat(folder,entry->d_name);

                // Create VM file
                char vmname[128] = "";
                strcat(vmname,dir_name);
                strcat(vmname,"/");
                strcat(vmname,entry->d_name);
                vmname[strlen(vmname) - 5] = '\0';
                strcat(vmname,".vm");
                FILE *file_ptr;
                file_ptr = fopen(vmname, "w");


                InitParser(folder);
                p = Parse();
                if (p.er != none){
                    return p;
                }

            }
        }
    }


    StopParser();
	return p;
}

int StopCompiler ()
{
    ClearAll();
    ClearMethodVar();
    ClearClassVar();
	return 1;
}


#ifndef TEST_COMPILER
int main ()
{
    ParserInfo p;
    #define NumberTestFiles 10
    char* testPrograms[NumberTestFiles] = {
            "Seven",
            "Fraction",
            "HelloWorld",
            "Square",
            "Average",
            "ArrayTest",
            "MathTest",
        "List",
            "ConvertToBin",
            "Pong"
    };

    for (int j = 0 ; j < NumberTestFiles ; j++) // for each test file
    {
        InitCompiler();
        p = compile(testPrograms[j]);
        printf("\nThe error token is %d at line %d ---> %s\n\n", p.er, p.tk.ln, p.tk.lx);
        StopCompiler();
    }
}
#endif
