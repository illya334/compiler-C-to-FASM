#include <stdio.h> // _wfopen, fgetwc
#include <stdlib.h> // malloc, realloc
#include <wchar.h> // wchar_t
#include <stdint.h> // uint8_t

#define wchar wchar_t
#define uint unsigned int
#define reg register
#define bool uint8_t
#define byte uint8_t

#define true 1
#define false 0

uint Lines = 0;

void errorParser(wchar *code, wchar *errorText){ // вывод ошибок парсера, отображение ошибок не очень
	if(code==0 || errorText==0) exit(1);
	wprintf(L"ERROR Parser: %s\n", errorText);
	if(lenstr(code) >= 15){
		code = cpystrMem(code, 0);
		code[15]=L'\0';
	}
	wprintf(L"Lines\t| Code\n");
	wprintf(L"%d\t| %s\n", Lines, code);
	exit(1);
}

#define MaxDefNameLen 20 // максимальная длина имени директивы
#define MaxDefValueLen 100 // максимальная длина макроса
#define MaxDefArgs 10 // максимум аргументов для директив
#define MaxDefArgLen 20 // максимальная длина аргумента (размер в момент инициализации)

struct def{ // структура макросов
	wchar name[MaxDefNameLen]; // имя макроса
	wchar value[MaxDefValueLen]; // значение (необязательно)
	wchar args[MaxDefArgs][MaxDefArgLen]; // аргументы (если есть)
	byte indexArgs; // количество аргументов
};

struct def *defines = 0;
uint defIndex = 0,
	 MaxDefIndex = 20;

uint i = 0, // временые переменые, для циклов
	 j = 0,
	 h = 0,
	 u = 0;

wchar *OutCode = 0; // Масив для хранения результата
uint OutCodeIndex = 0,
	 OutCodeMaxLen = 0;

wchar *buf = 0; // Временый буфер
uint BufIndex = 0,
	 BufMaxLen = 0;

wchar *tmp; // временый адресс

// =========== REF ===============
wchar *getPreprocName(wchar *code);

uint preproc_include(wchar *code);
uint preproc_define(wchar *code);
uint preproc_undef(wchar *code);
uint preproc_ifdef(wchar *code);
uint preproc_if(wchar *code);
// ===============================

wchar *_preproc(wchar *out, wchar *code){
	if(code[0]==0 || code==0) error("'_preproc' get zero."); // заглужка
	Lines = 0;
	
	code = formatStr(code);
	
	OutCodeMaxLen = BufMaxLen = lenstr(code);
	
	dynamic_array_process( &buf, BufIndex, &BufMaxLen, sizeof(wchar) );
	code = cpystr(buf, code, 0);
	
	wchar *EndCode = code + lenstr(code);
	
	while( code < EndCode ){
		
		dynamic_array_process( &defines, defIndex, &MaxDefIndex, sizeof(struct def) );
		
		dynamic_array_process( &OutCode, OutCodeIndex, &OutCodeMaxLen, sizeof(wchar) );
		
		if( code[0] == L'\n' ) Lines++;
		
		if( code[0] == L'/' && code[1] == L'/' ){ // однострочный коментарий
			for(i=0; code < EndCode; i++)
				if(code[i]==L'\n') break;
			code += i;
			continue;
		}
		
		if( code[0] == L'/' && code[1] == L'*' ){ // много строчный коментарий
			for(i=0; code < EndCode; i++)
				if(code[i] == L'*' && code[i+1] == L'/') break;
			code += i;
			continue;
		}
		
		if( code[0] == L'#' ){ // обработка команд
			code++;
			
			for(i=0; code+i < EndCode; i++)
				if( !( code[i]>=L'a' && code[i]<=L'z' ) ) break;
			
			if( i > 0 ){
				
				tmp = cpystrMem(code, i);
				code += i;
			
				if( cmpstr(tmp, L"include") ) // #include < somecode.c >
					code += preproc_include(code);
				else if( cmpstr(tmp, L"define") ) // #define ABC 123
					code += preproc_define(code);
				else if( cmpstr(tmp, L"undef") )  // #undef ABC
					code += preproc_undef(code);
				else if( cmpstr(tmp, L"ifdef") || cmpstr(tmp, L"ifndef") ) // #ifdef (#ifndef) ABC ... #endif (#else)
					code += preproc_ifdef(code);
				else if( cmpstr(tmp, L"if") )
					code += preproc_if(code);
				else
					errorParser(code, L"Invalid preprocessor command.");
			}
			continue;
		}
		
		OutCode[OutCodeIndex] = code[0];
		OutCodeIndex++;
		code++;
	}
	
	OutCode[OutCodeIndex] = 0; // На всякий случай
    return cpystr( out, OutCode, 0 );
}

