#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lexer.h"
#include "parser.h"

#include "symbols.h"
// you can declare prototypes of parser functions below

ParserInfo classDeclar();
ParserInfo memberDeclar();
ParserInfo classVarDeclar();
ParserInfo type();
ParserInfo subroutineDeclar();
ParserInfo paramList();
ParserInfo subroutineBody();
ParserInfo statement();
ParserInfo varDeclarStatement();
ParserInfo varDeclarStatementRecur();
ParserInfo letStatement();
ParserInfo ifStatement();
ParserInfo whileStatement();
ParserInfo doStatement();
ParserInfo subroutineCall();
ParserInfo expressionList();
ParserInfo returnStatement();
ParserInfo expression();
ParserInfo relationalExpression();
ParserInfo expression();
ParserInfo relationalExpression();
ParserInfo arithmeticExpression();
ParserInfo term();
ParserInfo factor();
ParserInfo operand();
int WriteVM();
int ClearCodeList();

int paren = 0;
char classname[128];
int classfield_count = 0;
int while_count = 0;
int if_count = 0;
int pop_point = 3;

char code_list[10000][10000];
int code_list_count = 1;

int localvar_count = 0;

char directory[128];

int expressionList_count = 0;

int isRecursive = 0;
int previous_expressionList_count = 0;

int isMethod = 0;
int initial_pointer = 0;


ParserInfo classDeclar (){
    ClearClassVar();
    ParserInfo pi;
    pi.er = none;
    Token t = GetNextToken();

    t = GetNextToken();
    if (t.tp != ID){
        pi.er = idExpected;
        pi.tk = t;
        return pi;
    }

    // Quickscan here
    strcpy(classname, t.lx);

    if (quickscan == 0){
        NextClass();
        if (FindClass(t.lx) != -1){

            pi.er = redecIdentifier;
            pi.tk = t;
            return pi;
        }
        InsertMethod(t.lx,0);
    }

    t = GetNextToken();
    if (t.tp == SYMBOL && !strcmp(t.lx, "{")){
        ;
    } else {
        pi.er = openBraceExpected;
        pi.tk = t;
        return pi;
    }

    t = PeekNextToken();
    while (t.tp != EOFile && strcmp(t.lx, "}")) // May have errors, possibly check for }
    {
        pi = memberDeclar();

        // if pi is error then return?
        if (pi.er != none){
            return pi;
        }
        t = PeekNextToken();
    }

    // Check for errors from here
    t = GetNextToken();

    if (t.tp == SYMBOL && !strcmp(t.lx, "}")){
        ;
    } else {
        pi.er = closeBraceExpected;
        pi.tk = t;
        return pi;
    }

    return pi;
}

ParserInfo memberDeclar(){
    ParserInfo pi;
    pi.er = none;

    Token t = PeekNextToken();

    if (t.tp == RESWORD && (!strcmp(t.lx, "field") || !strcmp(t.lx, "static"))){
        pi = classVarDeclar();
    } else if (t.tp == RESWORD && (!strcmp(t.lx, "constructor") || !strcmp(t.lx, "function") || !strcmp(t.lx, "method"))){
        pi = subroutineDeclar();


    } else{
        pi.er = memberDeclarErr;
        pi.tk = t;
    }

    return pi;
}


ParserInfo classVarDeclar(){
    ParserInfo pi;
    Token t = GetNextToken();
    int isField = 0;
    pi.er = none;
    if (t.tp == RESWORD && (!strcmp(t.lx, "field") || !strcmp(t.lx, "static"))){
        if (!strcmp(t.lx, "field")&& files == 1 && quickscan == 1){
            isField = 1;
        }
    } else{
        pi.er = classVarErr;
        pi.tk = t;
        return pi;
    }

    char varType[128];
    strcpy(varType,PeekNextToken().lx);
    pi = type();
    if (pi.er != none){
        return pi;
    }

    t = GetNextToken();
    if (t.tp == ID){
        if (isField && files == 1 && quickscan == 1){
            classfield_count += 1;
        }
    } else{
        pi.er = idExpected;
        pi.tk = t;
        return pi;
    }

    if (quickscan == 1){
        int x = FindClassVar(t.lx);
        if (x == -1){
            if (isField){
                InsertClassVar(t.lx, varType, "field");
            } else {
                InsertClassVar(t.lx, varType, "static");
            }
        } else{

            pi.er = redecIdentifier;
            pi.tk = t;
            return pi;
        }
    }


    t = GetNextToken();

    while(t.tp == ID || (t.tp == SYMBOL && !strcmp(t.lx, ","))){ // CHECK THIS
        if (quickscan == 1 && t.tp == ID){
            int x = FindClassVar(t.lx);
            if (x == -1){
                if (isField) {
                    InsertClassVar(t.lx, varType, "field");
                } else {
                    InsertClassVar(t.lx, varType, "static");
                }
            } else{

                pi.er = redecIdentifier;
                pi.tk = t;
                return pi;
            }
            if (isField){
                classfield_count += 1;
            }
        }
        t = GetNextToken();
    }

    if (t.tp == SYMBOL && !strcmp(t.lx, ";")){
        ;
    } else{
        pi.er = semicolonExpected;
        pi.tk = t;
        return pi;
    }

    return pi;
}


ParserInfo type(){
    ParserInfo pi;
    Token t = GetNextToken();

    if (t.tp == ID || (t.tp == RESWORD && (!strcmp(t.lx, "int") || !strcmp(t.lx, "char") | !strcmp(t.lx, "boolean")))){
        ;
    } else{
        pi.er = illegalType;
        pi.tk = t;
        return pi;
    }

    if (t.tp == ID && quickscan == 1){
        int x = FindClass(t.lx);

        if (x != -1){
            ;
        } else{
            pi.er = undecIdentifier;
            pi.tk = t;
            return pi;
        }
    }

    pi.er = none;
    return pi;
}


