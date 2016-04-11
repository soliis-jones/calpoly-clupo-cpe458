#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include <float.h>
#include <string.h>

#define NEXT 1
#define PREV 0
#define LINE_SIZE 50

double generate_tour(int**, int);

int main(int argc, char **argv) {
   double best_tour = DBL_MAX, compare_tour;
   int my_id, num_procs, num_cities = 0, num_iter, i, j;
   int **best_path, **compare_path, **city_map, node_num;
   char *line = malloc(LINE_SIZE), find_start = 0;
   MPI_Status* status;
   FILE* stream;

   // Initial set up of arguments and path array
   //num_cities = atoi(argv[1]); // This should come from the file
   if((stream = fopen(argv[1], "r")) == NULL) {
      perror("Error opening file");
      exit(-1);
   }
   num_iter = atoi(argv[2]);

   // Parse input file
   while (fgets(line, INT_MAX, stream) != NULL && strcmp(line, "EOF")) {
      //skip through the beginning of the file until you come to dimension line
      if (find_start == 0 && (strstr(line, "DIMENSION") && find_start = 1)) {
         //move forward through the line
         while (*line != '\0') {
            //upon finding first number, record and allocate map array
            if(isdigit((int)*line)) {
               num_cities = atoi(line);
               city_map = malloc(num_cities*2*sizeof(int));
            } else {
               ++line;
            }
         }
      }
      //keep skipping through the file until we find beginning of coordinates
      else if (find_start == 1 && (strcmp(line, "NODE_COORD_SECTION") || find_start = 2)) {
         continue;
      } 
      else if (find_start == 2) {
         // This is a city number/x/y line
         // Can ignore first token (node name)
         node_num = atoi(strtok(line, " "));
         city_map[node_num][0] = atoi(strtok(NULL, " "));
         city_map[node_num][1] = atoi(strtok(NULL, " "));
      }
   }
   free(line);
   
   best_path = malloc(num_cities*2*sizeof(int));
   compare_path = malloc(num_cities*2*sizeof(int));


   // Set default tour (sequential order)
   for (i = 1; i < num_cities - 1; i++) {
      path[i] = {i - 1, i +1};
   }
   path[0] = {num_cities, 1};
   path[num_cities-1] = {num_cities-2, 0};

   // MPI set up
   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
   MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

   // Main algorithm loop
   for (j = 0; j < num_iter; ++j) {
      if (my_id) {
         // Worker Process Logic:

         // Calculate distance of new "tour"
         compare_tour = generate_tour(compare_path, num_cities);

         // Send data to root process
         MPI_Send(compare_path, num_cities*2, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
         MPI_Send(compare_tour, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      } else {
         // Root Process Logic:

         // In the root node, receive computations for tour and the path array
         // for each of the other processes
         for (i = 1; i < num_procs; ++i) {
            MPI_Recv(compare_path, num_cities*2, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(compare_tour, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            if (compare_tour < best_tour) {
               best_tour = compare_tour;
               memcpy(best_path, compare_path, sizeof(int)*num_cities);
            }
         }
      }

      MPI_Bcast(best_path, num_cities, MPI_INT, 0, 0, MPI_COMM_WORLD);
   }

   MPI_Finalize();

   return 0;
}

// path array is [from, to]
double generate_tour(int **path, int num_cities) {
   int i = rand() % num_cities, j = rand() % num_cities;
   int i_next = path[i][NEXT], j_next = path[j][NEXT], current = i_next;
   int tmp;

   // To begin, swap the direction of everything between and including, i+1 and j
   while (current != j_next) {
      tmp = path[current][PREV];
      path[current][PREV] = path[current][NEXT];
      path[current][NEXT] = tmp;
      current = path[current][NEXT];
   }

   // Finish by swaping the to's and from's of i,j and i_next,j_next
   path[i][NEXT] = j;
   path[j][PREV] = i;
   path[i_next][NEXT] = j_next;
   path[j_next][PREV] = i_next;

   //TODO: calculate total distance and return
   return 0.0;
}
