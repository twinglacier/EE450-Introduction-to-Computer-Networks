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
#include <signal.h>
#include <ctype.h> 

#define AWSPORT "25434"  
#define RECVPORT "27434"
#define HOST "localhost"
#define BACKLOG 10 

void *get_in_addr(struct sockaddr *sa) 
{
	if (sa->sa_family == AF_INET) 
	{
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

int receiver(int link_id)
{
	//set up TCP --from Beej
	int sockfd_c, new_fd;  // listen on sock_fd, new connection to server on new_fd
    	struct addrinfo hints_c, *servinfo_c, *p_c;
    	struct sockaddr_storage server_addr; // the address information of aws
    	socklen_t sin_size;
    	int yes = 1;
    	char s[INET6_ADDRSTRLEN];
    	int rv_c;

    	memset(&hints_c, 0, sizeof hints_c);
    	hints_c.ai_family = AF_UNSPEC;
    	hints_c.ai_socktype = SOCK_STREAM;
    	hints_c.ai_flags = AI_PASSIVE; // use my IP

    	if ((rv_c = getaddrinfo(HOST, RECVPORT, &hints_c, &servinfo_c)) != 0) 
	{
        	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_c));
        	return 1;
	}

	// loop through all the results and bind to the first we can
	for (p_c = servinfo_c; p_c != NULL; p_c = p_c->ai_next) 
	{
		if ((sockfd_c = socket(p_c->ai_family, p_c->ai_socktype, p_c->ai_protocol)) == -1) 
		{
			perror("client: socket of server");
			continue;
		}
		if (setsockopt(sockfd_c, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) 
		{
			perror("setsockopt of server");
			exit(1);
		}
		if (bind(sockfd_c, p_c->ai_addr, p_c->ai_addrlen) == -1) 
		{
			close(sockfd_c);
			perror("client: bind with server");
			continue;
		}
		break;
	}
	if (p_c == NULL) 
	{
		fprintf(stderr, "client: failed to bind with server\n");
		return 2;
	}

	freeaddrinfo(servinfo_c); // all done with this structure

	//listen
	if (listen(sockfd_c, BACKLOG) == -1) 
	{
		perror("listen");
		exit(1);
	}


	while(1)
	{
		sin_size = sizeof server_addr;	
		new_fd = accept(sockfd_c, (struct sockaddr *) &server_addr, &sin_size);
		if (new_fd == -1) 
		{
			perror("accept");			
			exit(1);
		}


		// get the information of server
		inet_ntop(server_addr.ss_family, get_in_addr((struct sockaddr *) &server_addr), s, sizeof s);
		struct sockaddr_in serverAddress;
		memset(&serverAddress, 0, sizeof serverAddress);
		int server_len = sizeof serverAddress;
		getpeername(new_fd, (struct sockaddr *) &serverAddress, (socklen_t *) &server_len);
		int server_port = serverAddress.sin_port;


		//receive all the inforamtion from server
		int m_Aglo, m_Bglo;		
		double rttglo;

		recv(new_fd, (char *)& m_Aglo, sizeof m_Aglo, 0 );
		recv(new_fd, (char *)& m_Bglo, sizeof m_Bglo, 0 );
		if (m_Aglo != 0 || m_Bglo != 0)
		{
			recv(new_fd, (char *)& rttglo, sizeof rttglo, 0 );
			printf("The result for link %d Delay=%.2f ms\n", link_id, rttglo);
		}
		else
			printf("Found no matches for link %d.\n", link_id);

		close(new_fd);
		break;

	} 
}

int main(int argc, char* argv[])
{
	int link_id = atoi(argv[1]);
	int size = atoi(argv[2]);
	double power = atof(argv[3]);

	int sockfd = 0;
	struct addrinfo hints, *servinfo, *p;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(HOST, AWSPORT, &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can----Beej
	for (p = servinfo; p != NULL; p = p->ai_next) 
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
		{
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
		{
			close(sockfd);
			perror("client: connect");
			continue;
		}
		break;
	}

	if (p == NULL) 
	{
		fprintf(stderr, "client: failed to connect. \n");
		exit(0);
		return 2;
	}

	freeaddrinfo(servinfo);

	printf("The client is up and running. \n");

	send(sockfd, (char *)& link_id, sizeof link_id, 0);
	send(sockfd, (char *)& size, sizeof size, 0);
	send(sockfd, (char *)& power, sizeof power, 0);

	printf("The client sent ID=%d, size=%d, power=%f to AWS.\n", link_id, size, power);

	receiver(link_id);

}
