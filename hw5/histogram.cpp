#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <fstream>
#include <iostream>
#include <string>
#include <ios>
#include <time.h>
#include <sys/time.h>
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

typedef struct
{
    uint8_t R;
    uint8_t G;
    uint8_t B;
    uint8_t align;
} RGB;

typedef struct
{
    bool type;
    uint32_t size;
    uint32_t height;
    uint32_t weight;
    RGB *data;
} Image;

Image *readbmp(const char *filename)
{
    std::ifstream bmp(filename, std::ios::binary);
    char header[54];
    bmp.read(header, 54);
    uint32_t size = *(int *)&header[2];
    uint32_t offset = *(int *)&header[10];
    uint32_t w = *(int *)&header[18];
    uint32_t h = *(int *)&header[22];
    uint16_t depth = *(uint16_t *)&header[28];
    if (depth != 24 && depth != 32)
    {
        printf("we don't suppot depth with %d\n", depth);
        exit(0);
    }
    bmp.seekg(offset, bmp.beg);

    Image *ret = new Image();
    ret->type = 1;
    ret->height = h;
    ret->weight = w;
    ret->size = w * h;
    ret->data = new RGB[w * h]{};
    for (int i = 0; i < ret->size; i++)
    {
        bmp.read((char *)&ret->data[i], depth / 8);
    }
    return ret;
}

int writebmp(const char *filename, Image *img)
{

    uint8_t header[54] = {
        0x42,        // identity : B
        0x4d,        // identity : M
        0, 0, 0, 0,  // file size
        0, 0,        // reserved1
        0, 0,        // reserved2
        54, 0, 0, 0, // RGB data offset
        40, 0, 0, 0, // struct BITMAPINFOHEADER size
        0, 0, 0, 0,  // bmp width
        0, 0, 0, 0,  // bmp height
        1, 0,        // planes
        32, 0,       // bit per pixel
        0, 0, 0, 0,  // compression
        0, 0, 0, 0,  // data size
        0, 0, 0, 0,  // h resolution
        0, 0, 0, 0,  // v resolution
        0, 0, 0, 0,  // used colors
        0, 0, 0, 0   // important colors
    };

    // file size
    uint32_t file_size = img->size * 4 + 54;
    header[2] = (unsigned char)(file_size & 0x000000ff);
    header[3] = (file_size >> 8) & 0x000000ff;
    header[4] = (file_size >> 16) & 0x000000ff;
    header[5] = (file_size >> 24) & 0x000000ff;

    // width
    uint32_t width = img->weight;
    header[18] = width & 0x000000ff;
    header[19] = (width >> 8) & 0x000000ff;
    header[20] = (width >> 16) & 0x000000ff;
    header[21] = (width >> 24) & 0x000000ff;

    // height
    uint32_t height = img->height;
    header[22] = height & 0x000000ff;
    header[23] = (height >> 8) & 0x000000ff;
    header[24] = (height >> 16) & 0x000000ff;
    header[25] = (height >> 24) & 0x000000ff;

    std::ofstream fout;
    fout.open(filename, std::ios::binary);
    fout.write((char *)header, 54);
    fout.write((char *)img->data, img->size * 4);
    fout.close();
}

int main(int argc, char *argv[])
{
    
//     struct timespec st, end;
//     clock_gettime(CLOCK_REALTIME, &st);

    char *filename;
    if (argc >= 2)
    {
        int many_img = argc - 1;
        
        unsigned int *R = new unsigned int[256];
        unsigned int *G = new unsigned int[256];
        unsigned int *B = new unsigned int[256];
            
        // Load the kernel source code into the array source_str
        FILE *fp;
        char *source_str;
        size_t source_size;
            
        fp = fopen("histogram.cl", "r");
        if(!fp) {
            fprintf(stderr, "failed to load kernel.\n");
            exit(1);
        }
        source_str = (char*)malloc(0x100000);
        source_size = fread(source_str, 1, 0x100000, fp);
        fclose(fp);
        
        // Get platform and device information
        cl_platform_id platform_id = NULL;
        cl_device_id device_id = NULL;
        cl_uint ret_num_device;
        cl_uint ret_num_platform;
        cl_int ret = clGetPlatformIDs(1, &platform_id, &ret_num_platform);
        ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_device);
            
        // Create an OpenCL context
        cl_context context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
//          if(ret != 0)   {
//              printf("no context, %d\n", int(ret));
//          }

        // Create a command queue
        cl_command_queue command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
//          if(ret != 0)   {
//              printf("no command queue, %d\n", int(ret));
//          }

        // Create memory buffers on the device for each vector
        cl_mem R_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 256*sizeof(unsigned int), NULL, &ret);
        cl_mem G_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 256*sizeof(unsigned int), NULL, &ret);
        cl_mem B_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 256*sizeof(unsigned int), NULL, &ret);
//          if(ret != 0)   {
//              printf("no membuff, %d\n", int(ret));
//          }

        // Create a program from the kernel source 
        cl_program program = clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);
//          if(ret != 0)   {
//              printf("no program, %d\n", int(ret));
//          }
            
        // Build the program
        ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
