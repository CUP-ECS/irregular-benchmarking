#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <omp.h>

#include "l7/l7.h"
#include "ezcl/ezcl.h"

#include "l7_update_kern.inc"

void initialize_data_opencl(void **odata, int nowned, int nremote, int typesize, int my_start_index)
{
   int i, ierr;
   void *data = NULL;
   unsigned long datalen = nowned + nremote;

   data = ezcl_malloc(NULL, "data", &datalen, typesize,  CL_MEM_READ_WRITE, 0);

   /* Compile and invoke kernels to initialize the arrays */
   cl_context context = ezcl_get_context();
   cl_program program = ezcl_create_program_wsource(context, NULL, l7_update_kern_source);
   cl_kernel init_kernel;

   switch(typesize) {
      case 1:
         init_kernel = ezcl_create_kernel_wprogram(program, "init_char_array_cl");
         break;
      case 2:
         init_kernel = ezcl_create_kernel_wprogram(program, "init_short_array_cl");
         break;
      case 4:
         init_kernel = ezcl_create_kernel_wprogram(program, "init_int_array_cl");
         break;
      case 8:
         init_kernel = ezcl_create_kernel_wprogram(program, "init_double_array_cl");
         break;
   } 

   ezcl_program_release(program);

   cl_command_queue command_queue = ezcl_get_command_queue();
  
   ezcl_set_kernel_arg(init_kernel,  0, sizeof(cl_int),  (void *)&nowned);
   ezcl_set_kernel_arg(init_kernel,  1, sizeof(cl_int), (void *)&my_start_index);
   ezcl_set_kernel_arg(init_kernel,  2, sizeof(cl_mem),  (void *)&data);

   size_t local_work_size = 128;
   size_t global_work_size = ((nowned + local_work_size - 1) / local_work_size) * local_work_size;
   ezcl_enqueue_ndrange_kernel(command_queue, init_kernel, 1, NULL, &global_work_size, &local_work_size, NULL);
   ezcl_finish(command_queue);

   ezcl_kernel_release(init_kernel);

   *odata = data;
   return;
}
