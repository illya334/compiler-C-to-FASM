/*
 * Робота з Unicode рядками | Work with Unicode string
 * 
 * by illya334 (zeq52giw)
 * 
 * Habr: https://habr.com/ru/users/illya334/
 * GitHub: https://github.com/illya334/compiler-C-to-FASM
 * 
 */

#ifndef _wstring_INCLUDED_
	#define _wstring_INCLUDED_

	#include <stdlib.h> // malloc, realloc, free
	#include <wchar.h> // wchar_t
	#include <stdint.h> // uint8_t

	#define uint unsigned int
	#define reg register
	#define bool uint8_t

	wchar_t *_wstring_outstr = 0;
	uint _len_wstring_outstr = 0;

	#define LocalBuf _wstring_outstr
	#define LocalBufLen _len_wstring_outstr


	uint lenstr(wchar_t*);				// bug fix (?)
	wchar_t *cpystr(wchar_t*, wchar_t*, uint); // ^^^


	void setmem(char *buf, uint count, char ch){
		for(uint i=0; i<count; i++)
			buf[i] = ch;
	}
	
	void dynamic_array_process(void **arr, uint nowIndex, uint *MaxIndex, uint size){ // обробка масиву
		 /*
		  * Якщо не існує - створити.
		  * Якщо мало місця - розширити.
		  * 
		  * arr - адреса зміної в якій зберігається адреса на масив (або записана нова адреса).
		  * nowIndex - індекс поточного елемента масиву.
		  * MaxIndex - адреса на зміну в якій зберігається максимальний індекс елементу масиву
		  * 		   (при розширенні масиву буде записано нове максимальне число).
		  * size - розмір елемента масиву (у байтах).
		  */
		if( arr != 0 ){
			if( *arr == 0 ){
				
				if( nowIndex == 0 && *MaxIndex == 0 ) *MaxIndex = nowIndex = 20;
				else
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
	void *createBuf(uint size){ // створює новий буфер з певним розміром (у байтах).
		reg void *tmp = malloc( size );
		setmem(tmp, size, 0);
		return tmp;
	}

	wchar_t *createString(wchar_t *text){ // створює буфер в якому зберігається переданий текст
		reg wchar_t *tmp = createBuf( lenstr(text)+1 );
		cpystr( tmp, text, 0 );
		return tmp;
	}

	uint lenstr(wchar_t *buf){ // Повертає довжину рядка, яка закінчується 0 (не '0').
		if(buf==0 || buf[0]==0) return 0;
		reg uint i;
		for(i=0; buf[i]!=0; i++);
		return i;
	}
	wchar_t *cpystr(wchar_t *buf, wchar_t *str, uint len){ // копіює рядок у буфер, len - довжина рядка (0 - визначити довжину автоматично).
		reg uint i;
		if( len > lenstr(str) ) return 0; else
		if( len == 0 ) len = lenstr(str);
				 
		for(i = 0; i < len; i++) buf[i] = str[i];

		buf[i] = 0;
		return buf;
	}
	wchar_t *cpystrMem(wchar_t *str, uint len){ // копіює рядок у вже створений буфер
		reg uint i;
		if(len == 0) len = lenstr(str); else
		if(len > lenstr(str)) return 0;
		
		dynamic_array_process( &LocalBuf, len, &LocalBufLen, sizeof(wchar_t) );
		
		for(i = 0; i < len; i++) LocalBuf[i] = str[i];

		LocalBuf[i] = 0;
		return LocalBuf;
	}
	bool cmpstr(wchar_t *str1, wchar_t *str2){ // Порівнює рядки. Увага! Порівнює рядки не повністю! Це використовується.
		reg uint i, lenStr2 = lenstr(str2);
		if(lenStr2 > lenstr(str1) || str1[0] == 0 || str2[0] == 0 || str1 == 0 || str2 == 0 ) return 0;
		
		for(i=0; i<lenStr2; i++)
			if(str1[i] != str2[i]) return 0;

		return 1;
	}
	wchar_t *addstr(wchar_t *str1, wchar_t *str2){ // з'єднює два рядка в вже створений буфер.
		reg uint 	len1 = lenstr(str1),
					len2 = lenstr(str2);
		
		dynamic_array_process( &LocalBuf, (len1 + len2), &LocalBufLen, sizeof(wchar_t) );
		
		cpystr(LocalBuf, str1, 0);
		cpystr(LocalBuf + len1, str2, 0);
		
		return LocalBuf;
	}
	wchar_t *formatStr(wchar_t *str){ // чистить рядок від пробілів та залишає хочаб 1 пробіл між словами.
		if(str == 0 || str[0] == 0) return 0;
		
		dynamic_array_process( &LocalBuf, lenstr(str), &LocalBufLen, sizeof(wchar_t) );
		
		if( str != LocalBuf ) str = cpystrMem(str, 0);
		
		uint OutStrInd = 0,
			 i, j,
			 len = lenstr(str);
			 
		wchar_t *EndStr = str + len;
		
		j = 0;
		for(i=0; i+str<EndStr; i++){ // Змінює табуляцію на пробіл.
			if(str[i]==L'"' && j == 0){ // ігнорує все що в лапках.
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
		
		for(i=0; i+str<EndStr; i++) // чистить пробіли до початка слова.
			if(str[i]!=L' ') break;
		str+=i;
		
		for(i=0; i+str<EndStr; i++){ // бере слово та копіює у LocalBuf
			if(str[i]==L' '){
				for(j=i; j+str<EndStr; j++)
					if(str[j]!=L' ') break;
				if(j>=1) i+=j-i;
				LocalBuf[OutStrInd] = L' ';
				OutStrInd++;
			}
			
			if(str[i]==L'"'){ // не чіпає все що в лапках
				LocalBuf[OutStrInd] = L'\"'; OutStrInd++;
				for(j=i+1; j+str<EndStr; j++)
					if(str[j]==L'"') break; else { LocalBuf[OutStrInd] = str[j]; OutStrInd++;}
				i+=j-i;
			}
			if(str[i]==L'\''){ // ^^^
				LocalBuf[OutStrInd] = L'\''; OutStrInd++;
				for(j=i+1; j+str<EndStr; j++)
					if(str[j]==L'\'') break; else { LocalBuf[OutStrInd] = str[j]; OutStrInd++;}
				i+=j-i;
			}
			
			LocalBuf[OutStrInd] = str[i];
			OutStrInd++;
		}
		
		for(i=OutStrInd-1; i>0; i--){ // чистить пробіли в кінці рядка.
			if(LocalBuf[i]==L' ') LocalBuf[i] = 0; else
			if(LocalBuf[i]!=L' ' && LocalBuf[i]!=0) break;
		}
		
		LocalBuf[OutStrInd] = 0;
		
		return LocalBuf;
	}

	wchar_t *uintToStr(uint num){ // перекладає число в рядок. translate number to string.
		
		reg uint tmp = num;
		reg uint lenUint = 0; 
		
		do{ // get length number
			tmp = tmp/10;
			lenUint++;
		}while(tmp);
		
		dynamic_array_process( &LocalBuf, lenUint+1, &LocalBufLen, sizeof(wchar_t) );
		
		reg uint i=0;
		for( ; i<lenUint; i++ ){
			tmp = num % 10;
			num /= 10;
			LocalBuf[i] = tmp + '0';
		}
		LocalBuf[i] = 0;
		
		return LocalBuf;
	}

	#undef LocalBuf
	#undef LocalBufLen

#endif