#Makefile for TextEditor
#Создание исполняемого модуля
binary: source.c
	gcc -o program source.c -lncurses
#End Makefile
