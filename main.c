#include <stdio.h>
#include <pthread.h>
#include <GL/gl.h> 

void display() { 
    glClearColor(1,0,0,0); 
    glClear(GL_COLOR_BUFFER_BIT); 
    glFlush(); 
} 

int PASSENGERS; // Пассажиры

enum PEOPLE_STATUS{
	wait = 0,
	up = 1,
	down = 2
};

int main(void)
{
	printf("Введите количество пассажиров : ");
	scanf("%d", &PASSENGERS);

	glutInit(&argc, argv); 
    glutInitWindowPosition(100,100); 
    glutInitWindowSize(500,500); 
    glutCreateWindow(Hello World); 
    glutDisplayFunc(display); 
    glutMainLoop(); 

	return 0;
}