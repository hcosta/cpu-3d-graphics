build:
	g++ -I src/include -L src/lib -o bin/main *.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf
run:
	./bin/main.exe
clean:
	rm ./bin/main.exe