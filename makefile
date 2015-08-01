LIBS=-lws2_32 -lopencv_objdetect2411 -lopencv_imgproc2411 -lopencv_highgui2411 
LIBS += -lopencv_core2411 -lIlmImf -llibjpeg -llibtiff -llibpng -llibjasper
LIBS += -lzlib -lgdi32 -lole32 -loleaut32 -luuid -lvfw32

LIBDIR=-L$(shell cygpath -w /usr/local/lib)
INCLUDE=-I$(shell cygpath -w /usr/local/include)
SRC=$(shell /usr/bin/find src -name "*.cpp")
OBJ=$(SRC:%.cpp=%.o)

.PHONY: pbuild build run

pbuild:
	@make -j 4 build

build: build/build.exe

build/build.exe: $(OBJ)
	@[[ -d build ]] || mkdir build
	g++ -std=c++11 $^ $(LIBDIR) $(LIBS) -obuild/build.exe

%.o: %.cpp
	g++ -std=c++11 $(INCLUDE) -c $^ -o $@

run: build
	build/build.exe 1337