//          if(ret != 0)    {
//              printf("build failed, %d\n", int(ret));
//              size_t log_size;
//              clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
//              char *log = (char *)malloc(log_size);
//              clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, 0);
//              printf("%s\n", log);
//          }

        for (int i = 0; i < many_img; i++)
        {
            filename = argv[i + 1];
            Image *img = readbmp(filename);
            std::cout << img->weight << ":" << img->height << "\n";

            std::fill(R, R+256, 0);
            std::fill(G, G+256, 0);
            std::fill(B, B+256, 0);
            
            // Create memory buffers on the device for each vector
            cl_mem img_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, img->size*sizeof(RGB), NULL, &ret);
            
            // Copy data to their respective memory buffers
            ret = clEnqueueWriteBuffer(command_queue, img_mem_obj, CL_TRUE, 0, img->size*sizeof(RGB), img->data, 0, NULL, NULL);
            ret = clEnqueueWriteBuffer(command_queue, R_mem_obj, CL_TRUE, 0, 256*sizeof(unsigned int), R, 0, NULL, NULL);
            ret = clEnqueueWriteBuffer(command_queue, G_mem_obj, CL_TRUE, 0, 256*sizeof(unsigned int), G, 0, NULL, NULL);
            ret = clEnqueueWriteBuffer(command_queue, B_mem_obj, CL_TRUE, 0, 256*sizeof(unsigned int), B, 0, NULL, NULL);
//             if(ret != 0)   {
//                 printf("no copy, %d\n", int(ret));
//             }

            // Create the OpenCL kernel
            cl_kernel kernel = clCreateKernel(program, "histogram", &ret);
//             if(ret != 0)   {
//                 printf("no kernel, %d\n", int(ret));
//             }
            // Set the argument of the kernel
            ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), &img_mem_obj);
            ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), &R_mem_obj);
            ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), &G_mem_obj);
            ret = clSetKernelArg(kernel, 3, sizeof(cl_mem), &B_mem_obj);
//             ret = clSetKernelArg(kernel, 4, sizeof(unsigned int), &(img->size));
            if(ret != 0)   {
                printf("no argument, %d\n", int(ret));
            }
            
            // Execute the OpenCL kernel on the list
            size_t global_item_size = img->size;
            size_t local_item_size = 1;
            ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size, &local_item_size, 0, NULL, NULL);
//             if(ret != 0)   {
//                 printf("no execute, %d\n", int(ret));
//             }
            
            // Read the memory buffer R、G、B on the device to the local variable
            ret = clEnqueueReadBuffer(command_queue, R_mem_obj, CL_TRUE, 0, 256*sizeof(unsigned int), R, 0, NULL, NULL); 
            ret = clEnqueueReadBuffer(command_queue, G_mem_obj, CL_TRUE, 0, 256*sizeof(unsigned int), G, 0, NULL, NULL); 
            ret = clEnqueueReadBuffer(command_queue, B_mem_obj, CL_TRUE, 0, 256*sizeof(unsigned int), B, 0, NULL, NULL); 
//             if(ret != 0)   {
//                 printf("no read, %d\n", int(ret));
//             }

            // Clean up
            ret = clReleaseKernel(kernel);
            ret = clReleaseMemObject(img_mem_obj);
            
            int max = 0;
            for(int i=0;i<256;i++){
                max = R[i] > max ? R[i] : max;
                max = G[i] > max ? G[i] : max;
                max = B[i] > max ? B[i] : max;
            }
            
            Image *outcome = new Image();
            outcome->type = 1;
            outcome->height = 256;
            outcome->weight = 256;
            outcome->size = 256 * 256;
            outcome->data = new RGB[256 * 256];

            for(int i=0;i<outcome->height;i++){
                for(int j=0;j<256;j++){
                    if(R[j]*256/max > i)
                        outcome->data[256*i+j].R = 255;
                    else
                        outcome->data[256*i+j].R = 0;
                    if(G[j]*256/max > i)
                        outcome->data[256*i+j].G = 255;
                    else
                        outcome->data[256*i+j].G = 0;
                    if(B[j]*256/max > i)
                        outcome->data[256*i+j].B = 255;
                    else
                        outcome->data[256*i+j].B = 0;
                }
            }

            std::string newfile = "hist_" + std::string(filename); 
            writebmp(newfile.c_str(), outcome);
        }
        
        // Clean up
        ret = clReleaseProgram(program);
        ret = clReleaseMemObject(R_mem_obj);
        ret = clReleaseMemObject(G_mem_obj);
        ret = clReleaseMemObject(B_mem_obj);
        ret = clReleaseCommandQueue(command_queue);
        ret = clReleaseContext(context);
        
        delete [] R;
        delete [] G;
        delete [] B;
        
    }else{
        printf("Usage: ./hist <img.bmp> [img2.bmp ...]\n");
    }

//     clock_gettime(CLOCK_REALTIME, &end);
//     double secs = (( double)end.tv_sec - (double)st.tv_sec ) + (( double)end.tv_nsec - (double)st.tv_nsec )/1000000000.0;
//     printf("Time: %.6f seconds\n", secs);

    return 0;
}
