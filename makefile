CXX      = g++
CXXFLAGS = -std=c++17 -pthread -Wall -Wextra
SRCS     = ./src/Aircraft.cpp ./src/Quaternion.cpp ./src/Vector3.cpp ./src/main.cpp
TARGET   = flight_sim

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)
