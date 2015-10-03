#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <termios.h>

#define tableCoordX 60
#define tableCoordY 2


enum color
{
	Red = 1,
	Green = 2,
	Yellow = 3,
	Blue = 4,
	Magenta = 5,
	Cyan = 6,
	White = 7,
	Black = 8
};

enum status//статусы мудрецов
{
	think = 1,
	eat = 2,
	wait = 3
};

#define backcolor Yellow//цвет стола 

enum theme//отображение текста
{
	Normal = 0,//нормально
	Light = 1,//яркое
	Dark = 2,//затемненное
	Underlined = 4,//подчеркнутое
	Shaded = 7//закрашенное
};

struct character//харектеристики мудреца
{
	char* name;//имя
	int color;//цвет
	int status;//ткущий статус
};


char letter[1] = " ";//управляющий символ
int terminal;//хендл терминала
int freeSticksCount = 5;//количество свободных палочек
pthread_mutex_t chanise_sticks[5];//китайские палочки для еды
pthread_t display;//нить отрисовки стола и мудрецов
pthread_t wiseMen[7];//мудрецы-нити
struct termios new_mode, standart_mode;//режимы консольного ввода с клавиатуры
struct character wiseman_characters[7];//массив характеристик мудрецов


void cleanAll()//отчистка экрана и возвращение стандартного режима консоли
{
	int i;
	tcsetattr(terminal,TCSAFLUSH, &standart_mode);
	write(1, "\033[2J\033[0m", 8);
	for(i = 0; i < 5; i++)
	{
		pthread_mutex_destroy(&chanise_sticks[i]);
	}
}

void drawLine(int X, int Y, int lenght, int theme, int color)//нарисовать линию
{	
	int i;
	for(i = 0; i < lenght; i++)
	{
		printf("\033[%d;%dH\033[%d;3%dm=\n", Y, X+i, theme, color);
	} 
	write(1,"\033[0m",4);
}

void drawText(char* message, int X, int Y, int theme, int color)//отобразить текст
{
	printf("\033[%d;%dH\033[%d;3%dm%s\033[0m\n", Y, X, theme, color, message);
}

void drawSticks(int X, int Y, int color, int background, int Count)//отрисовка нужного количества китайскиз палочек
{
	int i;
	for(i = 0; i < Count; i++)	
	{
		printf("\033[%d;%dH\033[0;3%d;4%dm|\033[0m\n", Y, X+i, color, background);
	}
}

void drawTable(int X, int Y)//отрисовка стола
{
	int i, j;
	for(j = 0; j < 11; j++)
	{
		for(i = 0; i < 31; i++)
		{
			printf("\033[%d;%dH\033[0;4%dm \033[0m\n", Y+j, X+i, backcolor);
		}
	}
}

void displayFunc()//функция отображения на экран стола и таблицы мудрецов
{
	int i;
	while(strchr(letter, 'q') == NULL)
	{
		drawTable(tableCoordX, tableCoordY);//нарисовать стол
		drawSticks(tableCoordX + 13, tableCoordY + 5, Black, backcolor, freeSticksCount);//нарисовать не занятые палочки в центре стола
		for(i = 0; i < 7; i ++)//отрисовка таблицы мудрецов
		{
			drawLine(10, 1 + i*2, 45, Underlined, Black);//линия-разделитель
			drawText(wiseman_characters[i].name, 10, 2 + i*2, Light, wiseman_characters[i].color);//имя
			switch(wiseman_characters[i].status)//статус
			{
				case think:
				{
					drawText("***Now thinking***", 35, 2 + i*2, Light, wiseman_characters[i].color);
				}break;		
				case eat:
				{
					drawText("****Now eating****", 35, 2 + i*2, Light, wiseman_characters[i].color);
				}break;	
				case wait:
				{
					drawText("Waiting free stick", 35, 2 + i*2, Light, wiseman_characters[i].color);
				}break;	
				default: break; 
			}
		}
		usleep(100000);
	}
}

int catchSticks(int index, int dTX, int dTY, int Color, int sticks_indexes[])//функция хватания палочек, принимает параметры выводимых сообщений
{
	int i,sticksCount = 0;	
	while(sticksCount < 2)//до тех пор пока не схвачено 2 палочки(заблокированно 2 мьютекса)
	{
		for(i = 0; i < 5; i++)//пытаться схватить палочку (заблокировать мьютекс)
		{
			if(pthread_mutex_trylock(&chanise_sticks[i]) == 0)//в случае если удалось схватить палочку
			{
				sticks_indexes[sticksCount] = i;//сохранить индекс захваченной палочки
				drawSticks(tableCoordX + dTX + sticksCount, tableCoordY + dTY, Color, White, 1);//нарисовать палочку радом с мудрецом за столом
				sticksCount ++;//увеличить количество схваченных палочек
				freeSticksCount --;//уменьшить количесиво свободных палочек
				break;//и начать цикл попыток схватить палочку заново
			}
		}
		wiseman_characters[index].status = wait;//установка статуса в ожидание
	}
	return 0;
}

