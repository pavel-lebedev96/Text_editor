#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <curses.h>

//размер буфера
#define MAXLEN 1024

//коды символов
#define KEY_ESC 27
#define KEY_NEWLINE 10
#define KEY_TAB 9

//структура для хранения положения курсора
typedef struct
{
	int x, y;
} TCursPos;

//вставка символа в text в позицию pos со сдвигом остальных символов
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

//удаление символа из text с позиции pos со сдвигом остальных символов
bool delete_symbol(int text_pos, char *text)
{
	int size = (int) strlen(text);
	if (text_pos < 0 || text_pos >= size || size == 0)
		return false;
	for (int i = text_pos; i < size; i++)
		text[i] = text[i + 1];
	return true;
}

//загрузка текста из файла с пропуском символа возврата каретки
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

//запись текста в файл
bool text_output(const char* file_name, char* text)
{
	FILE* file = fopen(file_name, "w");
	if (!file)
		return false;
	fputs(text, file);
	fclose(file);
	return true;
}

/*возвращает следующее положение курсора и текста через указатели curs_pos и text_pos в зависимости от текущего символа в text;
записывает текущее положение курсора в таблицу curs_pos_table*/ 
bool next_pos(TCursPos *curs_pos, int *text_pos, const char* text, TCursPos *curs_pos_table)
{
	if (text[*text_pos] == '\0')
		return false;
	
	//получение ширины экрана
	int max_x = getmaxx(stdscr);

	//запоминание текущего положения курсора
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
	
	//обычный символ
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

//тоже самое, что и next_pos, только с перемещением курсора и обновлением окна
bool move_next_pos(TCursPos* curs_pos, int* text_pos, const char* text, TCursPos* curs_pos_table)
{
	if (!next_pos(curs_pos, text_pos, text, curs_pos_table))
		return false;
	move(curs_pos->y, curs_pos->x);
	refresh();
	return true;
}

/*возвращает предыдущее положение курсора и текста через указатели curs_pos и text_pos в зависимости от значения, 
записанного в таблице curs_pos_table*/
bool prev_pos(TCursPos *curs_pos, int *text_pos, const char* text, const TCursPos *curs_pos_table)
{
	if (*text_pos == 0)
		return false;
	*curs_pos = curs_pos_table[--(*text_pos)];
	return true;
}

//тоже самое, что и prev_pos только с перемещением курсора и обновлением окна
bool move_prev_pos(TCursPos* curs_pos, int* text_pos, const char* text, const TCursPos* curs_pos_table)
{
	if (!prev_pos(curs_pos, text_pos, text, curs_pos_table))
		return false;
	move(curs_pos->y, curs_pos->x);
	refresh();
	return true;
}

//вывод текста с позиции text_pos c возвращением курсора
void text_print(const TCursPos *curs_pos, int text_pos, const char* text)
{
	printw("%s", text + text_pos);
	move(curs_pos->y, curs_pos->x);
	refresh();
}

//очистка окна и вывод всего текста
void text_print_clear(const TCursPos* curs_pos, const char* text)
{
	clear();
	printw("%s", text);
	move(curs_pos->y, curs_pos->x);
	refresh();
}

//вывод на текущие позиции curs_pos и text_pos символа с в окно
bool insert_char(char c, TCursPos *curs_pos, int *text_pos, char* text, TCursPos* curs_pos_table)
{
	//вставка символа в текст
	if (!add_symbol(c, *text_pos, text))
		return false;
	//вывод части текста с новым символом
	text_print(curs_pos, *text_pos, text);
	//переместить курсор на следующий символ
	return move_next_pos(curs_pos, text_pos, text, curs_pos_table);
}

//удаление символа с предыдущей позиции в окне 
bool erase_char(TCursPos *curs_pos, int *text_pos, char* text, TCursPos* curs_pos_table)
{
	//перемещаемся на предыдущий символ
	if (!move_prev_pos(curs_pos, text_pos, text, curs_pos_table))
		return false;	
	//удаляем символ с текущей позиции
	delete_symbol(*text_pos, text);	
	//вывод части текста с новым символом
	text_print_clear(curs_pos, text);
	return true;
}

int main(int argc, char* argv[])
{
	//имя файла
	char file_name[100] = "";
	
	//буфер
	char text[MAXLEN] = "";

	//таблица положений курсора
	TCursPos curs_pos_table[MAXLEN] = { {0, 0} };
	
	//текущее положение курсора
	TCursPos curs_pos = { 0, 0 };
	
	//текущая позиция в тексте
	int text_pos = 0;
	
	int c;
	
	if (argc > 2)
	{
		printf("Incorrect number of arguments\n");
		return 1;
	}
	
	//если указан файл, прочитать содержимое
	if (argc == 2)
	{
		strncpy(file_name, argv[1], 100);
		text_input(file_name, text);
	}

	//инициализация
	initscr();
	cbreak();
	keypad(stdscr, true);
	curs_set(true);
	noecho();
	
	//вывод текста, если он есть
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

	//если файл не указан при вызове, спросить
	if (strlen(file_name) == 0)
	{
		printf("Enter the file name:\n");
		scanf("%s", file_name);
	}
	
	//вывод текста в файл
	if (text_output(file_name, text))
		printf("The file is successfully changed or created\n");
	else
		printf("Error!\n");
	return 0;
}