ParserInfo subroutineDeclar(){
    ClearMethodVar();
    ClearCodeList();
    while_count = 0;
    if_count = 0;
    isMethod = 0;
    pop_point = 3;
    ParserInfo pi;
    pi.er = none;
    Token t = GetNextToken();

    if (t.tp == RESWORD && (!strcmp(t.lx, "constructor") || !strcmp(t.lx, "function") || !strcmp(t.lx, "method"))){
        if (!strcmp(t.lx, "constructor")){
            char output[100] = "";
            char indexStr[100] = "";
            strcat(output,"push constant ");
            sprintf(indexStr, "%d", classfield_count);
            strcat(output, indexStr);
            strcat(output, "\n");
            strcpy(code_list[code_list_count++], output);

            strcpy(code_list[code_list_count++], "call Memory.alloc 1\n");
            strcpy(code_list[code_list_count++], "pop pointer 0\n");
            pop_point = 0;
        } else if (!strcmp(t.lx, "method")){
            isMethod = 1;
            addOneArgumentCount();
            initial_pointer = 0;

            if (isMethod == 1 && initial_pointer == 0){
                initial_pointer = 1;
                strcpy(code_list[code_list_count++],"push argument 0\n");
                strcpy(code_list[code_list_count++],"pop pointer 0\n");
                pop_point = 0;
            }
        }
    } else{
        pi.er = subroutineDeclarErr;
        pi.tk = t;
        return pi;
    }


    t = PeekNextToken();
    if (t.tp == RESWORD && !strcmp(t.lx, "void")){
        t = GetNextToken();
    } else{
        pi = type();
        if (pi.er != none){
            return pi;
        }
    }

    char function_name[100];
    t = GetNextToken();
    if (t.tp == ID){
        strcpy(function_name, t.lx);
    } else{
        pi.er = idExpected;
        pi.tk = t;
        return pi;
    }


    if (quickscan == 0){

        int x = FindMethod(classname,t.lx);

        if (x != -1){

            pi.er = redecIdentifier;
            pi.tk = t;
            return pi;
        }

        if (isMethod == 1) {
            InsertMethod(t.lx, 1);
        } else{
            InsertMethod(t.lx, 0);
        }
    }




    t = GetNextToken();
    if (t.tp == SYMBOL && !strcmp(t.lx, "(")){
        ;
    } else{
        pi.er = openParenExpected;
        pi.tk = t;
        return pi;
    }

    pi = paramList();
    if (pi.er != none){
        return pi;
    }

    t = GetNextToken();

    if (t.tp == SYMBOL && !strcmp(t.lx, ")")){
        ;
    } else{
        pi.er = closeParenExpected;
        pi.tk = t;
        return pi;
    }

    // REMEMBER TO PUT THIS BACK
    pi = subroutineBody();

    if (quickscan == 1 && files == 1){
        char output[100] = "";

        strcat(output, "function ");
        strcat(output, classname);
        strcat(output, ".");
        strcat(output, function_name);
        strcat(output, " ");
        char localvar_count_str[10];
        sprintf(localvar_count_str,"%d", localvar_count);
        strcat(output, localvar_count_str);
        strcat(output, "\n");

        strcpy(code_list[0],output);
    }

    if (quickscan == 1 && files == 1){
        WriteVM();

    }

    return pi;

}


ParserInfo paramList(){

    ParserInfo pi;
    pi.er = none;

    Token t = PeekNextToken();

    if (t.tp == SYMBOL && !strcmp(t.lx, ")")){

        return pi;
    }

    if ((!strcmp(t.lx, "{"))){
        pi.er = closeParenExpected;
        pi.tk = t;
        return pi;
    }

    char varType[128];
    strcpy(varType,PeekNextToken().lx);
    pi = type();
    if (pi.er != none){
        return pi;
    }

    t = GetNextToken();

    if (t.tp != ID){
        pi.er = idExpected;
        pi.tk = t;
        return pi;
    }

    if (quickscan == 1){
        int x = FindMethod(classname,t.lx);
        if (x != -1){
            pi.er = redecIdentifier;
            pi.tk = t;
            return pi;
        } else{
            int x = FindClassVar(t.lx);
            if (x != -1){
                pi.er = redecIdentifier;
                pi.tk = t;
                return pi;
            } else {
                InsertMethodVar(t.lx, varType, "argument");
            }
        }
    }

    t = PeekNextToken(); // either ) or ,
    if (!strcmp(t.lx, ",")){
        t = GetNextToken();
    } else if (!strcmp(t.lx, ")")){
        return pi;
    } else{
        pi.er = syntaxError; // or closeparen??
        pi.tk = t;
        return pi;
    }
    Token prev = t; // ,
    // Entering with prev =  ,
    while (!(t.tp == SYMBOL && !strcmp(t.lx, ")"))){

        t = PeekNextToken();

        if (!strcmp(t.lx, "{")){
            pi.er = closeParenExpected;
            pi.tk = t;
            return pi;
        }
        if (!strcmp(prev.lx, ",")){
            pi = type();
            if (pi.er != none){
                return pi;
            }
            prev = t; // 1prev is type
        }
        else if (!strcmp(prev.lx, "int") || !strcmp(prev.lx, "char") || !strcmp(prev.lx, "boolean") || (prev.tp == ID && prev.ln >= 0)){
            if (t.tp == ID){
                if (quickscan == 1){
                    int x = FindMethod(classname,t.lx);
                    if (x != -1){
                        pi.er = redecIdentifier;
                        pi.tk = t;
                        return pi;
                    } else{
                        int y = FindMethodVar(t.lx);
                        if (y != -1){
                            pi.er = redecIdentifier;
                            pi.tk = t;
                            return pi;
                        } else {
                            int z = FindClassVar(t.lx);
                            if (z != -1) {
                                pi.er = redecIdentifier;
                                pi.tk = t;
                                return pi;
                            } else {
                                InsertMethodVar(t.lx, varType, "argument");
                            }
                        }
                    }
                }
            } else{
                pi.er = idExpected;
                pi.tk = t;
                return pi;
            }

            t = GetNextToken();
            t.ln = -1;
            prev = t; // 2 prev is id
        } else if (prev.tp == ID){
            if (!strcmp(t.lx, ",")){
                t = GetNextToken();
                prev = t; // 3 prev is ,
            } else if (!strcmp(t.lx, ")")){
                ;
            } else{

                pi.er = closeParenExpected;
                pi.tk = t;
                return pi;
            }

        }

    }


    return pi;

}


