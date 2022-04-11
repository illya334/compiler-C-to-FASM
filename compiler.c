/*
 * Компилятор Си | Compiler C
 * 
 * by illya334 (zeq52giw)
 * 
 * Habr: https://habr.com/ru/users/illya334/
 * 
*/

#include <stdio.h> // wprintf
#include <stdlib.h> // malloc, realloc, free
#include <wchar.h> // wchar_t
#include <stdint.h> // uint8_t

// #include "wstring.c" // setmem, dynamic_array_process, lenstr, cpystr, cpystrMem, cmpstr, addstr, formatStr

// макросы для облегчения
#define wchar wchar_t
#define uint unsigned int
#define reg register
#define bool uint8_t
#define byte uint8_t

// по какойто причине мой компилятор не имеет этих макросов с старта
#define true 1
#define false 0

uint Lines;

// изменять эту функцию, остальные это алгоритм
void errorCompiler(wchar *errorText){
	wprintf(L"Compiling error on line [%d]: %s.\n", Lines, errorText);
	exit(1);
}


#define MaxWordLen 30
struct word_st{ wchar word[MaxWordLen+1]; };
struct word_st *words = 0; // здесь будут храница слова, после выполнения 'ToWord'
uint wordIndex = 0,
	 MaxWordIndex = 20;
	 
uint i, j, h, u, g; // временые переменые

wchar *tmp;


enum{ // базовые типы данных
	datatype_none = 0,
	datatype_void = 1,
	datatype_char,
	datatype_short,
	datatype_int,
	datatype_long,
	datatype_float,
	datatype_double
};
// bool
#define dopDatatype_unsigned 0x1 // 1 bit
#define dopDatatype_register 0x2 // 2 bit
#define dopDatatype_extern	 0x4 // 3 bit // переменные этого типа будут именть имя то которое указал пользователь
#define dopDatatype_stack	 0x8 // 4 bit // определяет где хранить переменую, в стеке или в секции данных

byte nowDopDatatype = 0; // храняца биты (bool) - unsigned, register, extern, long, stack
byte nowBaseDatatype = 0; // void, char, short, int, long, float, double
wchar nowFunc[MaxWordLen+1]; // имя текущей функции

struct st_datatype{
	byte DopDatatype;
	byte BaseDatatype;
}

struct st_variable{
	byte DopDatatype;
	byte BaseDatatype;
	wchar name[MaxWordLen+1];
};
struct st_variable *globalVariables = 0;
struct st_variable *localVariables = 0;
uint globalVarIndex = 0,
	 globalVarMaxIndex = 20,
	 
	 localVarIndex = 0,
	 localVarMaxIndex = 20;

#define MaxFuncArgs 15
struct st_funcRef{
	byte DopDatatype;
	byte BaseDatatype;
	wchar name[MaxWordLen+1];
	byte argsIndex; // количество аргументов
	struct st_datatype args[MaxFuncArgs];
};
struct st_funcRef *funcRef = 0;
uint funcRefIndex = 0,
	 funcRefMaxIndex = 20;



// ==== REF ====
void ToWord( wchar *code );
bool isNum( wchar *word );
bool isDatatype( uint index );
// =============

// функция для использования | function for using
wchar *compiler( wchar *code ){
	Lines = 0;
	nowFunc[0] = 0;
	
	ToWord(code); // разбиваем код на слова, результат в 'words'
	
	
	
}



