// Транслятор Си в FASM (FASM/C)
// Сделал Парфенов Илья (20.02.2022)
// 

#include <stdio.h> // printf
#include <windows.h> // LocalAlloc, LocalReAlloc, LocalHandle, LocalFree

// DEFINEs

#define bool unsigned char
#define uint unsigned int
#define byte unsigned char
#define true 1
#define false 0
#define null 0

// Global Variables

uint lines = 0;

// FOR malloc
struct malloc_struct{
	char *malloc_arr;
	uint malloc_arr_index;
	uint malloc_arr_index_max;
};
struct malloc_struct malloc_arr_struct[20];

bool malloc_init = false;
byte malloc_index_now = 0;

// =============================== FUNCs =======================================

// работа с памятью - выделение/освобождение/изменение размера/смена страницы
void malloc_init_func(){ // MALLOC INIT
	for(uint i=0; i<20; i++){
		malloc_arr_struct[i].malloc_arr = 0;
		malloc_arr_struct[i].malloc_arr_index = 0;
		malloc_arr_struct[i].malloc_arr_index_max = 20;
	}
	malloc_init = true;
}
char *Local_malloc(uint len){ // выделить память и записать адресс в масив, чтобы потом одной командой почистить
	if( malloc_init == false ) malloc_init_func();
	
	if(malloc_arr_struct[malloc_index_now].malloc_arr == 0){
		malloc_arr_struct[malloc_index_now].malloc_arr =
		LocalAlloc(
			LPTR,
			malloc_arr_struct[malloc_index_now].malloc_arr_index_max
		);
	}
	
	if(malloc_arr_struct[malloc_index_now].malloc_arr_index >= malloc_arr_struct[malloc_index_now].malloc_arr_index_max){
		malloc_arr_struct[malloc_index_now].malloc_arr_index_max+=20;
		malloc_arr_struct[malloc_index_now].malloc_arr =
		LocalReAlloc(
			LocalHandle(malloc_arr_struct[malloc_index_now].malloc_arr),
			LPTR,
			malloc_arr_struct[malloc_index_now].malloc_arr_index_max
		);
	}
	
	register char *tmp = LocalAlloc(LPTR, len);
	malloc_arr_struct[malloc_index_now].malloc_arr[malloc_arr_struct[malloc_index_now].malloc_arr_index] = tmp;
	malloc_arr_struct[malloc_index_now].malloc_arr_index++;
	return tmp;
}
char *Local_remalloc(char *adr, uint len){ // разщиряет память
	return LocalReAlloc( LocalHandle( adr ), LPTR, len );
}
void Local_free(){ // очищает масив malloc_arr, и освобождает память
	if( malloc_init == false ){ malloc_init_func(); }else{
		for(uint i=0; i < malloc_arr_struct[malloc_index_now].malloc_arr_index; i++)
			LocalFree( LocalHandle( malloc_arr_struct[malloc_index_now].malloc_arr[i] ) );
		
		LocalFree( LocalHandle( malloc_arr_struct[malloc_index_now].malloc_arr ) );
		malloc_arr_struct[malloc_index_now].malloc_arr = 0;
		malloc_arr_struct[malloc_index_now].malloc_arr_index = 0;
		malloc_arr_struct[malloc_index_now].malloc_arr_index_max = 20;
	}
}
void Local_malloc_free(char *adr){ // освободить по адресу
	LocalFree( LocalHandle(adr) );
}
bool swap_malloc(byte index){
	if(index > 20) return false;
	if( malloc_init == false ) malloc_init_func();
	malloc_index_now = index;
	return true;
}
byte getSwapIndex(){
	return malloc_index_now;
}

// работа с страницей
char *Local_malloc_swap(byte index, uint len){
	register byte swapIndex = getSwapIndex();
	if( swap_malloc(index) == false ) return 0;
	register char *tmp = Local_malloc(len);
	swap_malloc(swapIndex);
	return tmp;
}
char *Local_remalloc_swap(byte index, char *adr, uint len){
	register byte swapIndex = getSwapIndex();
	if( swap_malloc(index) == false ) return 0;
	register char *tmp = Local_remalloc(adr, len);
	swap_malloc(swapIndex);
	return tmp;
}
void Local_free_swap(byte index){
	register byte swapIndex = getSwapIndex();
	if( swap_malloc(index) == false ) return 0;
	Local_free();
	swap_malloc(swapIndex);
}

