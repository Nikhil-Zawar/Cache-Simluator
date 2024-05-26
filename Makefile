run:
	g++ -Wall -Wextra --pedantic main.cpp -o csim
	g++ -Wall -Wextra --pedantic main.cpp -o cacheSim
clean:
	rm csim cacheSim