ParserInfo subroutineBody(){
    ParserInfo pi;
    pi.er = none;

    Token t = GetNextToken();


    if (!strcmp(t.lx, "{")){
        ;
    } else{
        pi.er = openBraceExpected;
        pi.tk = t;
        return pi;
    }

    t = PeekNextToken();
    while (!(!strcmp(t.lx, "}")))
    {
        pi = statement();
        if (pi.er != none){
            return pi;
        }
        // if pi is error then return?
        t = PeekNextToken();
    }


    t = GetNextToken();
    if (t.tp == SYMBOL && !strcmp(t.lx, "}")){

    } else {
        pi.er = closeBraceExpected;
        pi.tk = t;
        return pi;
    }

    return pi;
}


ParserInfo statement(){
    paren = 0;
    ParserInfo pi;
    pi.er = none;

    Token t = PeekNextToken();

    if (!strcmp(t.lx, "var")){
        pi = varDeclarStatement();
    } else if (!strcmp(t.lx, "let")){
        pi = letStatement();
    } else if (!strcmp(t.lx, "if")){
        pi = ifStatement();
    } else if (!strcmp(t.lx, "while")){
        pi = whileStatement();
    } else if (!strcmp(t.lx, "do")){
        pi = doStatement();
    } else if (!strcmp(t.lx, "return")){
        pi = returnStatement();
    } else{
        pi.er  = syntaxError; // is it syntax?
        pi.tk = t;
    }

    return pi;

}

ParserInfo varDeclarStatement(){
    ParserInfo pi;
    pi.er = none;

    Token t = GetNextToken();

    if (!strcmp(t.lx, "var")){
        localvar_count += 1;
    } else{
        pi.er = syntaxError;
        pi.tk = t;
        return pi;
    }

    char varType[128];
    strcpy(varType,PeekNextToken().lx);
    pi = type();
    if (pi.er != none){
        return pi;
    }


    t = GetNextToken();
    if (t.tp == ID){
        if (quickscan == 1){
            int x = FindMethod(classname,t.lx);
            if (x != -1){
                pi.er = redecIdentifier;
                pi.tk = t;
                return pi;
            } else{
                int y = FindMethodVar(t.lx);
                if (y != -1){
                    pi.er = redecIdentifier;
                    pi.tk = t;
                    return pi;
                } else {
                    InsertMethodVar(t.lx, varType, "local"); // CHECK AGAIN
//                    int z = FindClassVar(t.lx);
//                    if (z != -1){
//                        pi.er = redecIdentifier;
//                        pi.tk = t;
//                        return pi;
//                    } else {
//                        InsertMethodVar(t.lx, varType);
//                    }
                }
            }
        }
    } else{
        pi.er = idExpected;
        pi.tk = t;
        return pi;
    }

    t = PeekNextToken();

    while(!(!strcmp(t.lx, ";"))){
        pi = varDeclarStatementRecur(varType);
        if (pi.er != none){
            return pi;
        }
        t = PeekNextToken();
    }

    t = GetNextToken();
    if (t.tp == SYMBOL && !strcmp(t.lx, ";")){
        ;
    } else{
        pi.er = semicolonExpected;
        pi.tk = t;
        return pi;
    }

    return pi;


}

ParserInfo varDeclarStatementRecur(char varType[128]){
    ParserInfo pi;
    pi.er = none;

    localvar_count += 1;

    Token t = GetNextToken();
    if (!strcmp(t.lx, ",")){
        ;
    } else{
        pi.er = syntaxError;
        pi.tk = t;
        return pi;
    }

    t = GetNextToken();

    if (t.tp == ID){
        if (quickscan == 1){
            int x = FindMethod(classname,t.lx);
            if (x != -1){
                pi.er = redecIdentifier;
                pi.tk = t;
                return pi;
            } else{
                int y = FindMethodVar(t.lx);
                if (y != -1){
                    pi.er = redecIdentifier;
                    pi.tk = t;
                    return pi;
                } else {
                    int z = FindClassVar(t.lx);
                    if (z != -1){
                        pi.er = redecIdentifier;
                        pi.tk = t;
                        return pi;
                    } else {
                        InsertMethodVar(t.lx,varType,"local");
                    }
                }
            }
        }
    } else{
        pi.er = idExpected;
        pi.tk = t;
        return pi;
    }

    t = PeekNextToken();
    if (!strcmp(t.lx, ",")){
        return varDeclarStatementRecur(varType);
    } else{
        return pi;
    }

}


