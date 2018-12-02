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
 * Mark convex corners (w/ corner-peek points). 
 ******************************************************************************/  

/*
 * The following code marks all convex corners on the map in three distinct stages.
 * Each stage below is preceded by a brief description of the algorithm used.
 *
 * 1. Iterate through each point in the ASCII map. For each point, first make sure
 *    that there's only whitespace in the immediate vicinity: there should only be
 *    whitespace at all the points in the square of "radius" 1 around the current
 *    point. That is, if 'l' represents the current location under inspection, and 
 *    'w' refers to a whitespace character, then we want spaces to look like
 *
 *       www
 *       wlw
 *       www
 *
 *    where the current location under the 'l' is also whitespace. After ensuring
 *    there is only whitespace in the immediate vicinity, check the square of
 *    "radius" 2 around the current location and check for corner points. In this
 *    case, we identify corner points by checking for three consecutive barrier
 *    points next to two whitespace points. That is, using 'l' and 'w' as above,
 *    omitting the whitespace within "radius" 1 around the current location, and
 *    using 'x' to denote a barrier, we would mark corners as follows:
 *
 *       wwwww        wwxxx        xxxww        wwxwx        wwwww
 *       w   w        w   w        x   w        w   w        w   w
 *       w l x        w l w        x l w        w l w        w l w
 *       w   x        w   w        w   w        w   w        w   w
 *       wwwwx        wwwww        wwwww        wwwww        xxxwx
 *
 *      (corner)     (corner)    (corner)   (not corner)  (not corner)  
 */ 

  //Iterate over all points in the map m[i][j], except for points on the edge of the
  //map or 1 unit away from the edge. Since our algorithm below will be checking the
  //square of "radius" 2 around each point, we don't want to run into indexing errors.
  //Also, the value for j will stop one more unit early (j < numCols - 3) because the
  //last column in the map matrix is all newline chars, and we needn't to check those.
  for(i = 2; i < numRows - 2; i++)
  {
    for(j = 2; j < numCols - 3; j++)
    {
      bool whitespaceNearby = true;
      bool wallL = false, wallT = false, wallR = false, wallB = false;

      //Check to make sure all points within "radius" 1 of the current point are white-
      //space. Assume all whitespace, and if we encounter a barrier in the nearby vicinity,
      //change the boolean variable accordingly.
      for(k = -1; k <= 1; k++)
      {
        for(l = -1; l <= 1; l++)
        {
          if(is_barrier(m[i+k][j+l]))
          {
            whitespaceNearby = false;
          }
        }
      }
      
      //If there's only whitespace nearby the current point, start checking for corner points,
      //identified by three contiguous corner points two units away from the current point, 
      //followed by two contiguous points of whitespace.
      if(whitespaceNearby)
      {
        //Check if there's a wall two points away to the left of the current point.
        if(is_barrier(m[i][j-2]))
        {
          wallL = (is_barrier(m[i-2][j-2]) && is_barrier(m[i-1][j-2]) && !is_barrier(m[i+1][j-2]) && !is_barrier(m[i+2][j-2]))
                  || (!is_barrier(m[i-2][j-2]) && !is_barrier(m[i-1][j-2]) && is_barrier(m[i+1][j-2]) && is_barrier(m[i+2][j-2]));
        }

        //Check for a wall above the current point.
        if(is_barrier(m[i-2][j]))
        {
          wallT = (is_barrier(m[i-2][j-2]) && is_barrier(m[i-2][j-1]) && !is_barrier(m[i-2][j+1]) && !is_barrier(m[i-2][j+2]))
                  || (!is_barrier(m[i-2][j-2]) && !is_barrier(m[i-2][j-1]) && is_barrier(m[i-2][j+1]) && is_barrier(m[i-2][j+2]));
        }

        //Check for a wall to the right of the current point.
        if(is_barrier(m[i][j+2]))
        {
          wallR = (is_barrier(m[i-2][j+2]) && is_barrier(m[i-1][j+2]) && !is_barrier(m[i+1][j+2]) && !is_barrier(m[i+2][j+2]))
                  || (!is_barrier(m[i-2][j+2]) && !is_barrier(m[i-1][j+2]) && is_barrier(m[i+1][j+2]) && is_barrier(m[i+2][j+2]));
        } 

        //Check for a wall below the current point.
        if(is_barrier(m[i+2][j]))
        {
          wallB = (is_barrier(m[i+2][j-2]) && is_barrier(m[i+2][j-1]) && !is_barrier(m[i+2][j+1]) && !is_barrier(m[i+2][j+2]))
                  || (!is_barrier(m[i+2][j-2]) && !is_barrier(m[i+2][j-1]) && is_barrier(m[i+2][j+1]) && is_barrier(m[i+2][j+2]));
        }
      }

      //If there's only whitespace nearby and we found a wall somewhere around this point,
      //mark this point as a corner point.
      if(whitespaceNearby && (wallL || wallT || wallR || wallB))
      {
        m[i][j] = 'c';
      }
    }
  }

