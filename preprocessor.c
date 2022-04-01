/*
 * Препроцессор для Си | Preprocessor for C
 * 
 * by illya334 (zeq52giw)
 * 
 * Habr: https://habr.com/ru/users/illya334/
 * 
 * Что в данный момент поддерживает (01.04.2020):
 * 		1. #include < > (" ")
 * 		2. #define (с аргументами)
 * 		3. #undef
 * 		4. #ifdef #ifndef
 * 
 * Делать #if пока не буду, так как эта команда не являеца часто используемой (лично для меня)
 * 
 * Функции я расположил по порядку:
 * 		1. Функции которые нужно подогнать к ОС\ПК.
 * 		2. Функции для использования (только алгоритм).
 * 		3. Функции для функций ниже (кроме одной: reload_preproc).
 * 		4. Функции для функций из 2 пункта.
 * 
 */

#include <stdio.h> // _wfopen, fgetwc, wprintf
#include <stdlib.h> // malloc, realloc
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


uint Lines = 1; // текущее положение строки.
wchar *StartCodeForError = 0; // начало кода, для инициализации функции ошибок.


// изменять эту функцию, остальные это алгоритм
void errorParser(wchar *code, wchar *errorText){ // вывод ошибок парсера, отображение ошибок не очень
	if(code==0 || errorText==0) exit(1);
	wprintf(L"ERROR Parser: %s\n\n", errorText);
	
	wchar *EndCode = StartCodeForError + lenstr(StartCodeForError);
	code = StartCodeForError;
	while(code < EndCode){ // получение адреса начала строки
		if(Lines==1) break;
		
		if(code[0]==L'\n')
			Lines--;
		
		code++;
	}
	if( code != EndCode ){
		uint i;
		for(i=0; code+i < EndCode; i++) // получение длины строки
			if( code[i] == L'\n' ) break;
		
		if( i <= 50 )
			wprintf( L"This:\n%s", cpystrMem( code, i ) );
		else { // иначе старый вариант
			if(lenstr(code) >= 15){
				code = cpystrMem(code, 0);
				code[15]=L'\0';
			}
			wprintf(L"Lines\t| Code\n");
			wprintf(L"%d\t| %s\n", Lines, code);
		}
		
		exit(1);
		
	}
	exit(1);
}


#define MaxDefNameLen 20 // максимальная длина имени директивы
#define MaxDefValueLen 200 // максимальная длина макроса, надо будет сделать отдельное выделение памяти под каждую строку
#define MaxDefArgs 10 // максимум аргументов для директив
#define MaxDefArgLen 20 // максимальная длина аргумента (размер в момент инициализации)

struct def{ // структура макросов
	wchar name[MaxDefNameLen]; // имя макроса
	wchar value[MaxDefValueLen]; // значение (необязательно)
	wchar args[MaxDefArgs][MaxDefArgLen]; // аргументы (если есть)
	byte indexArgs; // количество аргументов
};
struct def *defines = 0; // здесь храняца макросы
uint defIndex = 0,
	 MaxDefIndex = 20; // сколько макросов можно записать на выделеную память

uint i = 0, // временые переменые
	 j = 0,
	 h = 0,
	 u = 0,
	 g = 0;

wchar *OutCode = 0; // Масив для хранения результата
uint OutCodeIndex = 0,
	 OutCodeMaxLen = 0;

wchar *buf = 0; // Временый буфер, в нем храница строка для обработки. Точнее сюда она копируеца.
uint BufMaxLen = 0;

// Для preproc_definePush
struct InArgs_st{
	wchar *arg;
	uint maxLen;
};
struct InArgs_st InArgs[MaxDefArgs];
bool InArgsWasInit = false;
// =======================

wchar *tmp; // временый адрес

// =========== REF ===============
uint preproc_include(wchar *code);
uint preproc_define(wchar *code);
uint preproc_definePush(wchar *code);
uint preproc_undef(wchar *code);
uint preproc_ifdef(wchar *code);
uint preproc_if(wchar *code); // не реализовано, don't work

void reload_preproc(wchar **code, wchar **EndCode);
// ===============================


