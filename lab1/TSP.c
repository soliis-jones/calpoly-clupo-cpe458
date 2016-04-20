/* 
 * TSP.c
 * Authors: Travis E Michael & Bobby Jones
 *
 * TSP.c will take an input file of cities and their corresponding coordinates
 * and use an algorithm to produce random new mutations in several processes 
 * using the MPI framework (specifically OpenMPI). The algorithm will be 
 * repeated for the number of iterations specified by the command line
 * argument.
 *
 * Usage: mpirun -np [# Processes] TSP [Input File] [# Iterations]
 *
 * Last Updated: 4/19/16
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include <float.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>

#define NEXT 1
#define PREV 0
#define LINE_SIZE 50

double generate_tour(int *, double *, int);

int main(int argc, char **argv) {
   double best_tour = DBL_MAX, compare_tour;
   int my_id, num_procs, num_cities = 0;
   int *best_path, *compare_path, node_num, i;
   double *city_map;
   unsigned long num_iter, j = 0;
   char line[LINE_SIZE], find_start = 0;
   FILE* stream;

   //printf("Begin MPI processes\n");
   // MPI set up
   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
   MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

   srand(getpid()*time(NULL));
   
   if (argc != 3) {
      printf("Usage: %s input_file num_iterations\n", argv[0]);
      exit(-1);
   }

   // Initial set up of arguments and path array
   if((stream = fopen(argv[1], "r")) == NULL) {
      perror("Error opening file");
      exit(-1);
   }
   num_iter = atol(argv[2]);
   //printf("Number of iterations to run %d\n", num_iter);
   
   //if (!my_id) {
      //printf("Beginning parse of input file: %s\n", argv[1]);

      // Parse input file
      while (fgets(line, INT_MAX, stream) != NULL && !strstr(line, "EOF")) {
         //skip through the beginning of the file until you come to dimension line
         if (find_start == 0 && (strstr(line, "DIMENSION") && (find_start = 1))) {
            //move forward through the line
            while (*line != '\0') {
               //upon finding first number, record and allocate map array
               if(isdigit((int)line[j])) {
                  num_cities = atoi(line+j);
                  city_map = malloc(num_cities*2*sizeof(double));
                  //printf("Detected node dimension value of: %d\n", num_cities);
                  break;
               } else {
                  ++j;
               }
            }
         }
         //keep skipping through the file until we find beginning of coordinates
         else if (find_start == 1 && (strcmp(line, "NODE_COORD_SECTION\n")|| (find_start = 2))) {
            continue;
         }
         else if (find_start == 2) {
            // This is a city number/x/y line
            // Can ignore first token (node name)
            //printf("Parsing line: %s",line);
            node_num = atoi(strtok(line, " ")) - 1;
            //printf("nodenum: %d, X: %d, Y:%d\n", node_num,atoi(strtok(NULL, " ")),atoi(strtok(NULL, " ")) );
            
            city_map[node_num*2 + 0] = atof(strtok(NULL, " "));
            city_map[node_num*2 + 1] = atof(strtok(NULL, " "));
            //printf("Got #%d X[%d] Y[%d]\n", node_num, city_map[node_num*2 + 0], city_map[node_num*2 + 1]);
         }
      }
      fclose(stream);

      //puts("FLAG1");
      //printf("Finished input file parsing.\n");
      //MPI_Barrier(MPI_COMM_WORLD);
      
      //for (i = 1; i < num_procs; ++i) {
         //MPI_Send(&num_cities, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
      //}
   //} else {
      //printf("Node %d allocating city_map\n", my_id);
      //city_map = malloc(num_cities*2*sizeof(int*));
      
      //MPI_Recv(&num_cities, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
   //}
   //MPI_Bcast(city_map, num_cities*2, MPI_INT, 0, MPI_COMM_WORLD);
   //MPI_Barrier(MPI_COMM_WORLD);
   //printf("Succesful broadcast: node %d\n", my_id);
   
   // Allocate arrays for paths
   best_path = malloc(num_cities*2*sizeof(int));
   compare_path = malloc(num_cities*2*sizeof(int));
   
   //printf("Setting default best path\n");
   // Set default tour (sequential order)
   for (i = 1; i < num_cities - 1; i++) {
      best_path[i*2 + PREV] = i - 1;
      best_path[i*2 + NEXT] = i + 1;
   }
   best_path[0*2 + PREV] = num_cities - 1;
   best_path[0*2 + NEXT] = 1;
   best_path[(num_cities-1)*2 + PREV] = num_cities - 2;
   best_path[(num_cities-1)*2 + NEXT] = 0;

   memcpy(compare_path, best_path, num_cities*2*sizeof(int));
   
   //printf("Node %d entering main loop\n", my_id);
   // Main algorithm loop
   for (j = 0; j < num_iter; ++j) {
      //printf("Node %d in main loop iteration %d\n", my_id, j);
      if (my_id) {
         // Worker Process Logic:
         
         //printf("\nNode %d generating tour\n", my_id);
         // Calculate distance of new "tour"
         compare_tour = generate_tour(compare_path, city_map, num_cities);
         //printf("Node %d calculated path distance %lf\n\n", my_id, compare_tour);

         //printf("Sending compare_path to root from node %d\n", my_id);
         // Send data to root process
         MPI_Send(compare_path, num_cities*2, MPI_INT, 0, 0, MPI_COMM_WORLD);
         MPI_Send(&compare_tour, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
      } else {
         //sleep(2);
         // Root Process Logic:
         //printf("Root node ready to receive\n");
         // In the root node, receive computations for tour and the path array
         // for each of the other processes
         for (i = 1; i < num_procs; ++i) {
            //printf("Waiting for node %d\n", i);
            MPI_Recv(compare_path, num_cities*2, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&compare_tour, 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            //printf("Received from node %d\n", i);

            if (compare_tour < best_tour) {
               best_tour = compare_tour;
               memcpy(best_path, compare_path, sizeof(int)*2*num_cities);
               printf("Root process received new best tour of value %f on iteration %lu from node %d\n", best_tour, j, i);
            }
         }
      }

      // Broadcast the best overall path to all nodes
      MPI_Bcast(best_path, num_cities*2, MPI_INT, 0, MPI_COMM_WORLD);
      MPI_Barrier(MPI_COMM_WORLD);
      // Copy new best path into variable for nodes to alter
      memcpy(compare_path, best_path, num_cities*2*sizeof(int));
   }

   //if (my_id) {
   //   exit(0);
   //}
   //MPI_Barrier(MPI_COMM_WORLD);
   if (!my_id) {
      printf("TSP best route finder has completed.\n");
      printf("Best tour distance found was %f\n", best_tour);
      printf("Best path:\n");

      i = 0;
      do {
         printf("%d->", i+1);
      } while ((i = best_path[i*2 + NEXT]));
      printf("1\n");
   }
   
   free(city_map);
   free(best_path);
   free(compare_path);

   MPI_Finalize();
   return 0;
}

// path array is [from, to]
double generate_tour(int *path, double *map, int num_cities) {
   int i, j, i_next, j_next, current;
   int next, tmp;
   double tour = 0.0;
   
   do {
      i = rand() % num_cities;
      j = rand() % num_cities;
      i_next = path[i*2 + NEXT];
      j_next = path[j*2 + NEXT];
      current = i_next;
   } while (i >= j);

   //printf("i = %d, j = %d, i_next = %d, j_next = %d, current = %d\n", i, j, i_next, j_next, current);
   /*int count = 0;
   do {
      printf("%d->", count);
   } while ((count = path[count*2 + NEXT]));
   printf("0\n");*/

   // To begin, swap the direction of everything between and including, i+1 and j
   while (current != j_next) {
      //usleep(20000);
      //printf("current = %d\n", current);
      next = path[current*2 + NEXT];
      tmp = path[current*2 + PREV];
      path[current*2 + PREV] = path[current*2 + NEXT];
      path[current*2 + NEXT] = tmp;
      current = next;
   }
   
   // Finish by swaping the to's and from's of i,j and i_next,j_next
   path[i*2 + NEXT] = j;
   path[j*2 + PREV] = i;
   path[i_next*2 + NEXT] = j_next;
   path[j_next*2 + PREV] = i_next;
   
   /*count = 0;
   do {
      printf("%d->", count);
   } while ((count = path[count*2 + NEXT]));
   printf("0\n");*/
   
   //printf("Calculating tour distance\n");
   // Calculate total distance of tour and return
   current = 0;
   do {
      next = path[current*2 + NEXT];
      tour += sqrt(pow((map[current*2 + 0] - map[next*2 + 0]), 2.0) +
         pow((map[current*2 + 1] - map[next*2 + 1]), 2.0));
      //printf("next: %d, tour: %lf\n", next, tour);
   } while ((current = next));

   return tour;
}
