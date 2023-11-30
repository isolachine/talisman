#include <stdio.h>


typedef struct {
  int info;
} pk_info_t;

typedef struct {
  pk_info_t* pk_info;
  int pk_b[5];
} pk_t;


void do_stuff (pk_t *ctx){

  if(ctx->pk_info == NULL) // Vulnerable Branch reported
    ;
  if(ctx->pk_b[1] == 3) // Vulnerable Branch reported
    ;
  if(ctx->pk_info->info == 0) // vulnerable branch
    ;
  if(ctx != NULL) // address not tainted
    ;
}

int main(void)
{
  pk_t pk; // tainted value


  do_stuff(&pk); // &pk is not tainted
  return 0;
}
