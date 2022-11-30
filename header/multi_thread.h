#include <pthread.h>

# define MAX_THREADS 16
// Create other necessary functions here
pthread_mutex_t lock;

struct arguments {
    int start;
    int M;
    int N;
    int *matA;
    int *matB;
    int *output;
};


// Multithreaded implementation of normal RMM
void* multiThread_2(void *a)
{
    int start = ((struct arguments*) a)->start;
    int M = ((struct arguments*) a)->M;
    int N = ((struct arguments*) a)->N;
    int *matA = ((struct arguments*) a)->matA;
    int *matB = ((struct arguments*) a)->matB;
    int *output = ((struct arguments*) a)->output;

    assert( N>=4 and N == ( N &~ (N-1)));
    for(int rowA = start; rowA < M; rowA +=2) {
        for(int colB = 0; colB < N; colB += 2){
        int sum = 0;
        for(int iter = 0; iter < N; iter++) 
        {
            sum += matA[rowA * N + iter] * matB[iter * N + colB];
            sum += matA[(rowA+1) * N + iter] * matB[iter * N + colB];
            sum += matA[rowA * N + iter] * matB[iter * N + (colB+1)];
            sum += matA[(rowA+1) * N + iter] * matB[iter * N + (colB+1)];
        }

        // compute output indices
        int rowC = rowA>>1;
        int colC = colB>>1;
        int indexC = rowC * (N>>1) + colC;
        output[indexC] = sum;
        }
    }
}

// Multithreaded implementation of RMM optimized with AVX
void* multiThread_avx(void *a)
{
    int start = ((struct arguments*) a)->start;
    int M = ((struct arguments*) a)->M;
    int N = ((struct arguments*) a)->N;
    int *matA = ((struct arguments*) a)->matA;
    int *matB = ((struct arguments*) a)->matB;
    int *output = ((struct arguments*) a)->output;

    __m256i vec_multi = _mm256_setzero_si256();
    __m256i vec_mat1r1 = _mm256_setzero_si256(); 
    __m256i vec_mat1r2 = _mm256_setzero_si256(); 
    __m256i vec_mat2c1 = _mm256_setzero_si256(); 
    __m256i vec_mat2c2 = _mm256_setzero_si256(); 

    //cout<<"allocated the memory"<<endl;

    for(int i = 0; i<N; i+=8)
    {
        for(int j = start; j<M; j+=2)
        {
        vec_mat2c1 = _mm256_setr_epi32(matB[i*N + j], matB[(i+1)*N + j],matB[(i+2)*N + j],matB[(i+3)*N + j], matB[(i+4)*N + j],matB[(i+5)*N + j],matB[(i+6)*N + j],matB[(i+7)*N + j]);
        vec_mat2c2 = _mm256_setr_epi32(matB[i*N + j + 1], matB[(i+1)*N + j + 1],matB[(i+2)*N + j + 1],matB[(i+3)*N + j + 1], matB[(i+4)*N + j + 1],matB[(i+5)*N + j + 1],matB[(i+6)*N + j + 1],matB[(i+7)*N + j + 1]);
        
        for(int k = 0; k<N; k+=2)
        {
            vec_mat1r1 = _mm256_loadu_si256((const __m256i_u*)&matA[k*N + i]);
            vec_mat1r2 = _mm256_loadu_si256((const __m256i_u*)&matA[(k+1)*N + i]);

            vec_multi = _mm256_add_epi32(vec_multi ,_mm256_mullo_epi32(vec_mat1r1, vec_mat2c1));//Multiplies the vectors
            vec_multi = _mm256_add_epi32(vec_multi ,_mm256_mullo_epi32(vec_mat1r1, vec_mat2c2));
            vec_multi = _mm256_add_epi32(vec_multi ,_mm256_mullo_epi32(vec_mat1r2, vec_mat2c1));
            vec_multi = _mm256_add_epi32(vec_multi ,_mm256_mullo_epi32(vec_mat1r2, vec_mat2c2));

            //pthread_mutex_lock(&lock);
            output[(k/2)*(N/2) + (j/2)] += _mm256_extract_epi32(vec_multi, 0) + _mm256_extract_epi32(vec_multi, 1) +_mm256_extract_epi32(vec_multi, 2) +_mm256_extract_epi32(vec_multi, 3) +_mm256_extract_epi32(vec_multi, 4) +_mm256_extract_epi32(vec_multi, 5) +_mm256_extract_epi32(vec_multi, 6) +_mm256_extract_epi32(vec_multi, 7);
            vec_multi = _mm256_setzero_si256();
            //pthread_mutex_unlock(&lock);
        }

        }
    }
    //cout<<"done executing"<<endl;
}



// Fill in this function
void multiThread(int N, int *matA, int *matB, int *output)
{   
    int size = N/MAX_THREADS;
    int thread_num = min(MAX_THREADS,N/2);
    pthread_t threads[thread_num];
    size = max(2,size);
    for(int i = 0; i<thread_num; i++)
    {
        arguments *a = (struct arguments*)malloc(sizeof(struct arguments));
        a->start = i*size;
        a->M = i*size + size;
        a->N = N;
        a->matA = matA;
        a->matB = matB;
        a->output = output;
        pthread_create(&threads[i],NULL,multiThread_avx,(void *)a);
    }

    for(int i = 0; i<thread_num; i++)
    {
        pthread_join(threads[i],NULL);
    }

    // for(int i = 0; i<N/2; i+=4)
    // {
    //     multiThread_2(i,i+4,N,matA,matB,output);
    // }

    // for(int i = N/2; i<N; i+=4)
    // {
    //     multiThread_2(i,i+4,N,matA,matB,output);
    // }
}


