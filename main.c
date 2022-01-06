#include <stdio.h> // printf
#include <stdlib.h> // malloc

#include <conio.h>

//========== DEFINES ===========

// DATATYPES
#define DATATYPE_NONE 0
#define DATATYPE_LONG 1
#define DATATYPE_INT 2
#define DATATYPE_SHORT 3
#define DATATYPE_CHAR 4
#define DATATYPE_FLOAT 5
#define DATATYPE_DOUBLE 6
#define DATATYPE_VOID 7

// TYPE

#define TYPE_NONE 0
#define TYPE_VAR 1
#define TYPE_CALL_FUNC 2
#define TYPE_MFP 3
#define TYPE_IF 4
#define TYPE_FOR 5
#define TYPE_WHILE 6

// uint
#define uint unsigned int

//============= FUNC ===============

// length string
unsigned int lenstr(char *str){
    register unsigned int len=0;
    while(1){
        if(str[len]==0){ return len; }
        len++;
    }
}
// bool
char cmpstr(char *str, char *find){
    for(unsigned int i=0; i<lenstr(find); i++){
        if(str[i]==0){ return 0; }
        if(str[i]!=find[i]){ return 0; }
    }
}
// char* ( MALLOC!!! )
char *copystrAdr(char *start, char *end){
    unsigned int len = end-start;
    char *buf = malloc(len+1);
    for(unsigned int i=0; i<len; i++){
        buf[i]=start[i];
        buf[i+1]=0;
    }
    return buf;
}
char *copystr(char *start, unsigned int len){
    if(len == 0){
        len = lenstr(start);
    }
    char *buf = malloc(len+1);
    for(unsigned int i=0; i<len; i++){
        buf[i]=start[i];
        buf[i+1]=0;
    }
    return buf;
}
// CLEAR STRUCT/ARRAY
void cleararr(char *arr, unsigned int len){
    for(unsigned int i=0; i<len; i++){
        arr[i]=0;
    }
}

// PRINT ERROR
void error(char *code, char *errorText){
    printf("ERROR: %s\n", errorText);
    code = copystr(code, 0);
    code[10]='\0';
    printf("%s\n", code);
    exit(1);
}

// =========== STRUCT ===============

struct command{
        unsigned char type; // type command
        /*
            TYPE can be:
                0 - NONE
                1 - variable
                2 - call func/proc
                3 - macro/func/proc with { }
                4 - if
                5 - for
                6 - while
                ... - NONE
        */

        // type data
        unsigned char LONG; // bool
        unsigned char REGISTER; // bool
        unsigned char UNSIGNED; // bool
        unsigned char STATIC; // bool
        unsigned char EXTERN; // bool

        unsigned char datatype; // type data
        /*
            DATATYPE can be:
                0 - NONE
                1 - long
                2 - int
                3 - short
                4 - char
                5 - float
                6 - double
                7 - void
                ... - NONE
        */

        char *name; // name command/variable

        char *args; // arguments command

        struct command *dopCommand; // for macro/func/proc with { }
};

// ============= PARSER =============

struct command *parser(char *code, unsigned int *count, struct command *tmpcom){
    // code - string with code
    // count - address on variable, returns the number of cells in an array
    // the function returns an array of 'command' structures
    //struct command tmpcom;
    cleararr(tmpcom, sizeof(tmpcom));

    uint len = lenstr(code);

    // получаем тип данных (get datatype)
    if( cmpstr(code, "unsigned ") ){ tmpcom->UNSIGNED   = 1; code+=9; }
    if( cmpstr(code, "register ") ){ tmpcom->REGISTER   = 1; code+=9; }
    if( cmpstr(code, "long ") )    { tmpcom->LONG       = 1; code+=5; }
    if( cmpstr(code, "static ") )  { tmpcom->STATIC     = 1; code+=7; }
    if( cmpstr(code, "extern ") )  { tmpcom->EXTERN     = 1; code+=7; }

    if( cmpstr(code, "long ") )         { tmpcom->datatype = DATATYPE_LONG;     code+=5; } else
        if( cmpstr(code, "int ") )      { tmpcom->datatype = DATATYPE_INT;      code+=4; } else
        if( cmpstr(code, "short ") )    { tmpcom->datatype = DATATYPE_SHORT;    code+=6; } else
        if( cmpstr(code, "char ") )     { tmpcom->datatype = DATATYPE_CHAR;     code+=5; } else
        if( cmpstr(code, "float ") )    { tmpcom->datatype = DATATYPE_FLOAT;    code+=6; } else
        if( cmpstr(code, "double ") )   { tmpcom->datatype = DATATYPE_DOUBLE;   code+=7; } else
        if( cmpstr(code, "void ") )     { tmpcom->datatype = DATATYPE_VOID;     code+=5; }
    else { tmpcom->datatype = DATATYPE_NONE; }


    // получаем имя (get name)
    if( '0' <= code[0] && code[0] <= '9' ){ error( code, "You cannot create this name." ); }

    uint i = 0;
    while(1){
        if( code[i] == 0 ){ break; }
        if( (code[i] >= '0' && code[i] <= '9') || (code[i] >= 'a' && code[i] <= 'z') || (code[i] >= 'A' && code[i] <= 'Z') )
            { i++; } else { break; }
    }

    tmpcom->name = copystr(code, i); // save name
    code+=i;

    // получаем тип команды (get type command)
    i=0;
    while(1){
        if( code[i]==0 || code[i]!=' ' ){ break; }
        i++;
    }
    code+=i;

    if( code[0]!=0 ){
        if( code[0]=='=' ){
            tmpcom->type = TYPE_VAR;
        }
        if(code[0]=='(' || code[0]==')'){
            if(cmpstr(tmpcom->name, "if")){
                tmpcom->type = TYPE_IF;
            } else if(cmpstr(tmpcom->name, "for")){
                tmpcom->type = TYPE_FOR;
            } else if(cmpstr(tmpcom->name, "while")){
                tmpcom->type = TYPE_WHILE;
            } else {
                tmpcom->type = TYPE_CALL_FUNC;
            }
        }
    }

    // получаем аргументы (get arguments)
    i=0;
    while(1){
        if( code[i]==0 || code[i]!=' ' ){ break; }
        i++;
    }
    code+=i;

    if(tmpcom->type == TYPE_VAR){
        if(code[0]=='='){
            tmpcom->args = code; // tmp
            code++;

            i=0; // игнор пробела (ignore "space")
            while(1){
                if( code[i]==0 || code[i]!=' ' ){ break; }
                i++;
            }
            code+=i;

            if(code[0]==';'){
                tmpcom->args = "0";
            }else{
                i=0;
                while(1){
                    if( code[i]==0 || code[i]==';' || code[i]!=' ' ){ break; }
                    i++;
                }
                code+=i;

                char tmpbool = 0;
                i=0;
                while(1){
                    if( code[i]==0 || code[i]==';' ){ break; }
                    if( code[i]>='0' && code[i]<='9' ){ i++; } else { tmpbool = 1; break; }
                }

                if(tmpbool==0){
                    printf("IS NUM\n\n");
                    tmpcom->args = copystrAdr(tmpcom->args, code+i);
                } else {
                    printf("IS NOT NUM\n\n");
                    tmpcom->args = "=";
                    tmpcom->dopCommand = // THIS
                }



            }
        }
    }

    return tmpcom;
}

int main(){
    unsigned int count;
    struct command tmp;
    parser("unsigned int abc = foo();", &count, &tmp);
    printf("NAME: \"%s\"\nTYPE: %d\nARGS: \"%s\"\n", tmp.name, tmp.type, tmp.args);
    return 0;
}