wchar *preprocessor(wchar *code){
	
	/*
	 * Это главная функция препроцессора.
	 * 
	 * Что делает:
	 * 		1. уничтожает коментарии
	 * 		2. обработка команд
	 * 
	 * Возращает wchar строку результата.
	 */
	
	if(code[0]==0 || code==0) error("'preprocessor' get zero."); // заглужка, 'error' from main.c
	Lines = 1;
	StartCodeForError = code; // инициализируем errorParser
	
	code = formatStr(code); // форматируем строку: уберает \t, много пробелов (оставляет 1 пробел между словами)
	
	dynamic_array_process( &buf, lenstr(code), &BufMaxLen, sizeof(wchar) ); // обработка масива: если не существует - создать, если мало места то разщиряем.
	code = cpystr(buf, code, 0);
	
	wchar *EndCode = code + lenstr(code);
	
	if( InArgsWasInit == false ){ // инициализация InArgs
		InArgsWasInit = true;
		for(i=0; i < MaxDefArgs; i++){
			InArgs[i].arg = malloc( 20 * sizeof(wchar) );
			InArgs[i].maxLen = 20;
		}
	}
	
	while( code < EndCode ){
		
		dynamic_array_process( &defines, defIndex, &MaxDefIndex, sizeof(struct def) );
		dynamic_array_process( &OutCode, OutCodeIndex, &OutCodeMaxLen, sizeof(wchar) );
		
		if( code[0] == L'\n' ) Lines++;
		
		// игнор коментариев
		if( code[0] == L'/' && code[1] == L'/' ){ // однострочный коментарий
			for(i=0; code < EndCode; i++)
				if(code[i]==L'\n') break;
			code += ++i;
			continue;
		}
		if( code[0] == L'/' && code[1] == L'*' ){ // много строчный коментарий
			for(i=0; code < EndCode; i++)
				if(code[i] == L'*' && code[i+1] == L'/') break;
			code += i+2;
			continue;
		}
		
		// игнор строк
		if( code[0] == L'\"' ){
			for(i=1; code+i<EndCode; i++)
				if( code[i] == L'\"' ) break;
			i++;
			dynamic_array_process( &OutCode, OutCodeIndex + i, &OutCodeMaxLen, sizeof(wchar) );
			cpystr( OutCode+OutCodeIndex, code, i );
			OutCodeIndex += i;
			code += i;
			continue;
		}
		if( code[0] == L'\'' ){
			for(i=1; code+i<EndCode; i++)
				if( code[i] == L'\'' ) break;
			i++;
			dynamic_array_process( &OutCode, OutCodeIndex + i, &OutCodeMaxLen, sizeof(wchar) );
			cpystr( OutCode+OutCodeIndex, code, i );
			OutCodeIndex += i;
			code += i;
			continue;
		}
		
		if( code[0] == L'#' ){ // обработка команд
			code++;
			
			for(i=0; code+i < EndCode; i++) // получении имени команды, i - длина
				if( !( code[i]>=L'a' && code[i]<=L'z' ) ) break;
			
			if( i > 0 ){
				
				tmp = cpystrMem(code, i); // tmp - имя команды
				code += i;
			
				if( cmpstr(tmp, L"include") ){
					code += preproc_include(code);
					
					reload_preproc(&code, &EndCode);
					continue;
				} else if( cmpstr(tmp, L"define") )
					code += preproc_define(code);
				else if( cmpstr(tmp, L"undef") )
					code += preproc_undef(code);
				else if( cmpstr(tmp, L"ifdef") || cmpstr(tmp, L"ifndef") ){
					code += preproc_ifdef(code);
					
					reload_preproc(&code, &EndCode);
					continue;
				}
				//else if( cmpstr(tmp, L"if") ) // пока не буду делать
					//code += preproc_if(code);
				else
					errorParser(code, L"Invalid preprocessor command.");
			}
			continue;
		}
		
		// проверка имен макросов и запись их
		i = preproc_definePush(code);
		if( i > 0 ){
			code += i;
			reload_preproc(&code, &EndCode);
			continue;
		}
		
		OutCode[OutCodeIndex] = code[0]; // запись символа в выход
		OutCodeIndex++;
		code++;
	}
	
	OutCode[OutCodeIndex] = 0; // На всякий случай
    return OutCode;
}


