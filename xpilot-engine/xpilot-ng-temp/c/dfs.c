//Matthew Coffman - June 2018
#include <stdio.h>
#include <stdbool.h>

//vertex_status structure: during the execution of depth-first search,
//we'll be maintaining a queue of vertices that represent the current path,
//so for each vertex we'll need to keep track of
//its id,
//how we got to it, and 
//what it points to next. 
typedef struct vertex_status_f
{
  int id;
  struct vertex_status_f *prevptr;
  struct vertex_status_f *nextptr;
} vsf_t;

//Looks through the graph looking for edges that start from the vertex with
//the given id, and checks for such an edge where the other vertex has not
//yet been looked at (i.e. is not on our list of visited vertices).
int dfs_helper(graph_t g, int id, int *vis_vert)
{
  int i;
  int next_id;

  //Check each edge in the graph.
  for(i = 0; i < g.num_e; i++)
  {
    next_id = -1;

    //If either vertex has the given id, note the id of the other vertex.
    if(g.edges[i].v1.id == id)
    {
      next_id = g.edges[i].v2.id;
    }

    if(g.edges[i].v2.id == id)
    {
      next_id = g.edges[i].v1.id;
    }

    //If we found an edge where one vertex has the given id, and if the other
    //vertex has not been visited yet, return the next id.
    if(next_id != -1 && !found_in_array(vis_vert, next_id))
    {
      return next_id;
    }
  }

  //If no appropriate edge can be found, return -1.
  return -1;
}

//Use a depth-first search to find a path through the given graph from start
//to end. Store this path in the given path variable.
void dfs(graph_t g, vertex_t start, vertex_t end, int *path)
{
  int visited_vertices[g.num_v];
  int i, num_vv = 0, next_id;
  vsf_t *beginptr = NULL;
  vsf_t *currptr = NULL;
  bool path_found = false;

  //In the degenerate case, just return a path with the single start/end vertex.
  if(start.id == end.id)
  {
    path[0] = start.id;
    path[1] = '\0';
    return;
  }

  //If the start vertex is not in the graph, throw an error.
  if(!vertex_in_graph(g, start))
  {
    printf("Error: vertex %d not found in graph.\n", start.id);
    path[0] = '\0';
    return;
  }

  //If the end vertex is not in the graph, throw an error.
  if(!vertex_in_graph(g, end))
  {
    printf("Error: vertex %d not found in graph.\n", end.id);
    path[0] = '\0';
    return;
  }

  //Allocate memory for a vertex_status structure for the start vertex.
  beginptr = malloc(sizeof(vsf_t));
  if(!beginptr)
  {
    //Throw an error and abort if memory allocation fails.
    perror("Error: couldn't allocate memory");
    abort();
  }
  //If memory allocation succeeds, initialize the start vertex as pointing to
  //nothing.
  *beginptr = (vsf_t) {start.id, NULL, NULL};
  currptr = beginptr;

  //Initialize the visited_vertices array with null characters, and indicate
  //that the start vertex has been visited: it won't appear elsewhere in the path.
  for(i = 0; i < g.num_v; i++)
  {
    visited_vertices[i] = '\0';
  }
  visited_vertices[num_vv++] = start.id;

  //Loop as long as we haven't found a path and we haven't looked at all the vertices.
  while(!path_found && num_vv < g.num_v)
  {
    //Find an edge involving the current vertex that connects to a vertex we haven't
    //visited yet.
    next_id = dfs_helper(g, currptr->id, visited_vertices);

    //If we were able to find such a vertex, allocate memory for a vertex_status
    //structure and connect it to the current queue
    if(next_id != -1)
    {
      currptr->nextptr = malloc(sizeof(vsf_t));
      if(!currptr->nextptr)
      {
        perror("Error: couldn't allocate memory");
        abort();
      }
      //Start pointing to the new vertex, and indicate that it has been visited.
      *currptr->nextptr = (vsf_t) {next_id, currptr, NULL};
      visited_vertices[num_vv++] = next_id;
      currptr = currptr->nextptr;
    }
    //If the current vertex is a dead end...
    else
    {
      //If we have nowhere else to go from the start vertex, there must not be a
      //path from start to end.
      if(currptr->id == start.id)
      {
        printf("Error: no path from %d to %d\n", start.id, end.id);
        path[0] = '\0';
        return;
      }
      //But, if we aren't currently at the start vertex, just backtrack and try again.
      currptr = currptr->prevptr;
      free(currptr->nextptr);
    }
   
    //If we ever encounter the end vertex, we must have found a path.
    if(currptr->id == end.id)
    {
      path_found = true;
    }
  }

  currptr = beginptr;
  i = 0;
  //Loop through the list of vertices and store them to the given path variable.
  while(currptr)
  {
    path[i++] = currptr->id;
    currptr = currptr->nextptr;
  }
  //End the path with a null character, for safety.
  path[i] = '\0';

  //Finally, free up the dynamically allocated memory that we don't need anymore.
  free(beginptr);
}

