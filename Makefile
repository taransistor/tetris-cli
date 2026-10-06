CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++23

TARGET = tetris-cli
OBJ = src/main.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(TARGET) -lncurses

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
