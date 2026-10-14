CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -I/opt/homebrew/opt/readline/include
LDFLAGS = -L/opt/homebrew/opt/readline/lib
LDLIBS = -lreadline -lncurses

SRCS = main.cpp builtins.cpp utils.cpp externals.cpp parsers.cpp pipeline.cpp history.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = mysh

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f $(OBJS) $(TARGET)
