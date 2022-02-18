// Транслятор Си в FASM (FASM/C)
// Сделал Парфенов Илья (02.02.2022)
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
void errorParser(char *code, char *errorText, bool bLines){ // Выводит ощибку от парсера
    printf("ERROR Parser: %s\n", errorText);
    code = copystr(code, 0);
    code[10]='\0';
    if(bLines == true) printf("%d | ", lines+1);
	
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
	
	DATATYPE_UNSIGNED = 5,
	DATATYPE_REGISTER
};
enum{ // TYPE
	TYPE_NONE = 0,
	TYPE_NEW_VAR, 	// +
	TYPE_VAR,		// +
	TYPE_NEW_FUNC,
	TYPE_CALL_FUNC,
	TYPE_IF,
	TYPE_ELSE,
	TYPE_FOR,
	TYPE_WHILE,
	TYPE_DO,
	TYPE_START_BRACE, 	// +
	TYPE_END_BRACE,		// +
	TYPE_START_ROUND_BRACKETS,
	TYPE_END_ROUND_BRACKETS,
	TYPE_EQU,
	TYPE_ARRAY,
	TYPE_STRUCT_POINT, 	// .    (ABC.A)	// +
	TYPE_STRUCT_ARROW, 	// ->   (ABC->A) // +
	TYPE_IF_SHORT, 		// ?	(ABC>5 ? A)
	TYPE_ELSE_LABEL_SHORT, 	// :	(ABC>5 ? A : B) (LABEL:)
	TYPE_BREAK,			// break;
	TYPE_CONTINUE,		// continue;
	
	TYPE_IF_MORE, 		// >	(ABC > 5)
	TYPE_IF_LESS,		// <	(ABC < 5)
	TYPE_IF_EQU,		// ==
	TYPE_IF_NOEQU,		// !=
	TYPE_IF_MORE_EQU,	// >=
	TYPE_IF_LESS_EQU,	// <=
	TYPE_IF_AND,		// &&
	TYPE_IF_OR,			// ||
	
	TYPE_STRING,
	TYPE_NUMBER,
	TYPE_MATH, 			// & | ! << >> + - * /
	TYPE_TRANSFORM,
	TYPE_VOID
};

// for parser
struct parser0_struct{ // Первая часть 
	char *text;
	uint line;
};
struct parser1_struct{ // Вторая часть
	uint line;
	byte type;
	byte datatype;
	// 0b 0000 0000
	//    ^^^^ ^^^^	-- для типа данных (char, short, int, long, float, double)
	//	  ||||
	//	для unsigned, register, long.
	
	char *name;
	char *args;
};

// ========================================== CODE ==========================================

uint i = 0;
uint index = 0;

uint parser1_struct_Index = 0;

struct parser1_struct *parser1(char *code){
	if(code == 0 || code[0]==0) return 0;
	
	char *EndCode = lenstr(code) + code;
	char *lastCode = code;
	byte lastCodeIndex = 0;
	
	struct parser1_struct *com = Local_malloc(sizeof(struct parser1_struct)*10);
	uint ComMaxIndex = 10;
	
	com[parser1_struct_Index].line = 0;
	com[parser1_struct_Index].type = 0;
	com[parser1_struct_Index].datatype = 0;
	com[parser1_struct_Index].name = 0;
	com[parser1_struct_Index].args = 0;
	
