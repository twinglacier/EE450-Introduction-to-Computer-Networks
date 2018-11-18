Name: Yi Jiang
Student ID: 

What I have done in the assignment:
I had built a system exchanging data and output the delay by using two databases. After inputting link ID, size and power, first send it to the search server (Server A and B), and look at whether the databases have or not a link ID matched. If so, send the bandwidth, length, velocity and noisepower via AWS to the compute server (Server C). Then get the delay time and pass it to the client. Simutaneously, send the detailed information to the monitor.

What my code files are and what each one of them does:
I have 6 code files in my project.
	1. client.c: Send the basic link ID, Size and Power to AWS and get the delay time from AWS by TCP.
	2. monitor.c: Monitoring the input and output details (Tt, Tp and Delay) getting form AWS by TCP.
	3. serverA.c: Looking up whether there is matched informations with link ID received from AWS by UDP in database_a.csv and send bandwidth, length, velocity and noisepower back to AWS by UDP if matches are found.
	4. serverB.c: Looking up whether there is matched informations with link ID received from AWS by UDP in database_b.csv and send bandwidth, length, velocity and noisepower back to AWS by UDP if matches are found.
	5. serverC.c: Receiving information of link ID, size, power, andwidth, length, velocity and noisepower from AWS by UDP and calculate the Transmission Delay (Tt), Propgation Delay (Tp) and End-to-end Delay (Delay). Then send them back to AWS by UDP.
	6. aws.c: Performs the function of AWS. First it gets information of link id, size and power from client and sends it to monitor both by TCP. Then it sends the information to server A and B by UDP to find if there are any matched information. If so, gets this information from them and sends all the information to serverC to get the information of delays by UDP. Finally, it sends the information of delay to both client and monitor by TCP.

The format of all the messages exchanged:
All the messages exchanged are strings. I use the function atoi() and atof() to change the format of strings to integers and doubles, which will be more convinient when calculating in each program.

Any idiosyncrasy of my project:
Yes. When compiling serverC.c, as written in the Makefile, I should use "gcc -o serverC serverC.c -lm". Otherwise there will be an error saying that the log function is undefined even if I included math.h. Additionally, when running the project, I should make serverA, serverB, serverC and monitor earlier than making aws, or there may be an error.
(Recommended sequence: make all -> make serverA -> make serverB -> make serverC -> make monitor -> make aws -> ./client <LINK_ID> <SIZE> <POWER>)

Reused Code:
All reused codes are from Beej's Guidance, and I have identified them by comment in my program.
