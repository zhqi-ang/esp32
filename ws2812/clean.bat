@echo off

@echo start kill .o .d
move build\*.bin .\
move build\*.map .\
move build\*.elf .\

del /s /q build\*.* 

move *.bin build\
move *.map build\
move *.elf build\

@echo run success!!!
