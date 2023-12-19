#This is a makefile
CC := clang++
INC := inc
INC_FLAG :=-I$(INC)
CC_FLAGS :=-march=native -std=c++20 -c -Ofast $(INC_FLAG)
CC_DEBUG :=-march=native -std=c++20 -c -g $(INC_FLAG)


SRC := src
BUILD := bin
OBJ_DIR := $(BUILD)/intermediates
DEBUG_OBJ_DIR := $(BUILD)/debug_intermediates

#Get all src files and their object representation
SRC_FILES := $(wildcard ./src/*cpp)
OBJECTS := $(patsubst ./src/%.cpp, ./$(OBJ_DIR)/%.o, $(SRC_FILES))

OBJECTS_DEBUG := $(patsubst ./src/%.cpp, ./$(DEBUG_OBJ_DIR)/%.o, $(SRC_FILES))


.PHONY: directories

all: directories explorer

debug: directories explorer_debug


#Create objects directory
directories: $(OBJ_DIR) $(DEBUG_OBJ_DIR)

#Release dir
${OBJ_DIR}:
	@mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o : $(SRC)/%.cpp
	$(CC) $(CC_FLAGS) $< -o $@
	@echo "Compile success"

explorer: $(OBJECTS)
	$(CC) $(OBJECTS) -o $(BUILD)/explorer
	@echo "Build successful"

################################################################

#Debug dir
${DEBUG_OBJ_DIR}:
	@mkdir -p $(DEBUG_OBJ_DIR)


$(DEBUG_OBJ_DIR)/%.o : $(SRC)/%.cpp
	$(CC) $(CC_DEBUG) $< -o $@
	@echo "Debug compile success"

explorer_debug: $(OBJECTS_DEBUG)
	$(CC) $(OBJECTS_DEBUG) -g -o $(BUILD)/explorer_debug
	@echo "Debug build successful"

#bin/%.o: $(SRC)
#	@echo "Compiling $< ..."
#	$(CC) $(CC_FLAGS) $<
#	@echo "Complete"


#$(BUILD)explorerChess.o: $(SRC)explorerChess.cpp
	

#$(BUILD)test.o: $(SRC)test.cpp $(INC)test.h
#	@echo "Compiling test.cpp ..."
#	$(CC) $(CC_FLAGS) $(SRC)test.cpp
#	@echo "Complete"

	

clean:
	rm -f *.o explorer
	rm -r bin