	while(1){
		
		if(parser1_struct_Index > ComMaxIndex){
			ComMaxIndex+=10;
			Local_remalloc(com, sizeof(struct parser1_struct)*ComMaxIndex);
		}
		
		code += IgnoreSpace(code);
		if(code >= EndCode) break;
		
		if(lastCode == code) lastCodeIndex++; else{ lastCodeIndex = 0; lastCode = code; }
		if(lastCodeIndex > 3) return 0;
			
		switch(code[0]){
			case '=': com[parser1_struct_Index].type = TYPE_EQU; 		  code++; goto EndWhileParser1; break;
			case '.': com[parser1_struct_Index].type = TYPE_STRUCT_POINT; code++; goto EndWhileParser1; break;
			case '{': com[parser1_struct_Index].type = TYPE_START_BRACE;  code++; goto EndWhileParser1; break;
			case '}': com[parser1_struct_Index].type = TYPE_END_BRACE; 	  code++; goto EndWhileParser1; break;
			case '-': if(code[1]=='>'){ com[parser1_struct_Index].type = TYPE_STRUCT_ARROW; code+=2; goto EndWhileParser1; } break;
		}
			
		while(1){ // unsigned, register
			if(cmpstr(code, "unsigned ")){
				code+=9;
				com[parser1_struct_Index].datatype |= (1<<DATATYPE_UNSIGNED);
			}else if(cmpstr(code, "register ")){
				code+=9;
				com[parser1_struct_Index].datatype |= (1<<DATATYPE_REGISTER);
			}else
				break;
		}
			
		if(code >= EndCode) break;
			
		while(1){ // char, short, int, long, float, double
			if(cmpstr(code, "char ")){
				code+=5;
				com[parser1_struct_Index].datatype |= DATATYPE_CHAR;
			}else if(cmpstr(code, "short ")){
				code+=6;
				com[parser1_struct_Index].datatype |= DATATYPE_SHORT;
			}else if(cmpstr(code, "int ")){
				code+=4;
				com[parser1_struct_Index].datatype |= DATATYPE_INT;
			}else if(cmpstr(code, "long ")){
				code+=5;
				com[parser1_struct_Index].datatype |= DATATYPE_LONG;
			}else if(cmpstr(code, "float ")){
				code+=6;
				com[parser1_struct_Index].datatype |= DATATYPE_FLOAT;
			}else if(cmpstr(code, "double ")){
				code+=7;
				com[parser1_struct_Index].datatype |= DATATYPE_DOUBLE;
			}else
				break;
		}
			
		if(code >= EndCode) break;
		
		// проверка резервированых слов
		
		if(cmpstr(code, "if")){
			com[parser1_struct_Index].type = TYPE_IF;
			code += 2;
			i = IgnoreSpace(code);
			if(code+i > EndCode) errorParser(code, "'if' must be followed by parentheses ( )", true);
			if(code[i]=='('){
				code += i+1;
				i = IgnoreSpace(code);
				if(code+i > EndCode) errorParser(code, "I lost the parentheses ( )", true);
				if(code[i] == ')') errorParser(code, "where condition?", true);
				i=0;
				uint count = 1;
				while(1){
					if(code+i > EndCode) break;
					if(code[i]=='(') count++;
					if(code[i]==')') count--;
					if(count <= 0) break;
					i++;
				}
				if(count > 0) errorParser(code, "I lost the parentheses ( )", true);
				com[parser1_struct_Index].name = copystr(code, i);
				code += i+1;
			} else errorParser(code, "'if' must be followed by parentheses ( )", true);
			goto EndWhileParser1;
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
		if(i==0) com[parser1_struct_Index].name = 0; else
			com[parser1_struct_Index].name = copystr(code, i);
		code+=i;
		
		code+=IgnoreSpace(code);
		if(code > EndCode) break;
		
		if(com[parser1_struct_Index].name > 0 && code[0]==':'){
			code++;
			com[parser1_struct_Index].type = TYPE_ELSE_LABEL_SHORT;
			goto EndWhileParser1;
		} else if( com[parser1_struct_Index].datatype > 0 && com[parser1_struct_Index].name > 0 ){ // IS VAR\NEW VAR\NONE?
			com[parser1_struct_Index].type = TYPE_NEW_VAR;
		} else if(com[parser1_struct_Index].name > 0 && code[0]=='('){ // call func / create func
			if(com[parser1_struct_Index].datatype > 0) // create func
				com[parser1_struct_Index].type = TYPE_NEW_FUNC;
			else
				com[parser1_struct_Index].type = TYPE_CALL_FUNC;
			
			code += i+1;
			i = IgnoreSpace(code);
			if(code+i > EndCode) errorParser(code, "I lost the parentheses ( )", true);
			if(code[i] == ')'){
				com[parser1_struct_Index].name = 0;
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
				if(count > 0) errorParser(code, "I lost the parentheses ( )", true);
				com[parser1_struct_Index].name = copystr(code, i);
				code += i+1;
			}
				
			goto EndWhileParser1;
		}else if(com[parser1_struct_Index].name > 0){
			com[parser1_struct_Index].type = TYPE_VAR;
		}else{
			com[parser1_struct_Index].type = TYPE_NONE;
		}
		
		EndWhileParser1:
		
		parser1_struct_Index++;
	}
	return com;
}

void parser(char *code){ 
	
	// ================== Разбивает код на части ===================================
	struct parser0_struct *parser0_arr = Local_malloc(sizeof(parser0_arr) * 20);
	uint parser0_arr_index = 0;
	uint parser0_arr_index_max = 20;
	
	char *EndCode = code + lenstr(code);
	char *lastCode = code;
	byte lastCode_index = 0;
	
	// parser0_arr - масив адресов на строки
	
	while(1){ // главный цикл
		parser0_while_point:
		if( parser0_arr_index >= parser0_arr_index_max ){ // проверка не выходим ли мы из масива
			parser0_arr_index_max += 20;
			parser0_arr = Local_remalloc(parser0_arr, sizeof(parser0_arr) * parser0_arr_index_max);
		}
		
		if( code >= EndCode ) break; // выходим из цикла
		if( code == lastCode ){
			lastCode_index++;
		} else {
			lastCode_index = 0;
			lastCode = code;
		}
		if(lastCode_index > 3) errorParser(code, "I dont know what is it", true);
		
		if( code[0] == ';' ){
			code++;
		} else {
		
			i = IgnoreSpace(code); // игнорируем пробел (\t) и \n
			code += i;
			if(code > EndCode) break;
			
			// можно оптемизировать, но я этого делать не буду)))
			if(cmpstr(code, "if") || cmpstr(code, "for") || cmpstr(code, "while") ||
			   cmpstr(code, "elseif")){
				
				if(cmpstr(code, "if")) 		i=2; 	else
				if(cmpstr(code, "for")) 	i=3; 	else
				if(cmpstr(code, "while")) 	i=5; 	else
				if(cmpstr(code, "elseif")) 	i=6;
					
				i += IgnoreSpace(code);
				if(code+i > EndCode) break;
				
				if( code[i] == '(' ){
					i += IgnoreSpace(code+1);
					int count = 0;
					while(1){
						if( code+i > EndCode && count != 0 ) errorParser(code, "Im waiting ( )", true);
						if( code[i] == '\"' ){
							i++;
							while(1){
								if(code+i>EndCode) errorParser(code, "Im waiting \" \"", true);
								if(code[i]=='\"'){ i++; break; } else i++;
							}
						}
						if( code[i] == '(' ) count++;
						if( code[i] == ')' ) count--;
						if( count <= 0 ) break;
						i++;
					}
					i++;
					parser0_arr[parser0_arr_index].text = copystr(code, i);
					parser0_arr[parser0_arr_index].line = lines;
					parser0_arr_index++;
					
					code += i;
					
				}else{
					if (!( (code[i] <= '0' || code[i] >= '9') || (code[i] >= 'a' && code[i] <= 'z') ||
						(code[i] >= 'A' && code[i] <= 'Z') || code[i]=='_' )) 
							errorParser(code, "Im waiting ( )", true);
				}
				
			} else {
				if( code[0] == '{' ){
					parser0_arr[parser0_arr_index].text = "{";
					parser0_arr[parser0_arr_index].line = lines;
					parser0_arr_index++;
					code++;
					goto parser0_while_point;
				} else if( code[0] == '}' ){
					parser0_arr[parser0_arr_index].text = "}";
					parser0_arr[parser0_arr_index].line = lines;
					parser0_arr_index++;
					code++;
					goto parser0_while_point;
				}
				
				i = 0;
				while(1){
					if( code+i >= EndCode ) break;
					if( code[i] == '\"' ){
						i++;
						while(1){
							if(code+i>EndCode) errorParser(code, "Im waiting \" \"", true);
							if(code[i]=='\"'){ i++; break; } else i++;
						}
					}
					if( code[i] == ';' || code[i] == '{' || code[i] == '}' ) break; 
					i++;
				}
				if( i==0 ) break;
				parser0_arr[parser0_arr_index].text = copystr(code, i);
				parser0_arr[parser0_arr_index].line = lines;
				parser0_arr_index++;
				
				code += i;
			}
		}
	}
	
	// ==================================================================================
	
	#define _DEBUG_PARSER0
	#ifdef _DEBUG_PARSER0
		printf("Index: %d\nLines: %d\n", parser0_arr_index, lines);
		for(uint i=0; i < parser0_arr_index; i++)
			printf("%d | %s\n", parser0_arr[i].line, parser0_arr[i].text);
			
		printf("\n================================\n");
	#endif
	
	// ================================ Разбирает на команды ===========================
	
	swap_malloc(1); // меняем таблицу
	
	struct parser1_struct *parser1_arr = Local_malloc(sizeof(parser1_arr) * 20);
	uint parser1_arr_index = 0;
	uint parser1_arr_index_max = 20;
	
	for(index=0; index < parser0_arr_index; index++){
		lines = parser0_arr[index].line;
		
		struct parser1_struct *com =  parser1(parser0_arr[index].text);
		
		if(com == 0) errorParser(parser0_arr[index].text, "I dont know what is it...", false);
		
		#define _DEBUG_PARSER1
		
		#ifdef _DEBUG_PARSER1
		for(i=0; i<parser1_struct_Index; i++){
			printf("UNSIGNED: ");
			printf( (com[i].datatype & (1<<DATATYPE_UNSIGNED)) ? "TRUE" : "FALSE" );
			
			printf("\n");
			
			printf("REGISTER: ");
			printf( (com[i].datatype & (1<<DATATYPE_REGISTER)) ? "TRUE" : "FALSE" );
			
			printf("\n");
		
			com[i].datatype <<= 4;
			com[i].datatype >>= 4;
				
			printf("DATATYPE: ");
			switch(com[i].datatype){
				case DATATYPE_CHAR: 	printf("CHAR"); 			break;
				case DATATYPE_SHORT: 	printf("SHORT"); 			break;
				case DATATYPE_INT: 		printf("INT"); 				break;
				case DATATYPE_LONG: 	printf("LONG"); 			break;
				case DATATYPE_FLOAT: 	printf("FLOAT"); 			break;
				case DATATYPE_DOUBLE: 	printf("DOUBLE"); 			break;
				default: 				printf("NONE");				break;
			}
			printf("\n");
			
			printf("TYPE: ");
			switch(com[i].type){
				case TYPE_NEW_VAR: 		printf("TYPE_NEW_VAR"); 	break;
				case TYPE_VAR: 			printf("TYPE_VAR"); 		break;
				case TYPE_NEW_FUNC: 	printf("TYPE_NEW_FUNC"); 	break;
				case TYPE_CALL_FUNC: 	printf("TYPE_CALL_FUNC"); 	break;
				case TYPE_IF: 			printf("TYPE_IF"); 			break;
				case TYPE_ELSE: 		printf("TYPE_ELSE"); 		break;
				case TYPE_FOR: 			printf("TYPE_FOR"); 		break;
				case TYPE_WHILE: 		printf("TYPE_WHILE"); 		break;
				case TYPE_DO: 			printf("TYPE_DO"); 			break;
				case TYPE_START_BRACE: 	printf("TYPE_START_BRACE"); break;
				case TYPE_END_BRACE: 	printf("TYPE_END_BRACE"); 	break;
				case TYPE_START_ROUND_BRACKETS: printf("TYPE_START_ROUND_BRACKETS"); break;
				case TYPE_END_ROUND_BRACKETS: 	printf("TYPE_END_ROUND_BRACKETS"); 	 break;
				case TYPE_EQU: 			printf("TYPE_EQU"); 		break;
				case TYPE_ELSE_LABEL_SHORT:printf("TYPE_LABEL"); 	break;
				case TYPE_ARRAY: 		printf("TYPE_ARRAY"); 		break;
				case TYPE_STRUCT_POINT: printf("TYPE_STRUCT_POINT");break;
				case TYPE_STRUCT_ARROW: printf("TYPE_STRUCT_ARROW");break;
				case TYPE_STRING: 		printf("TYPE_STRING"); 		break;
				case TYPE_NUMBER: 		printf("TYPE_NUMBER"); 		break;
				case TYPE_MATH: 		printf("TYPE_MATH"); 		break;
				case TYPE_TRANSFORM: 	printf("TYPE_TRANSFORM"); 	break;
				case TYPE_VOID: 		printf("TYPE_VOID"); 		break;
				default: 				printf("NONE"); 			break;
			}
			printf("\n");
			
			printf("NAME: %s\n", com[i].name);
				
			printf("\n");
		}
		printf("\n========================\n");
		#endif
	}
	
}

int main(){
	parser("if(a)\n\tabc( a, b, c );");
	
	return 0;
}