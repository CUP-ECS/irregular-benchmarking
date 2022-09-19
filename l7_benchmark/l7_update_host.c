#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void initialize_data_host(void **odata, int nowned, int nremote, int typesize, int my_start_index)
{
   void *data = calloc( typesize, nowned + nremote);
   int i;

   if (!data) {
      fprintf(stderr, "Could not allocate %d elements of size %d in host memory space.\n", 
              nowned + nremote, typesize);
      exit(-1);
   }

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
      default:
         fprintf(stderr, "Unknown type of size %d.\n", typesize);
         exit(-1);
      } 
   }

   *odata = data;
   return;
}
