richess: main.o
	g++ -std=c++14 -Wall -Wextra -pedantic main.o -o richess -lm -lGLEW -lglfw -lGL -lX11 -lpthread -lXrandr -lXi

main.o: main.cpp stb_image.h head.h shader.h model.h camera.h
	g++ -std=c++14 -Wall -Wextra -pedantic main.cpp -c -lm -lGLEW -lglfw -lGL -lX11 -lpthread -lXrandr -lXi

clean:
	@rm -f richess *.o

run:
	@make -s -j2 && ./richess