ParserInfo letStatement(){
    ParserInfo pi;
    pi.er = none;
    char output[100] = "";
    char first[100] = "";
    int isBracket = 0;

    Token t = GetNextToken();
    if (!strcmp(t.lx, "let")){
        ;
    } else{
        pi.er = syntaxError;
        pi.tk = t;
        return pi;
    }

    t = GetNextToken();
    if (t.tp == ID){
        if (quickscan == 1){
            int x = FindMethodVar(t.lx);
            if (x == -1){
                int y = FindClassVar(t.lx);
                if (y == -1) {
                    pi.er = undecIdentifier;
                    pi.tk = t;
                    return pi;
                }
            }
        }
    } else{
        pi.er = idExpected;
        pi.tk = t;
        return pi;
    }

    strcpy(first, t.lx);


    t = GetNextToken(); // either = or [
    if (!strcmp(t.lx, "[")){
        //GetNextToken();
        isBracket = 1;
        pi = expression();
        if (pi.er != none){
            return pi;
        }

        t = GetNextToken();

        if (!strcmp(t.lx, "]")){
            ;
        } else{
            pi.er = closeBracketExpected;
            pi.tk = t;
            return pi;
        }


        // Write the first argument a[i] where this is 'a'
        char location[100] = "";
        strcpy(location,FindMemoryPosition(first));
        if (location[0] == 't' && pop_point != 0){
            strcpy(code_list[code_list_count++], "push argument 0\n");
            strcpy(code_list[code_list_count++], "pop pointer 0\n");
        }
        strcat(output,"push ");
        strcat(output, location);
        strcat(output, "\n");
        strcpy(code_list[code_list_count++], output);
        strcpy(code_list[code_list_count++], "add\n");

        t = GetNextToken();
    }

    if (!strcmp(t.lx, "=")){
        ;
    } else{
        pi.er = equalExpected;
        pi.tk = t;

        return pi;
    }

    pi = expression();
    if (pi.er != none){
        return pi;
    }

    t = GetNextToken();
    if (t.tp == SYMBOL && !strcmp(t.lx, ";")){
        ;
    } else {
        pi.er = semicolonExpected;
        pi.tk = t;
        return pi;
    }

    if (!isBracket){
        char location[100] = "";
        strcpy(location,FindMemoryPosition(first));
        if (location[0] == 't' && pop_point != 0){
            strcpy(code_list[code_list_count++], "push argument 0\n");
            strcpy(code_list[code_list_count++], "pop pointer 0\n");
        }
        strcat(output,"pop ");
        strcat(output, location);
        strcat(output, "\n");
        strcpy(code_list[code_list_count++], output);
    } else {
        strcpy(code_list[code_list_count++], "pop temp 0\n");
        strcpy(code_list[code_list_count++], "pop pointer 1\n");
        pop_point = 1;
        strcpy(code_list[code_list_count++], "push temp 0\n");
        strcpy(code_list[code_list_count++], "pop that 0\n");
    }


    return pi;
}

ParserInfo ifStatement(){
    ParserInfo pi;
    pi.er = none;
    char output[100] = "";
    char if_count_str[100] = "";
    sprintf(if_count_str, "%d", if_count);
    if (files == 1 && quickscan == 1){
        if_count++;
    }

    Token t = GetNextToken();
    if (!strcmp(t.lx, "if")){
        ;
    } else{
        pi.er = syntaxError;
        pi.tk = t;
        return pi;
    }

    t = GetNextToken();
    if (!strcmp(t.lx, "(")){
        ;
    } else{
        pi.er = openParenExpected;
        pi.tk = t;
        return pi;
    }

    pi = expression();

    if (pi.er != none){
        return pi;
    }


    t = GetNextToken();
    if (!strcmp(t.lx, ")")){
        ;
    } else{
        pi.er = closeParenExpected;
        pi.tk = t;
        return pi;
    }

    strcpy(output,"");
    strcat(output, "if-goto IF_TRUE");
    strcat(output, if_count_str);
    strcat(output, "\n");
    strcpy(code_list[code_list_count++], output);

    strcpy(output,"");
    strcat(output, "goto IF_FALSE");
    strcat(output, if_count_str);
    strcat(output, "\n");
    strcpy(code_list[code_list_count++], output);

    t = GetNextToken();
    if (!strcmp(t.lx, "{")){
        strcpy(output,"");
        strcat(output, "label IF_TRUE");
        strcat(output, if_count_str);
        strcat(output, "\n");
        strcpy(code_list[code_list_count++], output);
    } else{
        pi.er = openBraceExpected;
        pi.tk = t;
        return pi;
    }

    t = PeekNextToken();

    if (!strcmp(t.lx, "}")){
        ;
    } else{
        while (t.tp != EOFile && strcmp(t.lx, "}")){
            pi = statement();

            if (pi.er != none){

                return pi;
            }
            t = PeekNextToken();
        }
    }

    t = GetNextToken();
    if (!strcmp(t.lx, "}")){
        ;
    } else{
        pi.er = closeBraceExpected;
        pi.tk = t;
        return pi;
    }

    t = PeekNextToken();
    if (!strcmp(t.lx, "else")){
        GetNextToken();
        strcpy(output,"");
        strcat(output, "goto IF_END");
        strcat(output, if_count_str);
        strcat(output, "\n");
        strcpy(code_list[code_list_count++], output);

        strcpy(output,"");
        strcat(output, "label IF_FALSE");
        strcat(output, if_count_str);
        strcat(output, "\n");
        strcpy(code_list[code_list_count++], output);
        t = GetNextToken();
        if (!strcmp(t.lx, "{")){
            ;
        } else{
            pi.er = openBraceExpected;
            pi.tk = t;
            return pi;
        }

        t = PeekNextToken();

        if (!strcmp(t.lx, "}")){
            ;
        } else{
            while (t.tp != EOFile && strcmp(t.lx, "}")){ // Check for error here, is it really }
                pi = statement();
                if (pi.er != none){
                    return pi;
                }
                t = PeekNextToken();
            }
        }

        t = GetNextToken();
        if (!strcmp(t.lx, "}")){
            strcpy(output,"");
            strcat(output, "label IF_END");
            strcat(output, if_count_str);
            strcat(output, "\n");
            strcpy(code_list[code_list_count++], output);
        } else{
            pi.er = closeBraceExpected;
            pi.tk = t;
            return pi;
        }

    } else{
        strcpy(output,"");
        strcat(output, "label IF_FALSE");
        strcat(output, if_count_str);
        strcat(output, "\n");
        strcpy(code_list[code_list_count++], output);
        return pi;
    }

    return pi;
}

