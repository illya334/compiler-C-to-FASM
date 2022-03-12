/*
 * Парфёнов Илья Олегович
 * Последнее изменение: 10.03.2022
 * 
 * Транслятор/Компилятор Си в FASM
 * Должен поддерживать 16, 32, 64 битные генерации кода.
 * Не должен быть привязан к определеной ОС, вы должны написать сами все вам нужные функции.
 * Еще в планах чтобы он мог +/- генерировать оптемизированый код.
 */
 
 /*
  * Почему везде a==0, a==false, a==true, a>0
  *	мой компилятор, на момент написания кода, неправильно компилирует код,
  * по этому я принудительно указываю что я имею ввиду.
  * 
  * Чем больше пишу код тем больше кажется что это компилятор, так как в GCC транслируеца все в ассемблер
  * а потом в бинарник. Примерно так и у меня, только будет не AT&T, а уже INTEL формат, FASM.
  * 
  * Почему FASM? - Мне он понравился тем:
  * 	1. Досигпор идут обновления и поддержка
  * 	2. Большой арсонтимент макросов
  * 	3. Просто потому что это мой первый (и возможно последний) компилятор ассемблера
  * 
  * Вы можете спросить зачем я изобретал вилосипед? Свой компилятор?
  * - На момент написания компилятора я не нашол чтото подобное.
  * Я хотел чтобы пользователь смог генерировать не критичный код к скорости.
  * И возможно когда нибудь я сделаю ОС, но это не точно.
  * 
  * И вообще это все развлечения...
  */

#include <stdio.h> // printf, _wfopen, 
#include <stdlib.h> // malloc
#include <stdint.h> // uint8_t
#include <wchar.h> // wchar_t

#define bool uint8_t
#define byte uint8_t
#define uint unsigned int

#define true 1
#define false 0

uint Lines = 0;

/*
 * Я решил проблему с утечкой памяти так:
 * Я делаю масив (я его назвал страницей) созданых адресов чтобы не потерять и потом одним разам почистить.
 * Но часто требуеца оставить чтото в момент очистки, по этому создаеться еще масив
 * который хранит несколько страниц. (Максимум 20 страниц)
 */

#define MAX_LOCAL_ALLOC_LISTS 5 // максимальное число страниц (5/255)

struct st_alloc{ // структура одной страницы
	void **arrays; // масив адресов
	uint arrIndex; // индекс масива
	uint MaxArrIndex; // размер масива
};

struct st_alloc *lists = 0; // масив страниц
byte IndexList = 0; // Индекс текущей страницы
bool Local_init_bool = false; // Была ли инициализация страниц?

// =============================== FUNCs =======================================

// макрос чтобы сократить код
#define nowList lists[IndexList]

void error(char *str){ // тупо вывод ощибки
	printf("Error: %s\n", str);
	exit(1);
}

// работа с памятью - выделение/освобождение/изменение размера/смена страницы
void Local_init(){
	if(lists==0) lists = malloc( sizeof(struct st_alloc) * MAX_LOCAL_ALLOC_LISTS );
	for(uint i=0; i<MAX_LOCAL_ALLOC_LISTS; i++){
		nowList.arrays = malloc( sizeof(void*) * 20 );
		nowList.arrIndex = 0;
		nowList.MaxArrIndex = 20;
	}
	Local_init_bool = true;
}
char *Local_alloc(uint len){ // выделить память и записать адресс в масив, чтобы потом одной командой почистить
	if(Local_init_bool == false) Local_init();
	if(nowList.arrIndex >= nowList.MaxArrIndex){
		nowList.MaxArrIndex += 20;
		nowList.arrays = realloc(nowList.arrays, nowList.MaxArrIndex);
	}
	
	char *tmp = malloc(len);
	if(tmp == 0)
		error("Memory allocation error.");
		
	nowList.arrays[nowList.arrIndex] = tmp;
	nowList.arrIndex++;
	
	for(uint i=0; i<len; i++) // обнуление
		tmp[i] = 0;
	
	return tmp;
}
void Local_free(){ // очищает масив malloc_arr, и освобождает память (чистит страницу)
	for(uint i=0; i<nowList.arrIndex; i++)
		free(nowList.arrays[nowList.arrIndex]);
	nowList.arrIndex = 0;
}
bool swap(byte index){ // смена страницы
	if(index > MAX_LOCAL_ALLOC_LISTS) return false;
	if( Local_init_bool == false ) Local_init();
	IndexList = index;
	return true;
}
#define getSwapIndex() IndexList // Вечно забываю как называеца переменая

