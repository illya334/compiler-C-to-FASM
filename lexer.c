/*
 * LEXER - завдання його, розбити код на складові, які називають токен(и),
 * 			після спробувати перетворити в AST (дерево інтсрукцій).
 * 
 * Можливо він ще буде обробляти препоцесор, бо я не знаю як зробити його окремо.
 * 
 * by illya334 (zeq52giw)
 * 
 * Habr: https://habr.com/ru/users/illya334/
 * GitHub: https://github.com/illya334/compiler-C-to-FASM
 * 
*/

#include <stdio.h> // wprintf
#include <wchar.h> // wchar_t
#include <stdint.h> // uint8_t
// #include "wstring.c" // dynamic_array_process, lenstr, cpystr, cpystrMem, cmpstr, addstr, formatStr

#define wchar wchar_t
#define uint unsigned int
#define reg register
#define bool uint8_t
#define byte uint8_t

#define true 1
#define false 0

uint i, j, h, u, g; // тимчасові змінні

uint LINES = 0; // для зрозумілого вивода помилок
uint COLM = 0;
wchar *fileName = 0;

void errorLexer( wchar *errorText ){
	wprintf( L"In File: \"%s\"\n"
			 L"Lexer error %d:%d - %s\n\n", fileName, LINES, COLM, errorText );
	exit(1);
}


// Local variables \ structs

struct st_token{
	byte type;
	byte datatype;
	wchar *id; // адреса на ім'я об'єкта
	
	// для кращого виведення помилок
	wchar *fileName; // адреса на ім'я
	uint line;
	uint col;
};

enum{ // варіації 'type', тут по ідеї говориться, що це за 'token', та що це за команда.

	TYPE_NONE = 0,
	
	TYPE_NEWFUNC = 1,
	TYPE_NEWVAR,
	TYPE_VAR,
	
	TYPE_RETURN, 	// return
	
	TYPE_NUMBER, 	// any number: 123, 0.5, 0xA, 0Ah, 'ABC'
	
	TYPE_ENDCMD, 	// ;
	TYPE_EQU, 		// =
	TYPE_COMMA, 	// ,
	TYPE_OPENPARENTHESIS, 	// (
	TYPE_CLOSEPARENTHESIS, 	// )
	TYPE_OPENBRACE, // {
	TYPE_CLOSEBRACE,// }
	
};
enum{ // тип даних, включаючи: unsigned, register, stack
	// перші 4 біти це тип даних.
	// наступні 4 біти, це логічні біти: unsigned, register, stack (локальні змані виділена у стеку).
	
	DATATYPE_NONE = 0,
	DATATYPE_VOID = 1,
	DATATYPE_CHAR,
	DATATYPE_SHORT,
	DATATYPE_INT,
	DATATYPE_LONG,
	DATATYPE_FLOAT,
	DATATYPE_DOUBLE,
	
	DATATYPE_INT8,  // int8_t
	DATATYPE_INT16, // int16_t
	DATATYPE_INT32, // int32_t
	DATATYPE_INT64, // int64_t
	// 'uint*_t' - не буде створені, буде змінений у 'unsigned int*_t'
	// now 11/15 (4 bits)
	
	
	// NEXT 4 bits
	DATATYPE_UNSIGNED = 0x10, // 5 bit
	DATATYPE_REGISTER = 0x20, // 6 bit
	DATATYPE_STACK 	  = 0x40  // 7 bit
	
	// free 1 bit
};

struct st_token *tokens = 0; // масив токенів
uint TokensIndex = 0,
	 TokensMaxIndex = 0;

wchar *NowWord = 0; // поточне слово
uint NowWordLen = 0,
	 nowGCI = 0;

wchar *NextWord = 0; // наступне слово
uint NextWordLen = 0,
	 nextGCI = 0;

byte NowDatatype = 0;

wchar *GlobalCode = 0; // Змінні для всіх функцій нижче.
uint GlobalCodeIndex = 0;
uint GlobalCodeMaxIndex = 0;

#define GCI GlobalCodeIndex
#define GCMI GlobalCodeMaxIndex
#define GC GlobalCode

// End Local variables \ structs \ constants


// === REF ===
void getword();
void doDatatype();
void doVar();
// = END REF =


// For use
struct st_token *lexer( wchar *code ){ // перетворює код в набір об'єктів
	GlobalCodeMaxIndex = lenstr(code);
	GlobalCode = code;
	
	fileName = L"main.c"; // тимчасова заглушка
	
	while(GlobalCodeIndex < GlobalCodeMaxIndex){
		getword(); // Зчитує поточне слово та записує у 'NowWord'
		
		doDatatype(); // Записує поточний тип данних, якщо є.
		
		
	}
}


