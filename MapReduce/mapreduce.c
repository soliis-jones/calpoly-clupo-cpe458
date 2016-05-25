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
   int my_id, num_procs, num_lines = 0, i = 0;
   int num_bins, bin_width[3];
   double max[3] = {-DBL_MAX, -DBL_MAX, -DBL_MAX};
   double min[3] = {DBL_MAX, DBL_MAX, DBL_MAX};
   char line[100];
   FILE *fp;
   Entry *entries;

   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
   MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

   if (argc != 2 ){
      perror("Usage: mapreduce <input_file>\n");
      exit(-1);
   }

   if ((fp = fopen(argv[1], "r")) == NULL) {
      perror("fopen");
      exit(-2);
   }
   
   fgets(line, INT_MAX, fp);
   while (EOF != (fscanf(fp, "%*[^\n]"), fscanf(fp, "%*c")))
      ++num_lines;
   entries = malloc(num_lines*sizeof(Entry));

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

      printf("%s: %lf, %lf, %lf\n", entries[i].date, entries[i].vol, entries[i].adj, entries[i].diff);
      i++;
   }
   free(fp);
   
   num_bins = (int)(2*pow(num_lines, 0.333333) + 1);
   bin_width[VOL] = (max[VOL] - min[VOL])/num_bins;
   bin_width[ADJ] = (max[ADJ] - min[ADJ])/num_bins;
   bin_width[DIFF] = (max[DIFF] - min[DIFF])/num_bins;
   
   printf("num lines: %d\n", num_lines);
   printf("Num bins: %d\n", num_bins);
   for (i = 0; i < 3; ++i)
      printf("%d\n", bin_width[i]);
   
   free(entries);
   MPI_Finalize();
   return 0;
}
