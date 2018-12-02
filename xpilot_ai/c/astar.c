//Matthew Coffman - June 2018
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

//vertex_status structure: during the execution of the A* algorithm, we will need
//the following information on every vertex:
//its own id and information,
//the id of its predecessor,
//the smallest known distance between it and the start (g), 
//the estimated distance between it and the end (h),
//the sum of these two values (f),
//and a marker to indicate whether we are done looking at this vertex.
typedef struct vertex_status_a
{
  vertex_t v;
  int pred_id;
  int f;
  int g;
  int h;
  bool done;
} vsa_t;

//Given pointers to two vertex_status structures, compares them based on vertex id.
int vsa_cmp_by_id(const void *a, const void *b)
{
  vsa_t *ia = (vsa_t *)a;
  vsa_t *ib = (vsa_t *)b;

  return ia->v.id - ib->v.id;
}

//Given a pointer to an array of vertex_status structures and the length of the
//array, finds the index corresponding to the given id.
int get_vs_index_a(vsa_t *vsptr, int len, int id)
{
  //We'll use l and r to refer to the boundaries of the array chunk we're still
  //searching, and m will refer to the midpoint of this chunk. Hence, l and r are 
  //initially 0 and the length of the array, respectively.
  int l = 0, r = len - 1, m;

  while(l <= r)
  {
    //Compute the midpoint of the array chunk.
    m = l + (r - l)/2;

    //If we've found the desired id, return its index.
    if(vsptr[m].v.id == id)
    {
      return m;
    }

    //If the desired id is too big to be in the left half of the array chunk,
    //update the left boundary l accordingly.
    if(vsptr[m].v.id < id)
    {
      l = m + 1;
    }
    //Conversely, if the desired id is in the left chunk, update the right
    //boundary r accordingly.
    else
    {
      r = m - 1;
    }
  }

  //If we've gotten this far, it must be the case that l > r, meaning the desired
  //id must not be in the array at all, so returning -1 will indicate an invalid answer.
  return -1;
}

