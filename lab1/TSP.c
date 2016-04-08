#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>

int main(int argc, char **argv) {
   //double best = -1.0;
   int my_id, num_procs, num_cities, num_iter, i, j;
   int *paths;

   num_cities = atoi(argv[1]);
   path = malloc(atoi(num_cities*sizeof(int));
   num_iter = atoi(argv[2]);

   for (i = 0; i < num_cities - 1; i++) {
      path[i] = i + 1;
   }
   path[i] = 0;

   MPI_Init(&argc, &argv);
   
   MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
   MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
   
   i = rand() % num_cities;
   j = rand() % num_cities;

   paths[i] = j;
   paths[i+1] = j+1;

   MPI_Finalize();

   return 0;
}
