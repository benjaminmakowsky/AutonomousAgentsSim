//Matthew Coffman - June 2018
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h>

//vertex_status structure: in the execution of Dijkstra's algorithm, we'll need to
//keep track of the following information for each vertex:
//its id,
//the id of its predecessor,
//the total weight currently ascribed to it,
//and a marker for whether or not we're done looking at it.
typedef struct vertex_status_d
{
  int id;
  int pred_id;
  int t_weight;
  bool done;
} vsd_t;

//Given the id of a vertex, finds the index of the vertex_status structure with the
//same id.
int get_vs_index_d(vsd_t *vsptr, int len, int id)
{
  int i = -1;

  while(++i < len && vsptr[i].id != id);
  
  return i;
}

//Computes the shortest path from the start vertex to the end vertex, and stores the
//path in the given path variable.
void dijkstra(graph_t g, vertex_t start, vertex_t end, int *path)
{
  int i, curr_vsi, min_not_done_vsi, end_vsi;
  int backwards_path[g.num_v+1];
  vsd_t vsptr[g.num_v];
  bool path_found = false;

  //If the start vertex can't be found in the given graph's array of vertices,
  //throw an error and return.
  if(!vertex_in_graph(g, start))
  {
    printf("Error: start vertex %d not found in graph.\n", start.id);
    path[0] = '\0';
    return;
  }

  //Likewise, if the end vertex can't be found, throw an error and return.
  if(!vertex_in_graph(g, end))
  {
    printf("Error: end vertex %d not found in graph.\n", end.id);
    path[0] = '\0';
    return;
  }

  //In the vacuous case that we're trying to find a path from a vertex to itself,
  //we're done! Our path has only one point, and we can return.
  if(start.id == end.id)
  {
    path[0] = start.id;
    path[1] = '\0';
    return;
  }

  //After allocating the required memory, clear the memory by setting every byte to 0.
  //Then, initialize each vertex_status structure. Each structure will contain its own
  //id, a dummy value for its predecessor, an initial total weight of INT_MAX (taking 
  //the place of infinity in this algorithm (exception is the start vertex, which has a
  //total weight of 0), and a 'done' status of false, indicating we haven't examined it. 
  memset(vsptr, 0, sizeof(vsd_t) * g.num_v);
  for(i = 0; i < g.num_v; i++)
  {
    if(g.vertices[i].id == start.id)
    {
      vsptr[i] = (vsd_t) {g.vertices[i].id, -1, 0, false};
      curr_vsi = i;
    }
    else
    {
      vsptr[i] = (vsd_t) {g.vertices[i].id, -2, INT_MAX, false};
    }
  }

  while(!path_found)
  { 
    //Loop through all the edges in the given graph. For each edge, check if one of
    //its vertices is the vertex we're currently looking at. If so, do stuff with the
    //other vertex.
    for(i = 0; i < g.num_e; i++)
    {
      int other_vsi = -1;
      
      if(vsptr[curr_vsi].id == g.edges[i].v1.id)
      {
        other_vsi = get_vs_index_d(vsptr, g.num_v, g.edges[i].v2.id);
      }

      if(vsptr[curr_vsi].id == g.edges[i].v2.id)
      {
        other_vsi = get_vs_index_d(vsptr, g.num_v, g.edges[i].v1.id);
      }

      //If the total weight of the other vertex is greater than that of the current
      //vertex plus the edge weight between them, update the other vertex's pred_id
      //and total weight accordingly.
      if(vsptr[other_vsi].t_weight > vsptr[curr_vsi].t_weight + g.edges[i].weight)
      {
        vsptr[other_vsi].t_weight = vsptr[curr_vsi].t_weight + g.edges[i].weight;
        vsptr[other_vsi].pred_id = vsptr[curr_vsi].id;
      }
    }

    //After checking all the edges in the graph, indicate that we are done looking at
    //this vertex, meaning we never need to check it again.
    vsptr[curr_vsi].done = true;
    
    //If we've just gotten done looking at the end vertex, we're done.
    if(vsptr[curr_vsi].id == end.id)
    {
      path_found = true;
    }
    else
    {
      //Check if there are any vertices still not looked at.
      min_not_done_vsi = -1;
      while(++min_not_done_vsi < g.num_v 
            && vsptr[min_not_done_vsi].done 
            && vsptr[min_not_done_vsi].t_weight != INT_MAX);
  
      //If all vertices are done, we're done.
      if(min_not_done_vsi == g.num_v)
      {
        path_found = true;
      }
      //Otherwise, find the vertex that isn't done and has the lowest total weight
      //currently ascribed to it.
      else
      {
        for(i = 0; i < g.num_v; i++)
        {
          if(!vsptr[i].done && vsptr[i].t_weight < vsptr[min_not_done_vsi].t_weight)
          {
            min_not_done_vsi = i;
          }
        }
      }

      //Update the vertex we're currently looking at.
      curr_vsi = min_not_done_vsi;
    }
  }

  for(i = 0; i < g.num_v + 1; i++)
  {
    backwards_path[i] = '\0';
  }

  //If the end vertex still hasn't been looked at, there must be no path from
  //start to end.
  end_vsi = get_vs_index_d(vsptr, g.num_v, end.id);
  if(vsptr[end_vsi].pred_id == -2)
  {
    printf("Error: no path from %d to %d\n", start.id, end.id);
    path[0] = '\0';
    return;
  }

  i = 0;
  //Work backwards through predecessors to copy the path into an array.
  while(vsptr[end_vsi].pred_id != -1)
  {
    backwards_path[i++] = vsptr[end_vsi].id;
    end_vsi = get_vs_index_d(vsptr, g.num_v, vsptr[end_vsi].pred_id);
  }
  
  //Include the start vertex as the beginning of the path.
  backwards_path[i++] = vsptr[end_vsi].id; 
  
  //Put a null character at the end of the path, This is so that when we try to 
  //display the path up in main() and print_path() we don't see more nodes that 
  //might be left over from the last time dijkstra() was called.
  backwards_path[i] = '\0'; 

  //Reverse the path so it's in the right order, from start to end, and store these
  //vertex id's to the given path array.
  for(i = 0; i < length(backwards_path); i++)
  {
    path[i] = backwards_path[length(backwards_path)-i-1];
  }

  //Put a null character at the end of the path, like above.
  path[i] = '\0';
}

//Generates a path from start to end that first goes through an unspecified
//number of points.
void dijkstraN(graph_t g, vertex_t start, vertex_t end, int *path, int n, ...)
{
  int i, j, index = 0, temppath[g.num_v+1];
  va_list list;
  vertex_t temp_v1, temp_v2 = start;

  path[index++] = start.id;

  va_start(list, n);
  for(i = 0; i < n; i++)
  {
    temp_v1 = temp_v2;
    temp_v2 = va_arg(list, vertex_t);

    dijkstra(g, temp_v1, temp_v2, temppath);
    j = 1;
    while(temppath[j] != '\0')
    {
      path[index++] = temppath[j++];
    }
  }

  dijkstra(g, temp_v2, end, temppath);
  j = 1;
  while(temppath[j] != '\0')
  {
    path[index++] = temppath[j++];
  }

  path[index] = '\0';

  va_end(list);
}

