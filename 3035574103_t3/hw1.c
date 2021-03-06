#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <assert.h>

/* You may need to define struct here */
typedef struct _arg_t{
  int k;
  int i;
  size_t len;
  const float* vec;
} arg_t;

typedef struct _ret_t{
  double returnValue;
} ret_t;
/*!
 * \brief subroutine function
 *
 * \param arg, input arguments pointer
 * \return void*, return pointer
 */
void *l2_norm(void *arg) { /* TODO: Your code here */
  arg_t *p_arg = (arg_t*) arg;
  ret_t *r = (ret_t *) malloc(sizeof(ret_t));
  double sum = 0;
  int start = (p_arg-> len)/(p_arg -> k)*(p_arg -> i);
  int end = (p_arg-> len)/(p_arg -> k)*(p_arg -> i) + (p_arg-> len)/(p_arg -> k);
  for (int i = start; i < end; i++){
    sum += (p_arg->vec[i])*(p_arg->vec[i]);
  }
  r->returnValue = sum;
  return (void*)r;
}

/*!
 * \brief wrapper function
 *
 * \param vec, input vector array
 * \param len, length of vector
 * \param k, number of threads
 * \return float, l2 norm
 */
float multi_thread_l2_norm(const float *vec, size_t len, int k) { 
  /* TODO: your code here */
  double sum = 0.0f;
  pthread_t p[k];
  arg_t args[k];
  int rc;
  ret_t *re;
  for(int i = 0; i < k; i++){
    args[i].k = k;
    args[i].len = len;
    args[i].vec = vec;
    args[i].i = i;
    rc = pthread_create(&p[i], NULL, l2_norm, (void*)&args[i]);
    assert(rc == 0);
  }

  for(int i = 0; i < k; i ++){
    rc = pthread_join(p[i], (void**)&re);
    assert(rc == 0);
    sum += re->returnValue;
    free(re);
  }
  sum = sqrt(sum);
  return sum;
}

// baseline function
float single_thread_l2_norm(const float *vec, size_t len) {
  double sum = 0.0f;
  for (int i = 0; i < len; ++i) {
    sum += vec[i] * vec[i];
  }
  sum = sqrt(sum);
  return sum;
}

float *gen_array(size_t len) {
  float *p = (float *)malloc(len * sizeof(float));
  if (!p) {
    printf("failed to allocate %ld bytes memory!", len * sizeof(float));
    exit(1);
  }
  return p;
}

void init_array(float *p, size_t len) {
  for (int i = 0; i < len; ++i) {
    p[i] = (float)rand() / RAND_MAX;
  }
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr,
            "Error: The program accepts exact 2 intergers.\n The first is the "
            "vector size and the second is the number of threads.\n");
    exit(1);
  }
  const int num = atoi(argv[1]);
  const int k = atoi(argv[2]);
  if (num < 0 || k < 1) {
    fprintf(stderr, "Error: invalid arguments.\n");
    exit(1);
  }

  printf("Vector size=%d\tthreads num=%d.\n", num, k);

  float *vec = gen_array(num);
  init_array(vec, num);

  float your_result, groud_truth;
  // warmup
  single_thread_l2_norm(vec, num);

  {
    struct timespec start, end;
    printf("[Singlethreading]start\n");
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);

    groud_truth = single_thread_l2_norm(vec, num);

    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    float delta_us = (end.tv_sec - start.tv_sec) * 1.0e6 +
                     (end.tv_nsec - start.tv_nsec) * 1.0e-3;
    printf("[Singlethreading]The elapsed time is %.2f ms.\n",
           delta_us / 1000.0);
  }

  {
    struct timespec start, end;
    printf("[Multithreading]start\n");
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);

    your_result = multi_thread_l2_norm(vec, num, k);

    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    float delta_us = (end.tv_sec - start.tv_sec) * 1.0e6 +
                     (end.tv_nsec - start.tv_nsec) * 1.0e-3;
    printf("[Multithreading]The elapsed time is %.2f ms.\n", delta_us / 1000.0);
  }

  if (fabs(your_result - groud_truth) < 1e-6) {
    printf("Accepted!\n");
  } else {
    printf("Wrong Answer!\n");
    printf("your result=%.6f\tground truth=%.6f\n", your_result, groud_truth);
  }

  free(vec);
  return 0;
}
