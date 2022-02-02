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
        if(str[i]==0){ return 0; }
        if(str[i]!=find[i]){ return 0; }
    }
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
    printf("%d | \"", lines);
    for(uint i=0; i<=10; i++){
        if( code[i]==0 || code[i]=='\n' ){ break; }
        printf("%c", code[i]);
    }
    printf("\"\n");
    exit(1);
}
unsigned int IgnoreSpace(char *code){ // игнорирует пропуски (пробел, таб, и тд.), если встречает \n то lines++
    register unsigned int i = 0;
    while(1){
        if( code[i] == 0 ){ break; }
        if( code[i] == '\n' ){ lines++; }
        if( code[i] == ' ' || code[i] < '!' ){ i++; } else { break; }
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

// ========================================== CODE ==========================================

/* TEST
	int a = foo( foo2(), foo3() );
	if( a > 10 && a < 20 ){
		foo( foo2(), foo3() );
		foo();
		foo();
	}
	
	if( a == 1 ) foo();
*/

char **parser0_arr = 0;
uint parser0_arr_index = 0;
uint i = 0;

char *parser0(char *code){ // разбивает код на части
	char *EndCode = code + lenstr(code);
	uint parser0_arr_index_max = 20;
	char *tmp;
	char *lastCode = code;
	byte lastCode_index = 0;
	
	// parser0_arr - масив адресов на строки
	
	if( parser0_arr == 0 ) parser0_arr = Local_malloc(parser0_arr_index_max);
	
	while(1){
		if( parser0_arr_index >= parser0_arr_index_max ){ // проверка не выходим ли мы из масива
			parser0_arr_index_max += 20;
			parser0_arr = Local_remalloc(parser0_arr, parser0_arr_index_max);
		}
		
		if( code > EndCode ) break; // выходим из цикла
		if( code == lastCode ){
			lastCode_index++;
		} else {
			lastCode_index = 0;
			lastCode = code;
		}
		if(lastCode_index > 3) errorParser(code, "I dont know what is it");
		
		i = IgnoreSpace(code); // игнорируем пробел (\t) и \n
		code += i;
		if(code > EndCode) return parser0_arr;
		
		// можно оптемизировать, но я этого делать не буду)))
		if(cmpstr(code, "if") || cmpstr(code, "for") || cmpstr(code, "while") || cmpstr(code, "elseif")){
			tmp = code;
			
			if(cmpstr(code, "if")) 		i=2; 	else
			if(cmpstr(code, "for")) 	i=3; 	else
			if(cmpstr(code, "while")) 	i=5; 	else
			if(cmpstr(code, "elseif")) 	i=6;
				
			i += IgnoreSpace(code);
			if(code+i > EndCode) break;
			
			if( code[i] == '(' ){
				i += IgnoreSpace(code+1);
				if( code[i]==')' ) errorParser(code, "Now should be condition");
				int count = 0;
				while(1){
					if( code+i > EndCode && count != 0 ) errorParser(code, "Im waiting ( )");
					if( code[i] == '(' ) count++;
					if( code[i] == ')' ) count--;
					if( count <= 0 ) break;
					i++;
				}
				code+=i+1;
				
				parser0_arr[parser0_arr_index] = copystr(tmp, code-tmp);
				parser0_arr_index++;
				
			}else{
				if (!( (code[i] <= '0' || code[i] >= '9') || (code[i] >= 'a' && code[i] <= 'z') ||
					(code[i] >= 'A' && code[i] <= 'Z') || code[i]=='_' )) 
						errorParser(code, "Im waiting ( )");
			}
			
		} else {
			if( code[0] == '{' ){
				parser0_arr[parser0_arr_index] = "{";
				parser0_arr_index++;
				code++;
			} else if( code[0] == '}' ){
				parser0_arr[parser0_arr_index] = "}";
				parser0_arr_index++;
				code++;
			}
			
			i = 0;
			while(1){
				if( code+i > EndCode ) errorParser(code, "I left the text");
				if( code[i] == ';' || code[] )
			}
			
		}
	}
	return parser0_arr;
}

int main(){
	parser0("if( a < foo( foo() ) ) foo();");
	
	return 0;
}