nickname=screener

compile:
	g++ -std=c++20 main.cpp -o $(nickname) stopwords.cpp

clean:
	rm ./$(nickname)

run:
	g++ -std=c++20 main.cpp -o $(nickname) stopwords.cpp
	./$(nickname)