/*
 * 2. This stage fills in convex corner points that might have been missed in stage 
 *    1 above due to the walls not all being perfectly vertical or horizontal. Some 
 *    walls in the map might be diagonal, and we want to mark these corners as well. 
 *
 *    Again, iterate through each point in the map. For each point, again make sure 
 *    there's only whitespace in the points within "radius" 1 around the current 
 *    location. This done, for each point check if there's a 'c' diagonally two points
 *    away from the current location. Also, check if there's a barrier two points away
 *    vertically or horizontally. If these conditions are met, mark the current point.
 *    For example, the following point would be marked as a corner, where 'l' and 'x'
 *    mean the same as in the explanation of stage 1 above:
 *
 *         xxxxxxxxxxxxxxxxxxxxx
 *         x                   x
 *         x         c         x
 *         x                   x
 *         x      l  xxx       x
 *         x          xx       x
 *         x           x       x
 *         x                   x
 *         x                   x
 *         xxxxxxxxxxxxxxxxxxxxx           
 */

  //As above, iterate through every point in the map, except the edges to avoid indexing
  //errors and newline characters we don't care about.
  for(i = 2; i < numRows - 2; i++)
  {
    for(j = 2; j < numCols - 3; j++)
    {
      bool whitespaceNearby = true;

      //Also as above, check for whitespace around the current point.
      for(k = -1; k <= 1; k++)
      {
        for(l = -1; l <= 1; l++)
        {
          if(is_barrier(m[i+k][j+l]))
          {
            whitespaceNearby = false;
          }
        }
      }

      //Catch corner points around diagonal walls by checking for 'c's two diagonal units
      //away from the current point, as well as a barrier two units away horizontally or
      //vertically from the current point.
      if(whitespaceNearby)
      {
        //Check for a 'c' diagonally to the top-left.
        if(m[i-2][j-2] == 'c')
        {
          //Check for a barrier immediately above and/or left.
          if(is_barrier(m[i][j-2]) || is_barrier(m[i-2][j]))
          {
            m[i][j] = 'c';
          }
        }

        //Check for a 'c' diagonally to the top-right. 
        if(m[i-2][j+2] == 'c')
        {
          //Check for a barrier immediately above and/or right.
          if(is_barrier(m[i][j+2]) || is_barrier(m[i-2][j]))
          {
            m[i][j] = 'c';
          }
        }

        //Check for a 'c' diagonally to the bottom-right.
        if(m[i+2][j+2] == 'c')
        {
          //Check for a barrier immediately below and/or right.
          if(is_barrier(m[i][j+2]) || is_barrier(m[i+2][j]))
          {
            m[i][j] = 'c';
          }
        }

        //Check for a 'c' diagonally to the bottom-left.
        if(m[i+2][j-2] == 'c')
        {
          //Check for a barrier immediately below and/or left.
          if(is_barrier(m[i][j-2]) || is_barrier(m[i+2][j]))
          {
            m[i][j] = 'c';
          }
        }
      }
    }
  }