ParserInfo whileStatement(){
    ParserInfo pi;
    pi.er = none;
    char output[100] = "";
    char while_count_str[100];
    sprintf(while_count_str, "%d", while_count);

    if (files == 1 && quickscan == 1){
        while_count++;
    }

    strcat(output, "label WHILE_EXP");
    strcat(output, while_count_str);
    strcat(output, "\n");
    strcpy(code_list[code_list_count++], output);


    Token t = GetNextToken();
    if (!strcmp(t.lx, "while")){
        ;
    } else{
        pi.er = syntaxError;
        pi.tk = t;
        return pi;
    }

    t = GetNextToken();
    if (!strcmp(t.lx, "(")){
        ;
    } else{
        pi.er = openParenExpected;
        pi.tk = t;
        return pi;
    }

    pi = expression();
    if (pi.er != none){
        return pi;
    }


    t = GetNextToken();
    if (!strcmp(t.lx, ")")){
        ;
    } else{
        pi.er = closeParenExpected;
        pi.tk = t;
        return pi;
    }

    strcpy(code_list[code_list_count++], "not\n");

    strcpy(output,"");
    strcat(output, "if-goto WHILE_END");
    strcat(output, while_count_str);
    strcat(output, "\n");
    strcpy(code_list[code_list_count++], output);

    t = GetNextToken();
    if (!strcmp(t.lx, "{")){
        ;
    } else{
        pi.er = openBraceExpected;
        pi.tk = t;
        return pi;
    }

    t = PeekNextToken();

    if (!strcmp(t.lx, "}")){
        ;
    } else{
        while (t.tp != EOFile && strcmp(t.lx, "}")){ // Check for error here, is it really }
            pi = statement();
            if (pi.er != none){
                return pi;
            }
            t = PeekNextToken();
        }
    }

    t = GetNextToken();
    if (!strcmp(t.lx, "}")){
        strcpy(output,"");
        strcat(output, "goto WHILE_EXP");
        strcat(output, while_count_str);
        strcat(output, "\n");
        strcpy(code_list[code_list_count++], output);
    } else{
        pi.er = closeBraceExpected;
        pi.tk = t;
        return pi;
    }


    strcpy(output,"");
    strcat(output, "label WHILE_END");
    strcat(output, while_count_str);
    strcat(output, "\n");
    strcpy(code_list[code_list_count++], output);

    return pi;
}

ParserInfo doStatement(){
    ParserInfo pi;
    pi.er = none;

    Token t = GetNextToken();
    if (!strcmp(t.lx, "do")){
        ;
    } else{
        pi.er = syntaxError;
        pi.tk = t;
        return pi;
    }

    pi = subroutineCall();
    if (pi.er != none){
        return pi;
    }

    t = GetNextToken();
    if (!strcmp(t.lx, ";")){
        ;
    } else{
        pi.er = semicolonExpected;
        pi.tk = t;
        return pi;
    }

    return pi;
}


ParserInfo subroutineCall(){
    int callIsMethod = 0;
    ParserInfo pi;
    pi.er = none;
    char output[100] = "";

    Token t = GetNextToken();
    Token prev = t;
    char firstName[128];
    char secondName[128];
    if (t.tp == ID){
        strcpy(firstName,t.lx);
    } else{
        pi.er = idExpected;
        pi.tk = t;
        return pi;
    }

    t = PeekNextToken(); // peeking at . or (
    if (!strcmp(t.lx, ".")){
        if (quickscan == 1){
            int x = FindClass(firstName);
            if (x == -1){
                char* y = FindMethodVarType(firstName);
                if (FindClass(y) == -1){
                    char* z = FindClassVarType(firstName);
                    if (FindClass(z) == -1){
                        pi.er = undecIdentifier;
                        pi.tk = prev;
                        return pi;
                    }
                    else{
                        char location[100] = "";
                        strcpy(output, "");
                        strcpy(location,FindMemoryPosition(firstName));
                        strcat(output,"push ");
                        strcat(output, location);
                        if (location[0] == 't' && pop_point != 0){
                            pop_point = 0;
                            strcpy(code_list[code_list_count++], "push argument 0\n");
                            strcpy(code_list[code_list_count++], "pop pointer 0\n");
                        }
                        strcat(output,"\n");
                        strcpy(code_list[code_list_count++],output);
                        strcpy(output, "");

                        strcpy(firstName, z);
                    }
                } else{
                    char location[100] = "";
                    strcpy(output, "");
                    strcpy(location,FindMemoryPosition(firstName));
                    strcat(output,"push ");
                    strcat(output, location);
                    if (location[0] == 't' && pop_point != 0){
                        pop_point = 0;
                        strcpy(code_list[code_list_count++], "push argument 0\n");
                        strcpy(code_list[code_list_count++], "pop pointer 0\n");
                    }
                    strcat(output,"\n");
                    strcpy(code_list[code_list_count++],output);
                    strcpy(output, "");

                    strcpy(firstName, y);
                }
            }
        }
        GetNextToken(); // getting .
        t = GetNextToken(); // should be id

        if (t.tp == ID){
            if (quickscan == 1){
                int x = FindMethod(firstName,t.lx);
                if (x == -1){
                    pi.er = undecIdentifier;
                    pi.tk = t;
                    return pi;
                }
                callIsMethod = FindIsMethod(firstName,t.lx);
            }
        } else{
            pi.er = idExpected;
            pi.tk = t;
            return pi;
        }

        strcpy(secondName , t.lx);

    } else if (!strcmp(t.lx, "(")) {


        if (quickscan == 1){
            int x = FindMethod(classname,firstName);
            if (x == -1){
                pi.er = undecIdentifier;
                pi.tk = prev;
                return pi;
            }
            callIsMethod = FindIsMethod(classname,firstName);
        }

        strcpy(code_list[code_list_count++],"push pointer 0\n");
        strcpy(secondName , firstName);
        strcpy(firstName , classname);
    } else{
        pi.er = openParenExpected; // or syntax error?
        pi.tk = t;
        return pi;
    }

    t = GetNextToken();
    if (!strcmp(t.lx, "(")){
        ;
    } else{
        pi.er = openParenExpected;
        pi.tk = t;
        return pi;
    }


    t = PeekNextToken();
    if (!strcmp(t.lx, ")")){ // may be buggy
        expressionList_count = 0;
    } else{
        expressionList_count = 1;
        isRecursive = 1;
        pi = expressionList();
        if (isRecursive == 3){
            expressionList_count = previous_expressionList_count;
        }
        isRecursive = 0;
        if (pi.er != none){
            return pi;
        }
    }

    t = GetNextToken();
    if (!strcmp(t.lx, ")")){
        ;
    } else{
        pi.er = closeParenExpected;
        pi.tk = t;
        return pi;
    }

    char expressionList_count_str[100];
    sprintf(expressionList_count_str, "%d", expressionList_count + callIsMethod);
    strcat(output, "call ");
    strcat(output, firstName);
    strcat(output, ".");
    strcat(output, secondName);
    strcat(output, " ");
    strcat(output, expressionList_count_str);
    strcat(output, "\n");
    strcpy(code_list[code_list_count++],output);
    strcpy(code_list[code_list_count++],"pop temp 0\n");

    return pi;
}

