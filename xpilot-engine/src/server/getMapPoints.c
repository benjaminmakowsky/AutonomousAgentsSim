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
    numCols++;

  rewind(fp);

  //Count the number of rows.
  while((c = fgetc(fp)) != EOF)
    if(c == '\n')
      numRows++;

  rewind(fp);

  //Copy the ASCII map into a character matrix.
  char m[numRows][numCols];

  for(i = 0; i < numRows; i++)
    for(j = 0; j < numCols; j++)
      m[i][j] = fgetc(fp);

  //Close the file.
  fclose(fp);


/******************************************************************************
 * Mark convex corners (w/ corner-peek points). 
 ******************************************************************************/  

  for(i = 2; i < numRows - 2; i++)
  {
    for(j = 2; j < numCols - 3; j++)
    {
      bool wsNearby = true;
      bool wallL = false, wallT = false, wallR = false, wallB = false;

      for(k = -1; k <= 1; k++)
        for(l = -1; l <= 1; l++)
          if(is_barrier(m[i+k][j+l]))
            wsNearby = false;
      
      if(wsNearby)
      {
        if(is_barrier(m[i][j-2]))
        {
          wallL = (is_barrier(m[i-2][j-2]) && is_barrier(m[i-1][j-2]) && !is_barrier(m[i+1][j-2]) && !is_barrier(m[i+2][j-2]))
                  || (!is_barrier(m[i-2][j-2]) && !is_barrier(m[i-1][j-2]) && is_barrier(m[i+1][j-2]) && is_barrier(m[i+2][j-2]));
        }

        if(is_barrier(m[i-2][j]))
        {
          wallT = (is_barrier(m[i-2][j-2]) && is_barrier(m[i-2][j-1]) && !is_barrier(m[i-2][j+1]) && !is_barrier(m[i-2][j+2]))
                  || (!is_barrier(m[i-2][j-2]) && !is_barrier(m[i-2][j-1]) && is_barrier(m[i-2][j+1]) && is_barrier(m[i-2][j+2]));
        }

        if(is_barrier(m[i][j+2]))
        {
          wallR = (is_barrier(m[i-2][j+2]) && is_barrier(m[i-1][j+2]) && !is_barrier(m[i+1][j+2]) && !is_barrier(m[i+2][j+2]))
                  || (!is_barrier(m[i-2][j+2]) && !is_barrier(m[i-1][j+2]) && is_barrier(m[i+1][j+2]) && is_barrier(m[i+2][j+2]));
        } 

        if(is_barrier(m[i+2][j]))
        {
          wallB = (is_barrier(m[i+2][j-2]) && is_barrier(m[i+2][j-1]) && !is_barrier(m[i+2][j+1]) && !is_barrier(m[i+2][j+2]))
                  || (!is_barrier(m[i+2][j-2]) && !is_barrier(m[i+2][j-1]) && is_barrier(m[i+2][j+1]) && is_barrier(m[i+2][j+2]));
        }
      }

      if(wsNearby && (wallL || wallT || wallR || wallB))
        m[i][j] = 'c';
    }
  }

  for(i = 2; i < numRows - 2; i++)
  {
    for(j = 2; j < numCols - 3; j++)
    {
      bool wsNearby = true;

      for(k = -1; k <= 1; k++)
        for(l = -1; l <= 1; l++)
          if(is_barrier(m[i+k][j+l]))
            wsNearby = false;

      if(wsNearby)
      {
        if(m[i-2][j-2] == 'c')
          if(is_barrier(m[i][j-2]) || is_barrier(m[i-2][j]))
            m[i][j] = 'c';
 
        if(m[i-2][j+2] == 'c')
          if(is_barrier(m[i][j+2]) || is_barrier(m[i-2][j]))
            m[i][j] = 'c';

        if(m[i+2][j+2] == 'c')
          if(is_barrier(m[i][j+2]) || is_barrier(m[i+2][j]))
            m[i][j] = 'c';

        if(m[i+2][j-2] == 'c')
          if(is_barrier(m[i][j-2]) || is_barrier(m[i+2][j]))
            m[i][j] = 'c';
      }
    }
  }

  for(i = 2; i < numRows - 2; i++)
  {
    for(j = 2; j < numCols - 3; j++)
    {
      bool wsNearby = true;
    
      for(k = -1; k <= 1; k++)
        for(l = -1; l <= 1; l++)
          if(is_barrier(m[i+k][j+l]))
            wsNearby = false;

      if(wsNearby)
        if(m[i][j-2] == 'c' || m[i][j+2] == 'c')
          if(m[i-2][j] == 'c' || m[i+2][j] == 'c')
            m[i][j] = 'c';
    }
  }


