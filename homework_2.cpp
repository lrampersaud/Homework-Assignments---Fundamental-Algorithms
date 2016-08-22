/* Name: Lalchandra Rampersaud 
 * Course: I96 (Fundamental algorithms)
 * Homework #2*/
 


#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdio.h>



int rand_lp(int aNumDimensions, int aNumConstraints,double* A, double* B, double* C, double *aResult);
 
bool SatisfiesInequality(int aNumDimensions,double* A, double B, double* result);


int rand_lp(int aNumDimensions,int aNumConstraints,double* A,double* B,double* C,double *aResult) 
 {
	int recomputeCount = 0;
	
	 
	for (int i = 0; i < aNumDimensions; i++) 
	{
		 aResult[i] = 1000;
	}
	 
	if (aNumDimensions == 1) 
	{
	
		bool maxConstrained = false;
		double max;
		
		for (int i = 0; i < aNumConstraints; i++) 
		{
			 if (A[i] > 0 && !abs(A[i] - 0.0)< 0.00001) 
			 {
				 if ((!maxConstrained) || (SatisfiesInequality(1,&A[i], B[i], &max))) 
				 {
					 maxConstrained = true;
					 max = (B[i] / A[i]);
				 }
				
			 }
		 }

		if (maxConstrained) 
			aResult[0] = max;

	 } 
	 else 
	 {
		 int* randomNumbers = new int[aNumConstraints];

		 for (int i = 0; i < aNumConstraints; i++) 
			randomNumbers[i] = i;
		
		int temp=0;
		for (int i = 0; i < aNumConstraints; i++) 
		{
			 int offset = rand() % (aNumConstraints - i);
			 
			 temp = randomNumbers[i];
			 randomNumbers[i]=randomNumbers[i+offset];
			 randomNumbers[i+offset]=temp;

		 }
		 
		 
		 for (int i = 0; i < aNumConstraints; i++) 
		 {

			 double* currentConstraint = &A[randomNumbers[i] * aNumDimensions];
			 double currentB = B[randomNumbers[i]];
			 
			 if (SatisfiesInequality(aNumDimensions, currentConstraint, currentB, aResult)) 
			 {
				
				int dimensionToRemove = -1;
				for (int ia = 0; ia < aNumDimensions; ia++) 
				{
					 if (!abs(currentConstraint[ia] - 0.0)<=0.000001 && ((dimensionToRemove == -1) ||(abs(currentConstraint[ia]) > abs(currentConstraint[dimensionToRemove])))) 
					 {
						dimensionToRemove = ia;
					 }
				 }
				
				

				double* tempResults = new double[aNumDimensions-1];
				double* tempObjectives = new double[aNumDimensions-1];

				double* tempA = NULL;
				double* tempB = NULL;
				if (i > 0) 
				{
					 tempA = new double[(aNumDimensions-1) * i];
					 tempB = new double[i];
				}
			
				
				for (int ia = 0; ia < i; ia++) 
				{
					 double* oldA = &A[randomNumbers[ia]*(aNumDimensions)];
					 double* newA = &tempA[ia*(aNumDimensions-1)];
					 
					 for (int ja = 0; ja < dimensionToRemove; ja++) 
					 {
						 newA[ja] = oldA[ja] - oldA[dimensionToRemove] * (currentConstraint[ja]/currentConstraint[dimensionToRemove]);
					 }
					 for (int ja = dimensionToRemove+1; ja < aNumDimensions; ja++) 
					 {
						 newA[ja-1] = oldA[ja] - oldA[dimensionToRemove] * (currentConstraint[ja]/currentConstraint[dimensionToRemove]);
					 }
					 
					 tempB[ia] = B[randomNumbers[ia]] - (oldA[dimensionToRemove]*currentB/currentConstraint[dimensionToRemove]);
				 }
				
				
				for (int aj = 0; aj < dimensionToRemove; aj++) 
				{
					tempObjectives[aj] = C[aj] - (C[dimensionToRemove] * currentConstraint[aj]/currentConstraint[dimensionToRemove]);
				}
				for (int aj = dimensionToRemove + 1; aj < aNumDimensions; aj++) 
				{
					tempObjectives[aj-1] = C[aj] - (C[dimensionToRemove] * currentConstraint[aj]/currentConstraint[dimensionToRemove]);
				}
				
				
				rand_lp(aNumDimensions-1,i,tempA,tempB, tempObjectives,tempResults);
				
				
				
				aResult[dimensionToRemove] = currentB / currentConstraint[dimensionToRemove];
				
				for (int ia = 0; ia < dimensionToRemove; ia++) 
				{
					 aResult[ia] = tempResults[ia];
					 aResult[dimensionToRemove] -= (tempResults[ia] * currentConstraint[ia]) /	currentConstraint[dimensionToRemove];
				 }
				for (int ia = dimensionToRemove + 1; ia < aNumDimensions; ia++) 
				{
					 aResult[ia] = tempResults[ia-1];
					 aResult[dimensionToRemove] -= (tempResults[ia-1] * currentConstraint[ia]) /currentConstraint[dimensionToRemove];
				 }
				
				recomputeCount++;
				
				
				delete[] tempResults;
				delete[] tempObjectives;
				delete[] tempA;
				delete[] tempB;
			 }
			 
			 
			 
			 
			 
		 }
		 delete[] randomNumbers;
	 }

	return recomputeCount;
}




