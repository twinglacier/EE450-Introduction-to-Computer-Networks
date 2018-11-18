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

#define AWSPORT "26434"  // TCP port to monitor
#define HOST "localhost"
#define BACKLOG 10 

void sigchld_handler(int s)
{
    	// waitpid() might overwrite errno, so we save and restore it:
    	int saved_errno = errno;
    	while(waitpid(-1, NULL, WNOHANG) > 0);
    	errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) 
	{
        	return &(((struct sockaddr_in*)sa)->sin_addr);
    	}
    	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main()
{
	//set up TCP --from Beej
	int sockfd, new_fd;  // listen on sock_fd, new connection to server on new_fd
    	struct addrinfo hints, *servinfo, *p;
    	struct sockaddr_storage server_addr; // the address information of aws
    	socklen_t sin_size;
    	int yes = 1;
    	char s[INET6_ADDRSTRLEN];
    	int rv;

    	memset(&hints, 0, sizeof hints);
    	hints.ai_family = AF_UNSPEC;
    	hints.ai_socktype = SOCK_STREAM;
    	hints.ai_flags = AI_PASSIVE; // use my IP

    	if ((rv = getaddrinfo(HOST, AWSPORT, &hints, &servinfo)) != 0) 
	{
        	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        	return 1;
	}

	// loop through all the results and bind to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next) 
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
		{
			perror("server: socket of client");
			continue;
		}
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) 
		{
			perror("setsockopt of client");
			exit(1);
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
		{
			close(sockfd);
			perror("server: bind with client");
			continue;
		}
		break;
	}
	if (p == NULL) 
	{
		fprintf(stderr, "server: failed to bind with client\n");
		return 2;
	}

	freeaddrinfo(servinfo); // all done with this structure

	//listen
	if (listen(sockfd, BACKLOG) == -1) 
	{
		perror("listen");
		exit(1);
	}

	printf("The monitor is up and running.\n");

	while(1)
	{
		sin_size = sizeof server_addr;	
		new_fd = accept(sockfd, (struct sockaddr *) &server_addr, &sin_size);
		if (new_fd == -1) 
		{
			perror("accept");			
			exit(1);
		}


		// get the information of client
		inet_ntop(server_addr.ss_family, get_in_addr((struct sockaddr *) &server_addr), s, sizeof s);
		struct sockaddr_in serverAddress;
		memset(&serverAddress, 0, sizeof serverAddress);
		int server_len = sizeof serverAddress;
		getpeername(new_fd, (struct sockaddr *) &serverAddress, (socklen_t *) &server_len);
		int server_port = serverAddress.sin_port;


		//receive all the inforamtion from client
		int link_id;
		int size;
		double power;
		int m_Aglo, m_Bglo;
		double t_transglo, t_propglo, rttglo;

		recv(new_fd, (char *)& link_id, sizeof link_id, 0);	
		recv(new_fd, (char *)& size, sizeof size, 0);
		recv(new_fd, (char *)& power, sizeof power, 0 );
		printf("The monitor received link ID=%d, size=%d, and power=%f from the AWS. \n", link_id, size, power);

		recv(new_fd, (char *)& m_Aglo, sizeof m_Aglo, 0 );
		recv(new_fd, (char *)& m_Bglo, sizeof m_Bglo, 0 );		

		if (m_Aglo != 0 || m_Bglo != 0)
		{
			recv(new_fd, (char *)& t_transglo, sizeof t_transglo, 0 );
			recv(new_fd, (char *)& t_propglo, sizeof t_propglo, 0 );
			recv(new_fd, (char *)& rttglo, sizeof rttglo, 0 );
			printf("The result for link %d: \nTt=%.2f ms\nTp=%.2f ms\nDelay=%.2f ms\n", link_id, t_transglo, t_propglo, rttglo);
		}
		else
			printf("Found no matches for link %d.\n", link_id);


	} 
}
