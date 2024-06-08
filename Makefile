#This is a makefile
CC := clang++
INC := inc
INC_FLAG :=-I$(INC)
CC_BASE_FLAGS :=-march=native -std=c++20 -c -Wall -Wextra
CC_FLAGS :=$(CC_BASE_FLAGS) -Ofast -DSHALLOW_SEARCH -DPRINT_OUT $(INC_FLAG)
CC_DEBUG :=$(CC_BASE_FLAGS) -g -DPRINT_OUT $(INC_FLAG)
CC_TEST :=$(CC_BASE_FLAGS) -Ofast -DSHALLOW_SEARCH -DSKIP_UI $(INC_FLAG)


SRC := src
TEST := test
BUILD := bin
OBJ_DIR := $(BUILD)/intermediates
DEBUG_OBJ_DIR := $(BUILD)/debug_intermediates
TEST_OBJ_DIR := $(BUILD)/test_intermediates

#Get all src files and their object representation
SRC_FILES := $(wildcard ./src/*cpp)
TEST_SRC := $(filter-out ./src/ExplorerChess.cpp, $(wildcard ./src/*cpp))
TEST_FILES := $(wildcard ./test/*cpp)


OBJECTS := $(patsubst ./src/%.cpp, ./$(OBJ_DIR)/%.o, $(SRC_FILES))

OBJECTS_DEBUG := $(patsubst ./src/%.cpp, ./$(DEBUG_OBJ_DIR)/%.o, $(SRC_FILES))

OBJECTS_SRC_TEST := $(patsubst ./src/%.cpp, ./$(TEST_OBJ_DIR)/%.o, $(TEST_SRC))
OBJECTS_TEST := $(patsubst ./$(TEST)/%.cpp, $(TEST_OBJ_DIR)/%.o, $(TEST_FILES)) 


.PHONY: directories

# DEFAULT BUILD
all: directories explorer

# BUILD FOR PERFT SUITE
test: directories testsuite
	$(BUILD)/testsuite

lol:
	@echo $(OBJECTS_SRC_TEST)

debug: directories explorer_debug

run:
	./bin/explorer

#Create objects directory
directories: $(OBJ_DIR) $(DEBUG_OBJ_DIR) $(TEST_OBJ_DIR)

################################################################
##################### RELEASE BUILD ##############################
################################################################

${OBJ_DIR}:
	@mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o : $(SRC)/%.cpp
	$(CC) $(CC_FLAGS) $< -o $@
	@echo "Compile success"

explorer: $(OBJECTS)
	$(CC) $(OBJECTS) -o $(BUILD)/explorer	
	@echo "Build successful"



################################################################
##################### TEST BUILD ##############################
################################################################


# Test binary

$(TEST_OBJ_DIR):
	@mkdir -p $(TEST_OBJ_DIR)

testsuite: $(OBJECTS_SRC_TEST) $(OBJECTS_TEST)
	$(CC) $(OBJECTS_SRC_TEST) $(OBJECTS_TEST) -o $(BUILD)/testsuite
	@echo "Compile success"

$(TEST_OBJ_DIR)/%.o: $(TEST)/%.cpp
	$(CC) $(CC_TEST) $< -o $@

$(TEST_OBJ_DIR)/%.o : $(SRC)/%.cpp
	$(CC) $(CC_TEST) $< -o $@
	@echo "Compile success"

################################################################
##################### DEBUG BUILD ##############################
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

clean:
	rm -rf bin