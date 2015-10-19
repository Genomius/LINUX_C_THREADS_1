#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>

#define SHIP_CAPACITY 7
#define SHIP_COUNT 5
#define PASSENGER_COUNT 10
#define CITY_COUNT 5
#define WAY_TIME 10

#define size(x) sizeof(x)/sizeof(x[0])

typedef struct PASSENGER{
	int name;
	int status;
	int place_in_the_queue;
	int city;
}PASSENGER;

typedef struct SHIP{
	int name;
	int from;
	int to;
	bool in_way;
	int on_board;
	int arrive;
}SHIP;

typedef enum PASSENGER_STATUS{
	wait = 0,
	in_way = 1,
}PASSENGER_STATUS;

typedef enum CITIES{
	omsk = 0,
	tara = 1,
	tobolsk = 2,
	mansijsk = 3,
	salehad = 4
}CITIES;

pthread_t SHIPS_THREADS[SHIP_COUNT]; // Нити кораблей
pthread_t PASSENGERS_THREADS[PASSENGER_COUNT*CITY_COUNT]; // Нити пассажиров
struct PASSENGER PASSENGERS[PASSENGER_COUNT*CITY_COUNT]; // Характеристики пассажиров
struct SHIP SHIPS[SHIP_COUNT]; // Характеристики кораблей

static void *ship_creation(void *ship);
static void *passenger_creation(void *passenger);
const  char *city_display();
static void terminal_display();

int main(int argc, char** argv)
{
	srand(time(NULL));

	int random1, random2;

	//создаем корабли
	for(int i=0;i<SHIP_COUNT;i++){
		random1 = rand() % CITY_COUNT;

		SHIPS[i].name = i;
		SHIPS[i].from = random1;

		while(true){
			random2 = rand() % CITY_COUNT;
			if(random1 != random2){
				SHIPS[i].to = random1;
				break;
			}
		}

		SHIPS[i].to = rand() % CITY_COUNT;
		SHIPS[i].in_way = false;
		SHIPS[i].on_board = 0;
		SHIPS[i].arrive = 0;

		//printf("main_thread_ship_creating = %d\n", i);
		pthread_create(&SHIPS_THREADS[i], NULL, ship_creation, (void *)&SHIPS[i]);
	}

	//создаем пассажиров
	int passenger_number = 0;
	for(int i=0;i<CITY_COUNT;i++){
		for(int j=0;j<PASSENGER_COUNT;j++){
			PASSENGERS[passenger_number].name = passenger_number;
			PASSENGERS[passenger_number].status = 0;
			PASSENGERS[passenger_number].place_in_the_queue = j;
			PASSENGERS[passenger_number].city = i;

			//printf("main_thread_passenger_creating = %d\n", passenger_number);
			pthread_create(&PASSENGERS_THREADS[passenger_number], NULL, passenger_creation, (void *)&PASSENGERS[passenger_number]);

			passenger_number++;
		}
	}

	while(true){
		terminal_display();
		sleep(1);
	}

	return 0;
}

static void *ship_creation(void *ship){
	SHIP *cur_ship = (SHIP*) ship;

	//printf("additional_thread_ship_creating = %d\n", cur_ship->from);
}

static void *passenger_creation(void *passenger){
	PASSENGER *cur_passenger = (PASSENGER*) passenger;

	//printf("additional_thread_passenger_creating = %d\n", cur_passenger->name);
}

const char* city_display(enum CITIES cities) 
{
   switch (cities) 
   {
		case omsk: return "Омск    ";
		case tara: return "Тара    ";
		case tobolsk: return "Тобольск";
		case mansijsk: return "Мансийск";
		case salehad: return "Салехад ";
   }
}

static void terminal_display(){
	system("clear");

	int cities_passenger_count[CITY_COUNT];
	int cities_ships_count[CITY_COUNT];

	for(int i=0;i<CITY_COUNT;i++){
		cities_passenger_count[i] = 0;
		cities_ships_count[i] = 0;
	}

	for(int i=0;i<size(PASSENGERS);i++)
		cities_passenger_count[PASSENGERS[i].city]++;
	
	printf("\n\n%s\n|\t\t\t\t\t\t\t\t\t\t\t|\n", "-----------------------------------------------------------------------------------------");
	printf("|\t%s", "Корабли в пути:\t\t\t\t\t\t\t\t\t|\n");
	for(int i=0;i<size(SHIPS);i++)
		if(!SHIPS[i].in_way)
			cities_ships_count[SHIPS[i].from]++;
		else
			printf("|\t%s - %s\t\tна борту %d человек\tприбудет через %dс.\t|\n", city_display(SHIPS[i].from), city_display(SHIPS[i].to), SHIPS[i].on_board, 10);


	printf("%s", "|\t\t\t\t\t\t\t\t\t\t\t|\n|\t\t\t\t\t\t\t\t\t\t\t|\n");
	for(int i=0;i<CITY_COUNT;i++){
		printf("|\t%s\t\t\tВ ожидании: человек %d\tсудов: %d\t\t|\n", city_display(i), cities_passenger_count[i], cities_ships_count[i]);
	}
	printf("|\t\t\t\t\t\t\t\t\t\t\t|\n|\t\t\t\t\t\t\t\t\t\t\t|\n|\t\t\t\t\t\t\t\t\t  by DenisBaylo |\n%s\n\n", "-----------------------------------------------------------------------------------------");
}



