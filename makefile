#Makefile for TextEditor
#�������� ������������ ������
binary: source.c
	gcc -o program source.c -lncurses
#End Makefile
