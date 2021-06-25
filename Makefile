# Type 'make' or 'make game' to compile the serial version.
# Type 'make mpi' to compile the MPI version.
# Type 'make cuda' to compile the Cuda version.
CC = gcc
MPICC = mpicc
NVCC = nvcc
FLAGS = -std=c99 -Wall -O3
LDFLAGS=-lpthread -pthread

game:
	$(CC) $(FLAGS) $(LDFLAGS) src/game.c

mpi:
	$(MPICC) $(FLAGS) src/game_mpi.c -lm

cuda:
	$(NVCC) src/game_cuda.cu

clean:
	rm -f *.out
