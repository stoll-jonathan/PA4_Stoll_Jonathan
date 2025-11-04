mymalloc: mymalloc.o
	gcc mymalloc.o -o mymalloc -pthread -lrt

mymalloc.o: mymalloc.c
	gcc -c mymalloc.c -Wall -pthread -lrt

clean_csv:
	rm *.csv

clean:
	rm *.o mymalloc a.out