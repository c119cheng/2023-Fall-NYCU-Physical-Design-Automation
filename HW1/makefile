
CC = g++
CFLAGS =  -std=c++11 -O3
MAIN_FILE = main.o
LINK_FILES = FM.o
EXE_FILE = Lab1

CHECKCC = valgrind
CHECKFLAGS = --leak-check=full -s --show-leak-kinds=all --track-origins=yes 
#---------------------

all: $(LINK_FILES) $(MAIN_FILE)
	$(CC) -o $(EXE_FILE) $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

sample:
	time ./$(EXE_FILE) ./testcase/sample.dat output.txt
	./SolutionChecker ./testcase/sample.dat output.txt

demo1:
	time ./$(EXE_FILE) ./testcase/input_1.dat output.txt
	./SolutionChecker ./testcase/input_1.dat output.txt

demo2:
	time ./$(EXE_FILE) ./testcase/input_2.dat output.txt
	./SolutionChecker ./testcase/input_2.dat output.txt

demo3:
	time ./$(EXE_FILE) ./testcase/input_3.dat output.txt
	./SolutionChecker ./testcase/input_3.dat output.txt

demo4:
	time ./$(EXE_FILE) ./testcase/input_4.dat output.txt
	./SolutionChecker ./testcase/input_4.dat output.txt

demo5:
	time ./$(EXE_FILE) ./testcase/input_5.dat output.txt
	./SolutionChecker ./testcase/input_5.dat output.txt

demo6:
	time ./$(EXE_FILE) ./testcase/case4.in output.txt
	./SolutionChecker ./testcase/case4.in output.txt

clean:
	rm $(MAIN_FILE) $(LINK_FILES) $(EXE_FILE) check.log output.txt

check:
	$(CHECKCC) $(CHECKFLAGS) ./$(EXE_FILE) ./testcase/input_1.dat output.txt > check.log
