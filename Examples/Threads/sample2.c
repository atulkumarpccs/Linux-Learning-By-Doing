#include <math.h>
#include <stdlib.h>

void func1(double*array,int n)
{
  for (int i=1; i<n; i++)
  {
     array[i]+=array[i-1];
  }
}

void func2(double *array,int n)
{
  for(int i=0; i<n; i++)
  {
    array[i]=sin(array[i]);
  }
}

int main()
{
  double * array=calloc(sizeof(double),1024*1024);
  for (int i=0; i<100; i++)
  {
    func1(array,1024*1024);
    func2(array,1024*1024);
  }
  return 0;
}
