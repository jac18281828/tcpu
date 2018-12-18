all: tcpu

clean:
	rm -f tcpu tcpu.o

tcpu: tcpu.o
	cc -o $@ $<

tcpu.o: tcpu.c
	cc -c -o $@ $<