// работа со строками wchar_t
uint lenstr(wchar_t *str){ // получает длину строки
	if(str == 0) return 0;
    register uint len = 0;
    while(1){
        if(str[len] == 0) return len;
        len++;
    }
}
bool cmpstr(wchar_t *str, wchar_t *find){ // сравнивает две строки
	if(str==0 || find==0) return false;
	if(lenstr(str)>=lenstr(find)){
		for(uint i=0; i<lenstr(find); i++){
			if(str[i] == 0) return false;
			if(str[i] != find[i]) return false;
		}
		return true;
	}else return false;
}
wchar_t *copystr(wchar_t *start, uint len){ // создает новый масив и копирует туда строку
	if(start==0) return 0;

    if(len == 0)
        len = lenstr(start);

    wchar_t *buf = (wchar_t*)Local_alloc( (len+1)*sizeof(wchar_t) );
    for(register uint i=0; i<len; i++){
        buf[i] = start[i];
        buf[i+1] = 0;
    }
    return buf;
}
wchar_t *addstr(wchar_t *str1, wchar_t *str2){ // объединяет строки
	wchar_t *outstr = (wchar_t*)Local_alloc( ( lenstr(str1)+lenstr(str2) )*sizeof(wchar_t)+1 );
	register uint i;
	register uint osi = 0; // OutStrIndex
	for(i=0; i<lenstr(str1); i++){
		outstr[osi] = str1[i];
		osi++;
	}
	for(i=0; i<lenstr(str2); i++){
		outstr[osi] = str2[i];
		osi++;
	}
	return outstr;
}
wchar_t *formatStr(wchar_t *str){ // чистит строку от пробелов, но оставляет как минимум 1 пробел между словами
	if(str == 0) return 0;
	str = copystr(str, lenstr(str)); // копируем строку для табуляции
	wchar_t *EndStr = str + lenstr(str);
	wchar_t *OutStr = (wchar_t*)Local_alloc( lenstr(str)*sizeof(wchar_t) );
	uint OutStrInd = 0;
	uint i = 0;
	uint j = 0;
	
	for(i=0; i+str<EndStr; i++){ // меняем табуляцию на пробел
		if(str[i]==L'"' && j == 0){ // игнорируем все что в скобках
			j=1;
			continue;
		}
		if(str[i]==L'\'' && j == 0){ // ^^^
			j=2;
			continue;
		}
		if(str[i]==L'\t' && j == 0) str[i]=L' ';
		if((str[i]==L'"' && j == 1) || (str[i]==L'\'' && j == 2)) j=0; // ^^^
	}
	
	for(i=0; i+str<EndStr; i++) // чистим пробелы до начала слова
		if(str[i]!=L' ') break;
	str+=i;
	
	for(i=0; i+str<EndStr; i++){ // берем слово и копируем в OutStr
		if(str[i]==L' '){
			for(j=i; j+str<EndStr; j++)
				if(str[j]!=L' ') break;
			if(j>=1) i+=j-i;
			OutStr[OutStrInd] = L' ';
			OutStrInd++;
		}
		
		if(str[i]==L'"'){ // не трогать все что в скобках
			OutStr[OutStrInd] = L'\"'; OutStrInd++;
			for(j=i+1; j+str<EndStr; j++)
				if(str[j]==L'"') break; else { OutStr[OutStrInd] = str[j]; OutStrInd++;}
			i+=j-i;
		}
		if(str[i]==L'\''){ // ^^^
			OutStr[OutStrInd] = L'\''; OutStrInd++;
			for(j=i+1; j+str<EndStr; j++)
				if(str[j]==L'\'') break; else { OutStr[OutStrInd] = str[j]; OutStrInd++;}
			i+=j-i;
		}
		
		OutStr[OutStrInd] = str[i];
		OutStrInd++;
	}
	
	for(i=OutStrInd-1; i>0; i--){ // чистим пробелы в конце строки
		if(OutStr[i]==L' ') OutStr[i] = 0; else
		if(OutStr[i]!=L' ' && OutStr[i]!=0) break;
	}
	
	OutStr[OutStrInd] = 0;
	
	free(str);
	return OutStr;
}

