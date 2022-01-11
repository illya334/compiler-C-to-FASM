#include <stdio.h> // printf
#include <stdlib.h> // malloc

#include <conio.h> // getch

//========== DEFINES ===========

// DATATYPES
#define DATATYPE_NONE       0
#define DATATYPE_CHAR       1
#define DATATYPE_SHORT      2
#define DATATYPE_INT        3
#define DATATYPE_LONG       4
#define DATATYPE_FLOAT      5
#define DATATYPE_DOUBLE     6
#define DATATYPE_VOID       7

// TYPE
#define TYPE_NONE           0
/* TYPE_NONE:
    по идеи это пустая структура или произошла ощибка
*/
#define TYPE_NEW_VAR        1
#define TYPE_VAR            2
/* TYPE_VAR and TYPE_NEW_VAR:
    DATATYPE - только в TYPE_NEW_VAR
    *name - имя переменой
*/
#define TYPE_FUNC_CALL      3  // НЕ РЕАЛЕЗОВАНО
/* TYPE_FUNC_CALL:
        *name - имя
        args - количество аргументов
        *dopargs - масив указателей на масивы с структурой command
*/
#define TYPE_IF             4  // НЕ РЕАЛЕЗОВАНО
/* TYPE_IF:
    *name = аргументы в виде строки
    args - id

    берет следующую команду
 */
#define TYPE_ELSE           5  // НЕ РЕАЛЕЗОВАНО
/* TYPE_ELSE:
    args - id

    берет следующую команду
*/
#define TYPE_FOR            6  // НЕ РЕАЛЕЗОВАНО
/* TYPE_FOR:
    *name - условие
    *dopargs - масив указателей на масивы с структурами command

    берет следующую команду
*/
#define TYPE_WHILE          7  // НЕ РЕАЛЕЗОВАНО
/* TYPE_WHILE
    *name - условие

    берет следующую команду
*/
#define TYPE_START_DO_WHILE 8  // НЕ РЕАЛЕЗОВАНО
/* TYPE_START_DO_WHILE:
    args - id
    берет следующую команду
*/
#define TYPE_END_DO_WHILE   9  // НЕ РЕАЛЕЗОВАНО
/* TYPE_END_DO_WHILE:
    *name - условие
    args - id

    берет следующую команду
*/
#define TYPE_NEW_FUNC       10 // НЕ РЕАЛЕЗОВАНО
/* TYPE_NEW_FUNC:
        DATATYPE
        *name - имя функции
        *dopargs - аргументы в виде строки

        берет следующую команду
*/
#define TYPE_START_BRACE    11 // НЕ РЕАЛЕЗОВАНО
/* TYPE_START_BRACE:
    args - id
    берет следующие команды
*/
#define TYPE_END_BRACE      12 // НЕ РЕАЛЕЗОВАНО
/* TYPE_END_BRACE:
    args - id
*/

#define TYPE_STRING         13
#define TYPE_NUMBER         14
#define TYPE_VOID           15 // 50% - не строгать
#define TYPE_MATH           16 // НЕ РЕАЛЕЗОВАНО
#define TYPE_TRANSFORM      17 // НЕ РЕАЛЕЗОВАНО
/* TYPE_TRANSFORM:
        DATATYPE
        name - адрес на масив структур
        args - число структур
*/

#define TYPE_EQU            18
#define TYPE_POINT          19 // НЕ РЕАЛЕЗОВАНО

// uint, true, false, null, bool
#define uint    unsigned int
#define true    1
#define false   0
#define null    0
#define bool    char

// DEBUG

//#define DEBUG_PARSER

// ============= VAR ================

char *code = "    ";
char *EndCode;

unsigned int lines = 0;

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
    printf("%d | %s\n", lines, code);
    exit(1);
}

// IGNORE SPACE
unsigned int IgnoreSpace(char *code){
    unsigned int i = 0;
    while(1){
        if( code[i] == 0 ){ break; }
        if( code[i] == '\n' ){ lines++; }
        if( code[i] == ' ' || code[i] < '!' ){ i++; } else { break; }
    }
    return i;
}

