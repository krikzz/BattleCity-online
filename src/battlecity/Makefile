GDK= E:/devtools/m68k-3.4.6/
CC= $(GDK)bin/gcc
OBJC= $(GDK)bin/objcopy
ASMZ80= $(GDK)bin/asmz80
BINTOC= $(GDK)bin/bintoc
NM= $(GDK)bin/nm
NM2WCH= $(GDK)bin/nm2wch
OUTPUT= battlecity-online
BIN= $(GDK)bin
RESOL=bin

SRC_S = sega.s data.s
SRC_C = $(wildcard *.c)
SRC_LIB_C = $(wildcard $(GDK)lib/*.c)


OBJ = $(SRC_S:.s=.o)
OBJ += $(SRC_C:.c=.o)
OBJ += $(SRC_LIB_C:.c=.o)

LINKOBJ= $(OBJ)


FLAGS= $(OPTION) -m68000 -Wall -O3 -fomit-frame-pointer $(INCS)
FLAGSZ80= -c -i -x1 -x2 -x3 -z -lnul

all: $(OUTPUT).$(RESOL)


$(OUTPUT).$(RESOL): $(OUTPUT).out
	$(OBJC) --pad-to 0x20000 -O binary $(OUTPUT).elf $(OUTPUT).$(RESOL)

$(OUTPUT).out: $(OBJ)
#	$(CC) -T rom.ld -nostdlib $(LINKOBJ) $(GDK)lib/libgcc.a -o $(OUTPUT).out
	$(CC) -B$(BIN) -T rom.ld -nostdlib $(LINKOBJ) $(GDK)lib/libgcc.a -o $(OUTPUT).elf


%.o: %.c
	$(CC) $(FLAGS) -c $< -o $@

%.o: %.s
	$(CC) $(FLAGS) -c $< -o $@

clean:
	$(RM) *.o
	$(RM) *.out
	$(RM) *.wch
	$(RM) *.nm

