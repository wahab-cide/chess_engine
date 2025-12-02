CXX = g++
CXXFLAGS = -std=c++11 -O3 -Wall -Wextra
TARGET = chess_engine

SRCS = main.cpp board.cpp movegen.cpp evaluation.cpp search.cpp uci.cpp pawn_structure.cpp book.cpp
OBJS = $(SRCS:.cpp=.o)
HEADERS = constants.h types.h board.h movegen.h evaluation.h search.h uci.h pawn_structure.h book.h

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
