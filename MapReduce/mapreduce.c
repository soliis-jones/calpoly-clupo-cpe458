#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <float.h>
#include <math.h>

#include "cmapreduce.h"

#define MAX(A,B) (((A > B) ? A : B))
#define MIN(A,B) (((A < B) ? A : B))


void vol_map(int, void *, void *);
void adj_map(int, void *, void *);
void diff_map(int, void *, void *);
void my_reduce(char *, int, char *, int, int *, void *, void *);
void print_pairs(uint64_t, char *, int , char *, int, void *, void *);

enum Bins {
   VOL,
   ADJ,
   DIFF
};

typedef struct Entry {
   double vol;
   double adj;
   double diff;
} Entry;

double bin_width[3];

int main (int argc, char **argv) {
   int my_id, num_procs, num_entries = 0, num_bins, i = 0;
   double max[3] = {-DBL_MAX, -DBL_MAX, -DBL_MAX};
   double min[3] = {DBL_MAX, DBL_MAX, DBL_MAX};
   char line[200];
   FILE *fp;
   Entry *entries;

   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
   MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

   if (argc != 2) {
      if (my_id == 0)
         perror("Usage: mapreduce <input_file>\n");
      MPI_Abort(MPI_COMM_WORLD, 1);
   }

   if ((fp = fopen(argv[1], "r")) == NULL) {
      perror("fopen");
      MPI_Abort(MPI_COMM_WORLD, -1);
   }
   
   fgets(line, 200, fp);
   while (EOF != (fscanf(fp, "%*[^\n]"), fscanf(fp, "%*c")))
      ++num_entries;
   entries = malloc(num_entries*sizeof(Entry));

   fseek(fp, 0, SEEK_SET);
   fgets(line, 200, fp);
   while (fgets(line, INT_MAX, fp) != NULL && !strstr(line, "EOF")) {
      strtok(line, ",");
      entries[i].diff = atof(strtok(NULL, ","));
      strtok(NULL, ",");
      strtok(NULL, ",");
      entries[i].diff = atof(strtok(NULL, ",")) - entries[i].diff;
      entries[i].vol = atof(strtok(NULL, ","));
      entries[i].adj = atof(strtok(NULL, ","));

      max[VOL] = MAX(entries[i].vol, max[VOL]);
      max[ADJ] = MAX(entries[i].adj, max[ADJ]);
      max[DIFF] = MAX(entries[i].diff, max[DIFF]);
      min[VOL] = MIN(entries[i].vol, min[VOL]);
      min[ADJ] = MIN(entries[i].adj, min[ADJ]);
      min[DIFF] = MIN(entries[i].diff, min[DIFF]);

      entries[i].diff += -min[DIFF];
      
      //if (my_id == 0) {
      //   printf("%s: %lf, %lf, %lf\n", entries[i].date, entries[i].vol, entries[i].adj, entries[i].diff);
      i++;
   }

   max[DIFF] += -min[DIFF];
   min[DIFF] = 0;
   
   num_bins = (int)(2*pow(num_entries, 0.333333) + 1);
   bin_width[VOL] = (max[VOL] - min[VOL])/(num_bins-1);
   bin_width[ADJ] = (max[ADJ] - min[ADJ])/(num_bins-1);
   bin_width[DIFF] = (max[DIFF] - min[DIFF])/(num_bins-1);
   
   if (my_id == 0) {
      printf("Num entries: %d\n", num_entries);
      printf("Num bins: %d\n", num_bins);
      //for (i = 0; i < 3; ++i)
         //printf("%lf\n", bin_width[i]);
   }

   void *vol_mr = MR_create(MPI_COMM_WORLD);
   void *adj_mr = MR_create(MPI_COMM_WORLD);
   void *diff_mr = MR_create(MPI_COMM_WORLD);

   MPI_Barrier(MPI_COMM_WORLD);

   MR_map(vol_mr, num_entries, &vol_map, entries);
   MR_collate(vol_mr, NULL);
   MR_reduce(vol_mr, &my_reduce, NULL);
   MPI_Barrier(MPI_COMM_WORLD);
   MR_sort_keys_flag(vol_mr, 1); 
   if (my_id == 0) 
      printf("\nVolume Traded: \n");
   //MR_map_mr(vol_mr, vol_mr, &print_pairs, NULL);
   MR_print(vol_mr, -1, 1, 1, 1);
   
   MR_map(adj_mr, num_entries, &adj_map, entries);
   MR_collate(adj_mr, NULL);
   MR_reduce(adj_mr, &my_reduce, NULL);
   MPI_Barrier(MPI_COMM_WORLD);
   MR_sort_keys_flag(adj_mr, 1);
   if (my_id == 0)
      printf("\nAdjusted Closing Price: \n");
   //MR_map_mr(adj_mr, adj_mr, &print_pairs, NULL);
   MR_print(adj_mr, -1, 1, 1, 1);

   MR_map(diff_mr, num_entries, &diff_map, entries);
   MR_collate(diff_mr, NULL);
   MR_reduce(diff_mr, &my_reduce, NULL);
   MPI_Barrier(MPI_COMM_WORLD);
   MR_sort_keys_flag(diff_mr, 1);
   if (my_id == 0)
      printf("\nDaily Differential: \n");
   //MR_map_mr(diff_mr, diff_mr, &print_pairs, NULL);
   MR_print(diff_mr, -1, 1, 1, 1);


   MR_destroy(vol_mr);
   MR_destroy(adj_mr);
   MR_destroy(diff_mr);
   free(fp);
   free(entries);
   MPI_Finalize();
}

void vol_map(int itask, void *kv, void *ptr) {
   Entry *entry = (Entry *)ptr;
   int key; 

   key = entry[itask].vol/bin_width[VOL];

   MR_kv_add(kv, (char *)&key, sizeof(int), NULL, 0);
}

void adj_map(int itask, void *kv, void *ptr) {
   Entry *entry = (Entry *)ptr;
   int key; 

   key = entry[itask].adj/bin_width[ADJ];

   MR_kv_add(kv, (char *)&key, sizeof(int), NULL, 0);
}

void diff_map(int itask, void *kv, void *ptr) {
   Entry *entry = (Entry *)ptr;
   int key; 

   key = entry[itask].diff/bin_width[DIFF];

   MR_kv_add(kv, (char *)&key, sizeof(int), NULL, 0);
}

void my_reduce(char *key, int key_bytes, char *multivalue, int nvalues,
 int *value_bytes, void *kv, void *ptr) {
   MR_kv_add(kv, key, key_bytes, (char *)&nvalues, sizeof(int));
}

//void print_pairs(uint64_t itask, char *key, int key_bytes, char *value,
// int value_bytes, void *kv, void *ptr) {
//   printf("%02d: %13.2lf ", *(int *)key, bin_width[VOL]*itask);
//   for (int i = 0; i < *(int *)value; i += 100)
//      printf("|");
//   printf(" %d\n", *(int *)value);
//}
