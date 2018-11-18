#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define BPORT "22434"   // Port number for Server B
#define HOST "localhost"

char *trim(char *str)
{
	char *p = str;
	while (*p == ' ' || *p == '\t' || *p == 'r' || *p == '\n')
		p++;
	str = p;
	p = str + strlen(str) - 1;
	while (*p == ' ' || *p == '\t' || *p == 'r' || *p == '\n')
		--p;
	*(p + 1) = '\0';
	return str;
}

int main(void)
{
	// set up UDP  -- From Beej
	int sockfd;
	int rv;
	struct addrinfo hints;  // the struct addrinfo have already been filled with relevant info
	struct addrinfo *servinfo; //point out the result
	struct sockaddr_storage server_addr;
	struct addrinfo *p;  //tempoary point
	socklen_t addr_len;


	memset(&hints, 0, sizeof hints);  // make sure the struct is empty
	hints.ai_family = AF_UNSPEC; // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_DGRAM; // UDP dgram sockets
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(HOST, BPORT, &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 0;
	}
	// loop through all the results and bind to the first we can----Beej
	for (p = servinfo; p != NULL; p = p->ai_next) 
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
		{
			perror("serverB: socket");
			continue;
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
		{
			close(sockfd);
			perror("serverB: bind");
			continue;
		}
		break;
	}
	if (p == NULL) 
		{
		fprintf(stderr, "serverB: failed to bind socket\n");
		return 0;
	}
	freeaddrinfo(servinfo);
   	printf( "The Server B is up and running using UDP on port %s.\n", BPORT);

   	while(1)
	{
		addr_len = sizeof server_addr;
		int link_id;

		recvfrom(sockfd, (char *)& link_id, sizeof link_id, 0,(struct sockaddr *)&server_addr, &addr_len);
		printf("The Server B received input %d. \n", link_id);

		int m_B = 0;

		FILE *fp = fopen("database_b.csv", "r");
		if (fp == NULL)
		{
			return -1;		
		}

		char line[1024];
		char *id, *bandwidth, *length, *velocity, *noisepower;
		int bandwidth_B;
		double length_B, velocity_B, noisepower_B;
		while (fgets(line, sizeof(line), fp))
		{
			char *save_ptr;
			id = strtok_r(line, ",", &save_ptr);
			if (id == NULL)
				return -1;
			bandwidth = strtok_r(NULL, ",", &save_ptr);
			length = strtok_r(NULL, ",", &save_ptr);
			velocity = strtok_r(NULL, ",", &save_ptr);
			noisepower = strtok_r(NULL, ",", &save_ptr);
			if (link_id == atoi(id))
			{
				m_B = 1;
				break;	
			}
		}

		printf("The Server B has found %d match.\n", m_B);
		sendto(sockfd, (char *)& m_B, sizeof m_B, 0,(struct sockaddr *) &server_addr, addr_len);
		if (m_B == 1)
		{
			bandwidth_B = atoi(trim(bandwidth));
			length_B = atof(trim(length));
			velocity_B = atof(trim(velocity));
			noisepower_B = atof(trim(noisepower));
			sendto(sockfd, (char *)& bandwidth_B, sizeof bandwidth_B, 0,(struct sockaddr *) &server_addr, addr_len);
			sendto(sockfd, (char *)& length_B, sizeof length_B, 0,(struct sockaddr *) &server_addr, addr_len);
			sendto(sockfd, (char *)& velocity_B, sizeof velocity_B, 0,(struct sockaddr *) &server_addr, addr_len);
			sendto(sockfd, (char *)& noisepower_B, sizeof noisepower_B, 0,(struct sockaddr *) &server_addr, addr_len);
		}
		printf("The Server B finished sending the output to AWS.\n");
		//break;
		
	}
}