int putSticks(int dTX, int dTY, int sticks_indexes[])//функция возврата палочек на место (разблокирования мьютексов)
{
	int i;
	for(i = 0; i < 2; i++)//попытаться разблокировать все мьютексы
	{
		pthread_mutex_unlock(&chanise_sticks[sticks_indexes[i]]);
		drawText("1", tableCoordX + dTX + i, tableCoordY + dTY, Light, White);//закрасить палочку рядом с мудрецом за столом
		freeSticksCount ++;//увеличить количесиво свободных палочек
	}
	return 0;
}

void toEat(int index, int dTX, int dTY, int Color)//Функция питания мудреца
{
	int time, i, sticks_indexes[2];
	time = 3+rand()%5;//выбор времени на перекус
	drawText("O", tableCoordX + dTX, tableCoordY + dTY, Shaded, Color);//Изменение отображения мудреца за столом
	if(catchSticks(index, dTX+1, dTY, Color, sticks_indexes) == 0)//захватить палочки
	{
		wiseman_characters[index].status = eat;//установить статус "ест
	}
	sleep(time);//выполнить задержку на выбранное время
	putSticks(dTX + 1, dTY, sticks_indexes);//положить палочки на место
}

void wiseman1()//1-ый мудрец
{
	int time, i;
	int dTX = 0, dTY = -1;  
	time = 3+rand()%5;
	//заполнение характеристик мудреца
	wiseman_characters[0].name = "Mao"; 
	wiseman_characters[0].color = Red; 
	

	while(strchr(letter, 'q') == NULL || strchr(letter, 'Q') == NULL)//основной цикл
	{		
		wiseman_characters[0].status = think;//установить статус "размышляет"
		drawText("X", tableCoordX + dTX, tableCoordY + dTY, Shaded, wiseman_characters[0].color);//установить отображение мудреца за столом
		time = 10000 * (3+rand()%6);
		for(i = 0; i < time; i++)
		{	
			if(strchr(letter, 'a') != NULL || strchr(letter, 'A') != NULL)
			{
				letter[0] = ' ';
				break;
			}
			usleep(100);
		}
		toEat(0, dTX, dTY, wiseman_characters[0].color);
	}
}

void wiseman2()//2-ый мудрец
{
	int time, i;
	int dTX = 14, dTY = -1;  
	time = 3+rand()%5;

	wiseman_characters[1].name = "Siao"; 
	wiseman_characters[1].color = Green; 

	while(strchr(letter, 'q') == NULL)
	{
		wiseman_characters[1].status = think;
		drawText("X", tableCoordX + dTX, tableCoordY + dTY, Shaded, wiseman_characters[1].color);
		time = 10000 * (3+rand()%6);
		for(i = 0; i < time; i++)
		{	
			if(strchr(letter, 's') != NULL || strchr(letter, 'S') != NULL)
			{
				letter[0] = ' ';
				break;
			}
			usleep(100);
		}
		toEat(1, dTX, dTY, wiseman_characters[1].color);
	}
}

void wiseman3()//3-ый мудрец
{
	int time, i;
	int dTX = 28, dTY = -1;  
	time = 3+rand()%5;

	wiseman_characters[2].name = "Hao"; 
	wiseman_characters[2].color = Yellow; 

	while(strchr(letter, 'q') == NULL)
	{
		wiseman_characters[2].status = think;
		drawText("X", tableCoordX + dTX, tableCoordY + dTY, Shaded, wiseman_characters[2].color);
		time = 10000 * (3+rand()%6);
		for(i = 0; i < time; i++)
		{	
			if(strchr(letter, 'd') != NULL || strchr(letter, 'D') != NULL)
			{
				letter[0] = ' ';
				break;
			}
			usleep(100);
		}
		toEat(2, dTX, dTY, wiseman_characters[2].color);
	}
}

void wiseman4()//4-ый мудрец
{
	int time, i;
	int dTX = 0, dTY = 11;  
	time = 3+rand()%5;

	wiseman_characters[3].name = "Pel'"; 
	wiseman_characters[3].color = Blue; 

	while(strchr(letter, 'q') == NULL)
	{
		wiseman_characters[3].status = think;
		drawText("X", tableCoordX + dTX, tableCoordY + dTY, Shaded, wiseman_characters[3].color);
	33	time = 10000 * (3+rand()%6);
		for(i = 0; i < time; i++)
		{	
			if(strchr(letter, 'f') != NULL || strchr(letter, 'F') != NULL)
			{
				letter[0] = ' ';
				break;
			}
			usleep(100);
		}
		toEat(3, dTX, dTY, wiseman_characters[3].color);
	}
}

