GSTFLAGS=`pkg-config --cflags --libs gstreamermm-1.0 glibmm-2.4 cairomm-1.0 pangomm-1.4 gtkmm-3.0`

CFLAGS=-ggdb -Wno-deprecated-declarations

OBJ=main.o sensors.o hud.o
HEADER=sensors.hpp hud.hpp

all: simhud

run: simhud
	./simhud

simhud: $(OBJ) $(HEADER)
	g++ $(CFLAGS) -std=c++14 $(OBJ) -o $@ $(GSTFLAGS) 

%.o: %.cpp $(HEADER)
	g++ $(CFLAGS) -std=c++14 -c $< -o $@ $(GSTFLAGS) 


clean:
	-rm *.o
	-rm simhud
