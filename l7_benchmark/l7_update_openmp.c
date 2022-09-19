#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <omp.h>

void initialize_data_openmp(void **odata, int nowned, int nremote, int typesize, int my_start_index)
{
   int i;
   int datalen = nowned + nremote;
   int h = omp_get_initial_device(); // host
   int t = omp_get_default_device(); // target device

   void *data = omp_target_alloc(datalen * typesize, t);

   if (!data) {
      fprintf(stderr, "Could not allocate %d elements of size %d in openmp memory space.\n", 
              nowned + nremote, typesize);
      exit(-1);
   }

   #pragma omp target is_device_ptr(data) device(t)
   #pragma omp teams distribute parallel for
   for (i = 0; i < nowned; i++) {
      switch(typesize) {
      case 1:
         ((unsigned char *)data)[i] = my_start_index + 1;
         break;
      case 2:
         ((unsigned short *)data)[i] = my_start_index + 1;
         break;
      case 4:
         ((unsigned int *)data)[i] = my_start_index + 1;
         break;
      case 8:
         ((unsigned long *)data)[i] = my_start_index + 1;
         break;
      } 
   }

   *odata = data;
   return;
}
