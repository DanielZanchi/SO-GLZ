all: ascensore piani

piani: piani.c linkedList.o persona.o linkedList.h
	cc piani.c linkedList.o persona.o -o piani

ascensore: ascensore.c persona.o linkedList.o
	cc ascensore.c linkedList.o persona.o -o ascensore

linkedList.o: linkedList.c linkedList.h
	cc -c linkedList.c -o linkedList.o
	
persona.o: persona.c persona.h
	cc -c persona.c -o persona.o
cleanall: clean cleansocket cleanlog
	
clean:
	rm -f ascensore piani
	rm -f linkedList.o persona.o
cleansocket:
	rm -f piano0.sock piano1.sock piano2.sock piano3.sock
cleanlog:
	rm -f piano0.log piano1.log piano2.log piano3.log ascensore.log
