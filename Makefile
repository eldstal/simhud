GSTFLAGS=`pkg-config --cflags --libs gstreamermm-1.0 glibmm-2.4`

CFLAGS=-Wno-deprecated-declarations


all: simhud

run: simhud
	./simhud

simhud: main.o hud.o
	g++ $(CFLAGS) -std=c++14 $^ -o $@ $(GSTFLAGS) 

%.o: %.cpp
	g++ $(CFLAGS) -std=c++14 -c $< -o $@ $(GSTFLAGS) 


clean:
	-rm *.o
	-rm simhud