// Функции для сокращения кода
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
	for(uint i=0; i<lenstr(str); i++)
		if(	(str[i]>=L' ' && str[i]<=L'/') || (str[i]>=L':' && str[i]<=L'@') ||
			(str[i]>=L'[' && str[i]<=L'^') || (str[i]>=L'{' && str[i]<=L'~') ||
			str[i]==L'`' || str[i]==L'\t') return false;
	return true;
}
uint _if_loop(wchar *code){ // обработка if
	// задача: вернуть размер значения (от #if/#ifdef до #else/#endif)

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
			if( (code[i]==L'<' || code[i]==L'\"') && j==0 ) break;
		}
		i--;
		
		tmp = cpystrMem(code, i);
		
		tmp = openfile(tmp, true);
		if(tmp==0) errorParser(code, L"I can't open the file. Invalid path/name.");

		code += ++i;
		
		// записываем содержимое файла
		i = lenstr(tmp);
		dynamic_array_process( &OutCode, OutCodeIndex+i, &OutCodeMaxLen, sizeof(wchar) );
		cpystr(OutCode, tmp, i);
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
	
	tmp = getPreprocName(code);
	code += i;
	
	if( cmpformat(tmp) == false ) // Неправильное (возможно) имя для директивы
		errorParser(code-i, addstr( L"Invalid name for define: ", tmp ));
	
	if( lenstr(tmp) > MaxDefNameLen )
		errorParser(code-i, addstr( L"Name too big for define: ", tmp ));
	
	cpystr( nowDef.name, tmp, 0 );
	
	nowDef.indexArgs = 0;
	
	if(code[0]==L'('){ // есть ли аргументы
		code++;
							
		while(code<EndCode && code[0]!=0){
			if(nowDef.indexArgs > MaxDefArgs)
				errorParser(code-i, addstr( L"Too much argumets for define: ", tmp ));
			
			if(code[0]==L' ' || code[0]==L','){ code++; continue; }
			if(code[0]==L'\n') break;
			if(code[0]==L')'){ code++; break; }
			
			for(i=0; (i+code < EndCode && code[i]!=0); i++)
				if(code[i]==0 || code[i]==L',' || code[i]==L')') break;
			tmp = cpystrMem(code, i);
			code += i;
			
			if( cmpformat(tmp)==false ) // Неправильное (возможно) имя аргумента для директивы
				errorParser(code-i, addstr( L"Invalid argument name for define: ", tmp ));
			
			if( lenstr(tmp) > MaxDefArgLen )
				errorParser(code-i, addstr( L"Name too big for define: ", tmp ));
			
			cpystr( nowDef.args[nowDef.indexArgs], tmp, 0 );
			nowDef.indexArgs++;
		}
	}
	
	if(code[0]==L'\n'){ nowDef.value[0] = 0; defIndex++; goto preproc_define_end_label; }
	
	for(i=0; (i+code<EndCode && code[i]!=0); i++) // Get value
		if(code[i]==0 || code[i]==L'\n') break;
	
	tmp = cpystrMem(code, i);
	code += i;
	
	tmp = formatStr(tmp);
	
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
uint preproc_undef(wchar *code){
	wchar *startCode = code;
	
	tmp = getPreprocName(code);
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
	
	_if_loop(code); // вернул значение в i
				
	if( ( ifdefined(tmp) && h==0 ) || ( !ifdefined(tmp) && h==1 ) ){ // if defined (not)
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
	
	if( cmpstr(code, L"#endif") ) code += 6; else
		errorParser(code, L"I wait '#endif' after '#ifdef' or '#else'");
	
	
	preproc_if_end_label:
	return (uint)(code - startCode);
}
uint preproc_if(wchar *code){
	
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
	
	return (uint)(code - startCode);
}