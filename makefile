mymalloc: mymalloc.o
	gcc mymalloc.o -o mymalloc -pedantic

mymalloc.o: mymalloc.c
	gcc -c mymalloc.c -Wall -pedantic

clean_csv:
	rm *.csv

clean:
	rm *.o mymalloc a.out