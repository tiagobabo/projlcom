%define RTC_ADD_REG 70h
%define RTC_DATA_REG 71h

%define RTC_SEC 0
%define RTC_MIN 2
%define RTC_HOUR 4

%define RTC_STAT_A 10
%define RTC_STAT_B 11
%define RTC_STAT_C 12
%define RTC_STAT_D 13

%define RTC_SET 10000000b

; function arguments, assuming return adddress and old basepointer (ebp) in stack
%define arg1 [ebp+8]
%define arg2 [ebp+12]
%define arg3 [ebp+16]
%define arg4 [ebp+20]