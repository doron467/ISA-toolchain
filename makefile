CC = gcc
CXX = g++
CFLAGS = -ansi -pedantic -Wall
BUILD = build
PROGRAMS = programs


ASSEMBLER_OBJECTS = \
	$(BUILD)/assembler.o \
	$(BUILD)/secondpass.o \
	$(BUILD)/firstpass.o \
	$(BUILD)/preprocessor.o \
	$(BUILD)/symbols_table.o \
	$(BUILD)/memory_image.o \
	$(BUILD)/utils.o \
	$(BUILD)/operations.o

ASSEMBLER_HEADERS = \
	assembler/memory_image.h \
	assembler/symbols_table.h \
	assembler/preprocessor.h \
	assembler/utils.h \
	common/constants.h \
	common/operations.h 


LINKER_OBJECTS = \
	$(BUILD)/linker.o


EMULATOR_OBJECTS = \
	$(BUILD)/emulator.o \
	$(BUILD)/cpu.o \
	$(BUILD)/memory.o \
	$(BUILD)/decoder.o \
	$(BUILD)/operations.o

EMULATOR_HEADERS = \
	emulator/emulator.hpp \
	emulator/memory.hpp \
	emulator/cpu.hpp \
	emulator/decoder.hpp \
	common/operations.h \
	common/constants.h

DEBUGGER_OBJECTS = \
	$(BUILD)/debugger.o \
	$(BUILD)/debugger_parser.o

	

all: $(PROGRAMS)/assembler $(PROGRAMS)/linker $(PROGRAMS)/emulator $(PROGRAMS)/debugger



$(PROGRAMS)/assembler: $(ASSEMBLER_OBJECTS)
	$(CC) $(CFLAGS) $(ASSEMBLER_OBJECTS) -o $(PROGRAMS)/assembler

$(BUILD)/assembler.o: assembler/assembler.c assembler/preprocessor.h assembler/firstpass.h assembler/utils.h
	$(CC) $(CFLAGS) -c assembler/assembler.c -o $(BUILD)/assembler.o

$(BUILD)/secondpass.o: assembler/secondpass.c assembler/secondpass.h assembler/firstpass.h $(ASSEMBLER_HEADERS)
	$(CC) $(CFLAGS) -c assembler/secondpass.c -o $(BUILD)/secondpass.o

$(BUILD)/firstpass.o: assembler/firstpass.c assembler/firstpass.h $(ASSEMBLER_HEADERS)
	$(CC) $(CFLAGS) -c assembler/firstpass.c -o $(BUILD)/firstpass.o

$(BUILD)/preprocessor.o: assembler/preprocessor.c assembler/preprocessor.h assembler/utils.h common/constants.h
	$(CC) $(CFLAGS) -c assembler/preprocessor.c -o $(BUILD)/preprocessor.o

$(BUILD)/symbols_table.o: assembler/symbols_table.c assembler/symbols_table.h assembler/preprocessor.h assembler/utils.h common/constants.h
	$(CC) $(CFLAGS) -c assembler/symbols_table.c -o $(BUILD)/symbols_table.o

$(BUILD)/memory_image.o: assembler/memory_image.c assembler/memory_image.h assembler/utils.h common/constants.h
	$(CC) $(CFLAGS) -c assembler/memory_image.c -o $(BUILD)/memory_image.o

$(BUILD)/utils.o: assembler/utils.c assembler/utils.h common/constants.h common/operations.h
	$(CC) $(CFLAGS) -c assembler/utils.c -o $(BUILD)/utils.o



$(PROGRAMS)/linker: $(LINKER_OBJECTS)
	$(CXX) $(LINKER_OBJECTS) -o $(PROGRAMS)/linker

$(BUILD)/linker.o: linker/linker.cpp linker/linker.hpp common/constants.h
	$(CXX) -c linker/linker.cpp -o $(BUILD)/linker.o



$(PROGRAMS)/emulator: $(EMULATOR_OBJECTS) $(BUILD)/emulatorMain.o
	$(CXX) $(EMULATOR_OBJECTS) $(BUILD)/emulatorMain.o -o $(PROGRAMS)/emulator

$(BUILD)/emulatorMain.o: emulator/main.cpp $(EMULATOR_HEADERS)
	$(CXX) -c emulator/main.cpp -o $(BUILD)/emulatorMain.o

$(BUILD)/emulator.o: emulator/emulator.cpp $(EMULATOR_HEADERS)
	$(CXX) -c emulator/emulator.cpp -o $(BUILD)/emulator.o

$(BUILD)/cpu.o: emulator/cpu.cpp emulator/cpu.hpp emulator/memory.hpp emulator/decoder.hpp common/operations.h common/constants.h
	$(CXX) -c emulator/cpu.cpp -o $(BUILD)/cpu.o

$(BUILD)/memory.o: emulator/memory.cpp emulator/memory.hpp
	$(CXX) -c emulator/memory.cpp -o $(BUILD)/memory.o

$(BUILD)/decoder.o: emulator/decoder.cpp emulator/decoder.hpp common/constants.h common/operations.h
	$(CXX) -c emulator/decoder.cpp -o $(BUILD)/decoder.o



$(PROGRAMS)/debugger: $(EMULATOR_OBJECTS) $(DEBUGGER_OBJECTS)
	$(CXX) $(EMULATOR_OBJECTS) $(DEBUGGER_OBJECTS) -o $(PROGRAMS)/debugger

$(BUILD)/debugger.o: debugger/debugger.cpp debugger/input_parser.hpp $(EMULATOR_HEADERS)
	$(CXX) -c debugger/debugger.cpp -o $(BUILD)/debugger.o

$(BUILD)/debugger_parser.o: debugger/input_parser.cpp debugger/input_parser.hpp
	$(CXX) -c debugger/input_parser.cpp -o $(BUILD)/debugger_parser.o


$(BUILD)/operations.o: common/operations.c common/operations.h common/constants.h
	$(CC) -c common/operations.c -o $(BUILD)/operations.o

clean:
	rm -rf $(BUILD)/*