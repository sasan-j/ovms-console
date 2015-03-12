#
# Makefile for './release_build/ovms_controller'.
#
# Type 'make' or 'make ./release_build/ovms_controller' to create the binary.
# Type 'make clean' or 'make clear' to delete all temporaries.
# Type 'make run' to execute the binary.
# Type 'make debug' to debug the binary using gdb(1).
#

# build target specs
CC = gcc
CFLAGS = -O3 
OUT_DIR = objects
LIBS =

# first target entry is the target invoked when typing 'make'
default: ./release_build/ovms_controller

./release_build/ovms_controller: $(OUT_DIR)/constants.c.o $(OUT_DIR)/input_decode.c.o $(OUT_DIR)/kb_manager.c.o $(OUT_DIR)/main.c.o $(OUT_DIR)/routine.c.o $(OUT_DIR)/serial_link.c.o $(OUT_DIR)/tools.c.o
	@echo -n 'Linking ./release_build/ovms_controller... '
	@$(CC) $(CFLAGS) -o ./release_build/ovms_controller $(OUT_DIR)/constants.c.o $(OUT_DIR)/input_decode.c.o $(OUT_DIR)/kb_manager.c.o $(OUT_DIR)/main.c.o $(OUT_DIR)/routine.c.o $(OUT_DIR)/serial_link.c.o $(OUT_DIR)/tools.c.o $(LIBS)
	@echo Done.

$(OUT_DIR)/constants.c.o: constants.c constants.h
	@echo -n 'Compiling constants.c... '
	@$(CC) $(CFLAGS) -o $(OUT_DIR)/constants.c.o -c constants.c
	@echo Done.

$(OUT_DIR)/input_decode.c.o: input_decode.c kb_manager.h serial_link.h \
 tools.h constants.h routine.h
	@echo -n 'Compiling input_decode.c... '
	@$(CC) $(CFLAGS) -o $(OUT_DIR)/input_decode.c.o -c input_decode.c
	@echo Done.

$(OUT_DIR)/kb_manager.c.o: kb_manager.c kb_manager.h serial_link.h
	@echo -n 'Compiling kb_manager.c... '
	@$(CC) $(CFLAGS) -o $(OUT_DIR)/kb_manager.c.o -c kb_manager.c
	@echo Done.

$(OUT_DIR)/main.c.o: main.c serial_link.h input_decode.h constants.h
	@echo -n 'Compiling main.c... '
	@$(CC) $(CFLAGS) -o $(OUT_DIR)/main.c.o -c main.c
	@echo Done.

$(OUT_DIR)/routine.c.o: routine.c routine.h serial_link.h tools.h \
 constants.h
	@echo -n 'Compiling routine.c... '
	@$(CC) $(CFLAGS) -o $(OUT_DIR)/routine.c.o -c routine.c
	@echo Done.

$(OUT_DIR)/serial_link.c.o: serial_link.c serial_link.h tools.h
	@echo -n 'Compiling serial_link.c... '
	@$(CC) $(CFLAGS) -o $(OUT_DIR)/serial_link.c.o -c serial_link.c
	@echo Done.

$(OUT_DIR)/tools.c.o: tools.c tools.h
	@echo -n 'Compiling tools.c... '
	@$(CC) $(CFLAGS) -o $(OUT_DIR)/tools.c.o -c tools.c
	@echo Done.

run:
	././release_build/ovms_controller 

debug:
	gdb ././release_build/ovms_controller

clean:
	@echo -n 'Removing all temporary binaries... '
	@rm -f ./release_build/ovms_controller $(OUT_DIR)/*.o
	@echo Done.

clear:
	@echo -n 'Removing all temporary binaries... '
	@rm -f ./release_build/ovms_controller $(OUT_DIR)/*.o
	@echo Done.

