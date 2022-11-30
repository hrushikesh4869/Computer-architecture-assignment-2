// Optimize this function

#include<immintrin.h>


void single_thread_avx(int N, int *matA, int *matB, int *output)
{
  __m256i vec_multi = _mm256_setzero_si256(); 
  __m256i vec_mat1r1 = _mm256_setzero_si256(); 
  __m256i vec_mat1r2 = _mm256_setzero_si256(); 
  __m256i vec_mat2c1 = _mm256_setzero_si256(); 
  __m256i vec_mat2c2 = _mm256_setzero_si256(); 

  //cout<<"allocated the memory"<<endl;

  for(int i = 0; i<N; i+=8)
  {
    for(int j = 0; j<N; j+=2)
    {
      vec_mat2c1 = _mm256_setr_epi32(matB[i*N + j], matB[(i+1)*N + j],matB[(i+2)*N + j],matB[(i+3)*N + j], matB[(i+4)*N + j],matB[(i+5)*N + j],matB[(i+6)*N + j],matB[(i+7)*N + j]);
      vec_mat2c2 = _mm256_setr_epi32(matB[i*N + j + 1], matB[(i+1)*N + j + 1],matB[(i+2)*N + j + 1],matB[(i+3)*N + j + 1], matB[(i+4)*N + j + 1],matB[(i+5)*N + j + 1],matB[(i+6)*N + j + 1],matB[(i+7)*N + j + 1]);
      
      for(int k = 0; k<N; k+=2)
      {
        vec_mat1r1 = _mm256_loadu_si256((const __m256i_u*)&matA[k*N + i]);
        vec_mat1r2 = _mm256_loadu_si256((const __m256i_u*)&matA[(k+1)*N + i]);

        vec_multi = _mm256_add_epi32(vec_multi ,_mm256_mullo_epi32(vec_mat1r1, vec_mat2c1));
        vec_multi = _mm256_add_epi32(vec_multi ,_mm256_mullo_epi32(vec_mat1r1, vec_mat2c2));
        vec_multi = _mm256_add_epi32(vec_multi ,_mm256_mullo_epi32(vec_mat1r2, vec_mat2c1));
        vec_multi = _mm256_add_epi32(vec_multi ,_mm256_mullo_epi32(vec_mat1r2, vec_mat2c2));

        output[(k/2)*(N/2) + (j/2)] += _mm256_extract_epi32(vec_multi, 0) + _mm256_extract_epi32(vec_multi, 1) +_mm256_extract_epi32(vec_multi, 2) +_mm256_extract_epi32(vec_multi, 3) +_mm256_extract_epi32(vec_multi, 4) +_mm256_extract_epi32(vec_multi, 5) +_mm256_extract_epi32(vec_multi, 6) +_mm256_extract_epi32(vec_multi, 7);
        vec_multi = _mm256_setzero_si256();
      }

    }
  }
  //cout<<"done executing"<<endl;
}

// alternate implementation of RMM

void singleThread_add(int N, int *matA, int *matB, int *output)
{
  int *output_reference = new int[(N)*(N)];
  for(int i = 0; i<N; i++)
  {
    for(int j = 0; j<N; j++)
    {
      output_reference[i*N + j] = 0;
      for(int k = 0; k<N; k++)
      {
        output_reference[i*N + j] += matA[i*N + k]*matB[k*N + j];
      }
    }
  }
  for(int i = 0; i<N/2; i++)
  {
    for(int j = 0; j<N/2; j++)
    {
      output[i*(N/2) + j] = output_reference[(i*2)*N + (j*2)] + output_reference[(i*2 + 1)*N + (j*2)] + output_reference[(i*2)*N + (j*2) + 1] + output_reference[(i*2 + 1)*N + (j*2) + 1];
    }
  }
}



void singleThread(int N, int *matA, int *matB, int *output)
{
  single_thread_avx(N,matA,matB,output);
  //singleThread_add(N,matA,matB,output);
}






