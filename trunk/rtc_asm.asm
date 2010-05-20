CPU 686
BITS 32

%include 'rtc.inc'

global _get_time

extern _read_rtcv
extern _write_rtcv
extern _bcd2dec
extern _data_mode

section .text

_get_time:
	push ebp
	mov ebp, esp
	push ebx
	mov ecx, [_data_mode]
	
	push RTC_HOUR
	call _read_rtcv
	add esp, 4
	push eax
	cmp ecx,1
	je hora
	call _bcd2dec
	add esp, 4
	
	hora:
	mov ebx, arg1
	mov [ebx], eax
	
	push RTC_MIN
	call _read_rtcv
	add esp, 4
	push eax
	je min
	call _bcd2dec
	add esp, 4
	min:
	mov ebx, arg2
	mov [ebx], eax
	
	push RTC_SEC
	call _read_rtcv
	add esp, 4
	push eax
	je sec
	call _bcd2dec
	add esp, 4
	sec:
	mov ebx, arg3
	mov [ebx], eax
	
	pop ebx
	mov esp, ebp
	pop ebp
	ret
END
	
	