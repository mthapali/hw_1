/* 
    Please include compiler name below (you may also include any other modules you would like to be loaded)

COMPILER= gnu

    Please include All compiler flags and libraries as you want them run. You can simply copy this over from the Makefile's first few lines
 
CC = cc
OPT = -O3
CFLAGS = -Wall -std=gnu99 $(OPT)
MKLROOT = /opt/intel/composer_xe_2013.1.117/mkl
LDLIBS = -lrt -Wl,--start-group $(MKLROOT)/lib/intel64/libmkl_intel_lp64.a $(MKLROOT)/lib/intel64/libmkl_sequential.a $(MKLROOT)/lib/intel64/libmkl_core.a -Wl,--end-group -lpthread -lm

*/

// #include "immintrin.h"   

#include <immintrin.h>  //added

const char* dgemm_desc = "Simple blocked dgemm.";

#if !defined(BLOCK_SIZE)
#define BLOCK_SIZE 64
#endif

#define min(a,b) (((a)<(b))?(a):(b))

/* This auxiliary subroutine performs a smaller dgemm operation
 *  C := C + A * B
 * where C is M-by-N, A is M-by-K, and B is K-by-N. */
static void do_block (int lda, int M, int N, int K, double* A, double* B, double* C)
{
  double smallA[BLOCK_SIZE*BLOCK_SIZE] __attribute__ ((aligned (32)));

      for( int i = 0; i < M; i++ )
        for( int j = 0; j < K; j++ )
            smallA[j+i*BLOCK_SIZE] = A[i+j*lda];

  /* For each row i of A */
  for (int i = 0; i < M; ++i)
    /* For each column j of B */ 
    for (int j = 0; j < N; ++j) 
    {
      /* Compute C(i,j) */
      double cij = C[i+j*lda];
      for (int k = 0; k < K; ++k)
        cij += smallA[k+i*BLOCK_SIZE] * B[k+j*lda];
     C[i+j*lda] = cij;
   }
 }

 static void do_block_fast(int lda, int M, int N, int K, double* A, double* B, double* C)
 {
  static double a[BLOCK_SIZE*BLOCK_SIZE] __attribute__ ((aligned (32)));  //change to 32
  static double temp[4] __attribute__ ((aligned (32)));  //added

   //added
    // __m256d vecA1;
    // __m256d vecB1;
    // __m256d vecC1;
    // __m256d vecA2;
    // __m256d vecB2;
    // __m256d vecC2;
    // __m256d vecC0tmp;

      //  make a local aligned copy of A's block
  for( int j = 0; j < K; j++ ) 
    for( int i = 0; i < M; i++ )
      a[i+j*BLOCK_SIZE] = A[i+j*lda];


  /* For each row i of A */
    for (int i = 0; i < M; ++i){
    /* For each column j of B */ 
      for (int j = 0; j < N; ++j) 
      {
      /* Compute C(i,j) */
        double cij = C[i+j*lda];
        for (int k = 0; k < K; k +=8)   //change ++k to +=8

          //added
          {

          //       vecA1 = _mm256_load_pd (&a[k+i*BLOCK_SIZE]);
          //       vecB1 = _mm256_loadu_pd (&B[k+j*lda]);
          //       vecA2 = _mm256_load_pd (&a[k+4+i*BLOCK_SIZE]);
          //       vecB2 = _mm256_loadu_pd (&B[k+4+j*lda]);
          //       vecC1 = _mm256_mul_pd(vecA1, vecB1);
          //       vecC2 = _mm256_mul_pd(vecA2, vecB2);
          //       vecC0tmp = _mm256_add_pd(vecC1,vecC2);

          //       _mm256_store_pd(&temp[0], vecC0tmp);

          
          // cij += temp[0];
          // cij += temp[1];
          // cij += temp[2];
          // cij += temp[3];
        

             cij += a[i+k*BLOCK_SIZE] * B[k+j*lda];
         }
        C[i+j*lda] = cij;
      }
    }
}

/* This routine performs a dgemm operation
 *  C := C + A * B
 * where A, B, and C are lda-by-lda matrices stored in column-major format. 
 * On exit, A and B maintain their input values. */  
    void square_dgemm (int lda, double* A, double* B, double* C)
    {
  /* For each block-row of A */ 
      for (int i = 0; i < lda; i += BLOCK_SIZE)
    /* For each block-column of B */
        for (int j = 0; j < lda; j += BLOCK_SIZE)
      /* Accumulate block dgemms into block of C */
          for (int k = 0; k < lda; k += BLOCK_SIZE)
          {
	/* Correct block dimensions if block "goes off edge of" the matrix */
           int M = min (BLOCK_SIZE, lda-i);
           int N = min (BLOCK_SIZE, lda-j);
           int K = min (BLOCK_SIZE, lda-k);

	/* Perform individual block dgemm */
           if((M % BLOCK_SIZE == 0) && (N % BLOCK_SIZE == 0) && (K % BLOCK_SIZE == 0)){
            do_block_fast(lda, M, N, K, A + i + k*lda, B + k + j*lda, C + i + j*lda);
          }else{
    /* Perform individual block dgemm */
            do_block(lda, M, N, K, A + i + k*lda, B + k + j*lda, C + i + j*lda);
          }
        }
      }
