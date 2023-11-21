
CC = g++
CFLAGS =  -std=c++11 -O3
MAIN_FILE = main.o
LINK_FILES = B_tree.o skyline.o
EXE_FILE = Floorplan

CHECKCC = valgrind
CHECKFLAGS = --leak-check=full -s --show-leak-kinds=all --track-origins=yes 
#---------------------

all: $(LINK_FILES) $(MAIN_FILE)
	$(CC) -o $(EXE_FILE) $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<

sample:
	time ./$(EXE_FILE) sample.dat output.txt
	python ./checker.py ./sample.dat output.txt sample.png
#	./SolutionChecker ./testcase/sample.dat output.txt

t300:
	time ./$(EXE_FILE) t300.txt output.txt
	python ./checker.py ./t300.txt output.txt t300.png

sample300:
	time ./$(EXE_FILE) sample300.block output.txt
	python ./checker.py ./sample300.block output.txt sample300.png

ami49:
	time ./$(EXE_FILE) ami49_5.block output.txt
	python ./checker.py ./ami49_5.block output.txt ami49_5.png

clean:
	rm $(MAIN_FILE) $(LINK_FILES) $(EXE_FILE) check.log output.txt *.png 

check:
	$(CHECKCC) $(CHECKFLAGS) ./$(EXE_FILE) ./testcase/input_1.dat output.txt > check.log
