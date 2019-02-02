richess: main.o
	g++ -std=c++11 -Wall -Wextra -pedantic main.o -o richess -lm -lGLEW -lglfw -lGL -lX11 -lpthread -lXrandr -lXi

main.o: main.cpp shader.h model.h camera.h
	g++ -std=c++11 -Wall -Wextra -pedantic main.cpp -c -lm -lGLEW -lglfw -lGL -lX11 -lpthread -lXrandr -lXi

clean:
	rm richess *.o