// =========== STRUCT ===============

struct data{
    bool UNSIGNED;  // bool
    bool REGISTER;  // bool

    char datatype;  // int
    /*
    TYPE:
        0 - NONE
        1 - CHAR
        2 - SHORT
        3 - INT
        4 - LONG
        5 - FLOAT
        6 - DOUBLE
        7 - VOID
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

            18 - = (equ)
            19 - point
        */

        struct data datatype;

        char *name; // if "if" than this condition

        unsigned int args; // id ^^^

        char *dopArgs;
};

// ============= PARSER =============

struct command *tmpcom = 0;

struct command *parser(){
    // code - 'string' code
    // struct command *tmpcom - THIS NULL

    if( tmpcom==0 ){ tmpcom = malloc( sizeof( struct command ) ); }
    cleararr( tmpcom, sizeof(tmpcom) );
    tmpcom->name = 0;

    uint i = 0;

    // ========= CODE =========

    i = IgnoreSpace(code);
    if( code+i >= EndCode ){ return 0; }
    code+=i;

    if( code[0]==';' ){ return 0; }

    if( code[0]=='=' ){
        code++;
        tmpcom->type = TYPE_EQU;
        tmpcom->name = "="; // ...
        goto ExitParser;
    }

    if( ('0'<=code[0] && code[0]<='9') || code[0] == '\'' ){ // IS NUMBER?

        if( code[0]=='0' && code[1]=='x' ){ // HEX

            tmpcom->type = TYPE_NUMBER;
            i = 2;
            while(1){
                if( code+i > EndCode ){ return 0; }
                if( code[i] == 0 ){ break; }
                if( ('0'<=code[i] && code[i]<='9') || (code[i] == 'A' || code[i] == 'B' || code[i] == 'C' || code[i] == 'D' || code[i] == 'E' || code[i] == 'F'
                   || code[i] == 'a' || code[i] == 'b' || code[i] == 'c' || code[i] == 'd' || code[i] == 'e' || code[i] == 'f') ){ i++; } else { break; }
            }
            tmpcom->name = copystr(code, i);
            code+=i;
            goto ExitParser;

        } else if( code[0] == '\'' ){ // CHAR

            tmpcom->type = TYPE_NUMBER;
            i = 1;
            while(1){
                if( code+i > EndCode ){ return 0; }
                if( code[i] == '\\' && code[i+1] == '\'' ){ i+=2; }
                if( code[i] == 0 ){ error(code, "where \' ?"); }
                if( code[i] == '\'' ){ break; }
                i++;
            }
            i++;
            tmpcom->name = copystr(code, i);
            code+=i;
            goto ExitParser;

        } else { // SINGLE NUMBER

            tmpcom->type = TYPE_NUMBER;
            i = 0;
            while(1){
                if( code+i > EndCode ){ return 0; }
                if( code[i] == 0 ){ break; }
                if( ('0'<=code[i] && code[i]<='9') ){ i++; } else { break; }
            }
            if( code[i] == 'h' ){ i++; } // or HEX
            tmpcom->name = copystr(code, i);
            code+=i;
            goto ExitParser;
        }
    }

    if( code[0]=='\"' ){
        tmpcom->type = TYPE_STRING;
        i=1;
        while(1){
            if( code+i > EndCode ){ return 0; }
            if( code[i] == 0 || code[i]=='\"' ){ break; }
            i++;
        }
        tmpcom->name = copystr(code, ++i);
        code+=i;
        goto ExitParser;
    }

    // DATATYPE
    while(1){
        i = IgnoreSpace(code);
        if( code+i>=EndCode ){ return 0; }
        code+=i;

        if( code > EndCode ){ return 0; }

        if( cmpstr( code, "unsigned " ) ){ tmpcom->datatype.UNSIGNED  = true; code += 9; } else
        if( cmpstr( code, "register " ) ){ tmpcom->datatype.REGISTER  = true; code += 9; } else {
            break;
        }
    }

