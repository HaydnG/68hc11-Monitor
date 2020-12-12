@echo off
if not exist %1.s07 goto trap2
copy %1.s07 %1.asm
cas11 -l %1.asm
echo Your S file is called %1.s19
echo Your list/error file is called %1.lst
type %1.lst
del %1.asm
goto byebye
:trap1 
echo Missing source filename
goto byebye
:trap2 
echo Cannot find filename %1.s07
goto byebye
:byebye