/*
 * 3. This stage finishes the job of marking convex corners. The previous stages
 *    identify corner points and insert 'c's two points away in the horizontal
 *    and vertical directions. Here, we also insert 'c's diagonally two spots away
 *    from identified corner points. 
 *
 *    Again, we first iterate through the map and check for whitespace immediately
 *    around each point. Then, for each point, we check if there's a 'c' two spots
 *    away vertically, whether up or down, and a 'c' horizontally two spots away,
 *    whether left or right. If we find 2+ such 'c's, mark the current spot with a
 *    'c'. For example, the following would be marked with 'c's:
 *
 *         xxxxxxxxxxx
 *         x         x
 *         x  l c    x
 *         x         x
 *         x  c xxx  x
 *         x    xxx  x
 *         x         x 
 *         xxxxxxxxxxx
 */

  //As above, iterate through the whole map, except for edges and newlines.
  for(i = 2; i < numRows - 2; i++)
  {
    for(j = 2; j < numCols - 3; j++)
    {
      bool whitespaceNearby = true;
    
      //Again as above, check for whitespace immediately around the current point.
      for(k = -1; k <= 1; k++)
      {
        for(l = -1; l <= 1; l++)
        {
          if(is_barrier(m[i+k][j+l]))
          {
            whitespaceNearby = false;
          }
        }
      }

      //If there's only whitespace nearby, check if there's a 'c' above and/or below
      //the current point, then check if there's a 'c' to the left and/or right. If
      //these conditions are met, mark the current point.
      if(whitespaceNearby)
      {
        if(m[i][j-2] == 'c' || m[i][j+2] == 'c')
        {
          if(m[i-2][j] == 'c' || m[i+2][j] == 'c')
          {
            m[i][j] = 'c';
          }
        }
      }
    }
  }


/******************************************************************************
 * Mark concave corners. 
 ******************************************************************************/  

/*
 * Having marked all convex corners, now mark all the concave corners. To this end,
 * iterate through the whole map, except for points near the edges and the column
 * of newline characters. For each point, make sure it's immediately surrounded by
 * whitespace. Then, identify concave corners by checking if there are barrier points
 * in an L-shape around the current point. So, concave points might be marked as
 * follows, where 'w', 'x', and 'l' have the same meaning as in examples above, and
 * omitting the whitespace immediately around the current point:
 *
 *      wwxxx        xxxwx        xxwww  
 *      w   x        x   x        x   w
 *      w l x        x l w        x l w 
 *      w   w        w   w        w   w
 *      wwwww        wwwxx        wwwww
 * 
 *     (corner)     (corner)   (not corner)
 */ 

  //Iterate through all points in the map, except for points near edges and newlines.
  for(i = 2; i < numRows - 2; i++)
  {
    for(j = 2; j < numCols - 3; j++)
    {
      bool whitespaceNearby = true;
      bool tl = false, tr = false, br = false, bl = false;

      //Check for whitespace immediately surrounding the current point, as in the
      //procedure for marking convex points above.
      for(k = -1; k <= 1; k++)
      {
        for(l = -1; l <= 1; l++)
        {
          if(is_barrier(m[i+k][j+l]))
          {
            whitespaceNearby = false;
          }
        }
      }

      //If there's only whitespace in the square of "radius" 1 around the current point,
      //check for an L-shape of barriers two units away. It doesn't matter whether this
      //L-shape is to the top-left (tl), top-right (tr), bottom-left (bl), or bottom-right (br).
      if(whitespaceNearby)
      {
        tl = is_barrier(m[i][j-2]) && is_barrier(m[i-1][j-2]) && is_barrier(m[i-2][j-2]) && is_barrier(m[i-2][j-1]) && is_barrier(m[i-2][j]);
        tr = is_barrier(m[i][j+2]) && is_barrier(m[i-1][j+2]) && is_barrier(m[i-2][j+2]) && is_barrier(m[i-2][j+1]) && is_barrier(m[i-2][j]);
        bl = is_barrier(m[i][j-2]) && is_barrier(m[i+1][j-2]) && is_barrier(m[i+2][j-2]) && is_barrier(m[i+2][j-1]) && is_barrier(m[i+2][j]);
        br = is_barrier(m[i][j+2]) && is_barrier(m[i+1][j+2]) && is_barrier(m[i+2][j+2]) && is_barrier(m[i+2][j+1]) && is_barrier(m[i+2][j]);
      }

      //If we found only whitespace nearby the current point and an L-shape of barriers
      //two units away, mark the current point as a concave corner with a '$'.
      if(whitespaceNearby && (tl || tr || bl || br))
      {
        m[i][j] = '$';
      }
    }
  }


