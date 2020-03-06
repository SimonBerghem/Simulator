#include <stdio.h>
#include <sys/types.h>
#include <termios.h>

FILE* comsetup(){
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
}
