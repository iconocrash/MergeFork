/***************************************
James Jilek
MergeFork
Created: Sept. 28th, 2007
****************************************/

#define TRUE	1
#define FALSE	0

/* process type, used in testing code */
#define TYPE_ROOT_PROC		0
#define TYPE_LEFT_CHILD		1
#define TYPE_RIGHT_CHILD	2

#include <unistd.h>
#include <sys/types.h>

#include <stdio.h>

/* store array of ints parsed from the datfile into here */
int numArray[8];
int numCount;

/* function to read the data from the input datfile
   and place in the integer array */
void ParseInputFile(char* filename)
{
	char c;
	int index;
	int value;
	FILE* file;
	char numBuf[10]; /* numbers cannot be larger than 10 digits */
	
	/* try to open file, exit if fopen fails */
	if (( file = fopen(filename, "r") ) == NULL)
	{
		printf("Error: cannot open file.");
		exit(1);
	}
	
	/* the list of integers is formatted as a line of space delimited
	   numbers, so read each char until we hit a newline */
	
	index = 0;
	numCount = 0;
	while(TRUE)
	{
		c = fgetc(file);
		
		/* if the char is a space or newline, then we're done with the
		   current number */
		if (c == ' ' || c == '\n')
		{
			/* add null char to end string */
			numBuf[index] = '\0';
			
			/* get the integer value of integer string */
			value = atoi(numBuf);
			
			/* add the number to the list */
			numArray[numCount] = value;
			
			/* done with number */
			numCount++;
			
			if (c == '\n') /* if c is a newlne then we are done with the file */
				break;
			
			/* get the next number */
			index = 0;
			continue;
		}
		
		/* copy the char digit of our current number into
		   numBuf and increment the index to get the next digit */
		numBuf[index] = c;
		index++;
	}
	
	fclose(file);
}

/* print an int array of numbers */
void PrintNumbers(int* a, int size)
{
	int i;
	for (i = 0; i < size; i++)
		printf("%d ", a[i]);
}

/* print the numbers parsed from our datfile with formatting */
void PrintInputNumbers()
{
	printf("Input data: ");
	PrintNumbers(numArray, numCount);
	printf("\n\n");
}

/* like above but called after sort */
void PrintSortedNumbers()
{
	printf("\nSorted data: ");
	PrintNumbers(numArray, numCount);
	printf("\n\n");
}

/* merge two sub sorted arrays into a single sorted array. the arrays
cannot overlap! */
void Merge(int* array, int size,
	int* leftArray, int leftSize,
	int* rightArray, int rightSize)
{
	int i = 0; /* iterator for left array */
	int j = 0; /* iterator for right array */
	int k = 0; /* iterator for merged array */
	
	/* ASSERTION: Make sure leftSize and rightSize add up
	to size otherwise something is messed up. */
	/*
	if ( size != leftSize + rightSize )
	{
		printf("Error: Assertion of \"size == leftSize + rightSize\" failed in Merge().");
		exit(1);
	}*/

	/* ASSERTION: Check some overlapping cases */
	/*
	if ( array == leftArray )
	{
		printf("Merge(): Error! Left overlap!\n");
		exit(1);
	}
	if ( array == rightArray )
	{
		printf("Merge(): Error! Right overlap!\n");
		exit(1);
	}*/
	
	for ( ; ; k++)
	{
		/* if the current element in the left array is less
		than the current element in the right array then place
		the current left element as next item in our merged array */
		if (leftArray[i] < rightArray[j])
		{
			array[k] = leftArray[i];
			i++;
			if (i == leftSize)
			{
				/* left side is done so just add the rest
				of the right array */
				k++; /* move up to next element to place in */
				for( ; k < size ; k++, j++)
				{
					array[k] = rightArray[j];
				}
				break; // exit loop and Merge()
			}
			continue;
		}
		else /* current right element is less than or equal to left */
		{
			array[k] = rightArray[j];
			j++;
			if (j == rightSize)
			{
				/* right side is done so just add the rest
				of the left array */
				k++; /* move up to next element to place in */
				for( ; k < size ; k++, i++)
				{
					array[k] = leftArray[i];
				}
				break; // exit loop and Merge()
			}
			continue;
		}
	}
}

/******************************************************
Function:

  void MergeFork(int* array, int size);

  [input] int *array: Pointer to array to be sorted.
  [input] int size: Size of array to be sorted.
  [output] int* array: Input array will be sorted.
  [returns] void

Description:

  MergeFork will implement a multiprocess MergeSort
  using fork() and Unix pipes for interprocess
  communication (IPC). A maximum input size of 8 is
  assumed for the array.
  
*******************************************************/

