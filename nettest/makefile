
LIBS=-lws2_32
SRC=main.cpp
OBJ=$(SRC:%.cpp=%.o)

.PHONY: build

build: $(OBJ)
	g++ -std=c++11 $^ $(LIBS) -obuild.exe

%.o: %.cpp
	g++ -std=c++11 -c $^

run: build
	mintty "./build.exe"