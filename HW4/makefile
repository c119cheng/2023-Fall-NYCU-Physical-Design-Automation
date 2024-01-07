
CC = g++
CFLAGS =  -std=c++11 -O3
MAIN_FILE = main.o
LINK_FILES = Placer.o
EXE_FILE = Lab3

CHECKCC = valgrind
CHECKFLAGS = --leak-check=full -s --show-leak-kinds=all --track-origins=yes 
#---------------------

all: $(LINK_FILES) $(MAIN_FILE)
	$(CC) -o $(EXE_FILE) $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

sample:
	time ./$(EXE_FILE) sample.sp output.txt
	./SolutionChecker sample.sp output.txt
	python plot.py sample.sp output.txt output.png
	
DFF:
	time ./$(EXE_FILE) testcase/ASYNC_DFFHx1_ASAP7_75t_R.sp output.txt
	./SolutionChecker testcase/ASYNC_DFFHx1_ASAP7_75t_R.sp output.txt
	python plot.py testcase/ASYNC_DFFHx1_ASAP7_75t_R.sp output.txt output.png

clean:
	rm $(MAIN_FILE) $(LINK_FILES) $(EXE_FILE) check.log output.txt *.png 

check:
	$(CHECKCC) $(CHECKFLAGS) ./$(EXE_FILE) ./testcase/input_1.dat output.txt > check.log
