#include <stdio.h>
#include <stdbool.h>
//#include "graph.c"

//vertex_status structure: during the execution of breadth-first search, we'll be
//generating a queue (or in this case just a list) of vertices as we step through
//them looking for a path. For each vertex, we'll need information on
//its id,
//the vertex it points to next,
//and the vertex it points back to.
typedef struct vertex_status_b
{
  int id;
  struct vertex_status_b *nextptr;
  struct vertex_status_b *prevptr;
} vsb_t;

//Given a pointer to a list of vertex_status structres, looks to see if there's 
//an element with the given id in the list.
bool found_in_list(vsb_t *vsptr, int id)
{
  vsb_t *tempptr = vsptr;
  while(tempptr)
  {
    if(tempptr->id == id)
      return true;
    else
      tempptr = tempptr->nextptr;
  }
  return false;
}

//Given a graph, a start and end vertex, and a path variable, finds a path between the
//start and end points if possible, and copies this path to the path variable.
void bfs(graph_t g, vertex_t start, vertex_t end, int *path)
{ 
  int i, j, temp_path[g.num_v + 2];
  vsb_t *beginptr = NULL;
  vsb_t *endptr = NULL;
  vsb_t *currptr = NULL;
  bool path_found = false;

  //In the degenerate case, just return the start/end point.
  if(start.id == end.id)
  {
    path[0] = start.id;
    path[1] = '\0';
    return;
  }

  //If either the start or end point does not exist in the graph, throw an error.
  if(!vertex_in_graph(g, start))
  {
    printf("Error: vertex %d not found in graph.\n", start.id);
    path[0] = '\0';
    return;
  }

  if(!vertex_in_graph(g, end))
  {
    printf("Error: vertex %d not found in graph.\n", end.id);
    path[0] = '\0';
    return;
  }
   
  //Allocate memory for one vertex_status structure, and have the beginptr point
  //to this structure. We will allocate for more vertex_status structures as needed.
  beginptr = malloc(sizeof(vsb_t));
  if(!beginptr)
  {
    //Throw an error if something went wrong with the memory allocation.
    perror("Error: couldn't allocate memory");
    abort();
  }
  //Initialize the beginning vertex_status structure. It stores the id of the start
  //vertex and initially points to no predecessor or successor.
  *beginptr = (vsb_t) {start.id, NULL, NULL};
  
  //Initially, the beginning vertex is also the ending vertex on our list, as well
  //as the vertex we are currently looking at.
  endptr = beginptr;
  currptr = beginptr;

  //Loop while we have not found the desired path.
  while(!path_found)
  {
    //If the pointer of the "vertex" we're currently looking at is NULL, that means
    //we must have hit the end of the list and have exhausted all vertices in the
    //graph. Thus, we cannot find a path, and we must have a disconnected graph.
    if(!currptr)
    {
      printf("Error: no path from %d to %d\n", start.id, end.id);
      path[0] = '\0';
      return;
    }
    //If we ever find ourselves inspecting the vertex we're trying to get to,
    //that means we've found a path and we're done searching.
    else if(currptr->id == end.id)
      path_found = true;
    else
    {
      //Loop through and check every edge in the given graph.
      for(i = 0; i < g.num_e; i++)
      {
        int other_id = -1;

        if(g.edges[i].v1.id == currptr->id)
          other_id = g.edges[i].v2.id;

        if(g.edges[i].v2.id == currptr->id)
          other_id = g.edges[i].v1.id;

        bool other_not_found = false;
        if(other_id != -1)
          other_not_found = !found_in_list(beginptr, other_id);

        if(other_not_found)
        {
          //If we've found a vertex not currently on our list, add it to the end of
          //the list.
          vsb_t *tempptr = malloc(sizeof(vsb_t));
          if(!tempptr)
          {
            perror("Error: couldn't allocate memory");
            abort();
          }
          //Give this new vertex its id and have it point back to what used to be the
          //end of the list.
          *tempptr = (vsb_t) {other_id, NULL, currptr};
          //Adjust the endptr pointer accordingly.
          endptr->nextptr = tempptr;
          endptr = endptr->nextptr;
        }
      }

      currptr = currptr->nextptr;
    }
  }

  //After finding the desired path, start at the end and backtrack, storing the path
  //to a temporary integer array in backwards order.
  i = 0;
  while(currptr->prevptr)
  {
    temp_path[i++] = currptr->id;
    currptr = currptr->prevptr;
  }
  temp_path[i++] = currptr->id;
 
  //Store the path to the given path variable, this time in forward order.
  for(j = 0; j < i; j++)
  { 
    path[j] = temp_path[i-1-j];
  }
  path[i] = '\0';

  //Finally, free up the memory we allocated during this function execution.
  vsb_t *tempptr = beginptr;
  while(beginptr)
  {
    tempptr = beginptr->nextptr;
    free(beginptr);
    beginptr = tempptr;
  }
}