/******************************************************************************
 * Mark concave corners. 
 ******************************************************************************/  

  for(i = 2; i < numRows - 2; i++)
  {
    for(j = 2; j < numCols - 3; j++)
    {
      bool wsNearby = true;
      bool tl = false, tr = false, br = false, bl = false;

      for(k = -1; k <= 1; k++)
        for(l = -1; l <= 1; l++)
          if(is_barrier(m[i+k][j+l]))
            wsNearby = false;

      if(wsNearby)
      {
        tl = is_barrier(m[i][j-2]) && is_barrier(m[i-1][j-2]) && is_barrier(m[i-2][j-2]) && is_barrier(m[i-2][j-1]) && is_barrier(m[i-2][j]);
        tr = is_barrier(m[i][j+2]) && is_barrier(m[i-1][j+2]) && is_barrier(m[i-2][j+2]) && is_barrier(m[i-2][j+1]) && is_barrier(m[i-2][j]);
        bl = is_barrier(m[i][j-2]) && is_barrier(m[i+1][j-2]) && is_barrier(m[i+2][j-2]) && is_barrier(m[i+2][j-1]) && is_barrier(m[i+2][j]);
        br = is_barrier(m[i][j+2]) && is_barrier(m[i+1][j+2]) && is_barrier(m[i+2][j+2]) && is_barrier(m[i+2][j+1]) && is_barrier(m[i+2][j]);
      }

      if(wsNearby && (tl || tr || bl || br))
        m[i][j] = 'b';
    }
  }


/******************************************************************************
 * Mark large amounts of whitespace.
 ******************************************************************************/  

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


/******************************************************************************
 * Write the edited ASCII map to a file.
 ******************************************************************************/  

  FILE *f = fopen("parsemap.txt", "w");

  for(i = 0; i < numRows; i++)
    for(j = 0; j < numCols; j++)
      fputc(m[i][j], f);

  fclose(f); 


/******************************************************************************
 * Count the number of marked points.
 ******************************************************************************/  
  int count = 0;

  for(i = 0; i < numRows; i++)
    for(j = 0; j < numCols - 1; j++)
      if(m[i][j] == 'c' || m[i][j] == 'b')
        count++;


/******************************************************************************
 * Convert points to pixels and write to file.
 ******************************************************************************/  

  FILE *fptr = fopen("points.csv", "w");

  fprintf(fptr, "%d\n", count);
  
  //Write the corner-peek points first.
  for(i = 0; i < numRows; i++)
    for(j = 0; j < numCols - 1; j++)
      if(m[i][j] == 'c')
        fprintf(fptr, "%d %d\n", j*35, (numRows-1-i)*35);

  //Now write all the other points.
  for(i = 0; i < numRows; i++)
    for(j = 0; j < numCols - 1; j++)
      if(m[i][j] == 'b')
        fprintf(fptr, "%d %d\n", j*35, (numRows-1-i)*35);

  fclose(fptr);
}



/******************************************************************************
 * Helper function: determine if the given character represents a barrier.
 ******************************************************************************/  
bool is_barrier(char c)
{
  int i;

  for(i = 0; i < NUM_BARRIERS; i++)
    if(c == barriers[i])
      return true;

  return false;
}
