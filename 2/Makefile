cc = g++
prom = fat12
obj = func.o main.o

fat12: $(obj)
	$(cc) -o  $(prom) $(obj) -no-pie -g


func.o:  my_print.asm
	nasm -f elf64  my_print.asm -o func.o -g 


main.o: main.cpp
	$(cc) -g -m64 -c main.cpp

clean:
	rm -rf $(obj) $(prom)
