# makefile
all:
	gcc -o client client.c
	gcc -o aws aws.c
	gcc -o monitor monitor.c
	gcc -o serverA serverA.c
	gcc -o serverB serverB.c
	gcc -o serverC serverC.c -lm

.PHONY: serverA
.PHONY: serverB
.PHONY: serverC
.PHONY: monitor
.PHONY: aws

serverA: 
	./serverA

serverB:
	./serverB

serverC:
	./serverC

monitor:
	./monitor

aws:
	./aws

clean:
	$(RM) serverA
	$(RM) serverB
	$(RM) serverC
	$(RM) monitor
	$(RM) aws