// local funcs\procs for 'getword'
void _setWord( wchar *word, bool type ){ // Not Use
	if(type == false){
		dynamic_array_process( &NowWord, lenstr(word), &NowWordLen, sizeof(wchar) );
		cpystr( NowWord, word, 0 );
	} else {
		dynamic_array_process( &NextWord, lenstr(word), &NextWordLen, sizeof(wchar) );
		cpystr( NextWord, word, 0 );
	}
}
void __getNowOrNextWord( bool type ){ // if (type = 1) then is 'NextWord' (Not Use)
	while(GCI < GCMI){
		
		if( GC[GCI]==L' ' ){
			GCI++;
			continue;
		}
		if( GC[GCI]==L'\n' ){
			GCI++;
			continue;
		}
		
		// якщо рядок то як ціле слово.
		if( GC[GCI]==L'\"' ){
			for(i=GCI+1; GCI+i < GCMI; i++)
				if(GC[i]==L'\"' && GC[i-1]!=L'\\') break;
			i++;
			_setWord( cpystrMem(GC+GCI, i), type );
			GCI += i;
			break;
		}
		if( GC[GCI]==L'\'' ){
			for(i=GCI+1; GCI+i < GCMI; i++)
				if(GC[i]==L'\'' && GC[i-1]!=L'\\') break;
			i++;
			_setWord( cpystrMem(GC+GCI, i), type );
			GCI += i;
			break;
		}
		
		// якщо зустрів спец. символ то записує як ціле слово.
		if( (GC[GCI]>=L' ' && GC[GCI]<=L'/') || (GC[GCI]>=L':' && GC[GCI]<=L'@') ||
			(GC[GCI]>=L'[' && GC[GCI]<=L'^') || (GC[GCI]>=L'{' && GC[GCI]<=L'~') || GC[GCI]==L'`' ){
			
			_setWord( cpystrMem(GC+GCI, 1), type );
			
			GCI++;
			break;
		}
		
		
		for(i=GCI; GCI+i < GCMI; i++) // get word
			if( (GC[i]>=L' ' && GC[i]<=L'/') || (GC[i]>=L':' && GC[i]<=L'@') ||
				(GC[i]>=L'[' && GC[i]<=L'^') || (GC[i]>=L'{' && GC[i]<=L'~') || GC[i]==L'`' ) break;
		
		_setWord( cpystrMem(GC+GCI, i-GCI), type );
		
		GCI = i;
		break;
	}
}

void getword(){ // Записує поточне\наступне слово в 'NowWord' (For Use)
	
	if( NextWord == 0 ){
		__getNowOrNextWord( false ); // nowWord
		nowGCI = GCI;
		__getNowOrNextWord( true ); // nextWord
		nextGCI = GCI;
	} else {
		nowGCI = nextGCI;
		
		dynamic_array_process( &NowWord, NextWordLen, &NowWordLen, sizeof(wchar) );
		cpystr( NowWord, NextWord, 0 );
		
		__getNowOrNextWord( true ); // nextWord
		nextGCI = GCI;
		
	}
	
	COLM = 0;
	LINES = 0;
	for( i=0; i<nowGCI; i++ ){ // get LINES and COLM for errorLexer
		if(GC[i]==L'\n'){
			LINES++;
			COLM = 0;
		} else 
			COLM++;
	}
	
}


// funcs/procs for 'doVar'
void createToken(byte type, wchar *id){
	dynamic_array_process( &tokens, TokensIndex, &TokensMaxIndex, sizeof(struct st_token) );
	
	tokens[TokensIndex].type = type;
	tokens[TokensIndex].datatype = NowDatatype;
	tokens[TokensIndex].id = id;
	
	tokens[TokensIndex].fileName = fileName;
	tokens[TokensIndex].line = LINES;
	tokens[TokensIndex].col = COLM;
	
	TokensIndex++;
}

void doDatatype(){
	NowDatatype = 0;
	
	while( GCI < GCMI ){ // get datatype
		if( cmpstr(NowWord, L"unsigned") ) 	NowDatatype |= DATATYPE_UNSIGNED; 	else
		if( cmpstr(NowWord, L"register") ) 	NowDatatype |= DATATYPE_REGISTER; 	else
		if( cmpstr(NowWord, L"stack") ) 	NowDatatype |= DATATYPE_STACK; 		else
		
		if( cmpstr(NowWord, L"void") ) 		NowDatatype |= DATATYPE_VOID; 		else
		if( cmpstr(NowWord, L"char") ) 		NowDatatype |= DATATYPE_CHAR; 		else
		if( cmpstr(NowWord, L"short") ) 	NowDatatype |= DATATYPE_SHORT; 		else
		if( cmpstr(NowWord, L"int") ) 		NowDatatype |= DATATYPE_INT; 		else
		if( cmpstr(NowWord, L"long") ) 		NowDatatype |= DATATYPE_LONG; 		else
		if( cmpstr(NowWord, L"float") ) 	NowDatatype |= DATATYPE_FLOAT; 		else
		if( cmpstr(NowWord, L"double") ) 	NowDatatype |= DATATYPE_DOUBLE; 	else
		break;
			
		getword();
	}
	
	// перевірка типу даних на правильність
	
	// unsigned + float/double
	if( NowDatatype & DATATYPE_UNSIGNED && ( (NowDatatype & 0xf) == DATATYPE_FLOAT || (NowDatatype & 0xf) == DATATYPE_DOUBLE ) )
		errorLexer( L"float|double can't be 'unsigned': \"unsigned float|double\"." );
	
	// register + float/double
	if( NowDatatype & DATATYPE_REGISTER && ( (NowDatatype & 0xf) == DATATYPE_FLOAT || (NowDatatype & 0xf) == DATATYPE_DOUBLE ) )
		errorLexer( L"float|double can't be 'register': \"register float/double\"." );
	
	// register + stack
	if( NowDatatype & DATATYPE_REGISTER && NowDatatype & DATATYPE_STACK )
		errorLexer( L"data types 'register' and 'stack' cannot be used at the same time: \"register stack\"." );
	
}
void doVar(){
	if( NowDatatype != 0 ){ // maybe created variable
		createToken( TYPE_NEWVAR, createString(NowWord) );
	}
}