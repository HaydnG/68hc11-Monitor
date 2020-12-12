@echo off
set Cosmic_license_File=27400@crwnfcetls1
if "%1"== "" GOTO :EOF

if NOT EXIST %1 echo File does not exist
if NOT EXIST %1 GOTO :EOF

copy %1 "xxxx_01.c"

cx6811 -v -l +debug "xxxx_01.c"

if ERRORLEVEL 1 GOTO :CLEANUP

echo Compilation succesful......

clnk -o "linked.h11" -p -m "map.txt" "cram.lkf"

if ERRORLEVEL 1 GOTO :CLEANUP

echo Linking Succesful.....

chex -fm -h -s -e0x400 -o "outs19.txt" "linked.h11"

if ERRORLEVEL 1 GOTO :CLEANUP

echo Hex conversion succesful - output file is called outs19.txt
echo Linker map file  generated - output file is called map.txt
echo C/Assembly listing generated - output file is called assem.txt
echo New C program start  is 400 hex

:CLEANUP
if EXIST "assem.txt" del "assem.txt"
if EXIST "xxxx_01.ls" ren "xxxx_01.ls" "assem.txt"
if EXIST "xxxx_01.c" del "xxxx_01.c"
if EXIST "linked.h11" del "linked.h11"
if EXIST "xxxx_01.o" del "xxxx_01.o"
