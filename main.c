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
bool swap_malloc(byte index){
	if(index > 20) return false;
	if( malloc_init == false ) malloc_init_func();
	malloc_index_now = index;
	return true;
}

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
	DATATYPE_VOID
};
enum{ // TYPE
	TYPE_NONE = 0,
	TYPE_NEW_VAR,
	TYPE_VAR,
	TYPE_NEW_FUNC,
	TYPE_CALL_FUNC,
	TYPE_IF,
	TYPE_ELSE,
	TYPE_FOR,
	TYPE_WHILE,
	TYPE_DO,
	TYPE_START_BRACE,
	TYPE_END_BRACE,
	TYPE_START_ROUND_BRACKETS,
	TYPE_END_ROUND_BRACKETS,
	TYPE_EQU,
	TYPE_POINT,
	TYPE_ARRAY,
	TYPE_STRUCT_POINT, // .    (ABC.B)
	TYPE_STRUCT_ARROW, // ->   (ABC->B)
	
	TYPE_STRING,
	TYPE_NUMBER,
	TYPE_MATH,
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

struct parser0_struct *parser0_arr = 0;
uint parser0_arr_index = 0;
uint i = 0;
uint index = 0;

void parser(char *code){ 
	
	// ================== Разбивает код на части ===================================
	char *EndCode = code + lenstr(code);
	uint parser0_arr_index_max = 20;
	char *lastCode = code;
	byte lastCode_index = 0;
	
	// parser0_arr - масив адресов на строки
	
	if( parser0_arr == 0 ) parser0_arr = Local_malloc(sizeof(parser0_arr) * parser0_arr_index_max);
	
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
	
	#ifdef _DEBUG_PARSER0
		printf("Index: %d\nLines: %d\n", parser0_arr_index, lines);
		for(uint i=0; i < parser0_arr_index; i++)
			printf("%d | %s\n", parser0_arr[i].line, parser0_arr[i].text);
			
		printf("\n================================\n");
	#endif
	
	// ================================ Разбирает на команды ===========================
	
	for(index=0; index < parser0_arr_index; index++){
		
	}
	
}

int main(){
	parser(" if(\")\")\n{\n\tfoo();\n\tfoo2\"(asd;;;)\"();\n\"abc\"\n\"def\"\n} else {\n\tfoo1();\n\tfoo2()\n}\n");
	
	return 0;
}