/*
 * Можете заметить примитивные функции работы с строками.
 * И у вас может появица вопрос зачем? - В планах попробывать скомпилировать этот транслятор/компилятор на нем же.
 * А вот он с коробки не имеет вообще некаких библиотек, и чтобы потом не парица я создал их сейчас и не использую другие.
 */

bool cmpformat(wchar_t *str){ // проверка имени на правильность, не должно быть любых знаков кроме букв и _
	// судя по даблице Unicode (https://en.wikipedia.org/wiki/List_of_Unicode_characters#Basic_Latin)
	if( str[0]>=L'0' && str[0]<=L'9' ) return false;
	for(uint i=0; i<lenstr(str); i++)
		if(	(str[i]>=L'!' && str[i]<=L'/') || (str[i]>=L':' && str[i]<=L'@') ||
			(str[i]>=L'[' && str[i]<=L'^') || (str[i]>=L'{' && str[i]<=L'~') ||
			str[i]==L'`' || str[i]==L' ' || str[i]==L'\t') return false;
	return true;
}

void errorParser(wchar_t *code, wchar_t *errorText){ // вывод ощибок парсера, отображение ощибок не очень
	if(code==0 || errorText==0) exit(1);
	wprintf(L"ERROR Parser: %s\n", errorText);
	if(lenstr(code) >= 15){
		code = copystr(code, 0);
		code[15]=L'\0';
	}
	wprintf(L"Lines\t| Code\n");
	wprintf(L"%d\t| %s\n", Lines, code);
	exit(1);
}

struct def{ // структура директив
	wchar_t *name; // имя макроса
	wchar_t *value; // значение (необязательно)
	wchar_t **args; // аргументы (если есть)
	byte indexArgs; // количество аргументов
};

struct def *defines = 0;
uint defIndex = 0;
#define MaxDefIndex 100 // максимальное количество директив
#define MaxDefArgs 10 // максимум аргументов для директив

/*
 * Устройство парсера\препроцесора
 * 
 * Задача: Убрать все лишнее и обработать макросы
 * 
 * _parser_withOutLoop - функция которая просто одним проходом обрабатывает. (требует несколько проходов)
 * 
 * parser - функция которая загоняет в цикл _parser_withOutLoop пока не перестанет изменяца текст
 * после чего она чистит страницу памяти (уничтожая утечки памяти, их реально много) и возращает результат.
 * Тем самым реализуя несколько прогонов парсера, так как за 1 раз мой парсер не способен обработать все.
 */

uint i = 0;
uint j = 0;
uint tmp2;

wchar_t *openfile(wchar_t *path){ // открывает файл и копирует содержимое в масив
	FILE *fp = _wfopen(path, L"rt");
	wchar_t *buf = Local_alloc(100);
	uint MaxIndex = 100;
	i=0;
	while(1){
		if(i >= MaxIndex){
			MaxIndex+=1000;
			
			wchar_t *buf2 = Local_alloc(MaxIndex);
			for(j=0; j<i; j++)
				buf2[j] = buf[j];
			free(buf);
			buf = buf2;
		}
		buf[i] = fgetwc(fp);
		
		if(buf[i]==0xFFFF) break;
		
		i++;
	}
	buf[i]=0;
	return buf;
}

wchar_t *getPreprocName(wchar_t *code){ // получает имя после команды препроцессора: #define, #ifdef, ...
	for(j=0; (j<lenstr(code) && code[j]!=0); j++) // игнорируем пробелы после слова
		if(code[j]!=L' ') break;
	code+=j;
	
	for(i=0; (i<lenstr(code) && code[i]!=0); i++) // Get name
		if(	(code[i]>=L' ' && code[i]<=L'/') || (code[i]>=L':' && code[i]<=L'@') ||
			(code[i]>=L'[' && code[i]<=L'^') || (code[i]>=L'{' && code[i]<=L'~') ||
			 code[i]==L'`' ||code[i]==L'\t' || code[i]==L'\n') break;
	code = copystr(code, i);
	i+=j;
	return code;
}
bool ifdefined(wchar_t *name){ // была ли создана константа
	for(uint u=0; u<defIndex; u++)
		if(cmpstr(defines[u].name, name)) return true;
	return false;
}
void _if_loop(wchar_t *code){ // просто подумал что если часто повторяеца код то чего и не запихнуть его в функцию
	j=1;
	for(i=0; (i<lenstr(code) && code[i]!=0); i++){
		if(code[i]==L'\'' && tmp2==0){ tmp2 = 1; continue; }
		if(code[i]==L'\"' && tmp2==0){ tmp2 = 2; continue; }
		if( (code[i]==L'\'' && tmp2==1 ) || (code[i]==L'\"' && tmp2==2 ) ){ tmp2=0; continue; }
					
		if(cmpstr(code+i, L"#if") || cmpstr(code+i, L"#ifdef") || cmpstr(code+i, L"#ifndef")){ j++; continue; }
		if(cmpstr(code+i, L"#endif") && j>1){ j--; continue; }
		
		if((cmpstr(code+i, L"#endif") || cmpstr(code+i, L"#else")) && j<=1) break;
	}
}

