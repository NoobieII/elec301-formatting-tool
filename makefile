#makefile for quick formatter test
#Ryan Homsani
#September 28, 2019

shell = cmd
CC = gcc
CFLAGS = -O3
LDLIBS = -lmingw32 -lSDLmain -lSDL.dll -lSDL_image
OBJ = default_text.o load_image.o ryan_quick_format.o main.o

main: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o ryan_format.exe $(LDLIBS)

.PHONY: clean	
clean:
	del -f *.o test.exe
