# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# Target and source
TARGET = server
SRC = server.c++

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "Building succeeded !!"
	@echo " Running the programme"

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
	@echo "Clearning Finished : removed binary file !!"
