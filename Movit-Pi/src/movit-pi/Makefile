CPP_FILES = $(wildcard *.cpp)
OBJ_FILES = $(addprefix $(OBJ_DIR_MOVIT_PI)/,$(CPP_FILES:.cpp=.o))

$(TARGET_MOVIT_PI): $(OBJ_FILES)

$(OBJ_DIR_MOVIT_PI)/%.o: %.cpp
	$(CPP) $(CXXFLAGS) $(CPPFLAGS) $< -o $@
