typedef struct
{
    unsigned char R;
    unsigned char G;
    unsigned char B;
    unsigned char align;
} RGB;

__kernel void histogram(__global RGB *img, __global unsigned int *R, __global unsigned int *G, __global unsigned int *B){
    
    int i = get_global_id(0);
    int r = img[i].R;
    int g = img[i].G;
    int b = img[i].B;
    atomic_add(&R[r], 1);
    atomic_add(&G[g], 1);
    atomic_add(&B[b], 1);
}

