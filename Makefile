all: jit

jit: main.o IsE_LinesF.o IsE_binTrans.o
	g++ -o compiler main.o IsE_LinesF.o IsE_binTrans.o

main.o: main.cpp
	g++ -g -c -O3 main.cpp

IsE_LinesF.o: IsE_LinesF.cpp
	g++ -g -c -O3 -msse4.2 IsE_LinesF.cpp

IsE_binTrans.o: IsE_binTrans.cpp
	g++ -g -c -O3 IsE_binTrans.cpp

clean:
	rm *.o 