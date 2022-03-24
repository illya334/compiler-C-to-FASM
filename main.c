#include <stdio.h> // printf, wprintf
#include <stdlib.h> // malloc, realloc, free
#include <stdint.h> // uint8_t
#include <wchar.h> // wchar_t

#define wchar wchar_t
#define uint unsigned int
#define reg register
#define bool uint8_t
#define byte uint8_t

#define true 1
#define false 0

/*
void exit(uint errCode){
	
}
*/

void error(char *str){ // тупо вывод ощибки ASCII
	printf("Error: %s\n", str);
	exit(1);
}

wchar *openfile(wchar *path, bool retError){ // открывает файл и копирует содержимое в масив
	FILE *fp = _wfopen(path, L"rt");
	if(fp == NULL && retError == false) error("I can't open the file. Maybe invalid path."); else
	if(fp == NULL && retError == true) return NULL;
	
	wchar *buf = malloc( ( 1000 + 1 ) * sizeof(wchar) );
	uint MaxIndex = 1000;
	
	reg uint i = 0;
	while(1){
		if(i >= MaxIndex){
			MaxIndex += 1000;
			buf = realloc(buf, (MaxIndex + 1) * sizeof(wchar) );
		}
		buf[i] = fgetwc(fp);
		
		if( buf[i] == 0xFFFF ) break;
		
		i++;
	}
	buf[i] = 0;
	return buf;
}

int main(){
	
	wchar *out = malloc(200);
	
	_preproc(out, L"#define ABC(a, b) 123\n#ifndef ABC\n123\n#endif");
	
	wprintf(out);
	
	return 0;
}