ParserInfo expressionList(){
    ParserInfo pi;
    pi.er = none;

    pi = expression();
    if (pi.er != none){
        return pi;
    }


    Token t = PeekNextToken(); // , or something else
    if (!strcmp(t.lx, ",")){
        expressionList_count++;
        t = GetNextToken(); // Get ,
        return expressionList();
    } else{
        return pi;
    }
}

ParserInfo returnStatement(){
    ParserInfo pi;
    pi.er = none;

    Token t = GetNextToken();
    if (!strcmp(t.lx, "return")){
        ;
    } else{
        pi.er = syntaxError;
        pi.tk = t;
        return pi;
    }


    t = PeekNextToken(); // should be expression or ;
    if (!strcmp(t.lx, ";")){
        strcpy(code_list[code_list_count++],"push constant 0\n");
    } else if (t.tp == INT || t.tp == STRING || !strcmp(t.lx, "true") || !strcmp(t.lx, "false") || !strcmp(t.lx, "null") || !strcmp(t.lx, "this") || !strcmp(t.lx, "(") ||
    t.tp == ID || !strcmp(t.lx, "-") || !strcmp(t.lx, "~")){
        pi = expression();
        if (pi.er != none){
            return pi;
        }
    } else{
        pi.er = semicolonExpected;
        pi.tk = t;
        return pi;
    }


    t = GetNextToken(); // should be ;
    if (!strcmp(t.lx, ";")){
        ;
    } else{
        pi.er = semicolonExpected;
        pi.tk = t;
        return pi;
    }

    strcpy(code_list[code_list_count++], "return\n");

    return pi;
}


ParserInfo expression(){
    ParserInfo pi;
    pi.er = none;

    pi = relationalExpression();
    if (pi.er != none){
        return pi;
    }

    Token t = PeekNextToken();

    if (!strcmp(t.lx, "&") || !strcmp(t.lx, "|")){
        GetNextToken();
        pi = expression();
        if (!strcmp(t.lx, "&")){
            strcpy(code_list[code_list_count++], "and\n");
        } else{
            strcpy(code_list[code_list_count++], "or\n");
        }
        return pi;
    } else {
        return pi;
    }

}

ParserInfo relationalExpression(){
    ParserInfo pi;
    pi.er = none;

    pi = arithmeticExpression();
    if (pi.er != none){
        return pi;
    }

    Token t = PeekNextToken();

    if (!strcmp(t.lx, "=") || !strcmp(t.lx, ">") || !strcmp(t.lx, "<")){
        GetNextToken();
        pi = relationalExpression();
        if (!strcmp(t.lx, "=")){
            strcpy(code_list[code_list_count++], "eq\n");
        } else if (!strcmp(t.lx, ">")){
            strcpy(code_list[code_list_count++], "gt\n");
        } else{
            strcpy(code_list[code_list_count++], "lt\n");
        }
        return pi;
    } else {
        return pi;
    }
}


ParserInfo arithmeticExpression(){
    ParserInfo pi;
    pi.er = none;

    pi = term();
    if (pi.er != none){
        return pi;
    }

    Token t = PeekNextToken();

    if (!strcmp(t.lx, "+") || !strcmp(t.lx, "-") ){
        GetNextToken();
        pi = arithmeticExpression();
        if (!strcmp(t.lx, "+")){
            strcpy(code_list[code_list_count++], "add\n");
        } else{
            strcpy(code_list[code_list_count++], "sub\n");
        }
        return pi;
    } else {
        return pi;
    }

}