    while(1){
        i = IgnoreSpace(code);
        if( code+i>=EndCode ){ return 0; }
        code+=i;

        if( code > EndCode ){ return 0; }

        if( cmpstr( code, "char "  ) ){ tmpcom->datatype.datatype = DATATYPE_CHAR;   code += 5; } else
        if( cmpstr( code, "short " ) ){ tmpcom->datatype.datatype = DATATYPE_SHORT;  code += 6; } else
        if( cmpstr( code, "int "   ) ){ tmpcom->datatype.datatype = DATATYPE_INT;    code += 4; } else
        if( cmpstr( code, "long "  ) ){ tmpcom->datatype.datatype = DATATYPE_LONG;   code += 5; } else
        if( cmpstr( code, "float " ) ){ tmpcom->datatype.datatype = DATATYPE_FLOAT;  code += 6; } else
        if( cmpstr( code, "double ") ){ tmpcom->datatype.datatype = DATATYPE_DOUBLE; code += 7; } else
        if( cmpstr( code, "void "  ) ){ tmpcom->datatype.datatype = DATATYPE_VOID;   code += 5; } else {
            if(tmpcom->datatype.datatype == 0){
                tmpcom->datatype.datatype = DATATYPE_NONE;
            }
            break;
        }
    }

    i = IgnoreSpace(code);
    if( code+i>=EndCode ){ return 0; }
    code+=i;

    if( code > EndCode ){ return 0; }

    // NAME
    i = 0;
    while(1){
        if( code[i] == 0 ){ break; }
        if( code+i > EndCode ){ return 0; }
        if( ('0'<=code[i] && code[i]<='9') || ('A'<=code[i] && code[i]<='Z')
           || ('a'<=code[i] && code[i]<='z') || code[i]=='_' || code[i]=='*' || code[i]=='&' ){ i++; } else { break; }
    }

    tmpcom->name = copystr(code, i);
    code+=i;

    if( code > EndCode ){ return 0; }

    #ifdef DEBUG_PARSER
    printf("\tDEBUG:\n\t\tUNSIGNED: %d\n\t\tREGISTER: %d\n\n", (int)(tmpcom->datatype.UNSIGNED), (int)(tmpcom->datatype.REGISTER) );
    printf("\t\tDATATYPE: %d\n\t\tNAME: \'%s\'\n", tmpcom->datatype.datatype, tmpcom->name);
    #endif // DEBUG_PARSER

    i = IgnoreSpace(code);
    if( code+i>=EndCode ){ return 0; }
    code+=i;

    if( code[0] == '=' || code[0] == ';' ){
        if( tmpcom->datatype.REGISTER || tmpcom->datatype.UNSIGNED || tmpcom->datatype.datatype ){
            tmpcom->type = TYPE_NEW_VAR;
        }else{
            tmpcom->type = TYPE_VAR;
        }
    }

    #ifdef DEBUG_PARSER
        printf("\t\tTYPE: %d\n", tmpcom->type);
    #endif // DEBUG_PARSER

    ExitParser: // ============ EXIT PARSER ===============
    return tmpcom;
}

int main(){
    EndCode = code + lenstr(code);

    printf( "%d\n", (int)( parser(code) ) );

/*    for(uint i=0; i<4; i++){
        struct command *tmpcom = parser();

        if( tmpcom == 0 ){
            printf("\t\t(null)\n");
        } else {
            printf("\tDEBUG:\n\t\tUNSIGNED: %d\n\t\tREGISTER: %d\n\n", (int)(tmpcom->datatype.UNSIGNED), (int)(tmpcom->datatype.REGISTER) );
            printf("\t\tDATATYPE: %d\n\t\tNAME: ", tmpcom->datatype.datatype);
            if( tmpcom->name == 0 ){
                printf("(null)\n");
            } else {
                printf("\"%s\"\n", tmpcom->name);
            }
            printf("\t\tTYPE: %d\n\n", tmpcom->type);
        }
    }*/

    return 0;
}
