#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include "Parser.h"

int main(){
    /*char *code; // GET FILE (WINAPI)
    OFSTRUCT ofstr;
    HFILE hFile = OpenFile("main.c", &ofstr, OF_READ);
    if(hFile == HFILE_ERROR){
        printf("ERROR: Cannot open file.\n");
        getch();
        exit(1);
    }
    uint fileLen = GetFileSize(hFile, NULL);
    code = LocalAlloc(LPTR, fileLen);
    int cnt;
    if(!ReadFile(hFile, code, fileLen, &cnt, NULL)){
        printf("ERROR: Cannot read file.\n");
        getch();
        exit(1);
    }*/

    char *code =    "int   a=   10    ; int b=15;\n"
                    "printf(\"int a = 10;\\n\");\n"
                    "for(int i=0; i<10; i++){\n"
                    "printf(\"%d\\n\", i);\n"
                    "}   \n"
                    "\"int c= 10;\"; "
                    "void foo(int a, int b){\n"
                    "\tprintf(\"sum  :     %d\", a+b);\n"
                    "}  ";

    char **parse = Parser(code);
    for(uint i=0; i<20; i++){
        if(parse[i]==0){
            printf("(null)\n");
        }else{
            printf("%d\t| %s\n", i, parse[i]);
        }
    }
    getch();
    return 0;
}
