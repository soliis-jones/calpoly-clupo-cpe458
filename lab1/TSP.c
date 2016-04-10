#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>

int main(int argc, char **argv) {
   //double best = -1.0;
   int my_id, num_procs, num_cities, num_iter, i, j, count;
   int *path;

   // Initial set up of arguments and path array
   num_cities = atoi(argv[1]);
   path = malloc(atoi(num_cities*sizeof(int));
   num_iter = atoi(argv[2]);

   // Set default tour (sequential order)
   for (i = 0; i < num_cities - 1; i++) {
      path[i] = i + 1;
   }
   path[i] = 0;

   // MPI set up
   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
   MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

   // Main algorithm loop
   for (count = 0; count < num_iter; ++count) {
      // Select random i & j
      i = rand() % num_cities;
      j = rand() % num_cities;

      // Swap paths from i->j and (i+1)->(j+1)
      // TODO: Ensure that algorithm works (doesn't break cycle)
      paths[i] = j;
      paths[i+1] = j+1;

      // Calculate distance of new "tour"

      // Compare if new tour is less than current "best"
      if (cur_tour < best_tour) {
         // If so, broadcast new path
         MPI_Bcast()
      }
   }

   MPI_Finalize();

   return 0;
}
