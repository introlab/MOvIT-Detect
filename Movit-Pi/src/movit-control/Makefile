CPP_FILES = $(wildcard *.cpp)
OBJ_FILES = $(addprefix $(OBJ_DIR_MOVIT_CONTROL)/,$(CPP_FILES:.cpp=.o))

$(TARGET_MOVIT_CONTROL): $(OBJ_FILES)

$(OBJ_DIR_MOVIT_CONTROL)/%.o: %.cpp
	$(CPP) $(CXXFLAGS) $(CPPFLAGS) $< -o $@
