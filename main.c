#include <stdio.h> // printf
#include <stdlib.h> // malloc

#include <conio.h> // getch

//========== DEFINES ===========

// DATATYPES
#define DATATYPE_NONE       0
#define DATATYPE_CHAR       1
#define DATATYPE_SHORT      2
#define DATATYPE_INT        3
#define DATATYPE_FLOAT      4
#define DATATYPE_DOUBLE     5
#define DATATYPE_VOID       6

// TYPE
#define TYPE_NONE           0
#define TYPE_NEW_VAR        1
#define TYPE_VAR            2
#define TYPE_FUNC_CALL      3
#define TYPE_IF             4
#define TYPE_ELSE           5
#define TYPE_FOR            6
#define TYPE_WHILE          7
#define TYPE_START_DO_WHILE 8
#define TYPE_END_DO_WHILE   9
#define TYPE_NEW_FUNC       10
#define TYPE_START_BRACE    11
#define TYPE_END_BRACE      12

#define TYPE_STRING         13
#define TYPE_NUMBER         14
#define TYPE_VOID           15
#define TYPE_MATH           16
#define TYPE_TRANSFORM      17

// uint, true, false, null, bool
#define uint    unsigned int
#define true    1
#define false   0
#define null    0
#define bool    char

// DEBUG

#define DEBUG

// ============= FUNC ===============

// length string
unsigned int lenstr(char *str){
    register unsigned int len=0;
    while(1){
        if(str[len]==0){ return len; }
        len++;
    }
}
// bool
bool cmpstr(char *str, char *find){
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

struct data{
    bool UNSIGNED;  // bool
    bool REGISTER;  // bool
    bool LONG;      // bool
    bool STATIC;    // bool
    bool EXTERN;    // bool

    char datatype;  // int
    /*
    TYPE:
        0 - NONE
        1 - CHAR
        2 - SHORT
        3 - INT
        4 - FLOAT
        5 - DOUBLE
        6 - VOID
    */
};

struct command{
        char type;
        /*
        TYPE:
            0  - NONE
            1  - NEW VAR (=)
            2  - VAR (=)
            3  - FUNC. CALL
            4  - IF ( args )
            5  - ELSE
            6  - FOR( com; args; com)
            7  - while( args )
            8  - do...
            9  - ...while( args )
            10 - NEW FUNC.
            11 - { (start)
            12 - (end) }

            13 - string
            14 - number
            15 - void
            16 - ariphmetic (a+b*c)
            17 - transform ( (int)(abc) )
        */

        struct data datatype;

        char *name; // if "if" than this condition

        unsigned int args; // id ^^^
        /*
            0 - var = 0
            1 - var = (next command)
        */

        char *dopArgs;
};

// ============= PARSER =============

void parser(char *code){
    struct command tmpcom;
    cleararr( &tmpcom, sizeof(tmpcom) );

    // DATATYPE
    if( cmpstr( code, "unsigned " ) ){ tmpcom.datatype.UNSIGNED  = true; code += 9; }
    if( cmpstr( code, "register " ) ){ tmpcom.datatype.REGISTER  = true; code += 9; }
    if( cmpstr( code, "long "     ) ){ tmpcom.datatype.LONG      = true; code += 5; }
    if( cmpstr( code, "static "   ) ){ tmpcom.datatype.STATIC    = true; code += 7; }
    if( cmpstr( code, "extern "   ) ){ tmpcom.datatype.EXTERN    = true; code += 7; }

    if( cmpstr( code, "char "  ) ){ tmpcom.datatype.datatype = DATATYPE_CHAR;   code += 5; } else
    if( cmpstr( code, "short " ) ){ tmpcom.datatype.datatype = DATATYPE_SHORT;  code += 6; } else
    if( cmpstr( code, "int "   ) ){ tmpcom.datatype.datatype = DATATYPE_INT;    code += 4; } else
    if( cmpstr( code, "float " ) ){ tmpcom.datatype.datatype = DATATYPE_FLOAT;  code += 6; } else
    if( cmpstr( code, "double ") ){ tmpcom.datatype.datatype = DATATYPE_DOUBLE; code += 7; } else
    if( cmpstr( code, "void "  ) ){ tmpcom.datatype.datatype = DATATYPE_VOID;   code += 5; } else {
        tmpcom.datatype.datatype = DATATYPE_NONE;
    }

    // NAME
    uint i = 0;
    while(1){
        if( code[i] == 0 ){ break; }
        if( ('0'<=code[i] && code[i]<='9') || ('A'<=code[i] && code[i]<='Z')
           || ('a'<=code[i] && code[i]<='z') || code[i]=='_' ){ i++; } else { break; }
    }

    tmpcom.name = copystr(code, i);
    code+=i;

    #ifdef DEBUG
    printf("\tDEBUG:\n\t\tUNSIGNED: %d\n\t\tREGISTER: %d\n\t\tLONG: %d\n\t\tSTATIC: %d\n\t\tEXTERN: %d\n\n", (int)(tmpcom.datatype.UNSIGNED), (int)(tmpcom.datatype.REGISTER), (int)(tmpcom.datatype.LONG), (int)(tmpcom.datatype.STATIC), (int)(tmpcom.datatype.EXTERN));
    printf("\t\tNAME: \'%s\'\n", tmpcom.name);
    #endif // DEBUG

    i = 0;
    while(1){
        if( code[i] == 0 ){ break; }
        if( code[i] == ' ' || code[i] < '!' ){ i++; } else { break; }
    }
    code+=i;

    if( code[0]=='=' || code[0]==';' ){ // THIS 'NEW VAR' or 'VAR'
        if( tmpcom.datatype.UNSIGNED || tmpcom.datatype.STATIC || tmpcom.datatype.REGISTER || tmpcom.datatype.LONG || tmpcom.datatype.EXTERN || tmpcom.datatype.datatype ){
            tmpcom.type = TYPE_NEW_VAR;
        } else {
            tmpcom.type = TYPE_VAR;
        }
        if( code[0] == ';' ){ tmpcom.args=0; } else
        if( code[0] == '=' ){

            code++;
            i = 0;
            while(1){
                if( code[i] == 0 ){ break; }
                if( code[i] == ' ' || code[i] < '!' ){ i++; } else { break; }
            }
            code+=i;
            if( code[0] == ';' ){ tmpcom.args=0; goto IF_EXIT1; }

            uint i = 0;
            while(1){
                if( code[i] == 0 || code[i] == ';' || code[i] == '}' ){ break; }
                if( ('0'<=code[i] && code[i]<='9') || ('A'<=code[i] && code[i]<='Z')
                    || ('a'<=code[i] && code[i]<='z') || code[i]=='_' ){ i++; } else {
                        goto COM1;
                        break;
                    }
            }
            COM1:
                // THIS, нада прочитать знач, и понять это функция или число

        }
    }
    IF_EXIT1:

}

int main(){

    parser("unsigned int abc    =10;");

    return 0;
}
