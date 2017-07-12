CC = arm-linux-gnueabihf-gcc

CFLAGS=

LIBS=

C_SRC = devmem4arm.c 
ASM_SRC = asm_io128.s

C_OBJ = $(patsubst %.c,%.o,$(C_SRC))
ASM_OBJ = $(patsubst %.s,%.o,$(ASM_SRC))

all: devmem2

$(C_OBJ): %.o: %.c 
	$(CC) -c -o $@ $< $(CFLAGS)
	
$(ASM_OBJ): %.o: %.s
	$(CC) -c -o $@ $< $(CFLAGS)
	
devmem2: $(C_OBJ) $(ASM_OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)


.PHONY: clean all

clean:
	rm -f *.o devmem2
