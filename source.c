#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <curses.h>

//������ ������
#define MAXLEN 1024

//���� ��������
#define KEY_ESC 27
#define KEY_NEWLINE 10
#define KEY_TAB 9

//��������� ��� �������� ��������� �������
typedef struct
{
	int x, y;
} TCursPos;

//������� ������� � text � ������� pos �� ������� ��������� ��������
bool add_symbol(char c, int text_pos, char *text)
{
	int size = (int) strlen(text);
	if (text_pos < 0 || text_pos > size || size == MAXLEN - 1)
		return false;
	for (int i = size; i >= text_pos; i--)
		text[i + 1] = text[i];
	text[text_pos] = c;
	return true;
}

//�������� ������� �� text � ������� pos �� ������� ��������� ��������
bool delete_symbol(int text_pos, char *text)
{
	int size = (int) strlen(text);
	if (text_pos < 0 || text_pos >= size || size == 0)
		return false;
	for (int i = text_pos; i < size; i++)
		text[i] = text[i + 1];
	return true;
}

//�������� ������ �� ����� � ��������� ������� �������� �������
bool text_input(const char* file_name, char *text)
{
	FILE* file = fopen(file_name, "r");
	if (!file)
		return false;
	char c;
	int i = 0;
	while ((c = (char) fgetc(file)) != EOF && i < MAXLEN - 1)
	{
		if (c == '\r')
			continue;
		text[i++] = c;
	}
	text[i] = '\0';
	fclose(file);
	return true;
}

//������ ������ � ����
bool text_output(const char* file_name, char* text)
{
	FILE* file = fopen(file_name, "w");
	if (!file)
		return false;
	fputs(text, file);
	fclose(file);
	return true;
}

/*���������� ��������� ��������� ������� � ������ ����� ��������� curs_pos � text_pos � ����������� �� �������� ������� � text;
���������� ������� ��������� ������� � ������� curs_pos_table*/ 
bool next_pos(TCursPos *curs_pos, int *text_pos, const char* text, TCursPos *curs_pos_table)
{
	if (text[*text_pos] == '\0')
		return false;
	
	//��������� ������ ������
	int max_x = getmaxx(stdscr);

	//����������� �������� ��������� �������
	curs_pos_table[*text_pos] = *curs_pos;
	
	switch (text[(*text_pos)++])
	{
	case '\n':
		curs_pos->x = 0;
		curs_pos->y++;
		break;
	case '\t':
		while (++curs_pos->x % 8 != 0){}
		break;
	
	//������� ������
	default:
		curs_pos->x++;
	}
	
	if (curs_pos->x >= max_x)
	{
		curs_pos->x = 0;
		curs_pos->y++;
	}
	return true;
}

//���� �����, ��� � next_pos, ������ � ������������ ������� � ����������� ����
bool move_next_pos(TCursPos* curs_pos, int* text_pos, const char* text, TCursPos* curs_pos_table)
{
	if (!next_pos(curs_pos, text_pos, text, curs_pos_table))
		return false;
	move(curs_pos->y, curs_pos->x);
	refresh();
	return true;
}

/*���������� ���������� ��������� ������� � ������ ����� ��������� curs_pos � text_pos � ����������� �� ��������, 
����������� � ������� curs_pos_table*/
bool prev_pos(TCursPos *curs_pos, int *text_pos, const char* text, const TCursPos *curs_pos_table)
{
	if (*text_pos == 0)
		return false;
	*curs_pos = curs_pos_table[--(*text_pos)];
	return true;
}

//���� �����, ��� � prev_pos ������ � ������������ ������� � ����������� ����
bool move_prev_pos(TCursPos* curs_pos, int* text_pos, const char* text, const TCursPos* curs_pos_table)
{
	if (!prev_pos(curs_pos, text_pos, text, curs_pos_table))
		return false;
	move(curs_pos->y, curs_pos->x);
	refresh();
	return true;
}

//����� ������ � ������� text_pos c ������������ �������
void text_print(const TCursPos *curs_pos, int text_pos, const char* text)
{
	printw("%s", text + text_pos);
	move(curs_pos->y, curs_pos->x);
	refresh();
}

//������� ���� � ����� ����� ������
void text_print_clear(const TCursPos* curs_pos, const char* text)
{
	clear();
	printw("%s", text);
	move(curs_pos->y, curs_pos->x);
	refresh();
}

//����� �� ������� ������� curs_pos � text_pos ������� � � ����
bool insert_char(char c, TCursPos *curs_pos, int *text_pos, char* text, TCursPos* curs_pos_table)
{
	//������� ������� � �����
	if (!add_symbol(c, *text_pos, text))
		return false;
	//����� ����� ������ � ����� ��������
	text_print(curs_pos, *text_pos, text);
	//����������� ������ �� ��������� ������
	return move_next_pos(curs_pos, text_pos, text, curs_pos_table);
}

//�������� ������� � ���������� ������� � ���� 
bool erase_char(TCursPos *curs_pos, int *text_pos, char* text, TCursPos* curs_pos_table)
{
	//������������ �� ���������� ������
	if (!move_prev_pos(curs_pos, text_pos, text, curs_pos_table))
		return false;	
	//������� ������ � ������� �������
	delete_symbol(*text_pos, text);	
	//����� ����� ������ � ����� ��������
	text_print_clear(curs_pos, text);
	return true;
}

int main(int argc, char* argv[])
{
	//��� �����
	char file_name[100] = "";
	
	//�����
	char text[MAXLEN] = "";

	//������� ��������� �������
	TCursPos curs_pos_table[MAXLEN] = { {0, 0} };
	
	//������� ��������� �������
	TCursPos curs_pos = { 0, 0 };
	
	//������� ������� � ������
	int text_pos = 0;
	
	int c;
	
	if (argc > 2)
	{
		printf("Incorrect number of arguments\n");
		return 1;
	}
	
	//���� ������ ����, ��������� ����������
	if (argc == 2)
	{
		strncpy(file_name, argv[1], 100);
		text_input(file_name, text);
	}

	//�������������
	initscr();
	cbreak();
	keypad(stdscr, true);
	curs_set(true);
	noecho();
	
	//����� ������, ���� �� ����
	if (strlen(text) != 0)
		text_print_clear(&curs_pos, text);
	
	while ((c = getch()) != KEY_ESC)
	{
		switch (c)
		{
			case KEY_LEFT:
				move_prev_pos(&curs_pos, &text_pos, text, curs_pos_table);
				break;
			case KEY_RIGHT:
				move_next_pos(&curs_pos, &text_pos, text, curs_pos_table);
				break;
			case KEY_BACKSPACE:
				erase_char(&curs_pos, &text_pos, text, curs_pos_table);
				break;
			default:
				if (isprint(c) || c == KEY_TAB || c == KEY_NEWLINE)
					insert_char((char) c, &curs_pos, &text_pos, text, curs_pos_table);
		}
	}
	endwin();

	//���� ���� �� ������ ��� ������, ��������
	if (strlen(file_name) == 0)
	{
		printf("Enter the file name:\n");
		scanf("%s", file_name);
	}
	
	//����� ������ � ����
	if (text_output(file_name, text))
		printf("The file is successfully changed or created\n");
	else
		printf("Error!\n");
	return 0;
}