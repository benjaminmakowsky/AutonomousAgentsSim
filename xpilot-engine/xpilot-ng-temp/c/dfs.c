#include <stdio.h>
#include <stdbool.h>
//#include "graph.c"

typedef struct vertex_status_f
{
  int id;
  struct vertex_status_f *prevptr;
  struct vertex_status_f *nextptr;
} vsf_t;

int dfs_helper(graph_t g, int id, int *vis_vert)
{
  int i;
  int next_id = -1;

  for(i = 0; i < g.num_e; i++)
  {
    if(g.edges[i].v1.id == id)
      next_id = g.edges[i].v2.id;

    if(g.edges[i].v2.id == id)
      next_id = g.edges[i].v1.id;

    if(next_id != -1 && !found_in_array(vis_vert, next_id))
      return next_id;
  }

  return -1;
}

void dfs(graph_t g, vertex_t start, vertex_t end, int *path)
{
  int visited_vertices[g.num_v];
  int i, num_vv = 0, next_id;
  vsf_t *beginptr = NULL;
  vsf_t *currptr = NULL;
  bool path_found = false;

  if(start.id == end.id)
  {
    path[0] = start.id;
    path[1] = '\0';
    return;
  }

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

  beginptr = malloc(sizeof(vsf_t));
  if(!beginptr)
  {
    perror("Error: couldn't allocate memory");
    abort();
  }
  *beginptr = (vsf_t) {start.id, NULL, NULL};

  currptr = beginptr;

  for(i = 0; i < g.num_v; i++)
    visited_vertices[i] = '\0';
  visited_vertices[num_vv++] = start.id;

  while(!path_found && num_vv < g.num_v)
  {
    next_id = dfs_helper(g, currptr->id, visited_vertices);

    if(next_id != -1)
    {
      currptr->nextptr = malloc(sizeof(vsf_t));
      if(!currptr->nextptr)
      {
        perror("Error: couldn't allocate memory");
        abort();
      }
      *currptr->nextptr = (vsf_t) {next_id, currptr, NULL};
      visited_vertices[num_vv++] = next_id;
      currptr = currptr->nextptr;
    }
    else
    {
      if(currptr->id == start.id)
      {
        printf("Error: no path from %d to %d\n", start.id, end.id);
        path[0] = '\0';
        return;
      }
      currptr = currptr->prevptr;
      free(currptr->nextptr);
    }

    if(currptr->id == end.id)
      path_found = true;
  }

  currptr = beginptr;
  i = 0;
  while(currptr)
  {
    path[i++] = currptr->id;
    currptr = currptr->nextptr;
  }
  path[i] = '\0';

  free(beginptr);
}

