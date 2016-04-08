#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>

int main(int argc, char **argv) {
   //double best = -1.0;
    int my_id, num_procs;

   MPI_Init(&argc, &argv);

   MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
   MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

   printf("I am %d out of %d!\n", my_id, num_procs);

   MPI_Finalize();

   return 0;
}
