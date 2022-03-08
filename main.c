/*
 * Парфёнов Илья Олегович
 * Последнее изменение: 06.03.2022
 */
 
#include <stdio.h> // printf
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

// работа с памятью - выделение/освобождение/изменение размера/смена страницы
void Local_init(){
	if(lists==0) lists = malloc( sizeof(struct st_alloc) * MAX_LOCAL_ALLOC_LISTS );
	for(uint i=0; i<MAX_LOCAL_ALLOC_LISTS; i++){
		nowList.arrays = malloc( sizeof(void*) * 20 );
		nowList.arrIndex = 0;
		nowList.MaxArrIndex = 20;
	}
}
char *Local_alloc(uint len){ // выделить память и записать адресс в масив, чтобы потом одной командой почистить
	if(Local_init_bool == false) Local_init();
	if(nowList.arrIndex >= nowList.MaxArrIndex){
		nowList.MaxArrIndex += 20;
		nowList.arrays = realloc(nowList.arrays, nowList.MaxArrIndex);
	}
	register void *tmp = malloc(len);
	nowList.arrays[nowList.arrIndex] = tmp;
	nowList.arrIndex++;
	return tmp;
}
void Local_free(){ // очищает масив malloc_arr, и освобождает память (чистит страницу)
	for(uint i=0; i<nowList.arrIndex; i++)
		free(nowList.arrays[nowList.arrIndex]);
}
bool swap(byte index){ // смена страницы
	if(index > MAX_LOCAL_ALLOC_LISTS) return false;
	if( Local_init_bool == false ) Local_init();
	IndexList = index;
	return true;
}
byte getSwapIndex(){ // Вечно забываю как называеца переменая
	return IndexList;
}

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

    wchar_t *buf = Local_alloc( (len+1)*sizeof(wchar_t) );
    for(register uint i=0; i<len; i++){
        buf[i] = start[i];
        buf[i+1] = 0;
    }
    return buf;
}
wchar_t *formatStr(wchar_t *str){ // чистит строку от пробелов, но оставляет как минимум 1 пробел между словами
	if(str == 0) return 0;
	str = copystr(str, lenstr(str)); // копируем строку для табуляции
	wchar_t *EndStr = str + lenstr(str);
	wchar_t *OutStr = Local_alloc( lenstr(str)*sizeof(wchar_t) );
	uint OutStrInd = 0;
	uint i = 0;
	uint j = 0;
	
	for(i=0; i+str<EndStr; i++) // меняем табуляцию на пробел
		if(str[i]==L'\t') str[i]=L' ';
	
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

void errorParser(wchar_t *code, wchar_t *errorText){
	if(code==0 || errorText==0) exit(1);
	wprintf(L"ERROR Parser: %s\n", errorText);
	code = copystr(code, 0);
	if(lenstr(code) >= 15) code[15]=L'\0';
	wprintf(L"Lines\t| Code\n");
	wprintf(L"%d\t| %s\n", Lines, code);
	exit(1);
}

void clearMemory(char *adr, uint len){
	for(uint i=0; i<len; i++)
		adr[i]=0;
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

wchar_t buf[50]; // для вывода ощибок

/*
 * Устройство парсера\препроцесора
 * 
 * Задача: Убрать все лишнее и обработать макросы
 * 
 * _parser_withOutLoop - функция которая просто одним проходом обрабатывает все.
 * 
 * parser - функция которая загоняет в цикл _parser_withOutLoop пока не перестанет изменяца текст
 * после чего она чистит страницу памяти (уничтожая утечки памяти) и возращает результат.
 * Тем самым реализуя несколько прогонов парсера, так как за 1 раз мой парсер не
 * способен обработать все.
 */

wchar_t *_parser_withOutLoop(wchar_t *code){ // main func
	if(code[0]==0) return 0;
	Lines = 0;
    wchar_t *OutCode = Local_alloc(lenstr(code));
    uint OutCodeIndex = 0;
	
	if(defines==0){
		defines = Local_alloc(sizeof(struct def)*MaxDefIndex);
		clearMemory(defines, sizeof(struct def)*MaxDefIndex);
	}
	
    // local variables
    uint i = 0;
	uint j = 0; 
	wchar_t *tmp;
	wchar_t *tmp2;
	
	code = formatStr(code); // чистим все лишние пробелы и табуляцию
	wchar_t *EndCode = code + lenstr(code)*sizeof(wchar_t);
	
    while(code<EndCode){
		if(code[0]==0) break;
		
		if(code[0]==L' ' || code[0]==L'\n' || code[0]==L';'){
			if(code[0]==L'\n') Lines++;
			OutCode[OutCodeIndex] = code[0];
			OutCodeIndex++;
			code++;
		}else{
		
			for(i=0; i+code<EndCode; i++) // Get Word
				if(code[i]==L' ' || code[i]==L';' || code[i]==L'(' || code[i]==0) break;
			
			tmp = copystr(code, i);
			code+=i;
			
			if(cmpstr(tmp, L"#define")){ // DEFINE ==========================================
				for(i=0; i+code<EndCode; i++) // игнорируем пробелы после слова
					if(code[i]!=L' ') break;
				code+=i;
				
				
				for(i=0; i+code<EndCode; i++) // Get name
					if(code[i]==L' ' || code[i]==L'\n' || code[i]==L'(') break;
	
				if(i>0){
					defines[defIndex].name = copystr(code, i);
					code+=i;
					
					if(code[0]==L'('){
						code++;
						while(code<EndCode){
							if(code[0]==L','){ code++; continue; }
							if(code[0]==L' '){ code++; continue; }
							if(code[0]==L'\n' || code[0]==L')') break;
							for(j=0; j+code<EndCode; j++) // Get name
								if(code[j]==L' ' || code[j]==L'\n' || code[j]==L')' || code[j]==L',') break;
							if(defines[defIndex].args == 0) defines[defIndex].args = Local_alloc( sizeof(void*)*MaxDefArgs );
							defines[defIndex].args[ defines[defIndex].indexArgs ] = copystr(code, j);
							code+=j;
							defines[defIndex].indexArgs++;
						}
						code++;
					}
					
					for(i=0; i+code<EndCode; i++) // игнорируем пробелы после слова
						if(code[i]!=L' ') break;
					code+=i;
					
					for(i=0; i+code<EndCode; i++) // Get value
						if(code[i]==L'\n' || code[i]==0) break;
					
					if(i>0){
						defines[defIndex].value = copystr(code, i);
						code+=i;
					} else defines[defIndex].value = 0;
					
					defIndex++;
				}else{
					errorParser(code, "name should be here. #define NAME VALUE");
				}
				continue;
			}
			
			wchar_t **tmpArgs = Local_alloc( sizeof(wchar_t*)*MaxDefArgs );
			uint tmpArgsIndex = 0;
			
			register bool ret = 0; // не хачу делать мету и выходить через goto
			for(i=0; i<defIndex; i++){ // цикл проверки имен макросов
				if(cmpstr(tmp, defines[i].name)){
					if(defines[i].value == 0){
						swprintf(buf, 49, L"'%s' has no definition", defines[i].name);
						errorParser(code-lenstr(tmp), buf);
					} else {
						if(code[0]==L'('){ // если этот макрос вызвали с аргументами
							if(defines[i].args == 0 || defines[i].args[0] == 0){ // поддерживает ли данный макрос вызов с аргументами
								swprintf(buf, 49, L"'%s' cannot be used with arguments", defines[i].name);
								errorParser(code-lenstr(tmp), buf);
							}
							code++;
							while(code<EndCode){
								if(code[0]==L','){ code++; continue; }
								if(code[0]==L' '){ code++; continue; }
								if(code[0]==L';' || code[0]==L')') break;
								for(j=0; j+code<EndCode; j++) // Get name
									if(code[j]==L' ' || code[j]==L'\n' || code[j]==L')' || code[j]==L',') break;
								tmpArgs[tmpArgsIndex] = copystr(code, j);
								code+=j;
								defines[defIndex].indexArgs++;
							}
							code++;
						} else {
							for(j=0; j<lenstr(defines[i].value); j++) // копируем слово в выход
								OutCode[OutCodeIndex+j] = (defines[i].value)[j];
							OutCodeIndex+=j;
						}
					}
					ret = true;
					break;
				}
			}
			if(ret) continue;
			
			for(i=0; i<lenstr(tmp); i++) // копируем слово в выход
					OutCode[OutCodeIndex+i] = tmp[i];
			OutCodeIndex+=i;
			
		}
	}
	
    OutCode[OutCodeIndex] = 0;
    return OutCode;
}

wchar_t *parser(wchar_t *code){ // основной цикл обработки парсера
	wchar_t *lastCode = 0;
	while(1){
		code = _parser_withOutLoop(code);
		if(code==0) return 0;
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
    wchar_t *code = L"#define abc(a , b) (a + b)\nabc(10 , 15);";
	
	wprintf(L"BEFORE:\n\n%s\n================\nAFTER:\n\n%s\n\n", code, parser(code));
    return 0;
}