//Computes the shortest path from the start vertex to the end vertex, using the
//distance formula as a heuristic to guess how far away the goal is, and stores
//the path in the given path variable.
int astar(graph_t g, vertex_t start, vertex_t end, int *path)
{
  int i, curr_vsi, min_not_done_vsi, end_vsi;
  int backwards_path[g.num_v+1];
  bool path_found = false;
  vsa_t vsptr[g.num_v];

  //If the given start vertex doesn't actually exist in the given graph, throw
  //an error.
  if(!vertex_in_graph(g, start))
  {
    printf("Error: start vertex %d not found in graph\n", start.id);
    path[0] = '\0';
    return EXIT_FAILURE;
  }
  
  //Likewise, if the end vertex can't be found in the graph, throw an error.
  if(!vertex_in_graph(g, end))
  {
    printf("Error: end vertex %d not found in graph\n", end.id);
    path[0] = '\0';
    return EXIT_FAILURE;
  }

  //In the degenerate case where the path starts and ends at the same spot, no
  //work needs to be done: we've already found the shortest path!
  if(start.id == end.id)
  {
    path[0] = start.id;
    path[1] = '\0';
    return EXIT_SUCCESS;
  }

  //Otherwise, wipe clean the chunk of memory we'll be using for our vertex_status
  //structures by setting all that memory to 0. Then initialize each structure.
  memset(vsptr, 0, sizeof(vsa_t) * g.num_v);
  for(i = 0; i < g.num_v; i++)
  {
    //Compute the distance from each point to the end. This is the heuristic value.
    int dist = distance_formula(g.vertices[i].x, g.vertices[i].y, end.x, end.y);

    //For the start vertex, let its predecessor be -1 and have its g value be 0,
    //since there's no distance between the start vertex and itself.
    if(g.vertices[i].id == start.id)
    {
      vsptr[i] = (vsa_t) {g.vertices[i], -1, dist, 0, dist, false};
      curr_vsi = i;
    }
    //Otherwise, assume initially that there's infinite (INT_MAX) distance from
    //the point to the end, meaning the f and g fields are all INT_MAX.
    else
    {
      vsptr[i] = (vsa_t) {g.vertices[i], start.id, INT_MAX, INT_MAX, dist, false};
    }
  }
  
  //Sort the list of vertex_status structures, making searching easier later on.
  qsort(vsptr, g.num_v, sizeof(vsa_t), vsa_cmp_by_id);

  while(!path_found)
  {
    //Loop through every edge in the graph. For each edge, if one of its vertices
    //is the vertex we're currently looking at, make a note of the index of the
    //other vertex in the vertex_status array.
    for(i = 0; i < g.num_e; i++)
    {
      int other_vsi = -1;
      
      if(vsptr[curr_vsi].v.id == g.edges[i].v1.id)
      {
        other_vsi = get_vs_index_a(vsptr, g.num_v, g.edges[i].v2.id);
      }

      if(vsptr[curr_vsi].v.id == g.edges[i].v2.id)
      {
        other_vsi = get_vs_index_a(vsptr, g.num_v, g.edges[i].v1.id);
      }

      if(other_vsi != -1)
      {
        //If the other vertex has a g value bigger than the current vertex's g
        //value plus the weight of the edge between it and the current vertex,
        //update the other vertex's pred_id, g, and f accordingly.
        if(vsptr[other_vsi].g > vsptr[curr_vsi].g + g.edges[i].weight)
        {
          vsptr[other_vsi].pred_id = vsptr[curr_vsi].v.id;
          vsptr[other_vsi].g = vsptr[curr_vsi].g + g.edges[i].weight;
          vsptr[other_vsi].f = vsptr[other_vsi].g + vsptr[other_vsi].h;
        }
      }
    }

    //After looking through all the edges in the graph, mark this vertex as done.
    vsptr[curr_vsi].done = true;

    //If we're currently looking at the end vertex, we've found a path!
    if(vsptr[curr_vsi].v.id == end.id)
    {
      path_found = true;
    }

    //To carry on with the algorithm, find the vertex that currently is not done
    //and has the smallest f value.
    min_not_done_vsi = -1;
    while(++min_not_done_vsi < g.num_v && vsptr[min_not_done_vsi].done);

    //If we can't find any vertex that isn't done, we must be done looking.
    if(min_not_done_vsi == g.num_v)
    {
      path_found = true;
    }

    //Carrying on from before, find the not-done vertex with minimum f value.
    if(!path_found)
    {
      for(i = 0; i < g.num_v; i++)
      {
        if(!vsptr[i].done && vsptr[i].f < vsptr[min_not_done_vsi].f)
        {
          min_not_done_vsi = i;
        }
      }
    }

    //Update the vertex we're currently looking at.
    curr_vsi = min_not_done_vsi;
  }
  
  for(i = 0; i < g.num_v + 1; i++)
  { 
    backwards_path[i] = '\0';
  }

  //Find the entry in the vertex_status array corresponding to the end vertex. Step
  //backwards through predecessors to retrace the path from end to start, copying it
  //into a backwards_path array.
  end_vsi = get_vs_index_a(vsptr, g.num_v, end.id);
  i = 0;
  while(vsptr[end_vsi].pred_id != -1)
  {
    backwards_path[i++] = vsptr[end_vsi].v.id;
    end_vsi = get_vs_index_a(vsptr, g.num_v, vsptr[end_vsi].pred_id);
  }
  backwards_path[i++] = vsptr[end_vsi].v.id;
  backwards_path[i] = '\0';

  //Then, store the path in the given path variable, this time in forward order.
  for(i = 0; i < length(backwards_path); i++)
  {
    path[i] = backwards_path[length(backwards_path)-i-1];
  }
  path[i] = '\0';

  return EXIT_SUCCESS;
}