// работа со строками
unsigned int lenstr(char *str){ // получает длину строки ASCII
    register unsigned int len=0;
    while(1){
        if(str[len]==0){ return len; }
        len++;
    }
}
bool cmpstr(char *str, char *find){ // сравнивает две строки ASCII
    for(unsigned int i=0; i<lenstr(find); i++){
        if(str[i]==0){ return false; }
        if(str[i]!=find[i]){ return false; }
    }
	return true;
}
char *copystr(char *start, unsigned int len){ // создает новый масив и копирует туда строку ASCII
    if(len == 0){
        len = lenstr(start);
    }
    char *buf = Local_malloc(len+1);
    for(unsigned int i=0; i<len; i++){
        buf[i]=start[i];
        buf[i+1]=0;
    }
    return buf;
}
void errorParser(char *code, char *errorText){ // Выводит ощибку от парсера
    printf("ERROR Parser: %s\n", errorText);
    code = copystr(code, 0);
    code[10]='\0';
	printf("%d | ", lines+1);
	
    for(uint i=0; i<=10; i++){
        if( code[i]==0 || code[i]=='\n' ){ break; }
        printf("%c", code[i]);
    }
    printf("\n");
    exit(1);
}
unsigned int IgnoreSpace(char *code){ // игнорирует пропуски (пробел, таб, и тд.)
    register unsigned int i = 0;
    while(1){
        if( code[i] == 0 ){ break; }
		if( code[i] == '\n' ){ lines++; }
        if( code[i] < '!' ){ i++; } else { break; }
    }
    return i;
}

// ENUMs | STRUCTs
enum{ // DATATYPE
	DATATYPE_NONE = 0,
	DATATYPE_CHAR,
	DATATYPE_SHORT,
	DATATYPE_INT,
	DATATYPE_LONG,
	DATATYPE_FLOAT,
	DATATYPE_DOUBLE,
	DATATYPE_VOID,
	
	DATATYPE_UNSIGNED = 0x10, // 5 bit
	DATATYPE_REGISTER = 0x20, // 6 bit
	DATATYPE_ADRESS   = 0x40  // 7 bit
};
enum{ // TYPE
	TYPE_NONE = 0,
	TYPE_DATATYPE, 	// only datatype
	TYPE_NEW_VAR, 	// +
	TYPE_VAR,		// +
	TYPE_NEW_FUNC,	// +
	TYPE_CALL_FUNC, // +
	TYPE_IF,		// +
	TYPE_ELSE,		// +
	TYPE_FOR,		// +
	TYPE_WHILE,		// +
	TYPE_DO,			// do ... (while) +
	TYPE_START_BRACE,	// { +
	TYPE_END_BRACE,		// } +
	TYPE_START_ROUND_BRACKETS, // ( +
	TYPE_END_ROUND_BRACKETS,   // ) +
	TYPE_EQU,			// = +
	TYPE_ARRAY,			// [ ] +
	TYPE_STRUCT,		// struct ...
	TYPE_ENUM,			// enum ...
	TYPE_STRUCT_POINT, 	// .    (ABC.A) +
	TYPE_STRUCT_ARROW, 	// ->   (ABC->A) +
	TYPE_IF_SHORT, 		// ?	(ABC>5 ? A) +
	TYPE_ELSE_LABEL_SHORT, 	// :	(ABC>5 ? A : B) (LABEL:) +
	TYPE_BREAK,			// break; +
	TYPE_CONTINUE,		// continue; +
	TYPE_RETURN,		// return (...); +
	TYPE_COMMA,			// ,
	
	TYPE_IF_MORE, 		// >	(ABC > 5) +
	TYPE_IF_LESS,		// <	(ABC < 5) +
	TYPE_IF_EQU,		// == +
	TYPE_IF_NOEQU,		// != +
	TYPE_IF_MORE_EQU,	// >= +
	TYPE_IF_LESS_EQU,	// <= +
	TYPE_IF_AND,		// && +
	TYPE_IF_OR,			// || +
	
	TYPE_STRING,		// +
	TYPE_NUMBER,		// 123, 'ABC', 0x12, 12h +
	TYPE_MATH, 			// & | ! << >> + - * / (+)
	TYPE_GROUP			// FOR parser, маркирует груперовку команд, хранит масив в args
};

struct command{ // 11 bytes
	byte type;
	byte datatype;
	char *name;
	char *args;
	byte iargs; // if(iargs == 0) args is string, isn't array(s)
};

