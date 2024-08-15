#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

const char* keywords[21] = { "class" , "constructor" , "method", "function" , "int" , "boolean" , "char" , "void" , "var" , "static" , "field", "let", "do" , "if" , "else", "while","return" , "true" , "false", "null","this"};
const char* symbols[19] = { "(" , ")" , "[", "]" , "{" , "}" , "," , ";" , "=" , "." , "+", "-", "*" , "/" , "&", "|","~" , "<" , ">"};
int size;
int *charArray;
Token t;
int charPointer;
int LineCount;
int charCount;
int TokenReady;
FILE* file;


/* Skips through & ignores all whitespace / comments in the file
 * ---------------------------------
 * Scans through each character in the file to check for comments, spaces or linebreaks
 * ---------------------------------
 * Args: None
 * ---------------------------------
 * Returns 1 or 0
 * 0: The process of eating whitespace/comments has failed, ie: EOF in Comment
 * 1: The process of eating whitespace/comments is successful
 */
int testStar(){
    while (charArray[charPointer] != '*') {

        if (charArray[charPointer] == '\n'){
            LineCount++;
        }
        charPointer++;

        if (charPointer >= charCount){
            return 0;
        }
    }

    charPointer++;

    if (charPointer >= charCount){
        return 0;
    }

    if (charArray[charPointer] == '/'){
        charPointer++;
        return 1;
    } else {
        return testStar();
    }

};



/* Skips through & ignores all whitespace / comments in the file
 * ---------------------------------
 * Scans through each character in the file to check for comments, spaces or linebreaks
 * ---------------------------------
 * Args: None
 * ---------------------------------
 * Returns 1 or 0
 * 0: The process of eating whitespace/comments has failed, ie: EOF in Comment
 * 1: The process of eating whitespace/comments is successful
 */
int EatWC(){

    int c;
    c = charArray[charPointer];

    while (isspace(c)){
        if ((char) c == '\n'){
            LineCount++;
        }

        charPointer++;

        if (charPointer >= charCount){
            return 1;
        }

        c = charArray[charPointer];
    }

    if (c == '/'){
        charPointer++;

        if (charArray[charPointer] == '/'){

            charPointer++;

            while (charArray[charPointer] != '\n') {
                charPointer++;
            }
            LineCount++;
            charPointer++;
            return EatWC();
        }
        else if (charArray[charPointer] == '*'){
            charPointer++;
            if (testStar()){
                return EatWC();
            }
            else{
                return 0;
            }

        }
        else{
            charPointer--;
            return 1;
        }
    } else{
        return 1;
    }

}



/* Checks whether the string is of a keyword in the JACK language
 * ---------------------------------
 * Scans through all the defined keywords & compare with the string defined
 * ---------------------------------
 * Args: str
 * sym: A string passed in
 * ---------------------------------
 * Returns 1 or 0
 * 0: The string is a keyword
 * 1: The string is not a keyword
 */
int IsKeyWord(char* str)
{
    for (int i = 0; i < 21; i++)
        if (!strcmp(keywords[i], str))
            return 1;
    return 0;
}



/* Checks whether the character is of a legal symbol in the JACK language
 * ---------------------------------
 * Scans through all the legal symbols defined
 * ---------------------------------
 * Args: sym
 * sym: A character passed in
 * ---------------------------------
 * Returns 1 or 0
 * 0: It is a legal symbol
 * 1: It is an illegal symbol
 */
int IsSymbol(char sym)
{
    for (int i = 0; i < 19; i++){
        if (sym == *symbols[i])
            return 1;
    }
    return 0;
}



/* Builds the next token in the file
 * ---------------------------------
 * Scans through the next lexeme and classifies it into its type
 * Also updating the line number
 * ---------------------------------
 * Args: None
 * ---------------------------------
 * Returns t
 * t: A Token class object
 */
