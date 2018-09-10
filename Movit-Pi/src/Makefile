CPP_FILES = $(wildcard *.cpp)
OBJ_FILES = $(addprefix $(OBJ_DIR)/,$(CPP_FILES:.cpp=.o))

$(TARGET): $(OBJ_FILES)

$(OBJ_DIR)/%.o: %.cpp
	$(CPP) $(CXXFLAGS) $(CPPFLAGS) $< -o $@
