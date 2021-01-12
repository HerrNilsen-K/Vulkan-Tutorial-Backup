cc = clang++
appName = program

sources := $(shell find . -type f -iname \*.cpp)
objects = $(patsubst %.cpp, %.o, $(sources))

flags = -g -Wall -std=c++17
libPath = 
libs = -lglfw -lvulkan -ldl -lpthread -lX11 -lXrandr

#Compile, link and execute the program
all: program run

program: $(objects)
	$(cc) -o $(appName) $^ $(libPath) $(libs)
  
%.o : %.cpp
	$(cc) $(flags) -c -o $@ $<

#Create Spir-V file
shader:
	glslangValidator -V shader.vert
	glslangValidator -V shader.frag

#Delete all object files
#WARNING! The whole project needs to be recompiled after this
clean:
	find . -type f -iname \*.o -delete

.PHONY: all
  
.PHONY run:
	./$(appName)