Token BuildToken(){
    if (EatWC() == 0){
        t.tp = ERR;
        t.ec = EofInCom;
        t.ln = LineCount;
        strcpy(t.lx, "Error: unexpected eof in comment");
        return t;
    }

    if (charPointer >= charCount){
        t.ln = LineCount;
        t.ec = NoLexErr;
        t.tp = EOFile;
        strcpy(t.lx, "End Of File");
        return t;
    }

    char temp[128];
    int i = 0;
    int c = charArray[charPointer];

    if (isalpha(c) || c == '_'){

        while(isalpha(c)|| isdigit(c)||c == '_'){
            temp[i++] = c;
            charPointer++;

            if (charPointer >= charCount){
                break;
            }
            c = charArray[charPointer];
        }

        temp[i] = '\0';
        strcpy(t.lx, temp);

        if (IsKeyWord(temp)){
            t.tp = RESWORD;
        }
        else{
            t.tp = ID;
        }

        t.ln = LineCount;
        t.ec = NoLexErr;
        return t;
    }
    else if (isdigit(c)){

        while(isdigit(c)) {
            temp[i++] = c;
            charPointer++;

            if (charPointer >= charCount) {
                break;
            }
            c = charArray[charPointer];
        }

        temp[i] = '\0';
        strcpy(t.lx, temp);
        t.tp = INT;
        t.ec = NoLexErr;
        t.ln = LineCount;
        return t;
    }
    else if (c == '"'){
        charPointer++;
        c = charArray[charPointer];

        while (c != '"') {
            if (charArray[charPointer] == '\n'){
                t.tp = ERR;
                t.ec = NewLnInStr;
                // Line number need to increment?
                t.ln = LineCount;
                strcpy(t.lx, "Error: new line in string constant");
                return t;
            }

            temp[i++] = c;
            charPointer++;

            if (charPointer >= charCount) {
                t.tp = ERR;
                t.ec = EofInStr;
                t.ln = LineCount;
                strcpy(t.lx, "Error: unexpected eof in string constant");
                return t;
            }
            c = charArray[charPointer];
        }

        temp[i] = '\0';
        charPointer++;
        strcpy(t.lx, temp);
        t.tp = STRING;
        t.ln = LineCount;
        t.ec = NoLexErr;
        return t;

    }
    else{
        charPointer++;

        if (IsSymbol(c)){
            t.tp = SYMBOL;
            temp[0] = c;
            temp[1] = '\0';
            strcpy(t.lx, temp);
            t.ec = NoLexErr;
            t.ln = LineCount;
            return t;
        }
        else{
            t.tp = ERR;
            t.ec = IllSym;
            strcpy(t.lx, "Error: illegal symbol in source file");
            t.ln = LineCount;
            return t;
        }
    }

}



/* Initializer for the Lexer
 * ---------------------------------
 * Opens the file to process & store each character into an array
 * ---------------------------------
 * Args: file_name : The name of the file being processed
 * ---------------------------------
 * Returns 0 or 1
 * 0: File opening fail
 * 1: Initializing process successful
 */
int InitLexer (char* file_name)
{
    charPointer = 0;
    LineCount = 1;
    TokenReady = 0;
    size = 1000;
    strcpy(t.fl, file_name);
    charArray = (int *)malloc(size * sizeof(int));
    charCount = 0;
    file = fopen(file_name, "r");

    if (file == 0) {
        printf("File opening failed!");
        return 0;
    }

    int ch;
    while ((ch = fgetc(file)) != EOF) {
        charArray[charCount++] = (char)ch;

        if (charCount/size >= 0.75){
            size = size + 1000;
            charArray = (int *)realloc(charArray, size * sizeof(int));
        }
    }

    return 1;
}



/* Get the next token from the source file
 * ---------------------------------
 * Also removes the token from the stream
 * ---------------------------------
 * Args: No args
 * ---------------------------------
 * Returns t
 * t: A Token class object
 */
Token GetNextToken ()
{
    if (TokenReady)
    {
        TokenReady = 0;
        return t;
    }

    t = BuildToken();
    TokenReady = 0;
    return t;
}



/* peek (look) at the next token in the source file without removing it from the stream
 * ---------------------------------
 * Args: No args
 * ---------------------------------
 * Returns t
 * t: A Token class object
 */
Token PeekNextToken ()
{
    if (TokenReady)
        return t;
    t = BuildToken();
    TokenReady = 1;
    return t;
}



/* clean out at end, e.g. close files, free memory, ... etc
 * ---------------------------------
 * Args: No args
 * ---------------------------------
 * Returns 0
 * 0: Successfully cleaned out
 */
int StopLexer ()
{
    fclose(file);
    free(charArray);
    return 0;
}




#ifndef TEST

#endif
