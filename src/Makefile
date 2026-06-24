CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I. -I/usr/include/lua5.4
TARGET = rts
SOURCES = imp/main.cpp imp/game.cpp imp/input.cpp \
          sqd/mapgen.cpp sqd/render.cpp \
          cfg/config.cpp
OBJECTS = $(SOURCES:.cpp=.o)
LDFLAGS = -lncursesw -llua5.4

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

imp/%.o: imp/%.cpp types.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

sqd/%.o: sqd/%.cpp types.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

cfg/%.o: cfg/%.cpp types.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all run clean