// Функции для сокращения кода
void reload_preproc(wchar **code, wchar **EndCode){ // требуеца перезапустить парсер с самого начала, не обрабатывая код дальше, чтобы небыло конфликта с другими директивами
	tmp = addstr( OutCode, *code );
	OutCodeIndex = 0;
	
	dynamic_array_process( &buf, lenstr(tmp), &BufMaxLen, sizeof(wchar) );
	
	cpystr( buf, tmp, 0 );
	*code = buf;
	*EndCode = *code + lenstr(*code);
	Lines = 1;
	StartCodeForError = *code;
	
	setmem(OutCode, OutCodeMaxLen, 0);
}
wchar *getPreprocName(wchar *code){ // получает имя после команды препроцессора: #define, #ifdef, ...
	// возращает обработаную строку и смещение (i)
	
	for(j=0; (j < lenstr(code) && code[j]!=0); j++) // игнорируем пробелы после слова
		if(code[j]!=L' ') break;
	code += j;
	
	for(i=0; (i < lenstr(code) && code[i]!=0); i++) // Get name
		if(	(code[i]>=L' ' && code[i]<=L'/') || (code[i]>=L':' && code[i]<=L'@') ||
			(code[i]>=L'[' && code[i]<=L'^') || (code[i]>=L'{' && code[i]<=L'~') ||
			 code[i]==L'`' || code[i]==L'\t' || code[i]==L'\n') break;
	code = cpystrMem(code, i);
	i += j;
	return code;
}
bool cmpformat(wchar *str){ // проверка имени на правильность, не должно быть любых знаков кроме букв и _
	// судя по даблице Unicode (https://en.wikipedia.org/wiki/List_of_Unicode_characters#Basic_Latin)
	
	if( str[0]>=L'0' && str[0]<=L'9' ) return false;
	for(uint i=0; i < lenstr(str); i++)
		if(	(str[i]>=L' ' && str[i]<=L'/') || (str[i]>=L':' && str[i]<=L'@') ||
			(str[i]>=L'[' && str[i]<=L'^') || (str[i]>=L'{' && str[i]<=L'~') ||
			str[i]==L'`' || str[i]==L'\t') return false;
	return true;
}
uint _if_loop(wchar *code){ // обработка if
	// задача: вернуть размер значения (от #if/#ifdef до #else/#endif)
	
	// функция использует i, по этому часто я использую i, вместо присваивания значения к переменной
	
	u = 0;
	j = 1; // один if мы обнаружили (его мы обрабатываем)
	for(i=0; (i < lenstr(code) && code[i]!=0); i++){
		if(code[i]==L'\'' && u==0){ u = 1; continue; }
		if(code[i]==L'\"' && u==0){ u = 2; continue; }
		if( (code[i]==L'\'' && u==1 ) || (code[i]==L'\"' && u==2 ) ){ u=0; continue; }
					
		if(cmpstr(code+i, L"#if") || cmpstr(code+i, L"#ifdef") || cmpstr(code+i, L"#ifndef")){ j++; continue; }
		if(cmpstr(code+i, L"#endif") && j>1){ j--; continue; }
		
		if((cmpstr(code+i, L"#endif") || cmpstr(code+i, L"#else")) && j<=1) break;
	}
	return i;
}
bool ifdefined(wchar *name){ // была ли создана константа?
	for(uint u=0; u<defIndex; u++)
		if(cmpstr(defines[u].name, name)) return true;
	return false;
}
bool cmpif(wchar *cmp){ // обработка условий if...
	// самое сложное в препроцесоре
	// это заготовка.
	
	/*
	 * cmp - строка с условиями типа ( ( ABC > 5 ) || ( 1 == 1 && 2 < 5 ) )
	 * 
	 * задача этой функции вернуть true/false
	 */
}


