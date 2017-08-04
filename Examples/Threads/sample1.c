//sample for coding a form of numerical integration 
//called the rectangle rule
//
//if you are interested to know more, look into some good
//college text on mathematics
//
//higher the value of n, better the accuracy 
//higher the value of n, higher computing overhead 
//higher the value of n, parallelization is justified 
//higher the value of n, multicore requirements are justified
//

double area, pi, x;

int i,n;

area = 0.0;

for(i=0; i<n; i++)
{
   
     x = (i+0.5)/n;
     area += 4.0/(1.0 + x*x);
}

pi = area /n; //we have the result 

