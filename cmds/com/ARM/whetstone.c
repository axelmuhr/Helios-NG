#include <stdio.h>
#include <math.h>
#ifdef __STDC__
#include <time.h>
#else
typedef unsigned int time_t;
#include <sys/time.h>
#include <sys/resource.h>
#endif

typedef double REAL;

int J, K, L;
static REAL T, T1, T2, E1[5];

void PA(e)
REAL e[5];
{
  J=0;
loop:
  e[1] = (e[1]+e[2]+e[3]-e[4])*T;
  e[2] = (e[1]+e[2]-e[3]+e[4])*T;
  e[3] = (e[1]-e[2]+e[3]+e[4])*T;
  e[4] = (-e[1]+e[2]+e[3]+e[4])/T2;
  J=J+1;
  if ((J-6) < 0) goto loop;
}

void P0()
{
  E1[J] = E1[K];
  E1[K] = E1[L];
  E1[L] = E1[J];
}

void P3(x, y, z)
REAL x, y;
REAL *z;
{
  /* extern REAL T, T2; */
  REAL x1, y1;

  x1 = x;  y1 = y;
  x1 = T * (x1 + y1);
  y1 = T * (x1 + y1);
  *z = (x1 + y1) / T2;
}

void POUT(n, J, K, x1, x2, x3, x4)
int n, J, K;
REAL x1, x2, x3, x4;
{
  printf("%7d%7d%7d%12.4e%12.4e%12.4e%12.4e\n",
       n, J, K, x1, x2, x3, x4);
}

int main()
{
    REAL X1, X2, X3, X4, X, Y, Z;
    int I, ISAVE, J, K, L;
    int N1, N2, N3, N4, N5, N6, N7, N8, N9, N10, N11, N12;
    time_t time0;

    T=0.499975;
    T1=0.50025;
    T2=2.0;
setbuf(stdout,0);
    printf("\nC-Whetstone Benchmark (double precision)\n");
    printf("\nEach iteration is 100,000 Whetstones\n");
    printf("Number of iterations? : ");
    fflush(stdout);
    scanf("%d", &I);
    printf("\n");

#ifdef __STDC__
    time0 = clock();
#else
    { struct rusage r;
      getrusage(RUSAGE_SELF, &r);
      time0 = r.ru_utime.tv_sec*100 * r.ru_utime.tv_usec/10000;  /* cs */
    }
#endif
    ISAVE=I;

    N1=0;    N2=12*I;    N3=14*I;    N4=345*I;
    N5=0;    N6=210*I;   N7=32*I;    N8=899*I;
    N9=616*I;N10=0;      N11=93*I;   N12=0;

    X1=1.0;  X2 = -1.0;    X3 = -1.0;    X4 = -1.0;
    for (I = 1;  I <= N1;  ++I) {
        X1=(X1+X2+X3-X4)*T;
        X2=(X1+X2-X3+X4)*T;
        X3=(X1-X2+X3+X4)*T;
        X4=(-X1+X2+X3+X4)*T;
    }
    POUT(N1,N1,N1,X1,X2,X3,X4);

    E1[1]=1.0;   E1[2] = -1.0;    E1[3] = -1.0;    E1[4] = -1.0;
    for (I = 1;  I <= N2;  ++I) {
        E1[1]=(E1[1]+E1[2]+E1[3]-E1[4])*T;
        E1[2]=(E1[1]+E1[2]-E1[3]+E1[4])*T;
        E1[3]=(E1[1]-E1[2]+E1[3]+E1[4])*T;
        E1[4]=(-E1[1]+E1[2]+E1[3]+E1[4])*T;
    }
    POUT(N2,N3,N2,E1[1],E1[2],E1[3],E1[4]);

    for (I = 1;  I <= N3;  ++I) PA(E1);
    POUT(N3,N2,N2,E1[1],E1[2],E1[3],E1[4]);

    J=1;
    for (I = 1;  I <= N4;  ++I) {
        if ((J-1) == 0) J = 2;  else J = 3;
        if ((J-2) <= 0) J = 1;  else J = 0;
        if ((J-1) <  0) J = 1;  else J = 0;
    }
    POUT(N4,J,J,X1,X2,X3,X4);

    J=1;    K=2;    L=3;
    for (I = 1;  I <= N6;  ++I) {
        J=J*(K-J)*(L-K);
        K=L*K-(L-J)*K;
        L=(L-K)*(K+J);
        E1[L-1]=J+K+L;
        E1[K-1]=J*K*L;
    }
    POUT(N6,J,K,E1[1],E1[2],E1[3],E1[4]);

    X=0.5;    Y=0.5;
    for (I = 1;  I <= N7;  ++I) {
        X=T*atan(T2*sin(X)*cos(X)/(cos(X+Y)+cos(X-Y)-1.0));
        Y=T*atan(T2*sin(Y)*cos(Y)/(cos(X+Y)+cos(X-Y)-1.0));
    }
    POUT(N7,J,K,X,X,Y,Y);

    X=1.0;    Y=1.0;    Z=1.0;
    for (I = 1;  I <= N8;  ++I) P3(X,Y,&Z);
    POUT(N8,J,K,X,Y,Z,Z);

    J=1;    K=2;    L=3;
    E1[1]=1.0;    E1[2]=2.0;    E1[3]=3.0;

    for (I = 1;  I <= N9;  ++I) P0();
    POUT(N9,J,K,E1[1],E1[2],E1[3],E1[4]);

    J=2;    K=3;
    for (I = 1;  I <= N10;  ++I) {
        J=J+K;    K=J+K;    J=J-K;    K=K-J-J;
    }
    POUT(N10,J,K,X1,X2,X3,X4);

    X=0.75;
    for (I = 1;  I <= N11;  ++I) {
       X=sqrt(exp(log(X)/T1));
    }
    POUT (N11,J,K,X,X,X,X);

#ifdef __STDC__
    time0 = ((clock() - time0) * 100) / CLK_TCK;  /* cs */
#else
    { struct rusage r;
      getrusage(RUSAGE_SELF, &r);
      time0 = r.ru_utime.tv_sec*100 + r.ru_utime.tv_usec/10000 - time0;
    }
#endif
    if (ISAVE >= 10) {
      printf("\nTime for %u,%.1u00,000 whetstones is %u.%.2u seconds\n",
             ISAVE/10, ISAVE %10, time0/100, time0 % 100);
    } else {
      printf("\nTime for %u00,000 whetstones is %u.%.2u seconds\n",
             ISAVE, time0/100, time0 % 100);
    }
    I =  (ISAVE*100000/time0)*100;
    printf("(%u,%.3u Whetstones per second)\n\n", I/1000, I % 1000);

    return 0;
}