wchar_t *_parser_withOutLoop(wchar_t *code){ // main func
	if(code[0]==0 || code==0) error("'_parser_withOutLoop' get zero."); // заглужка
	Lines = 0;
    wchar_t *OutCode = (wchar_t*)Local_alloc(lenstr(code)); // сюда згружаеца обработаный код
    uint OutCodeIndex = 0;
	
	if(defines==0) // при запуске транслятора/компилятора все переменые (по идеи) должны быть в 0
		defines = (struct def*)Local_alloc(sizeof(struct def)*MaxDefIndex);
	
    // local variables
	uint u = 0;
	uint h = 0;
	wchar_t *tmp;
	wchar_t **tmpArgs = (wchar_t**)Local_alloc( sizeof(wchar_t*)*MaxDefArgs ); // в момент вставки макроса, если он с аргументами
	
	code = formatStr(code); // чистим все лишние пробелы и табуляцию
	wchar_t *EndCode = code + lenstr(code)*sizeof(wchar_t);
	
    while(code<EndCode && code[0]!=0){
		if(code[0]==0) break;
		
		if( (code[0]>=L' ' && code[0]<=L'/') || (code[0]>=L':' && code[0]<=L'@') || // все эти символы вызывают ощибки в парсере, по этому я их игнорирую
			(code[0]>=L'[' && code[0]<=L'^') || (code[0]>=L'{' && code[0]<=L'~') ||
			 code[0]==L'`' || code[0]==L'\t' || code[0]==L'\n'){
			if(code[0]==L'\n') Lines++;
			OutCode[OutCodeIndex] = code[0];
			OutCodeIndex++;
			code++;
			continue;
		}
			
		tmp2=0;
		for(i=0; (i+code<EndCode && code[i]!=0); i++){ // Get word
			if(code[i]==L'"' && tmp2==0){ tmp2 = 1; continue; } // берем даже с скобками
			if(code[i]==L'\'' && tmp2==0){ tmp2 = 2; continue; } // ^^^
			if(((code[i]>=L' ' && code[i]<=L'/') || (code[i]>=L':' && code[i]<=L'@') ||
				(code[i]>=L'[' && code[i]<=L'^') || (code[i]>=L'{' && code[i]<=L'~') ||
				 code[i]==L'`' || code[i]==L'\t' || code[i]==L'\n') && tmp2==0) break;
			if((code[i]==L'"' && tmp2==1) || (code[i]==L'\'' && tmp2==2)) tmp2=0; // ^^^
		}
		if(i==0) continue;
		tmp = copystr(code, i);
		code += i;
		
		if(cmpstr(tmp, L"include") && code[-8]==L'#'){ // INCLUDE =====================================
			for(i=0; (i<lenstr(code) && code[i]!=0); i++) // игнорируем пробелы после слова
				if(code[i]!=L' ') break;
			code+=i;
			
			if(code[0]==L'<' || code[0]==L'\"'){
				if(code[0]==L'\"') j=1; else j=0;
				code++;
				for(i=0; (i+code<EndCode && code[i]!=0); i++){
					if(code[i]==L'\"' && j==0){ j=1; continue; }
					if(code[i]==L'\'' && j==0){ j=2; continue; }
					if( (code[i]==L'\"' && j==1) || (code[i]==L'\'' && j==2) ){ j=0; continue; }
					if( (code[i]==L'<' || code[i]==L'\"') && j==0 ) break;
				}
				tmp = openfile( copystr(code, i) );
				if(tmp==0)
					errorParser(code, L"I can't open the file. Invalid path/name.");
				else{
					
				}
			} else
				errorParser(code, L"I wait '<' or '\"' after '#include'.");
			
		}
		
		if(cmpstr(tmp, L"define") && code[-7]==L'#'){ // DEFINE =======================================
			OutCodeIndex--; // убераем #
				
			#define nowDef defines[defIndex] // я не хачу видеть огромные строки обращений к масиву
				
			tmp = getPreprocName(code);
			code += i;
					
			if( cmpformat(tmp)==false ) // Неправильное (возможно) имя для директивы
				errorParser(code-i, addstr( L"Invalid name for define: ", tmp ));
					
			if(nowDef.indexArgs > MaxDefArgs)
				errorParser(code-i, addstr( L"Too much argumets for define: ", tmp ));
					
			if(defIndex > MaxDefIndex)
				errorParser(code-i, addstr( L"Too much defines: ", tmp ));
					
			nowDef.name = tmp;
					
			if(code[0]==L'('){ // есть ли аргументы
				code++;
						
				if(nowDef.args==0) nowDef.args = (wchar_t**)Local_alloc( MaxDefArgs*sizeof(wchar_t*) );
							
				while(code<EndCode && code[0]!=0){
					if(code[0]==L' ' || code[0]==L','){ code++; continue; }
					if(code[0]==L'\n') break;
					if(code[0]==L')'){ code++; break; }
									
					for(i=0; (i+code<EndCode && code[i]!=0); i++)
						if(code[i]==0 || code[i]==L',' || code[i]==L')') break;
					tmp = copystr(code, i);
					code+=i;
									
					if( cmpformat(tmp)==false ) // Неправильное (возможно) имя аргумента для директивы
						errorParser(code-i, addstr( L"Invalid argument name for define: ", tmp ));
									
					nowDef.args[nowDef.indexArgs] = tmp;
					nowDef.indexArgs++;
				}
			}
					
			if(code[0]==L'\n'){ nowDef.value = 0; defIndex++; continue; }
				
			for(i=0; (i+code<EndCode && code[i]!=0); i++) // Get value
				if(code[i]==0 || code[i]==L'\n') break;
					
			tmp = copystr(code, i);
			code+=i;
					
			tmp = formatStr(tmp);
					
			if(lenstr(tmp)==0)
				nowDef.value = 0;
			else
				nowDef.value = tmp;
					
			defIndex++;
			continue;
				
			#undef nowDef // чтоб глаза не мазолил
		}
			
		if(cmpstr(tmp, L"undef") && code[-6]==L'#'){ // UNDEF =========================================
			OutCodeIndex--; // убераем #
				
			tmp = getPreprocName(code);
			code += i;
				
			for(i=0; i<defIndex; i++){ // если есть такое имя то уничтожаем
				if(cmpstr(tmp, defines[i].name)){
					defines[i].name = 0;
					defines[i].value = 0;
					defines[i].args = 0;
					defines[i].indexArgs = 0;
				}
			}
					
			continue;
		}
			
		if( (cmpstr(tmp, L"ifdef") && code[-6]==L'#') || // IFDEF IFNDEF ==============================
			(cmpstr(tmp, L"ifndef") && code[-7]==L'#') ){
			if( cmpstr(tmp, L"ifndef") ) h = 1; else h = 0;
			OutCodeIndex--; // убераем #
				
			tmp = getPreprocName(code);
			code += i;
				
			tmp2 = 0;
			_if_loop(code);
				
			if( ( ifdefined(tmp) && h==0 ) || ( !ifdefined(tmp) && h==1 ) ){ // if defined
				if(i>0){
					for(j=0; j<i; j++)
						OutCode[OutCodeIndex+j] = code[j];
					OutCodeIndex += j;
					code+=i;
				}
				if(cmpstr(code, L"#else")){ // ignore else
					code += 5;
					_if_loop(code);
					code += i;
				}
			} else {
				code+=i;
				if(cmpstr(code, L"#else")){ // #else
					code+=5;
					tmp2 = 0;
					_if_loop(code);
					
					if(i>0){
						for(j=0; j<i; j++)
							OutCode[OutCodeIndex+j] = code[j];
						OutCodeIndex += j;
						code+=i;
					}
				}
			}
				
			if(cmpstr(code, L"#endif")) code += 6; else
				errorParser(code, L"I wait '#endif' after '#ifdef' or '#else'");
				
			continue;
		}
		
		if(cmpstr(tmp, L"else") && code[-5]==L'#') // вдруг наткнусь
			errorParser(code-5, L"no '#if' found for '#else'.");
		
		if(cmpstr(tmp, L"endif") && code[-6]==L'#') // ^^^
			errorParser(code-6, L"no '#if' found for '#endif'.");
		
		uint tmpArgsIndex = 0;
		for(i=0; (i+code<EndCode && code[i]!=0); i++){ // проверка всех имен макросов
			if(cmpstr(tmp, defines[i].name)){
					
				if(defines[i].value==0)
					errorParser(code-lenstr(defines[i].name),
						addstr( L"This define cannot be used! Since it has no value. Name define: ", defines[i].name ));
					
				if(code[0]==L'('){ // обнаружены аргументы
					code++;
						
					if(defines[i].args == 0)
						errorParser(code-lenstr(defines[i].name),
							addstr( L"This define does not support arguments. Name define: ", defines[i].name ));
						
					tmp2 = i; // defIndex
					j=1;
					while(code<EndCode && code[0]!=0){ // цикл получения аргументов
						if(tmpArgsIndex > defines[tmp2].indexArgs || tmpArgsIndex > MaxDefArgs) // слишком много аргументов
							errorParser(code, addstr( L"Too much argumets for define: ", defines[tmp2].name ));
							
						for(i=0; (i+code<EndCode && code[i]!=0); i++)
							if(code[i]!=L' ') break;
						code+=i;
							
						if(code[0]==L','){ code++; continue; } else
						if((code[0]==L')' && j<=0) || code[0]==0){ code++; break; }
							
						for(i=0; (i+code<EndCode && code[i]!=0); i++){ // получаем аргументы
							if(code[i]==L'(') j++;
							if(code[i]==L')') j--;
							if(j<=0 || (code[i]==L',' && j<=1)) break;
						}
						tmpArgs[tmpArgsIndex] = copystr(code, i);
						code+=i;
						tmpArgsIndex++;
					}
					// теперь вставляем эти параметры
						
					if(tmpArgsIndex > defines[tmp2].indexArgs)
						errorParser(code, addstr( L"Too much argumets for define: ", defines[tmp2].name ));
						
					#define nowDef defines[tmp2] // снова сокращаем код, а то легко запутаца в этой каше квадратных скобок
						
					for(i=0; i < lenstr(nowDef.value); ){
							register bool tmp3 = false;
							for(j=0; j < nowDef.indexArgs; j++){
								if( cmpstr(nowDef.value+i, nowDef.args[j]) ){
									tmp3 = true;
									for(u=0; u < lenstr(tmpArgs[j]); u++)
										OutCode[OutCodeIndex+u] = (tmpArgs[j])[u];
									OutCodeIndex += u;
									i += lenstr(nowDef.args[j]);
								}
							}
							
							if(tmp3 == false){
								OutCode[OutCodeIndex] = nowDef.value[i];
								OutCodeIndex++;
								i++;
							}
							
						}
						
				}
					
				#undef nowDef // чтоб глаза не мазолил
				tmp = 0;
				break;
			}
		}
			
		if(tmp!=0){ // копируем слово в выход
			for(i=0; i<lenstr(tmp); i++)
				OutCode[OutCodeIndex+i] = tmp[i];
			OutCodeIndex+=i;
		}
	}
	
    OutCode[OutCodeIndex] = 0; // На всякий случай
    return OutCode;
}

wchar_t *parser(wchar_t *code){ // основной цикл обработки парсера
	wchar_t *lastCode = 0;
	while(1){
		code = _parser_withOutLoop(code);
		if(code==0) return 0;
		if(code[0]==0) return code;
		if(cmpstr(lastCode, code)){
			swap(getSwapIndex()+1); // вытягиваем готовый код из страницы и чистим страницу
			code = copystr(code, 0);
			swap(getSwapIndex()-1);
			Local_free();
			return code;
		} else lastCode=code;
	}
}

int main(){
	wchar_t *code = openfile(L"C:\\C_C++\\test.c");
	
	wchar_t *tmp = parser(code);
	
	wprintf(L"%s", tmp);
    return 0;
}