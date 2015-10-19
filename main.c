#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define SHIP_CAPACITY 7
#define SHIP_COUNT 5
#define PASSENGER_COUNT 10

typedef struct PASSENGER{
	int name;
	int status;
	int place_in_the_queue;
}PASSENGER;

typedef struct SHIP{
	int name;
	int from;
	int to;
}SHIP;

enum PASSENGER_STATUS{
	wait = 0,
	in_way = 1,
};

enum CITIES{
	omsk = 0,
	tara = 1,
	tobolsk = 2,
	hanti_mansijsk = 3,
	salehad = 4
};

pthread_t SHIPS_THREADS[SHIP_COUNT]; // Нити кораблей
pthread_t PASSENGERS_THREADS[PASSENGER_COUNT]; // Нити пассажиров
struct PASSENGER PASSENGERS[PASSENGER_COUNT]; // Характеристики пассажиров
struct SHIP SHIPS[SHIP_COUNT]; // Характеристики кораблей

static void *ship_creation(void *ship);

int main(int argc, char** argv)
{
	//создаем корабли
	for(int i=0;i<SHIP_COUNT;i++){
		SHIPS[i].name = i;
		SHIPS[i].from = i;
		SHIPS[i].to = i;
		printf("main = %d\n", i);
		pthread_create(&SHIPS_THREADS[i], NULL, ship_creation, (void *)&SHIPS[i]);
	}

	return 0;
}

static void *ship_creation(void *ship){
	SHIP *cur_ship = (SHIP*) ship;

	printf("additional = %d\n", cur_ship->name);
}