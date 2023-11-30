typedef unsigned int mpi_limb_t;
typedef mpi_limb_t *mpi_ptr_t;
struct gcry_mpi
{
  int alloced;         /* Array size (# of allocated limbs). */
  int nlimbs;          /* Number of valid limbs. */
  int sign;	       /* Indicates a negative number and is also used
		          for opaque MPIs to store the length.  */
  unsigned int flags; /* Bit 0: Array to be allocated in secure memory space.*/
                      /* Bit 2: The limb is a pointer to some m_alloced data.*/
                      /* Bit 4: Immutable MPI - the MPI may not be modified.  */
                      /* Bit 5: Constant MPI - the MPI will not be freed.  */
  mpi_limb_t *d;      /* Array with the limbs */
  mpi_limb_t *d1;      /* Array with the limbs */
};
typedef struct gcry_mpi *gcry_mpi_t;

static int SP8[64] = {
    0x10001040, 0x00001000, 0x00040000, 0x10041040, 0x10000000, 0x10001040,
    0x00000040, 0x10000000, 0x00040040, 0x10040000, 0x10041040, 0x00041000,
    0x10041000, 0x00041040, 0x00001000, 0x00000040, 0x10040000, 0x10000040,
    0x10001000, 0x00001040, 0x00041000, 0x00040040, 0x10040040, 0x10041000,
    0x00001040, 0x00000000, 0x00000000, 0x10040040, 0x10000040, 0x10001000,
    0x00041040, 0x00040000, 0x00041040, 0x00040000, 0x10041000, 0x00001000,
    0x00000040, 0x10040040, 0x00001000, 0x00041040, 0x10001000, 0x00000040,
    0x10000040, 0x10040000, 0x10040040, 0x10000000, 0x00040000, 0x10001040,
    0x00000000, 0x10041040, 0x00040040, 0x10000040, 0x10040000, 0x10001000,
    0x10001040, 0x00000000, 0x10041040, 0x00041000, 0x00041000, 0x00001040,
    0x00001040, 0x00040040, 0x10000000, 0x10041000};

static int PUB_ARR[16] = {0x10001040, 0x00001000, 0x00040000, 0x10041040,
                          0x10000000, 0x10001040, 0x00000040, 0x10000000,
                          0x00040040, 0x10040000, 0x10041040, 0x00041000,
                          0x10041000, 0x00041040, 0x00001000, 0x00000040};

int bar(mpi_limb_t* expo) {
  mpi_ptr_t d1 = expo[0];
  expo[0] = 1;
  expo[0] = 1;
  expo[0] = 1;
  expo[0] = 1;
}

void baz1(mpi_limb_t* expo);
int baz2(gcry_mpi_t expo);

void baz1(mpi_limb_t* expo) {
  bar(expo);
}

void foo(long sec, long pub, long val, gcry_mpi_t expo, int *ptr) {
  // bar(expo->d);
  // bar(expo->d);
  // bar(expo->d);
  // bar(expo->d);
  // bar(expo->d);
  // bar(expo->d);
  // baz1(expo->d);
  // missing seed example
  int x = baz2(expo);
  if (x)
    ;

  // mpi_ptr_t ep2 = expo->d;
  // int d0 = ep2[0];
  // int x0 = ptr[d0]; // Cache side-channel
  
  // int *p1 = ptr + 0;
  // int y = (int)p1 % 2;
  // int *p2 = p1 + 2;
  // int y1 = (int)p2 % 2;
  // int *p3 = p1 + y1;
  // int y2 = (int)p3 % 2;
  // int *p4 = p1 + y2;
  // int y3 = (int)p4 % 2;
  // int *p5 = p1 + y3;
  // int y4 = p5[pub];

  // if (p5 == 3)
  //   ;

  // if (p5 == 0)
  //   ;

  // if (p5 == y4) // side-channel
  //   ;
  
  // if (y3)
  //   ;
  
  // if (y4) // side-channel
  //   ;

  // int y = (int)ptr % 2;
  // int z = PUB_ARR[y];
  // int *ptr1 = (int *)z;
  // int y1 = (int)ptr1 % 2;
  // int z2 = PUB_ARR[y1];

  // void *q = (void *)ptr;
  // q = q + pub;

  // int *r = (int *)q + 5;
  // q = q + sec;  // Cache side-channel
  // *r = val;

  // // gcry_mpi_t expo1 = expo + 0;
  // // unsigned int flag = expo1->flags;
  // // mpi_ptr_t ep = expo1->d;
  // // mpi_ptr_t ep1 = expo1->d1;
  // mpi_ptr_t ep = expo->d;
  // mpi_ptr_t ep1 = expo->d1;
  // int j = ep[pub];
  // int j1 = ep1[pub];
  // int j2 = ep + sec; // Cache side-channel
  // if (j) // side-channel
  //   ;
  // if (j1)
  //   ;

  
  // int k = expo->d[pub];
  // int l = expo->d[sec]; // Cache side-channel
  
  // int *x = ptr;
  // int y  = x[pub];

  // int e = SP8[0];
  // int f = SP8[10];
  // int a = SP8[pub];
  // int b = SP8[pub + 5];
  // int c = SP8[sec];     // Cache side-channel
  // int d = SP8[sec + 5]; // Cache side-channel
  // int g = x[0];
  // int h = x[10];
  // int i = x[sec];       // Cache side-channel

  // SP8[0] = val;
  // SP8[10] = val;
  // SP8[pub] = val;
  // SP8[pub + 5] = val;
  // SP8[sec] = val;     // Cache side-channel
  // SP8[sec + 5] = val; // Cache side-channel

  // PUB_ARR[1] = val;
  // PUB_ARR[pub] = val;
  // PUB_ARR[sec] = val; // Cache side-channel
  // PUB_ARR[a] = val;   // Cache side-channel
  // PUB_ARR[y] = val;   // Cache side-channel

  // if(a)
  // ;
  // if(y)
  // ;
}

// void foo(long pub, int val) {
  // int array[10] = {0};
  // void *q = (void *)array;
  // // q = q + pub;

  // int y = ((int)q) % 2;
  // array[y] = val;

  // int *r = (int *)q + 5;
  // if (*r == 0) // vulnerable branch
  //   ;
  // *r = val;
  // return;
// }
