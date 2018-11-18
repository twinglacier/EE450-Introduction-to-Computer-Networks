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

#define APORT "21434"           // UDP port to portA
#define BPORT "22434"           // UDP port to portB
#define CPORT "23434"           // UDP port to portC
#define UDPPORT "24434"		// UDP port
#define TCPPORTCLIENT "25434"   // TCP port to client
#define TCPPORTMONITOR "26434"  // TCP port to monitor
#define TCPPORTSEND "27434"   // TCP port to send
#define HOST "localhost"
#define BACKLOG 10 

int m_Aglo, m_Bglo;
int bandwidthglo;
double lengthglo, velocityglo, noisepowerglo;
double t_transglo, t_propglo, rttglo;

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


// using UDP to send link ID to back server A
int udpsendA(int link_id)
{
	int sockfd;
    	struct addrinfo hints, *servinfo, *p;
    	int rv;

    	//set up UDP
    	memset(&hints, 0, sizeof hints);
    	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    	hints.ai_socktype = SOCK_DGRAM;


    	if ((rv = getaddrinfo(HOST, APORT, &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	// loop through all the results and make a socket----Beej
	for (p = servinfo; p != NULL; p = p->ai_next) 
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
		{
			perror("talker: socket");
			continue;
		}
		break;
	}
	if (p == NULL) 
	{
		fprintf(stderr, "talker: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	//using UDP to send the data
	sendto(sockfd, (char *)& link_id, sizeof link_id, 0, p->ai_addr,p->ai_addrlen);
	printf("The AWS sent link ID=%d to BackendServer A using UDP overport %s. \n", link_id, APORT);

	int m_A = 0;
	int bandwidth_A;
	double length_A, velocity_A, noisepower_A;
	recvfrom(sockfd, (char *)& m_A, sizeof m_A, 0 , NULL, NULL);
	if (m_A == 1)
	{
		recvfrom(sockfd, (char *)& bandwidth_A, sizeof bandwidth_A, 0 , NULL, NULL);
		recvfrom(sockfd, (char *)& length_A, sizeof length_A, 0 , NULL, NULL);
		recvfrom(sockfd, (char *)& velocity_A, sizeof velocity_A, 0 , NULL, NULL);
		recvfrom(sockfd, (char *)& noisepower_A, sizeof noisepower_A, 0 , NULL, NULL);
		bandwidthglo = bandwidth_A;
		lengthglo = length_A;
		velocityglo = velocity_A;
		noisepowerglo = noisepower_A;
	}
	m_Aglo = m_A;
	printf("The AWS received %d matches from Backend-Server A using UDP over port %s. \n", m_A, APORT);

} 

// using UDP to send link ID to back server B
int udpsendB(int link_id)
{
	int sockfd;
    	struct addrinfo hints, *servinfo, *p;
    	int rv;

    	//set up UDP
    	memset(&hints, 0, sizeof hints);
    	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    	hints.ai_socktype = SOCK_DGRAM;


    	if ((rv = getaddrinfo(HOST, BPORT, &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	// loop through all the results and make a socket----Beej
	for (p = servinfo; p != NULL; p = p->ai_next) 
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
		{
			perror("talker: socket");
			continue;
		}
		break;
	}
	if (p == NULL) 
	{
		fprintf(stderr, "talker: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	//using UDP to send the data
	sendto(sockfd, (char *)& link_id, sizeof link_id, 0, p->ai_addr,p->ai_addrlen);
	printf("The AWS sent link ID=%d to BackendServer B using UDP overport %s. \n", link_id, BPORT);

	int m_B = 0;
	int bandwidth_B;
	double length_B, velocity_B, noisepower_B;
	recvfrom(sockfd, (char *)& m_B, sizeof m_B, 0 , NULL, NULL);
	if (m_B == 1)
	{
		recvfrom(sockfd, (char *)& bandwidth_B, sizeof bandwidth_B, 0 , NULL, NULL);
		recvfrom(sockfd, (char *)& length_B, sizeof length_B, 0 , NULL, NULL);
		recvfrom(sockfd, (char *)& velocity_B, sizeof velocity_B, 0 , NULL, NULL);
		recvfrom(sockfd, (char *)& noisepower_B, sizeof noisepower_B, 0 , NULL, NULL);
		bandwidthglo = bandwidth_B;
		lengthglo = length_B;
		velocityglo = velocity_B;
		noisepowerglo = noisepower_B;
	}
	m_Bglo = m_B;
	printf("The AWS received %d matches from Backend-Server B using UDP over port %s. \n", m_B, BPORT);
} 

// using UDP to send link ID, size and power to back server C
int udpsendC(int link_id, int size, double power)
{
	int sockfd;
    	struct addrinfo hints, *servinfo, *p;
    	int rv;

    	//set up UDP
    	memset(&hints, 0, sizeof hints);
    	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    	hints.ai_socktype = SOCK_DGRAM;


    	if ((rv = getaddrinfo(HOST, CPORT, &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	// loop through all the results and make a socket----Beej
	for (p = servinfo; p != NULL; p = p->ai_next) 
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) 
		{
			perror("talker: socket");
			continue;
		}
		break;
	}
	if (p == NULL) 
	{
		fprintf(stderr, "talker: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	//using UDP to send the data
	sendto(sockfd, (char *)& link_id, sizeof link_id, 0, p->ai_addr,p->ai_addrlen);
	sendto(sockfd, (char *)& size, sizeof size, 0, p->ai_addr,p->ai_addrlen);
	sendto(sockfd, (char *)& power, sizeof power, 0, p->ai_addr,p->ai_addrlen);
	if (m_Aglo == 1 || m_Bglo == 1)
	{
		sendto(sockfd, (char *)& bandwidthglo, sizeof bandwidthglo, 0, p->ai_addr,p->ai_addrlen);
		sendto(sockfd, (char *)& lengthglo, sizeof lengthglo, 0, p->ai_addr,p->ai_addrlen);
		sendto(sockfd, (char *)& velocityglo, sizeof velocityglo, 0, p->ai_addr,p->ai_addrlen);
		sendto(sockfd, (char *)& noisepowerglo, sizeof noisepowerglo, 0, p->ai_addr,p->ai_addrlen);
		printf("The AWS sent link ID=%d, size=%d, power=%f, and link information to BackendServer C using UDP overport %s. \n", link_id, size, power, CPORT);
	}

	double t_trans, t_prop, rtt;
	recvfrom(sockfd, (char *)& t_trans, sizeof t_trans, 0 , NULL, NULL);
	recvfrom(sockfd, (char *)& t_prop, sizeof t_prop, 0 , NULL, NULL);
	recvfrom(sockfd, (char *)& rtt, sizeof rtt, 0 , NULL, NULL);
	t_transglo = t_trans * 1000;  // in ms
	t_propglo = t_prop * 1000;    // in ms
	rttglo = rtt * 1000;          // in ms
} 

int TCPsend()
{
	int sockfd_c = 0;
	struct addrinfo hints_c, *servinfo_c, *p_c;
	int rv_c;

	memset(&hints_c, 0, sizeof hints_c);
	hints_c.ai_family = AF_UNSPEC;
	hints_c.ai_socktype = SOCK_STREAM;

	if ((rv_c = getaddrinfo(HOST, TCPPORTSEND, &hints_c, &servinfo_c)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv_c));
		return 1;
	}

	// loop through all the results and connect to the first we can----Beej
	for (p_c = servinfo_c; p_c != NULL; p_c = p_c->ai_next) 
	{
		if ((sockfd_c = socket(p_c->ai_family, p_c->ai_socktype, p_c->ai_protocol)) == -1) 
		{
			perror("server: socket");
			continue;
		}

		if (connect(sockfd_c, p_c->ai_addr, p_c->ai_addrlen) == -1) 
		{
			close(sockfd_c);
			perror("server: connect");
			continue;
		}
		break;
	}

	if (p_c == NULL) 
	{
		fprintf(stderr, "client: failed to connect. \n");
		exit(0);
		return 2;
	}

	freeaddrinfo(servinfo_c);

	return sockfd_c;
}

int main()
{	
	//set up TCP --from Beej
	int sockfd, new_fdc, new_fdm;  // listen on sock_fd, new connection to client on new_fdc, new connection to monitor on new_fdm
    	struct addrinfo hints, *servinfo, *p;
    	struct sockaddr_storage client_addr; // the address information of client
	struct sockaddr_storage monitor_addr; // the address information of monitor
    	socklen_t sin_size;
    	int yes = 1;
    	char s[INET6_ADDRSTRLEN];
    	int rv;

    	memset(&hints, 0, sizeof hints);
    	hints.ai_family = AF_UNSPEC;
    	hints.ai_socktype = SOCK_STREAM;
    	hints.ai_flags = AI_PASSIVE; // use my IP

    	if ((rv = getaddrinfo(HOST, TCPPORTCLIENT, &hints, &servinfo)) != 0) 
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
	

	int sockfd_m = 0;
	struct addrinfo hints_m, *servinfo_m, *p_m;
	int rv_m;

	memset(&hints_m, 0, sizeof hints_m);
	hints_m.ai_family = AF_UNSPEC;
	hints_m.ai_socktype = SOCK_STREAM;

	if ((rv_m = getaddrinfo(HOST, TCPPORTMONITOR, &hints_m, &servinfo_m)) != 0) 
	{
		fprintf(stderr, "getaddrinfo of monitor: %s\n", gai_strerror(rv_m));
		return 1;
	}

	// loop through all the results and connect to the first we can----Beej
	for (p_m = servinfo; p_m != NULL; p_m = p_m->ai_next) 
	{
		if ((sockfd_m = socket(p_m->ai_family, p_m->ai_socktype, p_m->ai_protocol)) == -1) 
		{
			perror("server: socket of monitor");
			continue;
		}

		if (connect(sockfd_m, p_m->ai_addr, p_m->ai_addrlen) == -1) 
		{
			close(sockfd_m);
			perror("server: connect to monitor");
			continue;
		}
		break;
	}

	if (p_m == NULL) 
	{
		fprintf(stderr, "server: failed to connect to monitor. \n");
		exit(0);
		return 2;
	}

	freeaddrinfo(servinfo_m); // all done with this structure	

	printf( "The AWS is up and running. \n");

	//the whole loop
	while(1)
	{
		sin_size = sizeof client_addr;	
		new_fdc = accept(sockfd, (struct sockaddr *) &client_addr, &sin_size);
		if (new_fdc == -1) 
		{
			perror("accept");			
			exit(1);
		}


		// get the information of client
		inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *) &client_addr), s, sizeof s);
		struct sockaddr_in clientAddress;
		memset(&clientAddress, 0, sizeof clientAddress);
		int client_len = sizeof clientAddress;
		getpeername(new_fdc, (struct sockaddr *) &clientAddress, (socklen_t *) &client_len);
		int client_port = clientAddress.sin_port;


		//receive all the inforamtion from client
		int link_id;
		int size;
		double power;

		recv(new_fdc, (char *)& link_id, sizeof link_id, 0);	
		recv(new_fdc, (char *)& size, sizeof size, 0);
		recv(new_fdc, (char *)& power, sizeof power, 0 );
		printf("The AWS received link ID=%d, size=%d, and power=%f from the client using TCP over port %s. \n", link_id, size, power, TCPPORTCLIENT);
		
		new_fdm = sockfd_m;
		send(new_fdm, (char *)& link_id, sizeof link_id, 0);
		send(new_fdm, (char *)& size, sizeof size, 0);
		send(new_fdm, (char *)& power, sizeof power, 0);
		printf("The AWS sent ID=%d, size=%d, power=%f to the monitor using TCP over port %s.\n", link_id, size, power, TCPPORTMONITOR);
		
		udpsendA(link_id);
		udpsendB(link_id);

		send(new_fdm, (char *)& m_Aglo, sizeof m_Aglo, 0);
		send(new_fdm, (char *)& m_Bglo, sizeof m_Bglo, 0);
		int sockfd_c;
		sockfd_c = TCPsend();
		send(sockfd_c, (char *)& m_Aglo, sizeof m_Aglo, 0);
		send(sockfd_c, (char *)& m_Bglo, sizeof m_Bglo, 0);

		if (m_Aglo == 0 && m_Bglo == 0)
		{
			printf("The AWS sent No Match to the monitor and the client using TCP over ports %s and %s, respectively.\n", TCPPORTMONITOR, TCPPORTCLIENT);
			break;
		}

		udpsendC(link_id, size, power);
		
		send(new_fdm, (char *)& t_transglo, sizeof t_transglo, 0);
		send(new_fdm, (char *)& t_propglo, sizeof t_propglo, 0);
		send(new_fdm, (char *)& rttglo, sizeof rttglo, 0);

		send(sockfd_c, (char *)& rttglo, sizeof rttglo, 0);
		
		if (m_Aglo != 0 || m_Bglo != 0)
		{
			printf("The AWS sent Delay=%.2f ms to the client using TCP over port %s.\n", rttglo, TCPPORTCLIENT);
			printf("The AWS sent detailed results to the monitor using TCP over port %s.\n", TCPPORTMONITOR);
		}

	}
	
}