void MergeFork(int* array, int size)
{
	pid_t pid;
	
	int level;
	/*int type;*/
	
	int leftArray[4];
	int leftSize;
	int rightArray[4];
	int rightSize;
	
	int fdLeft[2];
	int fdRight[2];
	int fd[2];
	
	int mid;
	
	level = 1;
	/*type = TYPE_ROOT_PROC;*/
	
	while (TRUE)
	{

		/*
		printf("Level[%d], PID[%d]: Got array = ", level, (int)getpid());
		PrintNumbers(array, size);
		printf(", size = %d", size);
		if (type == TYPE_ROOT_PROC)
			printf(", type = RootProc");
		if (type == TYPE_LEFT_CHILD)
			printf(", type = LeftChild");
		if (type == TYPE_RIGHT_CHILD)
			printf(", type = RightChild");
		printf("\n");
		*/
		
		if (size == 1)
		{
			/* a single element doesn't need to be
			sorted so break out of loop and message the element 
			back to the parent */
			break;
		}
		if (size == 2)
		{
			/* see if the two elements need to
			be sorted */
			if (array[0] > array[1])
			{
				/* swap, borrow "mid" for
				our swap variable */
				mid = array[0];
				array[0] = array[1];
				array[1] = mid;
			}
			
			/* exit loop and then message 
			sorted results to parent */
			break;
		}
		
		/* SPLIT */
		
		printf("Level[%d], PID[%d]: split\n", level, (int)getpid());
		
		/* make left subarray */
		mid = (size - 1) / 2;
		leftSize = mid + 1;
		memcpy(leftArray, array, sizeof(int) * leftSize);
		
		/* make right subarray */
		mid = mid + 1;
		rightSize = size - leftSize;
		memcpy(rightArray, array + mid, sizeof(int) * rightSize);
		
		/* FORKING */
		
		/* set up pipes */
		
		if ( pipe(fdLeft) == -1 )
		{
			printf("Error creating left pipe.");
			exit(1);
		}
		
		if ( pipe(fdRight) == -1 )
		{
			printf("Error creating right pipe.");
			exit(1);
		}
		
		/* FORK LEFT */
		
		printf("Level[%d], PID[%d]: fork (left)\n", level, (int)getpid());
		
		pid = fork();
		if (pid == -1)
		{
			printf("Error creating left fork.");
			exit(1);
		}
		if (pid == 0) /* child proc */
		{
			level++;
			/*type = TYPE_LEFT_CHILD;*/
			
			/* close the right pipe file descriptors since the
			left fork doesn't need it */
			close(fdRight[0]);
			close(fdRight[1]);
			
			/* close reading for left pipe by the child 
			(it will only be sending data back to parent) */
			close(fdLeft[0]);
			
			/* copy fdLeft file descriptors into fd since fdLeft
			will be overwritten when we loop around and we will need
			the write file descriptor to message back to the parent
			*/
			fd[0] = fdLeft[0];
			fd[1] = fdLeft[1];
			
			/* set the size and array for the fork-loop-around recursion */
			size = leftSize;
			memcpy(array, leftArray, sizeof(int) * leftSize);
			
			continue; /* restart loop */
		}
		
		/* parent process continues here */
		close(fdLeft[1]); /* close write end */
		
		/* FORK RIGHT */
		
		printf("Level[%d], PID[%d]: fork (right)\n", level, (int)getpid());
		
		pid = fork();
		if (pid == -1)
		{
			printf("Error creating right fork.");
			exit(1);
		}
		if (pid == 0) /* child proc */
		{
			level++;
			/*type = TYPE_RIGHT_CHILD;*/
			
			/* close the left pipe file descriptors since the
			right fork doesn't need it */
			close(fdLeft[0]);
			close(fdLeft[1]);
			
			/* close reading for right pipe by the child 
			(it will only be sending data back to parent) */
			close(fdRight[0]);
			
			/* copy fdRight file descriptors into fd since fdRight
			will be overwritten when we loop around and we will need
			the write file descriptor to message back to the parent
			*/
			fd[0] = fdRight[0];
			fd[1] = fdRight[1];
			
			/* set the size and array for the fork-loop-around recursion */
			size = rightSize;
			memcpy(array, rightArray, sizeof(int) * rightSize);
			
			continue; /* restart loop */
		}
		
		/* parent process continues here */
		close(fdLeft[1]); /* close write end */
		
		/* READ PIPE FROM CHILREN */
		
		/* read should block until data is placed in the buffer */
		printf("Level[%d], PID[%d]: pipe (read left)\n", level, (int)getpid());
		read(fdLeft[0], leftArray, sizeof(int) * leftSize);
		printf("Level[%d], PID[%d]: pipe (read right)\n", level, (int)getpid());
		read(fdRight[0], rightArray, sizeof(int) * rightSize);
		
		/* MERGE */
		
		printf("Level[%d], PID[%d]: merge\n", level, (int)getpid());
		
		/* after we have read in the sorted children merge the arrays */
		Merge(array, size, leftArray, leftSize, rightArray, rightSize);
		
		break; /* done with loop, all that's left to do is
			message the data back to the parent */
	}
	
	if (level == 1) /* if we are the top-level proc */
	{
		/* top-level proc doesn't need to write to pipe for parent and
		returns instead of exit()s */
		return; 
	}
	
	/* WRITE TO PIPE (MESSAGE SORTED CHILD ARRAY TO PARENT) AND TERMINATE */
	
	printf("Level[%d], PID[%d]: pipe (write)\n", level, (int)getpid());
	write(fd[1], array, sizeof(int) * size);
	
	exit(0); /* successful termination */
}

int main()
{	
	/* print name and UIN */
	printf("\nJames Jilek - 675523434\nCS 385 - Program #1\n\n");
	
	/* read the numbers from input.dat in to an array */
	ParseInputFile("input.dat");
	
	/* print the numbers from the array */
	PrintInputNumbers();
	
	/* sort the array using our multi-process MergeSort implentation */
	MergeFork(numArray, numCount);

	/* print again after sorting */
	PrintSortedNumbers();
	
  	return 0;
}