// ================================ CODE =======================================

struct command *parser_cmd(char *code, uint *OutIndex){
	if(code == 0 || code[0]==0) return 0;
	
	char *EndCode = lenstr(code) + code;
	char *lastCode = code;
	byte lastCodeIndex = 0;
	
	struct command *com = Local_malloc(sizeof(struct command)*10);
	uint comIndex = 0;
	uint ComMaxIndex = 10;
	
	com[comIndex].type = 0;
	com[comIndex].datatype = 0;
	com[comIndex].name = 0;
	com[comIndex].args = 0;
	com[comIndex].iargs = 0;
	
	uint i = 0;
	
	while(1){
		
		if(code[0]=='\0' || code[0]==';') break;
		
		if(comIndex > ComMaxIndex){
			ComMaxIndex+=10;
			Local_remalloc(com, sizeof(struct command)*ComMaxIndex);
		}
		
		code += IgnoreSpace(code);
		if(code >= EndCode) break;
		
		if(lastCode == code) lastCodeIndex++; else { lastCodeIndex = 0; lastCode = code; }
		if(lastCodeIndex > 3) return 0;
		
		switch(code[0]){
			case ',': com[comIndex].type = TYPE_COMMA; 		  code++; goto EndWhileParser1; break;
			case '?': com[comIndex].type = TYPE_IF_SHORT;	  code++; goto EndWhileParser1; break;
			case '=': com[comIndex].type = TYPE_EQU; 		  code++; goto EndWhileParser1; break;
			case '.': com[comIndex].type = TYPE_STRUCT_POINT; code++; goto EndWhileParser1; break;
			case '{': com[comIndex].type = TYPE_START_BRACE;  code++; goto EndWhileParser1; break;
			case '}': com[comIndex].type = TYPE_END_BRACE; 	  code++; goto EndWhileParser1; break;
			case '(': com[comIndex].type = TYPE_START_ROUND_BRACKETS; code++; goto EndWhileParser1; break;
			case ')': com[comIndex].type = TYPE_END_ROUND_BRACKETS;   code++; goto EndWhileParser1; break;
			case '-': if(code[1]=='>'){ com[comIndex].type = TYPE_STRUCT_ARROW; code+=2; goto EndWhileParser1; } break;
		}
		
		while(1){ // char, short, int, long, float, double, unsigned, register
			if(cmpstr(code, "char ")){
				code+=5;
				com[comIndex].datatype |= DATATYPE_CHAR;
			}else if(cmpstr(code, "short ")){
				code+=6;
				com[comIndex].datatype |= DATATYPE_SHORT;
			}else if(cmpstr(code, "int ")){
				code+=4;
				com[comIndex].datatype |= DATATYPE_INT;
			}else if(cmpstr(code, "long ")){
				code+=5;
				com[comIndex].datatype |= DATATYPE_LONG;
			}else if(cmpstr(code, "float ")){
				code+=6;
				com[comIndex].datatype |= DATATYPE_FLOAT;
			}else if(cmpstr(code, "double ")){
				code+=7;
				com[comIndex].datatype |= DATATYPE_DOUBLE;
				
			}else if(cmpstr(code, "unsigned ")){
				code+=9;
				com[comIndex].datatype |= DATATYPE_UNSIGNED;
			}else if(cmpstr(code, "register ")){
				code+=9;
				com[comIndex].datatype |= DATATYPE_REGISTER;
			}else
				break;
		}
			
		if(code >= EndCode) break;
		
		code += IgnoreSpace(code);
		if(code >= EndCode) break;
		
		if(com[comIndex].datatype != 0 && code[0]=='*'){ // DATATYPE_ADRESS
			com[comIndex].datatype |= DATATYPE_ADRESS;
			code++;
		}
		
		if(code[0]=='"'){ // string
			code++;
			i = 0;
			while(1){
				if(code[i]=='\\' && code[i+1]=='"') i+=2;
				if(code[i]==0 || code[i]=='"') break;
				i++;
			}
			com[comIndex].type = TYPE_STRING;
			com[comIndex].name = copystr(code, i);
			goto EndWhileParser1;
		}
		if(code[0]>='0' && code[0]<='9'){ // number
			i=0;
			if(code[1]=='x') i++;
			while(1)
				if( (code[i]>='0' && code[i]<='9') ||
					(code[i]>='A' && code[i]<='F') ||
					(code[i]>='a' && code[i]<='f') ||
					code[i]=='h' || code[i]=='H')
						i++; else break;
			com[comIndex].type = TYPE_NUMBER;
			com[comIndex].name = copystr(code, i);
			goto EndWhileParser1;
		}
		if(code[0]=='['){ // array
			com[comIndex].type = TYPE_ARRAY;
			
			code++;
			code += IgnoreSpace(code);
			if(code >= EndCode) break;
			
			if(code[0] == ']'){
				com[comIndex].name = 0;
			}else{
				i=0;
				uint count = 1;
				while(1){
					if(code+i > EndCode) break;
					if(code[i]=='[') count++;
					if(code[i]==']') count--;
					if(count <= 0) break;
					i++;
				}
				if(count>0) errorParser(code-1, "I lost the square brackets [ ]");
				com[comIndex].name = copystr(code, i);
			}
			goto EndWhileParser1;
		}
		
		// проверка резервированых слов =====
		if(cmpstr(code, "if") || cmpstr(code, "for") || cmpstr(code, "while")){
			if(cmpstr(code, "if")){ com[comIndex].type = TYPE_IF; code+=2; } else
			if(cmpstr(code, "for")){ com[comIndex].type = TYPE_FOR; code+=2; } else
			if(cmpstr(code, "while")){ com[comIndex].type = TYPE_WHILE; code+=5; }
			
			i = IgnoreSpace(code);
			if(code+i > EndCode) errorParser(code, "it must be followed by parentheses ( )");
			if(code[i]=='('){
				code += i+1;
				i = IgnoreSpace(code);
				if(code+i > EndCode) errorParser(code, "I lost the parentheses ( )");
				if(code[i] == ')') errorParser(code, "where condition?");
				i=0;
				uint count = 1;
				while(1){
					if(code+i > EndCode) break;
					if(code[i]=='(') count++;
					if(code[i]==')') count--;
					if(count <= 0) break;
					i++;
				}
				if(count > 0) errorParser(code, "I lost the parentheses ( )");
				com[comIndex].name = copystr(code, i);
				code += i+1;
			} else errorParser(code, "it must be followed by parentheses ( )");
			goto EndWhileParser1;
		} else if(cmpstr(code, "do")){
			com[comIndex].type = TYPE_DO;
			code+=2;
		} else if(cmpstr(code, "else")){
			com[comIndex].type = TYPE_ELSE;
			code+=4;
		}
		
		// IF logic
		switch(code[0]){
			case '>': if(code[1]=='='){com[comIndex].type = TYPE_IF_MORE_EQU; code+=2;}
					else { com[comIndex].type = TYPE_IF_MORE; code++;} goto EndWhileParser1; break;
			case '<': if(code[1]=='='){com[comIndex].type = TYPE_IF_LESS_EQU; code+=2;}
					else {com[comIndex].type = TYPE_IF_LESS; code++;} goto EndWhileParser1; break;
			case '=': if(code[1]=='='){com[comIndex].type = TYPE_IF_EQU; code+=2;} goto EndWhileParser1; break;
			case '!': if(code[1]=='='){com[comIndex].type = TYPE_IF_NOEQU; code+=2;} goto EndWhileParser1; break;
			case '&': if(code[1]=='&'){com[comIndex].type = TYPE_IF_AND; code+=2;} goto EndWhileParser1; break;
			case '|': if(code[1]=='|'){com[comIndex].type = TYPE_IF_OR; code+=2;} goto EndWhileParser1; break;
		}
		
		// TYPE_MATH
		if( code[0]=='&' || code[0]=='|' || code[0]=='!' || code[0]=='+' || code[0]=='-' ||
			code[0]=='*' || code[0]=='/'){
			com[comIndex].type = TYPE_MATH;
			com[comIndex].name = copystr(code, 1);
			goto EndWhileParser1;
		} else if( (code[0]=='>' && code[1]=='>') || (code[0]=='<' && code[1]=='<') ){
			com[comIndex].type = TYPE_MATH;
			com[comIndex].name = copystr(code, 2);
			goto EndWhileParser1;
		}
		// ======
		
		if(cmpstr(code, "break")){ // break, continue, return
			com[comIndex].type = TYPE_BREAK;
			code+=5;
			goto EndWhileParser1;
		} else if(cmpstr(code, "continue")){
			com[comIndex].type = TYPE_CONTINUE;
			code+=8;
			goto EndWhileParser1;
		} else if(cmpstr(code, "return ")){
			com[comIndex].type = TYPE_RETURN;
			code+=7;
			i = 0;
			while(1){
				if(code[i]==';') break;
				i++;
			}
			com[comIndex].args = copystr(code, i);
			com[comIndex].iargs = 0; // is string
		}
		
		// GET NAME
		i = 0;
		while(1){
			if(code+i > EndCode) break;
			if( (code[i] >= '0' && code[i] <= '9') ||
				(code[i] >= 'a' && code[i] <= 'z') ||
				(code[i] >= 'A' && code[i] <= 'Z') ||
				 code[i] == '_' ) i++; else break;
		}
		if(i==0) com[comIndex].name = 0; else
			com[comIndex].name = copystr(code, i);
		code+=i;
		
		code+=IgnoreSpace(code);
		if(code > EndCode) break;
		
		if(com[comIndex].name > 0 && code[0]==':'){
			code++;
			com[comIndex].type = TYPE_ELSE_LABEL_SHORT;
			goto EndWhileParser1;
		} else if(com[comIndex].name > 0 && code[0]=='('){ // call func / create func
			if(com[comIndex].datatype > 0) // create func
				com[comIndex].type = TYPE_NEW_FUNC;
			else
				com[comIndex].type = TYPE_CALL_FUNC;
			
			code++;
			i = IgnoreSpace(code);
			if(code+i > EndCode) errorParser(code, "I lost the parentheses ( )");
			if(code[i] == ')'){
				com[comIndex].args = 0;
				com[comIndex].iargs = 0;
				code++;
			}else{
				i=0;
				uint count = 1;
				while(1){
					if(code+i > EndCode) break;
					if(code[i]=='(') count++;
					if(code[i]==')') count--;
					if(count <= 0) break;
					i++;
				}
				if(count > 0) errorParser(code, "I lost the parentheses ( )");
				com[comIndex].args = copystr(code, i);
				com[comIndex].iargs = 0;
				code += i+1;
			}
				
			goto EndWhileParser1;
		} else if( com[comIndex].datatype > 0 && com[comIndex].name > 0 ){ // IS VAR\NEW VAR\NONE?
			com[comIndex].type = TYPE_NEW_VAR;
		} else if(com[comIndex].name > 0){
			com[comIndex].type = TYPE_VAR;
		}else{
			com[comIndex].type = TYPE_NONE;
		}
		
		if(com[comIndex].type == TYPE_NONE && com[comIndex].datatype > 0){
			com[comIndex].type = TYPE_DATATYPE;
		}
		
		EndWhileParser1:
		
		comIndex++;
	}
	// return
	*OutIndex = comIndex;
	return com;
}