bool SatisfiesInequality(int aNumDimensions, double* A, double B, double* result) 
{
	if(aNumDimensions >0)
	{
		double t = 0;
		 for (int i = 0; i < aNumDimensions; i++) 
		 {
			t = t + A[i] *result[i];
		 }
		return (B <= t);
	}
	else
	{
		return false;
	}
}



 
 int main(int argc, char* argv[]) 
 {
	int seed = 54646468;
	seed = time(0);
	srand(seed);
	
	
   double A[600000][4],  b[600000], c[4] ;
   double result[4];
   int i, j; 



   printf("Preparing test: 4 variables, 600000 inequalities\n");
   A[0][0] = 1.0; A[0][1] = 2.0; A[0][2] = 1.0; A[0][3] = 0.0; b[0] = 10000.0;
   A[1][0] = 0.0; A[1][1] = 1.0; A[1][2] = 2.0; A[1][3] = 1.0; b[1] = 10000.0;
   A[2][0] = 1.0; A[2][1] = 0.0; A[2][2] = 1.0; A[2][3] = 3.0; b[2] = 10000.0;
   A[3][0] = 4.0; A[3][1] = 0.0; A[3][2] = 1.0; A[3][3] = 1.0; b[3] = 10000.0;
   c[0]=1.0; c[1]=1.0; c[2]=1.0; c[3]=1.0;
   for( i=4; i< 100000; i++ )
   {  A[i][0] = (12123*i)%104729; 
      A[i][1] = (47*i)%104729; 
      A[i][2] = (2011*i)%104729; 
      A[i][3] = (7919*i)%104729;
      b[i] = A[i][0] + 2*A[i][1] + 3*A[i][2] + 4* A[i][3] + 1 + (i%137);
   }
   A[100000][0] = 0.0; A[100000][1] = 6.0; A[100000][2] = 1.0; 
   A[100000][3] = 1.0; b[100000] = 19.0;
   for( i=100001; i< 200000; i++ )
   {  A[i][0] = (2323*i)%101111; 
      A[i][1] = (74*i)%101111; 
      A[i][2] = (2017*i)%101111; 
      A[i][3] = (7915*i)%101111;
      b[i] = A[i][0] + 2*A[i][1] + 3*A[i][2] + 4* A[i][3] + 2 + (i%89);
   }
   A[200000][0] = 5.0; A[200000][1] = 2.0; A[200000][2] = 0.0; 
   A[200000][3] = 1.0; b[200000] = 13.0;
   for( i=200001; i< 300000; i++ )
   {  A[i][0] = (23123*i)%100003; 
      A[i][1] = (47*i)%100003; 
      A[i][2] = (2011*i)%100003; 
      A[i][3] = (7919*i)%100003;
      b[i] = A[i][0] + 2*A[i][1] + 3*A[i][2] + 4* A[i][3] + 2 + (i%57);
   }
   A[300000][0] = 1.0; A[300000][1] = 2.0; A[300000][2] = 1.0; 
   A[300000][3] = 3.0; b[300000] = 20.0;
   A[300001][0] = 1.0; A[300001][1] = 0.0; A[300001][2] = 5.0; 
   A[300001][3] = 4.0; b[300001] = 32.0;
   A[300002][0] = 7.0; A[300002][1] = 1.0; A[300002][2] = 1.0; 
   A[300002][3] = 7.0; b[300002] = 40.0;
   for( i=300003; i< 400000; i++ )
   {  A[i][0] = (13*i)%103087; 
      A[i][1] = (99*i)%103087; 
      A[i][2] = (2012*i)%103087; 
      A[i][3] = (666*i)%103087;
      b[i] = A[i][0] + 2*A[i][1] + 3*A[i][2] + 4* A[i][3] + 1;
   }
   for( i=400000; i< 500000; i++ )
   {  A[i][0] = 1; 
      A[i][1] = (17*i)%999983; 
      A[i][2] = (1967*i)%444443; 
      A[i][3] = 2;
      b[i] = A[i][0] + 2*A[i][1] + 3*A[i][2] + 4* A[i][3] + (1000000.0/(double)i);
   }
   for( i=500000; i< 600000; i++ )
   {  A[i][0] = (3*i)%111121; 
      A[i][1] = (2*i)%999199; 
      A[i][2] = (2*i)%444443; 
      A[i][3] = i;
      b[i] = A[i][0] + 2*A[i][1] + 3*A[i][2] + 4* A[i][3] + 1.3;
   }
   
   printf("Running test: 600000 inequalities, 4 variables\n");
   j = rand_lp(4, 600000, &(A[0][0]), &(b[0]), &(c[0]), &(result[0]));
   printf("Test: extremal point (%f, %f, %f, %f) after %d recomputation steps\n", 
          result[0], result[1], result[2], result[3], j);
   printf("Answer should be (1,2,3,4)\n End Test\n");

} 
 