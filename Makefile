OBJ := $(patsubst %.c, %.o, $(wildcard *.c))
mp3tag: $(OBJ)
	gcc -o $@ $^
clean:
	rm *.out *.o