struct command *parser0(char *code, uint *OutIndex){ // первый этап
	if(code==0 || code[0]==0) return 0;

	char *EndCode = lenstr(code) + code;
	char *lastCode = code;
	byte lastCodeIndex = 0;

	struct command *com = Local_malloc(sizeof(char*)*10);
	uint comIndex = 0;
	uint ComMaxIndex = 10;
	
	uint i = 0;
	uint indexCmd = 0;
	char *nowCode = 0;
	
	while(1){ // первый прогон
		
		if(code[0]==';') code++;
		
		if(code[0]==0) break;
		
		if(comIndex > ComMaxIndex){
			ComMaxIndex+=10;
			Local_remalloc(com, sizeof(struct command)*ComMaxIndex);
		}
		
		if(lastCode == code) lastCodeIndex++; else { lastCodeIndex = 0; lastCode = code; }
		if(lastCodeIndex > 3) return 0;
		
		i = 0;
		while(1){
			if(code[i] == 0 || code[i] == ';') break;
			else i++;
		}
		
		com[comIndex].type = TYPE_GROUP;
		com[comIndex].args = parser_cmd(copystr(code, i), &indexCmd);
		com[comIndex].iargs = indexCmd;
		code+=i;
		
		#ifdef _DEBUG_PARSER0
		struct command *cmd = com[comIndex];
		printf("comIndex: %d\ncmdIndex: %d\n", comIndex, indexCmd);
		for(uint i=0; i<indexCmd; i++){ // not full
			printf("\tTYPE: ");
			switch(cmd[i].type){
				case TYPE_NONE:				printf("TYPE_NONE");		break;
				case TYPE_DATATYPE:			printf("TYPE_DATATYPE");	break;
				case TYPE_NEW_VAR: 			printf("TYPE_NEW_VAR"); 	break;
				case TYPE_VAR: 				printf("TYPE_VAR"); 		break;
				case TYPE_NEW_FUNC: 		printf("TYPE_NEW_FUNC"); 	break;
				case TYPE_CALL_FUNC: 		printf("TYPE_CALL_FUNC"); 	break;
				case TYPE_IF: 				printf("TYPE_IF"); 			break;
				case TYPE_ELSE: 			printf("TYPE_ELSE"); 		break;
				case TYPE_FOR: 				printf("TYPE_FOR"); 		break;
				case TYPE_WHILE: 			printf("TYPE_WHILE"); 		break;
				case TYPE_DO: 				printf("TYPE_DO"); 			break;
				case TYPE_START_BRACE: 		printf("TYPE_START_BRACE"); break;
				case TYPE_END_BRACE: 		printf("TYPE_END_BRACE"); 	break;
				case TYPE_START_ROUND_BRACKETS: printf("TYPE_START_ROUND_BRACKETS"); break;
				case TYPE_END_ROUND_BRACKETS: 	printf("TYPE_END_ROUND_BRACKETS"); 	 break;
				case TYPE_EQU: 				printf("TYPE_EQU"); 		break;
				case TYPE_ELSE_LABEL_SHORT:	printf("TYPE_LABEL"); 		break;
				case TYPE_ARRAY: 			printf("TYPE_ARRAY"); 		break;
				case TYPE_STRUCT_POINT: 	printf("TYPE_STRUCT_POINT");break;
				case TYPE_STRUCT_ARROW: 	printf("TYPE_STRUCT_ARROW");break;
				case TYPE_STRING: 			printf("TYPE_STRING"); 		break;
				case TYPE_NUMBER: 			printf("TYPE_NUMBER"); 		break;
				case TYPE_MATH: 			printf("TYPE_MATH"); 		break;
				case TYPE_TRANSFORM: 		printf("TYPE_TRANSFORM"); 	break;
				default: 					printf("Error"); 			break;
			}
			printf("\n");
					
			printf("\tDATATYPE: ");
			if(cmd[i].datatype & DATATYPE_UNSIGNED) printf("UNSIGNED ");
			if(cmd[i].datatype & DATATYPE_REGISTER) printf("REGISTER ");
			
			if(cmd[i].datatype & DATATYPE_CHAR) printf("CHAR"); else
			if(cmd[i].datatype & DATATYPE_SHORT) printf("SHORT"); else
			if(cmd[i].datatype & DATATYPE_INT) printf("INT"); else
			if(cmd[i].datatype & DATATYPE_LONG) printf("LONG"); else
			if(cmd[i].datatype & DATATYPE_FLOAT) printf("FLOAT"); else
			if(cmd[i].datatype & DATATYPE_DOUBLE) printf("DOUBLE"); else
				printf("(null)");
			printf("\n");
			
			printf("\tName: ");
			if(cmd[i].name > 0) printf("%s\n", cmd[i].name);
				else printf("(null)\n");
			
			printf("\tAGRS: ");
			if(cmd[i].iargs == 0 && cmd[i].args > 0) printf("%s\n", cmd[i].args);
				else printf("(null)\n");
			
			printf("\tIARGS: %d\n", cmd[i].iargs);
			
			printf("\n\n");
		}
		
		printf("=========================================\n");
		#endif
		
		comIndex++;
	}
	*OutIndex = comIndex;
	return com;
}

struct command *

struct command **parser(char *code, uint *OutIndex){
	if(code == 0 || code[0] == '\0') return 0;
	
	char *EndCode = lenstr(code) + code;
	char *lastCode = code;
	byte lastCodeIndex = 0;
	
	uint comIndex = 0;
	
	uint i = 0;
	uint indexCmd = 0;
	struct command *cmd = 0;
	
	struct command *com = parser0(code, &comIndex);
	
	for(uint icom=0; icom<comIndex; icom++){
		cmd = com[icom].args;
		for(uint icmd=0; icmd < com[icom].iargs; icmd++){
		}
	}
	
}

int main(){
	uint Index = 0;
	struct command **cmd = parser("unsigned int abc = sizeof(unsigned int *);\nint *abc = 10;", &Index);
	
	return 0;
}