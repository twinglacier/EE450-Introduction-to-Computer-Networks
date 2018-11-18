#include <stdio.h>
#include <math.h>
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

#define CPORT "23434"   // Port number for Server C
#define HOST "localhost"

int main(void)
{
	// set up UDP  -- From Beej
	int sockfd;
	int rv;
	struct addrinfo hints;  // the struct addrinfo have already been filled with relevant info
	struct addrinfo *servinfo; //point out the result
	struct addrinfo *p;  //tempoary point
	struct sockaddr_storage server_addr;
	socklen_t addr_len;


	memset(&hints, 0, sizeof hints);  // make sure the struct is empty
	hints.ai_family = AF_UNSPEC; // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_DGRAM; // UDP dgram sockets
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(HOST, CPORT, &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 0;
	}
	// loop through all the results and bind to the first we can----Beej
	for (p = servinfo; p != NULL; p = p->ai_next) 
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
		{
			perror("serverC: socket");
			continue;
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
		{
			close(sockfd);
			perror("serverC: bind");
			continue;
		}
		break;
	}
	if (p == NULL) 
	{
		fprintf(stderr, "serverC: failed to bind socket\n");
		return 0;
	}
	freeaddrinfo(servinfo);
    	printf( "The Server C is up and running using UDP on port %s.\n", CPORT);

  	while(1)
	{
		addr_len = sizeof server_addr;
		int link_id;
		int size;
		double power;
		int bandwidthglo;
		double lengthglo, velocityglo, noisepowerglo;

		recvfrom(sockfd, (char *)& link_id, sizeof link_id, 0,(struct sockaddr *)&server_addr, &addr_len);
		recvfrom(sockfd, (char *)& size, sizeof size, 0,(struct sockaddr *)&server_addr, &addr_len);
		recvfrom(sockfd, (char *)& power, sizeof power, 0,(struct sockaddr *)&server_addr, &addr_len);
		printf("The Server C received link information of link %d, file size %d, and signal power %f. \n", link_id, size, power);

		recvfrom(sockfd, (char *)& bandwidthglo, sizeof bandwidthglo, 0,(struct sockaddr *)&server_addr, &addr_len);
		recvfrom(sockfd, (char *)& lengthglo, sizeof lengthglo, 0,(struct sockaddr *)&server_addr, &addr_len);
		recvfrom(sockfd, (char *)& velocityglo, sizeof velocityglo, 0,(struct sockaddr *)&server_addr, &addr_len);
		recvfrom(sockfd, (char *)& noisepowerglo, sizeof noisepowerglo, 0,(struct sockaddr *)&server_addr, &addr_len);

		//printf("BW=%d, L=%f, V=%f, N=%f.\n", bandwidthglo, lengthglo, velocityglo, noisepowerglo);
		double capacity, t_trans, t_prop, rtt;
		//By Shannon's Law
		capacity = (bandwidthglo * 1e6) * (log (1 + power / noisepowerglo) / log (2));
		t_trans = size / capacity;
		t_prop = (lengthglo * 1000) / (velocityglo * 1e7);
		rtt = t_trans + t_prop;
		
		printf("The Server C finished calculation for link %d.\n", link_id);

		sendto(sockfd, (char *)& t_trans, sizeof t_trans, 0,(struct sockaddr *) &server_addr, addr_len);
		sendto(sockfd, (char *)& t_prop, sizeof t_prop, 0,(struct sockaddr *) &server_addr, addr_len);
		sendto(sockfd, (char *)& rtt, sizeof rtt, 0,(struct sockaddr *) &server_addr, addr_len);

		printf("The Server C finished sending the output to AWS. \n");

		//close(sockfd);
		//break;

	}

}
