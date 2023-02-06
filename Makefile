all:
	gcc -Wall -D_GLIBCXX_USE_CXX11_ABI=1 -c ssd1322_6800.c ssd1322_graphics.c vu_meters.c -I/usr/include/freetype2
	g++ -Wall -D_GLIBCXX_USE_CXX11_ABI=1 -o dspoled dspoled.cpp easywsclient.cpp ssd1322_6800.o ssd1322_graphics.o vu_meters.o -l pigpio -l freetype -ffast-math -lm -ldl
install:
	cp dspoled ~/
clean:
	-rm -f *.o
	-rm -f *.gch
	-rm -f dspoled
