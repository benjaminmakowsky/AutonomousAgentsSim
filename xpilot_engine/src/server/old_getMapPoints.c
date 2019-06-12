#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>

#define NUM_BARRIERS 5
char barriers[NUM_BARRIERS] = {'x', 's', 'w', 'q', 'a'};

bool is_barrier(char c);

int main(int argc, char *argv[])
{
	FILE *fp;
	int numRows = 0;	//Default with the number of rows to 0.
	int numCols = 1;	//Start with 1 column, to account for the column of newlines.
	int c, i, j, k, l, n;
	int radius = 1, r;	//the whitespace radius
	int cwR, cwD;
	bool whiteHere;
	int count = 0;

	if(argc < 2) return 0;

	//Open the file containing the ASCII map.
	fp = fopen(argv[1], "r");

	//Count the number of columns in the map.
	while((c = fgetc(fp)) != '\n')
		numCols++;

	//Reset the cursor.
	rewind(fp);

	//Count the number of rows in the map.
	while((c = fgetc(fp)) != EOF)
		if(c == '\n')
			numRows++;

	//Reset the cursor.
	rewind(fp);

	//Copy in the map's contents into a character matrix.
	char m[numRows][numCols];

	for(i = 0; i < numRows; i++)
		for(j = 0; j < numCols; j++)
			m[i][j] = fgetc(fp);

	//Close the connection to the file.
	fclose(fp);

	for(i = 2; i < numRows - 2; i++)
	{
		for(j = 2; j < numCols - 3; j++)
		{
			bool inner_whitespace;
			bool tlC, tlC1, tlC2, tlC3, tlC4, trC, trC1, trC2, trC3, trC4;
			bool blC, blC1, blC2, blC3, blC4, brC, brC1, brC2, brC3, brC4;

			//From the current location, check if there is a corner, whether concave or convex,
			//in the top-left of its field of vision. Do this by ensuring that there's a 
			//barrier in the corner, either isolated or L-shaped, and then make sure there's 
			//only whitespace in the square of grid points 1 away from the current location.
			tlC1 = is_barrier(m[i-2][j-2]);
			tlC2 = is_barrier(m[i-2][j-1]) && is_barrier(m[i-1][j-2]) && is_barrier(m[i-2][j]) && is_barrier(m[i][j-2]);
			tlC3 = m[i-2][j-1] == ' ' && m[i-1][j-2] == ' ' && m[i-2][j] == ' ' && m[i][j-2] == ' ';
			tlC4 = m[i-1][j] == ' ' && m[i-1][j-1] == ' ' && m[i][j-1] == ' ';

			//Check for a corner in the top-right.
			trC1 = is_barrier(m[i-2][j+2]);
			trC2 = is_barrier(m[i-2][j+1]) && is_barrier(m[i-1][j+2]) && is_barrier(m[i-2][j]) && is_barrier(m[i][j+2]);
			trC3 = m[i-2][j+1] == ' ' && m[i-1][j+2] == ' ' && m[i-2][j] == ' ' && m[i][j+2] == ' ';
			trC4 = m[i-1][j] == ' ' && m[i-1][j+1] == ' ' && m[i][j+1] == ' ';

			//Check for a corner in the bottom-left.
			blC1 = is_barrier(m[i+2][j-2]);
			blC2 = is_barrier(m[i+2][j-1]) && is_barrier(m[i+1][j-2]) && is_barrier(m[i+2][j]) && is_barrier(m[i][j-2]);
			blC3 = m[i+2][j-1] == ' ' && m[i+1][j-2] == ' ' && m[i+2][j] == ' ' && m[i][j-2] == ' ';
			blC4 = m[i+1][j] == ' ' && m[i+1][j-1] == ' ' && m[i][j-1] == ' ';

			//Check for a corner in the bottom-right.
			brC1 = is_barrier(m[i+2][j+2]);
			brC2 = is_barrier(m[i+2][j+1]) && is_barrier(m[i+1][j+2]) && is_barrier(m[i+2][j]) && is_barrier(m[i][j+2]);
			brC3 = m[i+2][j+1] == ' ' && m[i+1][j+2] == ' ' && m[i+2][j] == ' ' && m[i][j+2] == ' ';
			brC4 = m[i+1][j] == ' ' && m[i+1][j+1] == ' ' && m[i][j+1] == ' ';

			//Ensure that the drone is immediately surrounded by whitespace only.
			inner_whitespace = tlC4 && trC4 && blC4 && brC4;

			//Summarize the work above into four neat boolean values that will be used below.
			tlC = tlC1 && (tlC2 || tlC3);
			trC = trC1 && (trC2 || trC3);
			blC = blC1 && (blC2 || blC3);
			brC = brC1 && (brC2 || brC3);

			//If there's a corner nearby, mark the current location with a decorative 'b'.
			if(m[i][j] == ' ' && (tlC || trC || blC || brC) && inner_whitespace)
				m[i][j] = 'b';
		}
	}

	//Mark large amounts of whitespace on the map by checking smaller and smaller squares
	//around each point, looking for large amounts of whitespace.
	for(r = 10; r > 4; r--)
	{
		for(i = r; i < numRows - r; i++)
		{
			for(j = r; j < numCols - (r+1); j++)
			{
				bool allWhiteSpace = true;

				for(k = -r; k <= r; k++)
					for(l = -r; l <= r; l++)
						if(m[i+k][j+l] != ' ')
							allWhiteSpace = false;

				if(allWhiteSpace)
					m[i][j] = 'b';
			}
		}
	}

	FILE *f = fopen("parsemap.txt", "w");
	for(i = 0; i < numRows; i++)
		for(j = 0; j < numCols; j++)
			fputc(m[i][j], f);
	fclose(f); 

	int points[numRows*numCols][2];  
	int index = 0;

	for(i = 0; i < numRows; i++)
	{
		for(j = 0; j < numCols - 1; j++)
		{
			if(m[i][j] == 'b')
			{
				points[index][0] = j * 35;
				points[index][1] = (numRows - 1 - i) * 35;
				index++;
			}
		}
	}

	points[index][0] = -1;
	points[index][1] = -1;

	index = -1;

	while(points[++index][0] != -1);

	FILE *fptr = fopen("points.csv", "w");

	fprintf(fptr, "%d\n", index);

	for(i = 0; i < index; i++)
		fprintf(fptr, "%d %d\n", points[i][0], points[i][1]);

	fclose(fptr);
}

bool is_barrier(char c)
{
	int i;

	for(i = 0; i < NUM_BARRIERS; i++)
		if(c == barriers[i])
			return true;

	return false;
}
