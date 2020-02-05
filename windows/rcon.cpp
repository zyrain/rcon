// rcon.cpp -- RCON for Windows (originally VC++ 6.0)
//

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#define BUF_LEN 8192
#define DEBUG 0

int main(int argc, char **argv)
{
	SOCKET sock;
	WSADATA wsaData;
	
	struct sockaddr_in a;
	char msg[BUF_LEN];
	int ret, i;
	char password[512]="YOUR_PASSWORD_HERE";
	short port = 27015;
	char address[512] = "172.0.0.1";
	int arg;
	char number[16];
	
	if(argc<2)
    {
		fprintf(stderr, "Syntax: rcon [-P\"rcon_password\"] [-a127.0.0.1] [-p27015] command\n");
		return 0;
    }
	
	for(arg = 1;arg<argc;arg++) {
		if(DEBUG) fprintf(stderr, "Processing arg \"%s\"\n", argv[arg]);
		if(argv[arg][0] != '-')
			break; /* done with args */
		switch(argv[arg][1]) {
		case 'a':
			strncpy(address, argv[arg]+2, 512); 
			break;
		case 'p':
			port = atoi(argv[arg]+2);
			break;
		case 'P':
			strncpy(password, argv[arg]+2, 512); 
			break;
		default:
			fprintf(stderr, "Unknown option -%c\n",argv[arg][1]);
			return 0;
		}
	}
	
	if (WSAStartup(0x202,&wsaData) == SOCKET_ERROR) {
        fprintf(stderr, "Couldn't Initialize Winsock\n");
        return -1;
	}	
	
	sock = socket(AF_INET, SOCK_DGRAM,0); // UDP socket
	
	a.sin_family = AF_INET;
	a.sin_addr.s_addr = inet_addr(address);
	a.sin_port = htons(port);
	
	ret = connect(sock,(struct sockaddr *)&a,sizeof(a));
	
	if(ret == SOCKET_ERROR)
		fprintf(stderr, "connect() failed: %d\n",WSAGetLastError());
	
	strncpy(msg, "\377\377\377\377challenge rcon\n", BUF_LEN);
	if(DEBUG) fprintf(stderr, "Sending: \"%s\"\n",msg);
	
	ret = send(sock,msg,strlen(msg),0);
	
	if(ret == SOCKET_ERROR) {
		perror("challenge send failed");
		WSACleanup();
		return -1;
	}
	
	if(DEBUG) fprintf(stderr, "send() succeeded ret == %d\n", ret);
		
	ret = recv(sock,msg,BUF_LEN,0);
	if(ret == SOCKET_ERROR) {
		perror("recv() failed:");
		WSACleanup();
		return -1;
	}
	
	if(DEBUG) fprintf(stderr, "recv() succeeded ret == %d\n",ret);
			
	if(DEBUG) fprintf(stderr, "Received: \"%s\"\n",msg);
	if(strncmp(msg, "\377\377\377\377challenge rcon ",19)==0) {
		if(DEBUG) fprintf(stderr, "Challenge received\n");
	} else {
		fprintf(stderr, "Challenge not received\n");
		WSACleanup();
		return -1;
	}
	for(i=0;i<15;i++) {
		number[i] = msg[i+19];
		if(number[i]=='\n') {
			break;
		}
	}
	number[i] = 0;
			
	if(DEBUG) fprintf(stderr, "Magic number is \"%s\"\n",number);
			
	ret = sprintf(msg, "\377\377\377\377rcon %s \"%s\" ", number, password);
			
	while(arg < argc) {
		if(strlen(argv[arg]) + ret < BUF_LEN) {
			strcpy(msg + ret, argv[arg]);
			ret += strlen(argv[arg]);
			msg[ret] = ' ';
			ret++;
			arg++;
		} else {
			fprintf(stderr, "cmd too long to send\n");
			WSACleanup();
			return -1;
		}
	}

	strcpy(msg+ret, "\n");
	ret++;
			
	if(DEBUG) fprintf(stderr, "Sending: %s\n", msg);
			
	ret = send(sock,msg,strlen(msg),0);
			
	if(ret == SOCKET_ERROR) {
		perror("cmd send failed");
		WSACleanup();
		return -1;
	}
	
	if(DEBUG) fprintf(stderr,"send() succeeded ret == %d\n", ret);
		
	ret = recv(sock,msg,BUF_LEN,0);
	if(ret == SOCKET_ERROR) {
		perror("recv() failed");
		WSACleanup();
		return -1;
	}
	
	if(DEBUG) fprintf(stderr, "recv() succeeded ret == %d\n",ret);
					
	if(strncmp(msg, "\377\377\377\377",4)==0) {
		if(DEBUG) fprintf(stderr,"Response code %c\n",msg[4]);
		printf("%s\n",msg+5);
	} else {
		fprintf(stderr, "Unknown response:\n%s\n",msg);
		WSACleanup();
		return -1;
	}
	WSACleanup();
	return 0;
}



