/*
 * Fibonacci application, version 1
 * Sistemas Operativos, DEI/IST/ULisboa 2015-16
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#define NUM_DUMMY_ITERATIONS 100000

int main (int argc, char** argv) {

  unsigned long n, first = 0, second = 1, next, c, caux;

  if (argc != 2) { 
	  printf("Incorrect arguments: fibonacci number_of_terms\n");
	  exit(EXIT_FAILURE);
  }
  	

   n = atoi(argv[1]);

   printf("Process %d started to generate Fibonacci up to %lu.\n", (int) getpid(), n);
 
   for ( c = 0 ; c < n ; c++ )
   {
     for (caux = 0; caux < NUM_DUMMY_ITERATIONS; caux ++) ;

      if ( c <= 1 )
         next = c;
      else
      {
         next = first + second;
         first = second;
         second = next;
      }
   }

   printf("Process %d finished Fibonacci series at value: %lu.\n", (int) getpid(), next);
 
   exit(EXIT_SUCCESS);
}
