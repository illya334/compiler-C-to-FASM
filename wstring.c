/*
 * Работа со Unicode строками | Work with Unicode string
 * 
 * by illya334 (zeq52giw)
 * 
 * Habr: https://habr.com/ru/users/illya334/
 */

#include <stdlib.h> // malloc, realloc, free
#include <wchar.h> // wchar_t
#include <stdint.h> // uint8_t

#define uint unsigned int
#define reg register
#define bool uint8_t

wchar_t *_wstring_outstr = 0;
uint _len_wstring_outstr = 0;

void setmem(char *buf, uint count, char ch){
	for(uint i=0; i<count; i++)
		buf[i] = ch;
}

void dynamic_array_process(void **arr, uint nowIndex, uint *MaxIndex, uint size){
	if( arr != 0 ){
		if( *arr == 0 ){
			
			if( *MaxIndex == 0 ) *MaxIndex = nowIndex;
			
			if( *MaxIndex != 0 ){
				*arr = malloc( (*MaxIndex + 1) * size );
				setmem(*arr, (*MaxIndex + 1) * size, 0);
			}
			
		} else if( nowIndex > *MaxIndex ){
			
			reg uint tmpIndex = *MaxIndex;
			*MaxIndex = nowIndex+1;
			*arr = realloc( *arr, (*MaxIndex + 1) * size );
			setmem( *arr + tmpIndex, (*MaxIndex - tmpIndex + 1) * size, 0 );
			
		} else if( nowIndex == *MaxIndex ){
			
			reg uint tmpIndex = *MaxIndex;
			*MaxIndex *= 2;
			*arr = realloc( *arr, (*MaxIndex + 1) * size );
			setmem( *arr + tmpIndex, (*MaxIndex - tmpIndex + 1) * size, 0 );
			
		}
	}
}

uint lenstr(wchar_t *buf){
	if(buf==0 || buf[0]==0) return 0;
	reg uint i;
	for(i=0; buf[i]!=0; i++){ }
	return i;
}
wchar_t *cpystr(wchar_t *buf, wchar_t *str, uint len){
	reg uint i;
	if( len > lenstr(str) ) return 0; else
	if( len == 0 ) len = lenstr(str);
			 
	for(i = 0; i < len; i++) buf[i] = str[i];

	buf[i] = 0;
	return buf;
}
wchar_t *cpystrMem(wchar_t *str, uint len){
	reg uint i;
	if(len == 0) len = lenstr(str); else
	if(len > lenstr(str)) return 0;
	
	dynamic_array_process( &_wstring_outstr, len, &_len_wstring_outstr, sizeof(wchar_t) );
	
	for(i = 0; i < len; i++) _wstring_outstr[i] = str[i];

	_wstring_outstr[i] = 0;
	return _wstring_outstr;
}
bool cmpstr(wchar_t *str1, wchar_t *str2){
	reg uint i, lenStr2 = lenstr(str2);
	if(lenStr2 > lenstr(str1) || str1[0] == 0 || str2[0] == 0 || str1 == 0 || str2 == 0 ) return 0;
	
	for(i=0; i<lenStr2; i++)
		if(str1[i] != str2[i]) return 0;

	return 1;
}
wchar_t *addstr(wchar_t *str1, wchar_t *str2){
	reg uint 	len1 = lenstr(str1),
				len2 = lenstr(str2);
	
	dynamic_array_process( &_wstring_outstr, (len1 + len2), &_len_wstring_outstr, sizeof(wchar_t) );
	
	cpystr(_wstring_outstr, str1, 0);
	cpystr(_wstring_outstr + len1, str2, 0);
	
	return _wstring_outstr;
}
wchar_t *formatStr(wchar_t *str){ // чистит строку от пробелов, но оставляет как минимум 1 пробел между словами
	if(str == 0 || str[0] == 0) return 0;
	
	dynamic_array_process( &_wstring_outstr, lenstr(str), &_len_wstring_outstr, sizeof(wchar_t) );
	
	str = cpystrMem(str, 0);
	
	uint OutStrInd = 0,
		 i, j,
		 len = lenstr(str);
		 
	wchar_t *EndStr = str + len;
	
	j = 0;
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
	
	for(i=0; i+str<EndStr; i++){ // берем слово и копируем в _wstring_outstr
		if(str[i]==L' '){
			for(j=i; j+str<EndStr; j++)
				if(str[j]!=L' ') break;
			if(j>=1) i+=j-i;
			_wstring_outstr[OutStrInd] = L' ';
			OutStrInd++;
		}
		
		if(str[i]==L'"'){ // не трогать все что в скобках
			_wstring_outstr[OutStrInd] = L'\"'; OutStrInd++;
			for(j=i+1; j+str<EndStr; j++)
				if(str[j]==L'"') break; else { _wstring_outstr[OutStrInd] = str[j]; OutStrInd++;}
			i+=j-i;
		}
		if(str[i]==L'\''){ // ^^^
			_wstring_outstr[OutStrInd] = L'\''; OutStrInd++;
			for(j=i+1; j+str<EndStr; j++)
				if(str[j]==L'\'') break; else { _wstring_outstr[OutStrInd] = str[j]; OutStrInd++;}
			i+=j-i;
		}
		
		_wstring_outstr[OutStrInd] = str[i];
		OutStrInd++;
	}
	
	for(i=OutStrInd-1; i>0; i--){ // чистим пробелы в конце строки
		if(_wstring_outstr[i]==L' ') _wstring_outstr[i] = 0; else
		if(_wstring_outstr[i]!=L' ' && _wstring_outstr[i]!=0) break;
	}
	
	_wstring_outstr[OutStrInd] = 0;
	
	return _wstring_outstr;
}
