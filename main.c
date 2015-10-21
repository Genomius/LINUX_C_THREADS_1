#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#define SHIP_CAPACITY 7
#define SHIP_COUNT 5
#define PASSENGER_COUNT 10
#define CITY_COUNT 5
#define WAY_TIME 10

#define size(x) sizeof(x)/sizeof(x[0])
#define log printf

typedef struct PASSENGER{
	int id;
	bool in_way;
	int city;
	int ship;
}PASSENGER;

typedef struct SHIP{
	int id;
	int from;
	int to;
	bool in_way;
	int on_board;
	int arrive_time;
}SHIP;

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

		SHIPS[i].id = i;
		SHIPS[i].from = random1;

		while(true){
			random2 = rand() % CITY_COUNT;
			//printf("%d - %d\n", random1, random2);

			if(random1 != random2){
				SHIPS[i].to = random2;
				break;
			}			
		}
		
		SHIPS[i].in_way = false;
		SHIPS[i].on_board = 0;
		SHIPS[i].arrive_time = 0;

		pthread_create(&SHIPS_THREADS[i], NULL, ship_creation, (void *)&SHIPS[i]);
	}

	//создаем пассажиров
	int passenger_number = 0;
	for(int i=0;i<CITY_COUNT;i++){
		for(int j=0;j<PASSENGER_COUNT;j++){
			PASSENGERS[passenger_number].id = passenger_number;
			PASSENGERS[passenger_number].in_way = false;
			PASSENGERS[passenger_number].city = i;
			PASSENGERS[passenger_number].ship = 0;

			pthread_create(&PASSENGERS_THREADS[passenger_number], NULL, passenger_creation, (void *)&PASSENGERS[passenger_number]);

			passenger_number++;
		}
	}

	while(true){ // Каждую секунду обновляем консоль
		terminal_display();
		sleep(1);
	}

	return 0;
}

static void *ship_creation(void *ship){
	SHIP *cur_ship = (SHIP*) ship; // Текущий корабль

	int wait_time = rand() % 3+1;
	sleep(wait_time); // Перед очередной рейсом ждем случайное время от 1 до 3 секунд

	while(true){ // Работа кора бля
		int city_passanger_count;

		while(!SHIPS[cur_ship->id].in_way){
			city_passanger_count = 0;

			for(int i=0;i<CITY_COUNT*PASSENGER_COUNT;i++){ // Узнаем есть ли в этом городе еще пассажиры
				if(PASSENGERS[i].city == SHIPS[cur_ship->id].from){
					city_passanger_count++;
				}
			}

			if(SHIPS[cur_ship->id].on_board == 7){ // Отплываем, если корабль полон или в городе не осталось пассажиров
				SHIPS[cur_ship->id].in_way = true;
				//log("Корабль %d выплывает\t Пассажиров: %d\t В городе %s осталось %d\n", cur_ship->id, SHIPS[cur_ship->id].on_board, city_display(SHIPS[cur_ship->id].from), city_passanger_count);
			}
		}
	
		int way_time = 10;
		while(true){ // Пока корабль в пути
			SHIPS[cur_ship->id].arrive_time = way_time;
			if(way_time == 0){ // Корабль прибыл в порт
				SHIPS[cur_ship->id].in_way = false;
				SHIPS[cur_ship->id].from = SHIPS[cur_ship->id].to;

				while(true){
					int random2 = rand() % CITY_COUNT;

					if(SHIPS[cur_ship->id].from != random2){
						SHIPS[cur_ship->id].to = random2;
						break;
					}
				}

				break;
			}
			way_time--; // Время в пути снижается на секунду
			sleep(1);
		}
	}
}

static void *passenger_creation(void *passenger){
	PASSENGER *cur_passenger = (PASSENGER*) passenger; // Текущий пассажир

	while(true){ // Жизнедеятельность пассажира
		while(!PASSENGERS[cur_passenger->id].in_way){ // Пока ищем свободный корабль в нашем порту
			for(int i=0;i<SHIP_COUNT;i++){ // Находим корабль, находящийся в порту того же города, что и пассажир
				if(!SHIPS[i].in_way && SHIPS[i].from == cur_passenger->city && SHIPS[i].on_board < SHIP_CAPACITY){
					PASSENGERS[cur_passenger->id].in_way = true; // Меняем статус на "В пути"
					PASSENGERS[cur_passenger->id].ship = SHIPS[i].id; // Указываем на каком корабле поплывет пассажир

					SHIPS[i].on_board++; // Занимаем место на корабле

					sleep(1); // Ждем секунду, пока пассажир заходит на борт корабля :)
					break;
				}
			}
		}

		while(PASSENGERS[cur_passenger->id].in_way){ // Пока пассажир на борту
			if(!SHIPS[PASSENGERS[cur_passenger->id].ship].in_way){ // Если корабль прибыл в порт пункта назначения
				PASSENGERS[cur_passenger->id].in_way = false; // Прибыли, ставим статус на ожидание к следующей отправки

				SHIPS[PASSENGERS[cur_passenger->id].ship].on_board--; // Освобождаем место на корабле

				sleep(1); // Подождем секунду, пока пассажир сойдет с корабля :)
				PASSENGERS[cur_passenger->id].city = SHIPS[PASSENGERS[cur_passenger->id].ship].from; // Теперь пассажир в городе, в который нас доставляет корабль
			}
		}
	}
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
		if(!PASSENGERS[i].in_way){
			cities_passenger_count[PASSENGERS[i].city]++;
		}
	
	printf("\n\n%s\n   |\t\t\t\t\t\t\t\t\t\t\t|\n", "   +------------------------------------------------------------------------------------+");
	printf("   |\t%s", "Корабли в пути:\t\t\t\t\t\t\t\t\t|\n");
	for(int i=0;i<size(SHIPS);i++){
		if(!SHIPS[i].in_way)
			cities_ships_count[SHIPS[i].from]++;
		else
			printf("   |\t%d %s - %s\t\tна борту %d человек\tприбудет через %dс.\t|\n", SHIPS[i].id+1, city_display(SHIPS[i].from), city_display(SHIPS[i].to), SHIPS[i].on_board, SHIPS[i].arrive_time);
	}

	printf("%s", "   |\t\t\t\t\t\t\t\t\t\t\t|\n   |\t\t\t\t\t\t\t\t\t\t\t|\n");
	for(int i=0;i<CITY_COUNT;i++){
		printf("   |\t%s\t\t\tВ ожидании: человек %d\tсудов: %d\t\t|\n", city_display(i), cities_passenger_count[i], cities_ships_count[i]);
	}
	printf("   |\t\t\t\t\t\t\t\t\t\t\t|\n   |\t\t\t\t\t\t\t\t\t\t\t|\n   |\t\t\t\t\t\t\t\t\t  by DenisBaylo |\n%s\n\n", "   +------------------------------------------------------------------------------------+");

	printf("   Уровень загруженности судна:\n"); 
	for(int i=0;i<SHIP_COUNT;i++){
		printf("   %d корабль: %s %dc\t", SHIPS[i].id+1, SHIPS[i].in_way ? "(в пути)" : "( ждем )", SHIPS[i].arrive_time);
		for(int j=0;j<SHIPS[i].on_board;j++){
			printf("| ");
		}
		printf("\n");
	}

}



