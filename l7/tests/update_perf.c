#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#include "l7/l7.h"

void 
report_results_update(int penum, double *time_total_pe, int count_updated_pe, int num_timings,
      		      int num_timings_cycle)
{
   int i, count_updated_global, bytes_updated, remainder;
   
   double *time_total_global;
   time_total_global = (double *)malloc(num_timings*sizeof(double));
   
   L7_Array_Max(time_total_pe, num_timings, L7_DOUBLE, time_total_global);

   L7_Sum(&count_updated_pe, 1, L7_INT, &count_updated_global);

   if (penum == 0) 
   {
      printf("\n");
      printf("======================================\n");
      printf("    L7_Update test results            \n");
      printf("======================================\n");
      printf("\n");
      printf("Performance\n");
      printf("\n");
      printf("    L7_Setup:    %lf seconds\n",time_total_global[0]);
      printf("\n");
      printf("      L7_Update:   time (secs)      bandwidth    cycle\n");
      
      for (i=1; i<num_timings; i++){
         if (time_total_global[i] != 0.0) {
            remainder = i%2;
            if (remainder == 1){
               bytes_updated = count_updated_global*4;
            }
            else {
               bytes_updated = count_updated_global*8;
            }
            
            printf("                    %lf       %8.0lf        %d\n",time_total_global[i],
                  (double)(bytes_updated)/time_total_global[i],(i+1)/num_timings_cycle);
         }
         else{
            printf("                    %lf       %8.0s        %d\n",0.0,"n/a",(i+1)/num_timings_cycle);
         }
      }
      printf("\n");
      printf("======================================\n");
      printf("    End L7_Update testing             \n");
      printf("======================================\n");
      printf("\n");
   }


   /*
    * Testing complete
    */
   free(time_total_global);
   
   return;
}

int main(int argc, char *argv[])
{
    int penum =0,
        numpes=0,
        ierr;

   int i, j, num_indices_owned, num_updates,
     my_start_index, max_num_partners, remainder,
     num_partners_lo, num_partners_hi, num_partners,
     offset, num_indices_offpe, num_indices_per_partner,
     inum, l7_id, gtime, count_updated_pe, num_timings_cycle,
     num_timings, iout;
   
   double time_start, time_stop;
   double *time_total_pe;
   
   int *partner_pe;
   int *needed_indices;
   
   int * idata;
   double * rdata;
   
   int num_indices_per_pe = 1<<20;
   int num_iterations = 100;
   int num_updates_per_cycle = 2;
   int stride = 0;

   ierr = L7_Init(&penum, &numpes, &argc, argv, 0, 0);

   if (penum == 0)
      printf("\n\t\tRunning the CUDA Update test\n\n");
   
   num_updates = num_iterations * num_updates_per_cycle;
 
   time_total_pe = (double *)malloc((num_updates+1) * sizeof(double));
   
   num_indices_owned = num_indices_per_pe;
   my_start_index = penum * num_indices_owned;
   
   if (numpes > 4) {
      max_num_partners = (int)sqrt( (double)numpes);
   }
   else {
      max_num_partners = numpes/2;
   }
   
   /*
    * Create and load needed_indices array
    */
   
   remainder = max_num_partners % 2;
   
   if (penum < (numpes /2) ) {
      num_partners_lo = max_num_partners / 2;
      num_partners_hi = max_num_partners / 2 + remainder;
   }
   else {
      num_partners_lo = max_num_partners / 2 + remainder;
      num_partners_hi = max_num_partners / 2;
   }
   
   for (;penum - num_partners_lo < 0;num_partners_lo--);

   for (;penum + num_partners_hi >= numpes; num_partners_hi--);

   num_partners = num_partners_lo + num_partners_hi;
   partner_pe = (int *)malloc(num_partners * sizeof(int));
   
   offset = 0;

   /* Indices below this PE - make sure to go in ascending PE order so the
    * indices are in ascending order! */
   for (i=num_partners_lo; i>=1; i--){
      //printf("[pe %d] offset %d penum %d i %d \n",penum, offset, penum, i);
      partner_pe[offset] = penum - i;
      offset++;
   }

   /* Indices above this PE */
   for (i=1; i<=num_partners_hi; i++){
      //printf("[pe %d] offset %d penum %d i %d \n",penum, offset, penum, i);
      partner_pe[offset] = penum + i;
      offset++;
   }
   
   if (num_partners != 0) {
      num_indices_offpe = num_indices_owned / 2;
      num_indices_per_partner = num_indices_offpe / num_partners;
   }
   else {
      num_indices_offpe = 0;
      num_indices_per_partner = 0;
   }

   needed_indices = (int *)malloc(num_indices_offpe * sizeof(int));
   
   /*
    * Generate needed indices
    */
   
   num_indices_offpe = 0;
   for (i=0; i<num_partners; i++) {
      inum = partner_pe[i] * num_indices_owned;
      for (j=0; j<num_indices_per_partner; j++){
         needed_indices[num_indices_offpe] = inum;
         num_indices_offpe++;
         inum+=(1+stride);
      }
   }
   
   /*
    * Allocate data arrays on device and wait for initialization to complete
    */
   unsigned long data_size = num_indices_owned + num_indices_offpe;
   idata = calloc( sizeof(int) , data_size);
   rdata = calloc( sizeof(double) , data_size);
 
   for (i = 0; i < num_indices_owned; i++) {
	idata[i] = my_start_index + i;
	rdata[i] = my_start_index + i;
   } 
   
   //printf("[pe %d] Finished array initialization.\n", penum);

   /*
    * Register decomposition with L7
    */
  
   l7_id = 0;
   
#ifdef _L7_DEBUG
   if (penum == 0) {
      printf("\n");
      printf("======================================\n");
      printf("    Begin L7_CUDA_Update testing\n");
      printf("======================================\n");
      printf("\n");
   }
#endif

   time_start=L7_Wtime();
   
   /*
    * Register decomposition with L7
    */
   L7_Setup(0, my_start_index, num_indices_owned, needed_indices, 
       num_indices_offpe, &l7_id);

   time_stop = L7_Wtime();
   time_total_pe[0] = time_stop - time_start;
   
   /*
    * Begin updating data
    */
   gtime = 1;
   for (i=1; i<=num_iterations; i++){

      time_start = L7_Wtime();
      
      L7_Update(idata, L7_INT, l7_id);

      time_stop = L7_Wtime();
      time_total_pe[gtime] = time_stop - time_start;
      gtime++;
      
      time_start = L7_Wtime();

      L7_Update(rdata, L7_DOUBLE, l7_id);

      time_stop = L7_Wtime();
      time_total_pe[gtime] = time_stop - time_start;
      gtime++;
   }
   
   /*
    * Report results
    */
   count_updated_pe = num_indices_offpe;
   num_timings_cycle = num_updates_per_cycle;
   num_timings = num_updates +1;

   // This involves collectives, so we can't shut down MPI yet.
   report_results_update(penum, time_total_pe, count_updated_pe, 
      		         num_timings, num_timings_cycle);

   free(time_total_pe);
   free(partner_pe);
   free(needed_indices);

   L7_Free(&l7_id);
   L7_Terminate();

   return 0;
   
}

