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

// изменять эту функцию, остальные это алгоритм
void errorCompiler(){
	
}

#define MaxWordLen 30
struct word_st{
	wchar word[MaxWordLen+1];
};
struct word_st *words = 0; // здесь будут храница слова, после выполнения 'ToWord'
uint wordIndex = 0,
	 MaxWordIndex = 20;
	 
uint i, j, h, u, g; // временые переменые

wchar *tmp;

enum{ // тип объекта команды
	Object_NONE = 0,
	Object_var = 1,
	Object_new_var,
	Object_struct,
	Object_new_Func,
	Object_call_Func,
	
	Object_if,
	Object_else,
	Object_for,
	Object_while,
	Object_switch,
	Object_case,
	Object_enum,
	Object_goto,
	Object_return,
	
	Object_setSym, // = += -= *= /= & | >> <<
	Object_logicSym, // == && || > < <= >=
	Object_NOT // !
};

struct object_st{ // объект команды
	
}

// ==== REF ====
void ToWord( wchar *code );

// =============

// функция для использования
wchar *compiler( wchar *code ){
	
	ToWord(code); // разбиваем код на слова, результат в 'words'
	
	
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
			addWord(L"\\n"); // TO DO
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