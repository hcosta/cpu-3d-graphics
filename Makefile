build:
	g++ -I src/include -L src/lib  -o bin/main *.cpp src/include/imgui/*.cpp -lmingw32 -lSDL2main -lSDL2
run:
	./bin/main.exe
clean:
	rm ./bin/main.exe