#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>

//global constants
#define ASCII_TO_PIXELS 35	//Each ascii character corresponds to 35 pixels on the map.

//function prototypes
bool is_barrier(char c);

int main(int argc, char *argv[])
{
	FILE *fp;
	char c;
	int numRows = 0;	//Default with the number of rows to 0.
	int numCols = 1;	//Start with 1 column, to account for the column of newlines.
	int i, j, k, l, r;

	/******************************************************************************
	 * Read in the ASCII map.
	 ******************************************************************************/

	//If we weren't given an ASCII map, just return;
	if(argc < 2) return 0;

	//Open the file.
	fp = fopen(argv[1], "r");

	//Count the number of columns.  
	while((c = fgetc(fp)) != '\n')
	{
		numCols++;
	}

	rewind(fp);

	//Count the number of rows.
	while((c = fgetc(fp)) != EOF)
	{
		if(c == '\n')
		{
			numRows++;
		}
	}

	rewind(fp);

	//Copy the ASCII map into a character matrix.
	char m[numRows][numCols];

	for(i = 0; i < numRows; i++)
	{
		for(j = 0; j < numCols; j++)
		{
			m[i][j] = fgetc(fp);
		}
	}

	//Close the file.
	fclose(fp);

	/******************************************************************************
	 * Count the number of marked points
	 ******************************************************************************/  

	int baseCount = 0;
  int fuelCount = 0;

	for(i = 0; i < numRows; i++)
	{
		for(j = 0; j < numCols - 1; j++)
		{
      // if is a base ( 1 - 9 )
			if(m[i][j] >= '1' && m[i][j] <= '9')
			{
				baseCount++;
			}
      // if is a fuel depot ( # )
      else if(m[i][j] == '#')
      {
        fuelCount++;
      }
		}
	}


	/******************************************************************************
	 * Convert points to pixels and write to file.
	 ******************************************************************************/  

	FILE *fptr = fopen("fuelpoints.csv", "w");

	fprintf(fptr, "%d\n", baseCount);
	fprintf(fptr, "%d\n", fuelCount);

	//Write the base points first
	for(i = 0; i < numRows; i++)
	{
		for(j = 0; j < numCols - 1; j++)
		{
			if(m[i][j] >= '1' && m[i][j] <= '9')
			{
				fprintf(fptr, "%d %d %d\n", m[i][j],  j * ASCII_TO_PIXELS, (numRows-1-i) * ASCII_TO_PIXELS);
			}
		}
	}

	//Now write the fuel points.
	for(i = 0; i < numRows; i++)
	{
		for(j = 0; j < numCols - 1; j++)
		{
			if(m[i][j] == '#')
			{
				fprintf(fptr, "%d %d\n", j * ASCII_TO_PIXELS, (numRows-1-i) * ASCII_TO_PIXELS);
			}
		}
	}

	fclose(fptr);
}