ParserInfo term(){
    ParserInfo pi;
    pi.er = none;

    pi = factor();
    if (pi.er != none){
        return pi;
    }

    Token t = PeekNextToken();

    if (!strcmp(t.lx, "*") || !strcmp(t.lx, "/") ){
        GetNextToken();
        pi = term();
        if (!strcmp(t.lx, "*")){
            strcpy(code_list[code_list_count++], "call Math.multiply 2\n");
        } else{
            strcpy(code_list[code_list_count++], "call Math.divide 2\n");
        }
        return pi;
    } else{
        return pi;
    }
}

ParserInfo factor(){
    ParserInfo pi;
    pi.er = none;
    int isNeg = 0;

    Token t = PeekNextToken();

    if (!strcmp(t.lx, "-") || !strcmp(t.lx, "~") ){
        isNeg = 1;
        GetNextToken();
    }

    pi = operand();
    if (isNeg == 1) { // CHECK
        if (!strcmp(t.lx, "-")){
            strcpy(code_list[code_list_count++], "neg\n");
        } else{
            strcpy(code_list[code_list_count++], "not\n");
        }
    }
    if (pi.er != none){
        return pi;
    }


    return pi;

}


ParserInfo operand(){
    ParserInfo pi;
    pi.er = none;
    char output[100] = "";
    int callIsMethod = 0;



    Token t = PeekNextToken();

    if (t.tp == INT || t.tp == STRING || !strcmp(t.lx, "true") || !strcmp(t.lx, "false") || !strcmp(t.lx, "null") || !strcmp(t.lx, "this")){

        if (t.tp == STRING){
            int count = 0;
            while (t.lx[count] != '\0') {
                count++;
            }
            char count_str[10];
            sprintf(count_str, "%d", count);
            strcat(output, "push constant ");
            strcat(output, count_str);
            strcat(output, "\n");
            strcpy(code_list[code_list_count++],output);
            strcpy(code_list[code_list_count++],"call String.new 1\n");

            // Coverting each char to number & calling String.appendChar()
            count = 0;
            while (t.lx[count] != '\0') {
                strcpy(output,"");
                char asciiStr[10];
                int asciiValue = (int)t.lx[count];
                sprintf(asciiStr, "%d", asciiValue);
                strcat(output, "push constant ");
                strcat(output, asciiStr);
                strcat(output, "\n");
                strcpy(code_list[code_list_count++],output);
                strcpy(code_list[code_list_count++],"call String.appendChar 2\n");
                count++;
            }

        } else if (t.tp == INT){
            strcat(output, "push constant ");
            strcat(output, t.lx);
            strcat(output, "\n");
            strcpy(code_list[code_list_count++],output);
        } else if (!strcmp(t.lx, "this")){
            if (pop_point == 3){
                pop_point = 0;
                strcpy(code_list[code_list_count++],"push argument 0\n");
                strcpy(code_list[code_list_count++],"pop pointer 0\n");
                strcpy(code_list[code_list_count++],output);
            }
            strcat(output, "push pointer 0\n");
            strcpy(code_list[code_list_count++],output);
        } else if (!strcmp(t.lx, "true")){
            strcat(output, "push constant 0\n");
            strcpy(code_list[code_list_count++],output);
            strcpy(code_list[code_list_count++],"not\n");
        } else if (!strcmp(t.lx, "false")){
            strcat(output, "push constant 0\n");
            strcpy(code_list[code_list_count++],output);
        } else if (!strcmp(t.lx, "null")){
            strcat(output, "push constant 0\n");
            strcpy(code_list[code_list_count++],output);
        }
        GetNextToken();
        return pi;
    } else if (!strcmp(t.lx, "(")){

        GetNextToken();
        paren ++;
        pi = expression();

        if (pi.er != none){
            return pi;
        }
        t = GetNextToken();
        if (!strcmp(t.lx, ")")){
            ;
        } else{
            pi.er = closeParenExpected;
            pi.tk = t;
            return pi;
        }
    } else if (t.tp == ID){
        GetNextToken();
        char firstName[128];
        char secondName[128];
        Token prev = t;

        t = PeekNextToken();
        if (!strcmp(t.lx, ".")){
            strcpy(firstName,prev.lx);
            if (quickscan == 1){
                int x = FindClass(firstName);
                if (x == -1){
                    char* y = FindMethodVarType(firstName);
                    if (FindClass(y) == -1){
                        char* z = FindClassVarType(firstName);
                        if (FindClass(z) == -1){
                            pi.er = undecIdentifier;
                            pi.tk = prev;
                            return pi;
                        } else{
                            char location[100] = "";
                            strcpy(output, "");
                            strcpy(location,FindMemoryPosition(firstName));
                            strcat(output,"push ");
                            strcat(output, location);
                            if (location[0] == 't' && pop_point != 0){
                                pop_point = 0;
                                strcpy(code_list[code_list_count++], "push argument 0\n");
                                strcpy(code_list[code_list_count++], "pop pointer 0\n");
                            }
                            strcat(output,"\n");
                            strcpy(code_list[code_list_count++],output);
                            strcpy(output, "");

                            strcpy(firstName,z);
                        }
                    } else{
                        char location[100] = "";
                        strcpy(output, "");
                        strcpy(location,FindMemoryPosition(firstName));
                        strcat(output,"push ");
                        strcat(output, location);
                        if (location[0] == 't' && pop_point != 0){
                            pop_point = 0;
                            strcpy(code_list[code_list_count++], "push argument 0\n");
                            strcpy(code_list[code_list_count++], "pop pointer 0\n");
                        }
                        strcat(output,"\n");
                        strcpy(code_list[code_list_count++],output);
                        strcpy(output, "");

                        strcpy(firstName,y);
                    }
                }
            }

            GetNextToken(); // .
            t = GetNextToken(); // id expected

            if (t.tp == ID){
                prev = t;
                strcpy(secondName, t.lx);

            } else{
                pi.er = idExpected;
                pi.tk = t;
                return pi;
            }
        } else{
            strcpy(firstName,classname);
            strcpy(secondName, prev.lx);
        }


        t = PeekNextToken(); // either [ or ( or smtg else

        if (!strcmp(t.lx, "[")){

            GetNextToken(); // [

            if (quickscan == 1){ // Only class or method variable can have []
                int x = FindMethodVar(secondName);
                if (x == -1){
                    int y = FindClassVar(secondName);
                    if (y == -1){
                        pi.er = undecIdentifier;
                        pi.tk = prev;
                        return pi;
                    }
                }
            }

            pi = expression();
            if (pi.er != none){
                return pi;
            }
            t = GetNextToken();
            if (!strcmp(t.lx, "]")){
                ;
            } else{
                pi.er = closeBracketExpected;
                pi.tk = t;
                return pi;
            }


            char location[100];
            strcpy(location, FindMemoryPosition(secondName));
            if (location[0] == 't' && pop_point != 0){
                pop_point = 0;
                strcpy(code_list[code_list_count++], "push argument 0\n");
                strcpy(code_list[code_list_count++], "pop pointer 0\n");
            }
            strcat(output,"push ");
            strcat(output, location);
            strcat(output, "\n");
            strcpy(code_list[code_list_count++], output);
            strcpy(code_list[code_list_count++], "add\n");
            strcpy(code_list[code_list_count++], "pop pointer 1\n");
            pop_point = 1;
            strcpy(code_list[code_list_count++], "push that 0\n");

        } else if (!strcmp(t.lx, "(")){

            if (isRecursive == 1){
                isRecursive = 3;
                previous_expressionList_count = expressionList_count;
            }

            GetNextToken(); // (

            if (quickscan == 1){ // Only methods can have (), in this case its methods within this class

                int x = FindMethod(firstName,secondName);
                if (x == -1){
                    pi.er = undecIdentifier;
                    pi.tk = prev;
                    return pi;

                }
               callIsMethod = FindIsMethod(firstName,secondName);

            }


            t = PeekNextToken();

            if (strcmp(t.lx, ")")){
                expressionList_count = 1;
                isRecursive = 1;
                pi = expressionList();
                if (isRecursive == 3){
                    expressionList_count = previous_expressionList_count;
                }
                isRecursive = 0;

                if (pi.er != none){
                    return pi;
                }
            } else{
                expressionList_count = 0;
            }



            t = GetNextToken(); // )

            if (!strcmp(t.lx, ")")){
                ;
            } else{
                pi.er = closeParenExpected;
                pi.tk = t;
                return pi;
            }


            char expressionList_count_str[100];
            sprintf(expressionList_count_str, "%d", expressionList_count + callIsMethod);
            strcat(output, "call ");
            strcat(output, firstName);
            strcat(output, ".");
            strcat(output, secondName);
            strcat(output, " ");
            strcat(output, expressionList_count_str);
            strcat(output, "\n");
            strcpy(code_list[code_list_count++],output);


        } else{
            if (quickscan == 1){ // Only methods can have (), in this case its methods within this class
                int x = FindMethodVar(secondName);
                if (x == -1){
                    int y = FindClassVar(secondName);
                    if (y == -1) {
                        pi.er = undecIdentifier;
                        pi.tk = prev;
                        return pi;
                    }
                }
            }

            char location[100]; // Check here to check for this & that
            strcpy(location, FindMemoryPosition(secondName));
            if (location[0] == 't' && pop_point != 0){
                pop_point = 0;
                strcpy(code_list[code_list_count++], "push argument 0\n");
                strcpy(code_list[code_list_count++], "pop pointer 0\n");
            }
            strcat(output,"push ");
            strcat(output, location);
            strcat(output, "\n");
            strcpy(code_list[code_list_count++], output);

            return pi;
        }

        return pi;

    } else{
        pi.er  = syntaxError;
        pi.tk = t;
    }

    return pi;

}


