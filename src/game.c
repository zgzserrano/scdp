#define _XOPEN_SOURCE 600 /* Or higher */

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>

#include <time.h>

#define THREADS_NUMBER 4

#define GEN_LIMIT 1000
#define TRUE 1
#define FALSE 0

typedef unsigned char cell_t;

int size;
int width = 0, height = 0;
int stop = FALSE, k;
int num_threads, lines, reminder;

cell_t ** prev, ** next, ** tmp;
pthread_barrier_t barrier;

cell_t ** allocate_board () {
  cell_t ** board = (cell_t **) malloc(sizeof(cell_t*)*size);
  int i;
  for (i=0; i<size; i++)
    board[i] = (cell_t *) malloc(sizeof(cell_t)*size);
  return board;
}

void free_board (cell_t ** board) {
  int     i;
  for (i=0; i<size; i++)
  free(board[i]);
  free(board);
}

int empty (cell_t ** board) {
  int     i, j;
  for (i=0; i<height; i++) for (j = 0; j < width; j++)
    if(board[i][j] == 1) return FALSE;
  return TRUE;
}



/* return the number of on cells adjacent to the i,j cell */
int adjacent_to (cell_t ** board, int i, int j) {
  int k, l, count=0;

  int sk = (i>0) ? i-1 : i;         //IZQ
  int ek = (i+1 < size) ? i+1 : i;  //DES
  int sl = (j>0) ? j-1 : j;         //ARR
  int el = (j+1 < size) ? j+1 : j;  //ABA

  for (k=sk; k<=ek; k++)
    for (l=sl; l<=el; l++)
      count+=board[k][l];
  count-=board[i][j];

  return count;
}

/* read a file into the life board */
void read_file (FILE * f, cell_t ** board) {
  int i, j;
  char  *s = (char *) malloc(size);

  /* read the life board */
  for (j=0; j<size; j++) {
    /* get a string */
    fgets (s, size,f);
    /* copy the string to the life board */
    for (i=0; i<size; i++)
    board[i][j] = s[i] == '1';   // !!! Cambiado para poner 1 y no x
  }
}

void print_to_file(cell_t **univ, int width, int height)
{
    FILE *fout = fopen("./game_output.out", "w"); // printing the result to a file with
                                                 // 1 or 0 (1 being an alive cell and 0 a dead cell)
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            fprintf(fout, "%c", (univ[i][j] == 1) ? '1': '0' );
        }
        fprintf(fout, "\n");
    }

    fflush(fout);
    fclose(fout);
}

void play (int this_start, int this_end, int thread_id) {
  /* for each cell, apply the rules of Life */
  int a;

  while (k < GEN_LIMIT) {   //!!! De donde vendra esa k?
  //SOL: k es la generacion x la que van entiendo, la acutaliada threadID == 0

    for (int i=this_start; i<this_end; i++) {
        for (int j=0; j<size; j++) {
          a = adjacent_to (prev, i, j);
          if (a == 2) next[i][j] = prev[i][j];
          if (a == 3) next[i][j] = 1;
          if (a < 2) next[i][j] = 0;
          if (a > 3) next[i][j] = 0;
      }
    }

    // Barreira pra todas as threads terminarem de processar
    pthread_barrier_wait(&barrier);

    // Uma única thread executa o final do step
    if(thread_id == 0) {
      if(empty(prev)) stop = TRUE;
      else { 
        tmp = next;
        next = prev;
        prev = tmp; 
        k++;
      }
    }

    // Barreira para esperarem o final do step
    pthread_barrier_wait(&barrier);
    if (stop) break;
    
  }

  pthread_exit(NULL);
}

void* defineWork(void* arg) {
  int thread_id = *((int *) arg);
  int this_start = thread_id * lines;
  int this_end = this_start + lines;

  // El ultimo thread usa el resto
  if (thread_id == num_threads - 1) {
    this_end+= reminder;
  }

  play(this_start, this_end, thread_id);

  return 0;
}

int main (int argc, char ** argv) {

  // !!!!! A MODIFICAR PARA HACER MISMO IMPUT QUE EL RESTO

  //Establecer tamaño
  if (argc > 1)
    width = atoi(argv[1]);
  if (argc > 2)
    height = atoi(argv[2]);
  size = width*height;
  prev = allocate_board ();
  
  FILE* f = fopen(argv[3], "r");
  read_file (f, prev);
  fclose(f);
  next = allocate_board ();
  num_threads = THREADS_NUMBER;


  // Dividiendo trabajo
  lines = size/num_threads; 
  reminder = size%num_threads;

  // Inicializar threads
  pthread_barrier_init(&barrier, NULL, num_threads);
  pthread_t threads[num_threads];


  for (int i = 0; i < num_threads; ++i)
  { 
    int *arg = malloc(sizeof(int));
    *arg = i;
    pthread_create(&threads[i], NULL, defineWork, arg);
  }

  time_t start, end;
  start = clock();
  for (int i = 0; i < num_threads; ++i)
  {
    pthread_join(threads[i], NULL);
  }
  end = clock();
  float msecs = ((float) (end - start)*1000) / CLOCKS_PER_SEC;
  //Escribir en final
  print_to_file(prev, width, height);
  printf("Generations:\t%d\n", k);
  printf("Execution time:\t%.2f msecs\n", msecs);

  printf("Finished\n");
  pthread_barrier_destroy(&barrier);
  free_board(prev);
  free_board(next);
}