void wiseman5()//5-ый мудрец
{
	int time, i;
	int dTX = 14, dTY = 11;  
	time = 3+rand()%5;

	wiseman_characters[4].name = "Men'"; 
	wiseman_characters[4].color = Magenta; 

	while(strchr(letter, 'q') == NULL)
	{
		wiseman_characters[4].status = think;
		drawText("X", tableCoordX + dTX, tableCoordY + dTY, Shaded, wiseman_characters[4].color);
		time = 10000 * (3+rand()%6);
		for(i = 0; i < time; i++)
		{	
			if(strchr(letter, 'g') != NULL || strchr(letter, 'G') != NULL)
			{
				letter[0] = ' ';
	3			break;
			}
			usleep(100);
		}
		toEat(4, dTX, dTY, wiseman_characters[4].color);
	}
}

void wiseman6()//6-ый мудрец
{
	int time, i;
	int dTX = 28, dTY = 11;
	time = 3+rand()%5;

	wiseman_characters[5].name = "Lee"; 
	wiseman_characters[5].color = Cyan; 

	while(strchr(letter, 'q') == NULL)
	{
		wiseman_characters[5].status = think;
		drawText("X", tableCoordX + dTX, tableCoordY + dTY, Shaded, wiseman_characters[5].color);
		time = 10000 * (3+rand()%6);
		for(i = 0; i < time; i++)
		{	
			if(strchr(letter, 'h') != NULL || strchr(letter, 'H') != NULL)
			{
				letter[0] = ' ';
				break;
			}
			usleep(100);
		}
		toEat(5, dTX, dTY, wiseman_characters[5].color);
	}
}

void wiseman7()//7-ый мудрец
{
	int time, i;
	int dTX = 31, dTY = 5;
	time = 3+rand()%5;

	wiseman_characters[6].name = "Chang"; 
	wiseman_characters[6].color = Black; 

	while(strchr(letter, 'q') == NULL)
	{
		wiseman_characters[6].status = think;
		drawText("X", tableCoordX + dTX, tableCoordY + dTY, Shaded, wiseman_characters[6].color);
		time = 10000 * (3+rand()%6);
		for(i = 0; i < time; i++)
		{	
			if(strchr(letter, 'j') != NULL || strchr(letter, 'J') != NULL)
			{
				letter[0] = ' ';
				break;
			}
			usleep(100);
		}
		toEat(6, dTX, dTY, wiseman_characters[6].color);
	}
}





void main()
{
	int i,WM[7], dispID;//итератор идентификаторы мудрецов WiseMan и нити отрисовки
	srand(time(NULL));
	
	void  (*wmThr[7])();//массив указателей на функции-нити
	wmThr[0] = &wiseman1;
	wmThr[1] = &wiseman2;
	wmThr[2] = &wiseman3;
	wmThr[3] = &wiseman4;
	wmThr[4] = &wiseman5;
	wmThr[5] = &wiseman6;
	wmThr[6] = &wiseman7;
	write(1, "\033[2J", 4);//отчистка экрана
	terminal = open("/dev/tty", O_RDONLY);//изменение режима терминала
	if( terminal == -1)
	{
		drawLine(10,5,100, Light,Red);//сообщение об ошибке
		drawText("**ERROR!**", 50, 7, Shaded, Red); 
		drawLine(10,9,100, Light,Red);
		sleep(1);
		return;	
	}
	tcgetattr(terminal, &standart_mode);
	tcgetattr(terminal, &new_mode);
	new_mode.c_lflag &= ~(ICANON | ECHO);
	new_mode.c_cc[VMIN]=1;
	new_mode.c_cc[VTIME]=0;
	tcsetattr(terminal,TCSAFLUSH, &new_mode);//изменение режима терминала
	for(i = 0; i < 5; i++)//инициализация палочек
	{
		pthread_mutex_init(&chanise_sticks[i], NULL);
	}
	for(i = 0; i < 7; i++)//инициализация мудрецов
	{ 
		WM[0] = pthread_create(&wiseMen[i], NULL, wmThr[i], (void*)1);
	}	
	dispID = pthread_create(&display, NULL, (void*)displayFunc, (void*)1);//начало отображения
	while(strchr(letter, 'q') == NULL)
	{	
		read(terminal, letter, 1);
	}
	cleanAll();
	return;
}