// функции обработки команд препроцесора
uint preproc_include(wchar *code){
	uint lenCode = lenstr(code);
	wchar *EndCode = code + lenCode;
	wchar *startCode = code;
	
	for(i=0; (i < EndCode && code[i] != 0); i++) // игнорируем пробелы после слова
		if(code[i]!=L' ') break;
	code += i;
			
	if(code[0]==L'<' || code[0]==L'\"'){ // у данного компилятора нет встроеных библиотек, так что < > и " " работают одинаково
				
		if(code[0]==L'\"') j=1; else j=0;
		code++;
		for(i=0; (i+code<EndCode && code[i]!=0); i++){ // get path
			if(code[i]==L'\"' && j==0){ j=1; continue; }
			if(code[i]==L'\'' && j==0){ j=2; continue; }
			if( (code[i]==L'\"' && j==1) || (code[i]==L'\'' && j==2) ) j=0;
			if( (code[i]==L'>' || code[i]==L'\"') && j==0 ) break;
			if( code[i]==L'\n' ) break;
		}
		
		reg wchar *tmp2 = tmp = cpystrMem(code, i); // tmp - путь к файлу
		
		tmp = openfile(tmp, true); // функция в main.c, tmp - содержимое файла
		if(tmp==0) errorParser(code, L"I can't open the file. Invalid path/name.");

		code += ++i; // перепрыгивает path
		
		// записываем содержимое файла
		i = lenstr(tmp);
		dynamic_array_process( &OutCode, OutCodeIndex+i, &OutCodeMaxLen, sizeof(wchar) );
		cpystr(OutCode+OutCodeIndex, tmp, i);
		OutCodeIndex += i;
		
		OutCode[ OutCodeIndex ] = 0;
		
	} else
		errorParser(code, L"I wait '<' or '\"' after '#include'.");
		
	return (uint)(code - startCode);
}
uint preproc_define(wchar *code){
	#define nowDef defines[defIndex]
	
	wchar *startCode = code;
	wchar *EndCode = code + lenstr(code);
	
	tmp = getPreprocName(code); // tmp - имя макроса, i - длина
	code += i;
	
	if( cmpformat(tmp) == false ) // Неправильное (возможно) имя для директивы
		errorParser(code-i, addstr( L"Invalid name for define: ", tmp ));
	
	if( lenstr(tmp) > MaxDefNameLen )
		errorParser(code-i, addstr( L"Name too big for define: ", tmp ));
	
	cpystr( nowDef.name, tmp, 0 );
	
	nowDef.indexArgs = 0;
	
	if(code[0]==L'('){ // есть ли аргументы
		code++;
		
		bool WasNameArg = false;
		while(code<EndCode && code[0]!=0){
			if(nowDef.indexArgs > MaxDefArgs)
				errorParser(code-i, addstr( L"Too much argumets for define: ", tmp ));
			
			if(code[0]==L','){
				if(WasNameArg == false)
					errorParser(code-i, addstr( L"Invalid argument for define: ", nowDef.name ));
				
				code++;
				WasNameArg = false;
				continue;
			}
			WasNameArg = false;
			
			if(code[0]==L' '){ code++; continue; }
			if(code[0]==L'\n') break;
			if(code[0]==L')'){ code++; break; }
			
			for(i=0; (i+code < EndCode && code[i]!=0); i++)
				if(code[i]==0 || code[i]==L',' || code[i]==L')') break;
			tmp = cpystrMem(code, i);
			code += i;
			tmp = formatStr(tmp);
			
			if( cmpformat(tmp) == false || lenstr(tmp) == 0 ) // Неправильное (возможно) имя аргумента для директивы
				errorParser(code-i, addstr( L"Invalid argument name for define: ", nowDef.name ));
			
			if( lenstr(tmp) > MaxDefArgLen )
				errorParser(code-i, addstr( L"Name too big for define: ", tmp ));
			
			cpystr( nowDef.args[nowDef.indexArgs], tmp, 0 );
			nowDef.indexArgs++;
			WasNameArg = true;
		}
	}
	
	if(code[0]==L'\n'){ nowDef.value[0] = 0; defIndex++; goto preproc_define_end_label; }
	
	for(i=0; (i+code<EndCode && code[i]!=0); i++) // Get value
		if(code[i]==0 || code[i]==L'\n') break;
	
	tmp = cpystrMem(code, i); // tmp - value, i - length value
	code += i;
	
	tmp = formatStr(tmp); // убераем пробелы и табуляцию
	
	if(lenstr(tmp)==0)
		nowDef.value[0] = 0;
	else
		if( lenstr(tmp) > MaxDefValueLen )
			errorParser(code-i, addstr( L"Value too big for define: ", tmp ));
		else
			cpystr( nowDef.value, tmp, 0 );
	
	defIndex++;
				
	#undef nowDef // чтоб глаза не мазолил
	
	preproc_define_end_label:
	return (uint)(code - startCode);
}
uint preproc_definePush(wchar *code){ // вставка макросов в код
	wchar *startCode = code;
	wchar *EndCode = code + lenstr(code);

	#define nowDef defines[i] // легче читаеца
	for(i=0; i<defIndex; i++){
		if( cmpstr(code, nowDef.name) ){
		
			// перепрыгиваем имя макроса
			code += lenstr(nowDef.name);
					
			if( nowDef.value[0] == 0 ) // может ли данный макрос использоваца?
				errorParser(code-lenstr(nowDef.name),
					addstr( L"This define cannot be used! Since it has no value. Name define: ", nowDef.name ));
					
			if( code[0] == L'(' ){ // вводит ли пользователь аргументы?
				code++;
				if( nowDef.args[0][0] == 0 )
					errorParser(code-lenstr(nowDef.name),
						addstr( L"This define does not support arguments. Name define: ", nowDef.name ));
				
				// получение вводимых аргументов
				#define nowInArgs InArgs[u]
				u = 0; // текуший индекс аргумета
				while(code < EndCode){
					
					h = 0;
					for(j=0; code+j < EndCode; j++){ // получаем аргумент который нужно вписать
						if( code[j] == L'(' ) h++; else
						if( code[j] == L')' && h != 0 ) h--; else
						if( h == 0 && ( code[j] == L',' || code[j] == L')' ) ) break;
					}
					
					tmp = formatStr( cpystrMem( code, j ) );
					
					if( lenstr(tmp) == 0 )
						errorParser(code-lenstr(nowDef.name),
							addstr( L"Invalid argument for define: ", nowDef.name ));
					
					dynamic_array_process( &(nowInArgs.arg), j, &(nowInArgs.maxLen), sizeof(wchar) );
					cpystr( nowInArgs.arg, tmp, 0 );
					code += j;
					
					u++;
					
					if( u > MaxDefArgs || u > nowDef.indexArgs )
						errorParser(code-lenstr(nowDef.name),
							addstr( L"Too much argumets for define: ", nowDef.name ));
					
					if( code[0]==L',' ) code++; else
					if( code[0]==L')' ){ code++; break; }
				}
				#undef nowInArgs
				
				
				// вставка в код этих аргументов
				#define nowDefValue nowDef.value
				for(j=0; j < lenstr(nowDefValue); j++){
					for(g=0; g < nowDef.indexArgs; g++){
						if( cmpstr( nowDefValue+j, nowDef.args[g] ) ){
							reg tmpLen = lenstr( InArgs[g].arg );
							dynamic_array_process( &OutCode, OutCodeIndex + tmpLen, &OutCodeMaxLen, sizeof(wchar) );
							cpystr( OutCode + OutCodeIndex, InArgs[g].arg, 0 );
							j += lenstr( nowDef.args[g] ); // сдвигаем текст на длину аргумента
							OutCodeIndex += tmpLen;
						}
					}
					dynamic_array_process( &OutCode, OutCodeIndex + 1, &OutCodeMaxLen, sizeof(wchar) );
					OutCode[OutCodeIndex] = nowDefValue[j];
					OutCodeIndex++;
				}
				#undef nowDefValue
				
			} else { // Макрос без аргументов
				reg uint tmpLenValue = lenstr(nowDef.value); // длина value
				dynamic_array_process( &OutCode, OutCodeIndex + tmpLenValue, &OutCodeMaxLen, sizeof(wchar) ); // сможет ли влезть value, если нет то разщиряем
				cpystr( OutCode + OutCodeIndex, nowDef.value, 0 ); // копируем value в OutCode
				OutCodeIndex += tmpLenValue;
			}
		
			break;
		}
	}
	#undef nowDef
	
	return (uint)(code - startCode);
}
uint preproc_undef(wchar *code){
	wchar *startCode = code;
	
	tmp = getPreprocName(code); // tmp - define name
	code += i;
	
	for(i=0; i < defIndex; i++){ // если есть такое имя то уничтожаем
		if( cmpstr(tmp, defines[i].name) ){
			defines[i].name[0] = 0;
			defines[i].value[0] = 0;
			for(j = 0; j < MaxDefArgs; j++) defines[i].args[j][0] = 0;
			defines[i].indexArgs = 0;
		}
	}
	
	return (uint)(code - startCode);
}
uint preproc_ifdef(wchar *code){
	
	wchar *startCode = code;
	wchar *EndCode = code + lenstr(code);
	
	if( cmpstr(tmp, L"ifndef") ) h = 1; else h = 0;
	
	tmp = getPreprocName(code); // Get name define
	code += i;
	
	_if_loop(code); // вернул в i
				
	if( ( ifdefined(tmp) && h==0 ) || ( !ifdefined(tmp) && h==1 ) ){ // if (not) defined
		if(i>0){ // копируем все содержимое #ifdef...#else or #ifdef...#endif
			dynamic_array_process( &OutCode, OutCodeIndex+i, &OutCodeMaxLen, sizeof(wchar) );
			cpystr( OutCode + OutCodeIndex, code, i );
			OutCodeIndex += i;
			
			code += i;
		}
		
		if(code >= EndCode) goto preproc_if_end_label;
		
		if(cmpstr(code, L"#else")){ // игнорируем "#else"
			code += 5;
			_if_loop(code); // вернул значение в i
			code += i;
		}
		
	} else { // #else - обработка
		code += i;
		
		if(code >= EndCode) goto preproc_if_end_label;
		
		if(cmpstr(code, L"#else")){
			code += 5;
			_if_loop(code); // вернул значение в i
			
			if(i > 0){ // копируем содержимое #else...#endif
				dynamic_array_process( &OutCode, OutCodeIndex+i, &OutCodeMaxLen, sizeof(wchar) );
				cpystr( OutCode + OutCodeIndex, code, i );
				OutCodeIndex += i;
				
				code += i;
			}
		}
		
	}
	
	if( cmpstr(code, L"#endif") ) code += 6; else
		errorParser(code, L"I wait '#endif' after '#ifdef' or '#else'");
	
	
	preproc_if_end_label:
	return (uint)(code - startCode);
}
uint preproc_if(wchar *code){ // не реализовано, don't work
	
	// code copyed from preproc_ifdef
	
	wchar *startCode = code;
	wchar *EndCode = code + lenstr(code);
	
	for(i=0; code+i<EndCode; i++)
		if(code[i]==L'\n') break;
	
	tmp = cpystrMem(code, i);
	code += i;
	
	_if_loop(code); // вернул в i
	
	if( cmpif(tmp) ){
		if(i>0){ // копируем все содержимое if
			dynamic_array_process( &OutCode, OutCodeIndex+i, &OutCodeMaxLen, sizeof(wchar) );
			cpystr( OutCode + OutCodeIndex, code, i );
			OutCodeIndex += i;
			
			code += i;
		}
		
		if(code >= EndCode) goto preproc_if_end_label;
		
		if(cmpstr(code, L"#else")){ // code == "#else"
			code += 5; // игнорируем "#else"
			_if_loop(code); // вернул значение в i
			code += i;
		}
		
	} else { // #else - обработка
		code += i;
		
		if(code >= EndCode) goto preproc_if_end_label;
		
		if(cmpstr(code, L"#else")){ // code == "#else"
			code += 5;
			_if_loop(code); // вернул значение в i
			
			if(i > 0){
				dynamic_array_process( &OutCode, OutCodeIndex+i, &OutCodeMaxLen, sizeof(wchar) );
				cpystr( OutCode + OutCodeIndex, code, i );
				OutCodeIndex += i;
				
				code += i;
			}
		}
	}
	preproc_if_end_label:
	return (uint)(code - startCode);
}