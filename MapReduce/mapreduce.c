#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <float.h>
#include <math.h>

#include "mrmpi-7Apr14/src/cmapreduce.h"

#define MAX(A,B) (((A > B) ? A : B))
#define MIN(A,B) (((A < B) ? A : B))


void vol_map(int, void *, void *);
void adj_map(int, void *, void *);
void diff_map(int, void *, void *);

void my_reduce(char *, int, char *, int, int *, void *, void *);
int ncompare(char *, int, char *, int);

enum Bins {
   VOL,
   ADJ,
   DIFF
};

typedef struct Entry {
   char date[11];
   double vol;
   double adj;
   double diff;
} Entry;

int main (int argc, char **argv) {
   int my_id, num_procs, num_entries = 0, i = 0;
   int num_bins, bin_width[3];
   double max[3] = {-DBL_MAX, -DBL_MAX, -DBL_MAX};
   double min[3] = {DBL_MAX, DBL_MAX, DBL_MAX};
   char line[100];
   FILE *fp;
   Entry *entries;

   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
   MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

   if (argc != 2 ) {
      if (my_id == 0)
         perror("Usage: mapreduce <input_file>\n");
      MPI_Abort(MPI_COMM_WORLD, 1);
   }

   if ((fp = fopen(argv[1], "r")) == NULL) {
      perror("fopen");
      MPI_Abort(MPI_COMM_WORLD, -1);
   }
   
   fgets(line, INT_MAX, fp);
   while (EOF != (fscanf(fp, "%*[^\n]"), fscanf(fp, "%*c")))
      ++num_entries;
   entries = malloc(num_entries*sizeof(Entry));

   fseek(fp, 0, SEEK_SET);
   fgets(line, INT_MAX, fp);
   while (fgets(line, INT_MAX, fp) != NULL && !strstr(line, "EOF")) {
      strcpy(entries[i].date, strtok(line, ","));
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
      
      //if (my_id == 0) {
      //   printf("%s: %lf, %lf, %lf\n", entries[i].date, entries[i].vol, entries[i].adj, entries[i].diff);
      i++;
   }
   free(fp);
   
   num_bins = (int)(2*pow(num_entries, 0.333333) + 1);
   bin_width[VOL] = (max[VOL] - min[VOL])/num_bins;
   bin_width[ADJ] = (max[ADJ] - min[ADJ])/num_bins;
   bin_width[DIFF] = (max[DIFF] - min[DIFF])/num_bins;
   
   if (my_id == 0) {
      printf("Num entries: %d\n", num_entries);
      printf("Num bins: %d\n", num_bins);
      for (i = 0; i < 3; ++i)
         printf("%d\n", bin_width[i]);
   }

   void *mr = MR_create(MPI_COMM_WORLD);
   MR_set_verbosity(mr, 2);
   MR_set_timer(mr, 1);

   MPI_Barrier(MPI_COMM_WORLD);

   MR_map(mr, num_entries, &vol_map, entries);
   MR_collate(mr, NULL);
   //MR_map(mr, num_entries, &adj_map, entries);
   //MR_map(mr, num_entries, &diff_map, entries);
   MR_reduce(mr, &my_reduce, NULL);

   MPI_Barrier(MPI_COMM_WORLD);
   MR_sort_values(mr, &ncompare);
   
   MR_destroy(mr);
   free(entries);
   MPI_Finalize();
   return 0;
}

void vol_map(int itask, void *kv, void *ptr) {
   Entry *entry = (Entry *)ptr;

   MR_kv_add(kv, (char *)&entry->vol, sizeof(double), NULL, 0);
}

void my_reduce(char *key, int key_bytes, char *multivalue, int nvalues,
 int *value_bytes, void *kv, void *ptr) {
   MR_kv_add(kv, key, key_bytes, (char *)&nvalues, sizeof(int));
}

void adj_map(int itask, void *kv, void *ptr) {
   Entry *entry = (Entry *)ptr;

   MR_kv_add(kv, entry->date, strlen(entry->date), (char *)&entry->adj, 
    sizeof(double));
}

void diff_map(int itask, void *kv, void *ptr) {
   Entry *entry = (Entry *)ptr;

   MR_kv_add(kv, entry->date, strlen(entry->date), (char *)&entry->diff, 
    sizeof(double));
}

int ncompare(char *p1, int len1, char *p2, int len2) {
   int i1 = *(int *)p1;
   int i2 = *(int *)p2;
   
   if (i1 == i2)
      return 0;
   return i1 > i2 ? -1 : 1;
}