/******************************************************************************
 * Mark large amounts of whitespace.
 ******************************************************************************/  

/*
 * Here, we mark large amounts of whitespace. We set a "radius", initially with
 * a value of 10. Then we iterate through each point in the map (except for edges
 * and newline characters, as above). For each point, we check all points within
 * the "radius" around the current point. If we find no barriers in this square,
 * mark the current point. After checking all points in the map, decrement the
 * "radius" and start again. This way, we mark the larger regions of whitespace
 * before checking the smaller ones, hopefully to limit the number of unnecessary
 * points that show up in the final version of the map.
 */

  //Start the "radius" at 10, and decrement to 4.
  for(r = 10; r > 4; r--)
  {
    //Iterate through every point in the map (except edges and newlines).
    for(i = r; i < numRows - r; i++)
    {
      for(j = r; j < numCols - (r+1); j++)
      {
        bool allWhiteSpace = true;

        //Check all points within the given "radius" of the current point.
        for(k = -r; k <= r; k++)
        {
          for(l = -r; l <= r; l++)
          {
            if(m[i+k][j+l] != ' ')
            {
              allWhiteSpace = false;
            }
          }
        }
        
        //If we find only whitespace within the square of the given "radius",
        //mark the current point with a '$'.
        if(allWhiteSpace)
        {
          m[i][j] = '$';
        }
      }
    }
  }


/******************************************************************************
 * Write the edited ASCII map to a file.
 ******************************************************************************/  

  FILE *f = fopen("parsemap.txt", "w");

  for(i = 0; i < numRows; i++)
  {
    for(j = 0; j < numCols; j++)
    {
      fputc(m[i][j], f);
    }
  }

  fclose(f); 


/******************************************************************************
 * Count the number of marked points.
 ******************************************************************************/  

  int count = 0;

  for(i = 0; i < numRows; i++)
  {
    for(j = 0; j < numCols - 1; j++)
    {
      if(m[i][j] == 'c' || m[i][j] == '$')
      {
        count++;
      }
    }
  }


/******************************************************************************
 * Convert points to pixels and write to file.
 ******************************************************************************/  

  FILE *fptr = fopen("points.csv", "w");

  fprintf(fptr, "%d\n", count);
  
  //Write the corner-peek points first.
  for(i = 0; i < numRows; i++)
  {
    for(j = 0; j < numCols - 1; j++)
    {
      if(m[i][j] == 'c')
      {
        fprintf(fptr, "%d %d\n", j * ASCII_TO_PIXELS, (numRows-1-i) * ASCII_TO_PIXELS);
      }
    }
  }

  //Now write all the other points.
  for(i = 0; i < numRows; i++)
  {
    for(j = 0; j < numCols - 1; j++)
    {
      if(m[i][j] == '$')
      {
        fprintf(fptr, "%d %d\n", j * ASCII_TO_PIXELS, (numRows-1-i) * ASCII_TO_PIXELS);
      }
    }
  }

  fclose(fptr);
}



/******************************************************************************
 * Helper function: determine if the given character represents a barrier.
 ******************************************************************************/  

bool is_barrier(char c)
{
  static char barriers[] = {'x', 's', 'w', 'q', 'a'};
  static int num_barriers = 5;
  int i;

  //Loop through the pre-determined barrier characters, checking if the given
  //character matches any of those. If so, return true.
  for(i = 0; i < num_barriers; i++)
  {
    if(c == barriers[i])
    {
      return true;
    }
  }

  return false;
}
