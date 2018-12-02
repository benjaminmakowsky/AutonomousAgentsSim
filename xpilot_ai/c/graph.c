//Matthew Coffman - June 2018
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define MAX_EDGES 10000		//Note: 59818 maximum, otherwise seg faults
#define MAX_VERTS 5000

//vertex structure: a vertex has a unique id and an x- and y-coordinate
typedef struct vertex
{
  int id;
  int x;
  int y;
} vertex_t;

//edge structure: an edge has two vertices and a weight
typedef struct edge
{
  vertex_t v1;
  vertex_t v2;
  int weight;
} edge_t;

//graph structure: a graph comprises a set of unique vertices and edges, along
//with values num_v and num_e store the number of vertices and edges currently
//in the graph
typedef struct graph
{
  vertex_t vertices[MAX_VERTS];
  int num_v;
  edge_t edges[MAX_EDGES];
  int num_e;
} graph_t;

//computes the Euclidean distance between two points
int distance_formula(int x1, int y1, int x2, int y2)
{
  return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

//determines whether the given vertex is in the given graph
bool vertex_in_graph(graph_t g, vertex_t v)
{
  int i;

  for(i = 0; i < g.num_v; i++)
  {
    if(g.vertices[i].id == v.id)
    {
      return true;
    }
  }

  return false;
}

//determines whether the given integer is found in the given integer array
bool found_in_array(int arr[], int n)
{
  int i;

  for(i = 0; i < length(arr); i++)
  {
    if(arr[i] == n)
    {
      return true;
    }
  }

  return false;
}

//adds the given vertex to the given graph, after checking that the vertex
//has a unique id and does not overlap with another vertex
graph_t add_vertex(graph_t g, vertex_t v)
{
  int i;

  if(vertex_in_graph(g, v))
  {
    printf("Error: each vertex must have a unique id\n");
    return g;
  }

  for(i = 0; i < g.num_v; i++)
  {
    if(g.vertices[i].x == v.x && g.vertices[i].y == v.y)
    {
      printf("Error: two vertices cannot occupy the same location\n");
      return g;
    }
  }

  g.vertices[g.num_v++] = v;

  return g;
}

//adds an edge between the given vertices to the given graph, with weight
//equal to the distance between the two vertices, after checking that there
//isn't already such an edge in the graph
graph_t add_edge(graph_t g, vertex_t v1, vertex_t v2)
{
  int i, w;
  bool found_v1 = vertex_in_graph(g, v1);
  bool found_v2 = vertex_in_graph(g, v2);

  if(!found_v1)
  {
    printf("Error: vertex %d not found, couldn't add an edge\n", v1.id);
    return g;
  }

  if(!found_v2)
  {
    printf("Error: vertex %d not found, couldn't add an edge\n", v2.id);
    return g;
  }

  if(v1.id == v2.id)
  {
    printf("Error: can't add an edge from a vertex to itself\n");
    return g;
  }

  for(i = 0; i < g.num_e; i++)
  {
    int id1 = g.edges[i].v1.id;
    int id2 = g.edges[i].v2.id;
    if((id1 == v1.id && id2 == v2.id) || (id1 == v2.id && id2 == v1.id))
    {
      printf("An edge between %d and %d already exists!\n", v1.id, v2.id);
      return g;
    }
  }

  w = distance_formula(v1.x, v1.y, v2.x, v2.y);
  g.edges[g.num_e++] = (edge_t) {v1, v2, w};

  return g;
}

//removes the edge between the two given vertices from the given graph,
//after checking to ensure that such an edge exists
graph_t remove_edge(graph_t g, vertex_t v1, vertex_t v2)
{
  int i, edge_index = -1;
  bool found_v1 = vertex_in_graph(g, v1);
  bool found_v2 = vertex_in_graph(g, v2);

  if(!found_v1 || !found_v2)
  {
    printf("Error: vertex not found, couldn't remove an edge\n");
    return g;
  }

  if(v1.id == v2.id)
  {
    printf("Error: can't remove an edge from a vertex to itself\n");
    return g;
  }

  for(i = 0; i < g.num_e; i++)
  {
    int id1 = g.edges[i].v1.id;
    int id2 = g.edges[i].v2.id;

    if((id1 == v1.id && id2 == v2.id) || (id1 == v2.id && id2 == v1.id))
    {
      edge_index = i;
    }
  }

  if(edge_index != -1)
  {
    for(i = edge_index + 1; i < g.num_e; i++)
    {
      g.edges[i-1] = g.edges[i];
    }
    
    g.num_e--;
  }

  return g;
}

//removes the given vertex from the given graph, along with all edges that
//involved the given vertex
graph_t remove_vertex(graph_t g, vertex_t v)
{
  int i, vert_index;
  graph_t temp_g = g;

  if(!vertex_in_graph)
  {
    printf("Error: vertex %d not found, couldn't remove the vertex\n", v.id);
    return g;
  }

  for(i = 0; i < temp_g.num_e; i++)
  {
    if(temp_g.edges[i].v1.id == v.id || temp_g.edges[i].v2.id == v.id)
    {
      temp_g = remove_edge(temp_g, temp_g.edges[i].v1, temp_g.edges[i].v2);
      i--;
    }
  }
  
  vert_index = temp_g.num_v;
  for(i = 0; i < temp_g.num_v - 1; i++)
  {
    if(temp_g.vertices[i].id == v.id)
    {
      vert_index = i;
    }

    if(i >= vert_index)
    {
      temp_g.vertices[i] = temp_g.vertices[i+1];
    }
  }

  temp_g.num_v--;
  return temp_g;
}

//given a pointer to a path of integers, prints out those integers in order
void print_path(int *p)
{
  int i = 0, l = length(p);

  if(l == 0)
  {
    return;
  }

  printf("Path from %d to %d:  ", p[0], p[l-1]);
  for(i = 0; i < l-1; i++)
  {
    printf("%d -> ", p[i]);
  }
  printf("%d\n", p[l-1]);
}

//computes the length of an array of integers by looking for a null terminator
int length(int arr[])
{
  int count = 0;

  while(arr[count] != 0)
  {
    count++;
  }

  return count;
}

