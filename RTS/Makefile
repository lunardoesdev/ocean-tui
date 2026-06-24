CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I. -I/usr/include/lua5.4
TARGET = rts
SOURCES = src/imp/main.cpp src/imp/game.cpp src/imp/input.cpp \
          src/sqd/mapgen.cpp src/sqd/render.cpp \
          src/cfg/config.cpp
OBJECTS = $(SOURCES:.cpp=.o)
LDFLAGS = -lncursesw -llua5.4

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

src/imp/%.o: src/imp/%.cpp types.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

src/sqd/%.o: src/sqd/%.cpp types.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

src/cfg/%.o: src/cfg/%.cpp types.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all run clean
