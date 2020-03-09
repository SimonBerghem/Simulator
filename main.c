#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <semaphore.h>

//Declarations.
int COM1;
int north_Queue = 0;
int south_Queue = 0;
int Bridge_Count = 0;
bool greenNorth = false;
bool greenSouth = false;
sem_t semo;

//The two threads needed.
pthread_t reader;
pthread_t carTracker;
pthread_t passedCars;


void updateGUI(void){
	sem_wait(&semo);
	printf("North Light: %d| North Queue: %d \n", greenNorth, north_Queue);
	printf("South Light: %d| South Queue: %d \n", greenSouth, south_Queue);
	printf("Cars on Bridge: %d \n", Bridge_Count);
	printf("------------------------------------------------------------ \n");
	sem_post(&semo);

}

//Open the serial.
void serialPortConfig(void) {

	//Open it to read and write.
	COM1 = open("/dev/ttyS0", O_RDWR);

	if (COM1 < 0){
		printf("Cannot open the file: %s\n", strerror(errno));
	}
	
	//Create the struct for the serial settings.
	struct termios ter;

	//Give the struct the attributes.
	if (tcgetattr(COM1, &ter)){
		printf("Could not get termios attributes. (openSerialPort)\n");
		exit(-1);
	}

	//Queue Selector. Flush data not read.
	tcflush(COM1, TCIFLUSH);

	// Set Baud Rate. Set size to 8 bits. Enable Receiver. Set local Control. Closed thread on finish.
	ter.c_cflag = B9600 | CS8 | CREAD | CLOCAL | HUPCL;

	// NOT (Echo input characters | echo the NL character even if ECHO is not set)

	ter.c_lflag &= ~(ECHO | ECHONL | ICANON);

	//sleep until input of 1 char. TIME = 10*0,1 SEC.
	ter.c_cc[VTIME] = 10;
	ter.c_cc[VMIN] = 1;

	//Set input speed and output  speed.
	cfsetispeed(&ter, B9600);
	cfsetospeed(&ter, B9600);

	//Save the set attributes. If not return exit.
	if(tcsetattr(COM1, TCSANOW, &ter)){
		printf("Could not set termios attributes. (openSerialPort)\n");
		exit(-1);

	}

}

//Send signal to the AVR.
void transmitSignal(uint8_t signal){

	//Write to the file/Serial.
	int text = write(COM1, &signal, sizeof(signal));

	//Check if it was successful.
	if (text < 0) {
		printf("Could not write instruction to avr. (serialWrite)\n");
	}

}


void *updateBridge(void *arg){
	
	while(true){
		
		if(greenNorth && north_Queue > 0){
			north_Queue--;
			transmitSignal(0b0010);
			Bridge_Count++;
			updateGUI();
		}
		else if (greenSouth && south_Queue > 0){
			south_Queue--;
			transmitSignal(0b1000);
			Bridge_Count++;
			updateGUI();
		}
		
		sleep(1);	
	}
}


void *bridge_remove(void *arg){
	
	while(true){s
		
		if(Bridge_Count>0){
			
			sleep(5);
			Bridge_Count--;
			
			updateGUI();
			
		}
	}
}


//The controller reads the light. Needs to be pointers for threads to work.
void *lightReader(void *arg) {

	//the in data
	uint8_t dataIn;
	
	while(true) {

		//Read the data in.
		int data = read(COM1, &dataIn, sizeof(dataIn));

		//See if there is a message or not.
		if (data > 0) {

			//Both Red.
			if (dataIn == 0b1010) {
				
				greenNorth = false;
				greenSouth = false;

				//Green for north.
			} else if (dataIn == 0b1001) {

				greenNorth = true;
				greenSouth = false;

				//Green for south.
			} else if (dataIn == 0b0110) {

				greenNorth = false;
				greenSouth = true;

			}
		}
	}
}

void inputReader(void){
	
	//No enter needed command.
	fcntl(fileno(stdin), F_SETFL, O_NONBLOCK);
	
	//Remove the ECHO.
	struct termios ter;
	tcgetattr(STDIN_FILENO, &ter);
	ter.c_lflag &= ~(ICANON | ECHO);
	ter.c_cc[VMIN] = 1;
	tcsetattr(STDIN_FILENO, TCSANOW, &ter);
		
	while(true){
		
		//Get the Key press.
		char key = getchar();
		
		//North Car Add.
		if (key == 'n') {
			transmitSignal(0x1);
			north_Queue++;
			updateGUI();
		}
		
		//South Car Add.
		else if (key == 's') {
			transmitSignal(0x4);
			south_Queue++;
			updateGUI();

		}
		
		//Exit
		else if (key == 'e') {
			return;
		}
	}
}




int main(void) {
	
printf("Hello World \n}");

serialPortConfig();
sem_init(&semo, 0, 1);

pthread_create(&reader, NULL, lightReader, 0);
pthread_create(&carTracker, NULL, updateBridge, 0);
pthread_create(&passedCars, NULL, bridge_remove, 0);

inputReader();

}



/* FILE* comsetup(){
	struct termios ter;
	FILE* f;
	f = fopen("/dev/ttyS0", "r+");
	cfsetospeed(&ter, B9600);
	cfsetispeed(&ter, B9600);
	tcsetattr(fileno(f), 0, &ter);
}


int main(int argc, char* argv[]){
	printf("Hello world");
	
	int buf = 0;
	char in;
	while(1 == 1){
		buf = fgetc(f);
		printf("%d\n", buf);
		in = getc(stdin);
		if (in == 'e'){
			return 0;
		} else {
			fputc(in - '0', f);
		}
			
	}
	return 0;
} */