bool isDatatype( uint index ){ // проверка типа данных, если да - то заполняем nowDopDatatype, nowBaseDatatype и возращяем true
	// index - смещение в words
	// RETURN:
	// bool - было ли найден тип данных (базовый)
	// u - смещение
	
	u = index; // return
	
	nowDopDatatype = 0;
	
	bool wasDopDatatype = false;
	while(1){
		if( words[u].word[0] == L'\n' )
			Lines++;
		else if( cmpstr(words[u].word, L"unsigned") ){
			wasDopDatatype = true;
			nowDopDatatype |= dopDatatype_unsigned;
		} else if( cmpstr(words[u].word, L"register") ){
			wasDopDatatype = true;
			nowDopDatatype |= dopDatatype_register;
		} else if( cmpstr(words[u].word, L"extern") ){
			wasDopDatatype = true;
			nowDopDatatype |= dopDatatype_extern;
		} else if( cmpstr(words[u].word, L"stack") ){
			wasDopDatatype = true;
			nowDopDatatype |= dopDatatype_stack;
		} else
			break;
		u++;
	}
	
	bool wasBaseDatatype = false;
	while(1){
		if( words[u].word[0] == L'\n' )
			Lines++;
		else if( cmpstr(words[u].word, L"void") ){
			wasBaseDatatype = true;
			nowBaseDatatype = datatype_void;
		} else if( cmpstr(words[u].word, L"char") ){
			wasBaseDatatype = true;
			nowBaseDatatype = datatype_char;
		} else if( cmpstr(words[u].word, L"short") ){
			wasBaseDatatype = true;
			nowBaseDatatype = datatype_short;
		} else if( cmpstr(words[u].word, L"int") ){
			wasBaseDatatype = true;
			nowBaseDatatype = datatype_int;
		} else if( cmpstr(words[u].word, L"long") ){
			wasBaseDatatype = true;
			nowBaseDatatype = datatype_long;
		} else if( cmpstr(words[u].word, L"float") ){
			wasBaseDatatype = true;
			nowBaseDatatype = datatype_float;
		} else if( cmpstr(words[u].word, L"double") ){
			wasBaseDatatype = true;
			nowBaseDatatype = datatype_double;
		} else 
			break;
		u++;
	}
	
	if( wasDopDatatype == true && wasBaseDatatype == false ) errorCompiler( L"Invalid datatype" );
	if( (nowDopDatatype & dopDatatype_unsigned) > 0 && (nowBaseDatatype == datatype_float || nowBaseDatatype == datatype_double ) ) errorCompiler( L"You can't use 'unsigned float(double)'" );
	if( (nowDopDatatype & dopDatatype_stack) > 0 && (nowDopDatatype & dopDatatype_register) > 0 ) errorCompiler( L"I can't create register variable in stack" );
	
	if( wasDopDatatype == true || wasBaseDatatype == true ) return true;
	
	return false;
}
bool isNum( wchar *word ){ // проверяет строку - это число или чтото другое
	// "10", "1.5", "0x10", "10h", "'ABC'" - это число
	
	uint lenWord = lenstr(word);
	
	if(word[0] == L'-'){
		word++;
		lenWord--;
	}
	
	// "10" - simple int
	for( u=0; u<lenWord; u++)
		if( !(word[u] >= L'0' && word[u] <= L'9') ) break;
	if( u == lenWord ) return true;
	
	// "1.5" - float
	bool wasPoint = false; // была ли '.'
	for( u=0; u<lenWord; u++ ){
		if( word[u] == L'.' && wasPoint == false ){
			wasPoint = true;
			continue;
		}
		if( !(word[u] >= L'0' && word[u] <= L'9') ) break;
	}
	if( u == lenWord ) return true;
	
	// "0x1A" - hex
	if( word[0] == L'0' && word[1] == L'x' ){
		for( u=2; u<lenWord; u++)
			if( !(  (word[u] >= L'0' && word[u] <= L'9') ||
					(word[u] >= L'a' && word[u] <= L'f') ||
					(word[u] >= L'A' && word[u] <= L'F') ) ) break;
		if( u == lenWord ) return true;
	}
	
	// "1Ah" - hex2
	for( u=0; u<lenWord; u++)
		if( !(  (word[u] >= L'0' && word[u] <= L'9') ||
				(word[u] >= L'a' && word[u] <= L'f') ||
				(word[u] >= L'A' && word[u] <= L'F') ) ) break;
	if( u == lenWord-1 && word[lenWord-1] == L'h' ) return true;
	
	// "'ABC'" - symbols
	if( word[0]==L'\'' ){
		for( u=1; u<lenWord; u++)
			if( word[u] == L'\'' ) break;
		if( u+1 == lenWord ) return true;
	}
	
	return false;
}

// Перевод кода в набор слов
void addWord( wchar *word ){ // добавить слово в 'words'
	if( words == 0 )
		words = malloc( 21 * sizeof(struct word_st) );

	if( wordIndex >= MaxWordIndex ){
		MaxWordIndex += 20;
		words = realloc(words, (MaxWordIndex+1)*sizeof(struct word_st) );
	}
	
	if( lenstr(word) > MaxWordLen )
		error("The word is too long. This is a compiler issue - set 'MaxWordLen' greater than specified when compiling.");
	
	cpystr( words[wordIndex].word, word, 0 );
	wordIndex++;
}
void ToWord( wchar *code ){ // перпевод кода в набор слов, заполняет массив 'words'
	wchar *EndCode = code + lenstr(code);
	
	while(code < EndCode){
		
		if( code[0]==L' ' ){
			code++;
			continue;
		}
		if( code[0]==L'\n' ){
			code++;
			addWord(L"\n");
			continue;
		}
		
		// если строки то как единое слово
		if( code[0]==L'\"' ){
			for(i=1; code+i < EndCode; i++)
				if(code[i]==L'\"' && code[i-1]!=L'\\') break;
			i++;
			addWord( cpystrMem(code, i) );
			code += i;
			continue;
		}
		if( code[0]==L'\'' ){
			for(i=1; code+i < EndCode; i++)
				if(code[i]==L'\'' && code[i-1]!=L'\\') break;
			i++;
			addWord( cpystrMem(code, i) );
			code += i;
			continue;
		}
		
		// если встретили спец символ
		if( (code[0]>=L' ' && code[0]<=L'/') || (code[0]>=L':' && code[0]<=L'@') ||
			(code[0]>=L'[' && code[0]<=L'^') || (code[0]>=L'{' && code[0]<=L'~') || code[0]==L'`' ){
			
			addWord( cpystrMem(code, 1) );
			
			code++;
			continue;
		}
		
		
		for(i = 0; code+i < EndCode; i++) // get word
			if( (code[i]>=L' ' && code[i]<=L'/') || (code[i]>=L':' && code[i]<=L'@') ||
				(code[i]>=L'[' && code[i]<=L'^') || (code[i]>=L'{' && code[i]<=L'~') || code[i]==L'`' ) break;
		
		addWord( cpystrMem(code, i) );
		
		code += i;
	}
	
}