int InitParser (char* file_name)
{
    InitLexer(file_name);
    strcpy(directory,file_name);
    directory[strlen(directory) - 5] = '\0';
    strcat(directory,".vm");
    return 1;
}

ParserInfo Parse ()
{
    ParserInfo pi;
    pi.er = none;
    classfield_count = 0;

    // implement the function
    pi.er = none;

    Token t = PeekNextToken();
    if (t.tp == RESWORD && !strcmp(t.lx, "class")){
        pi = classDeclar();
    } else if (t.tp == EOFile){
        return pi;
    } else if (t.tp == ERR){
        pi.er = lexerErr;
        pi.tk = t;
    }
    else{
        pi.er = classExpected;
        pi.tk = t;
    }
    return pi;
}

int ClearCodeList(){
    for (int i = 0; i < code_list_count; i++) {
        strcpy(code_list[i], "");
    }
    code_list_count = 1;
    localvar_count = 0;
    return 1;

}

int WriteVM(){

    FILE *file_ptr = fopen(directory, "a");
    for (int i = 0; i < code_list_count; i++) {
        fprintf(file_ptr, "%s", code_list[i]);
    }

    fclose(file_ptr);
    return 1;
}


int StopParser ()
{
    ClearCodeList();
     paren = 0;

     classfield_count = 0;
     while_count = 0;
     if_count = 0;
     pop_point = 3;

     code_list_count = 1;

     localvar_count = 0;

     expressionList_count = 0;

     isRecursive = 0;
     previous_expressionList_count = 0;

     isMethod = 0;
     initial_pointer = 0;
     files = 0;
     quickscan = 0;

    return 1;
}

#ifndef TEST_PARSER
//int main ()
//{
//    InitParser("text.jack");
//    ParserInfo pi = Parse();
//    printf("%d\n", pi.er);
//    return 1;
//}
#endif



