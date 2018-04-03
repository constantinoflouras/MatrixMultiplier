#OBJS specifies which files to compile as part of the project
OBJS = matrix_multiplication.c

#CC specifies which compiler we're using
CC = gcc

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
COMPILER_FLAGS = -Wall -m32

#DEBUG_STATEMENTS specifies any debug statements we want to add
DEBUG_STATEMENTS = -DDEBUG

#LINKER_FLAGS specifies the libraries we're linking against
LINKER_FLAGS =

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = matrix_multiplication

#This is the target that compiles our executable
all : $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(DEBUG_STATEMENTS) $(LINKER_FLAGS) -o $(OBJ_NAME)
