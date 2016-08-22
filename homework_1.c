/* Name: Lalchandra Rampersaud 
 * Course: I96 (Fundamental algorithms)
 * Homework #1*/
 
 
 
/* compiles with command line  gcc xlibdemo.c -lX11 -lm -L/usr/X11R6/lib */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define IN 999999999
#define N 1000


typedef int bool;
#define true 1
#define false 0

Display *display_ptr;
Screen *screen_ptr;
int screen_num;
char *display_name = NULL;
unsigned int display_width, display_height;

Window win;
int border_width;
unsigned int win_width, win_height;
int win_x, win_y;

XWMHints *wm_hints;
XClassHint *class_hints;
XSizeHints *size_hints;
XTextProperty win_name, icon_name;
char *win_name_string = "Dijsktra";
char *icon_name_string = "Icon for Dijsktra Window";

XEvent report;

GC gc, gc_yellow, gc_red, gc_grey;
unsigned long valuemask = 0;
XGCValues gc_values, gc_yellow_values, gc_red_values, gc_grey_values;
Colormap color_map;
XColor tmp_color1, tmp_color2;

int max(int a, int b)
{
	if(a>b)
	{return a;}
	else
	{return b;}
}
int min(int a, int b)
{
	if(a>b)
	{return b;}
	else
	{return a;}
}

typedef struct Point
{
    int x;
    int y;
}Point;

typedef struct Triangle
{
	struct Point *a;
	struct Point *b;
	struct Point *c;
}Triangle;

struct Node 
{
	struct Point *element;
	int visited;
	struct Edge *edges;
	struct Node *next;
	int Cost;
	int position;
} ;

struct Edge 
{
	struct Node *connectsTo;
	struct Edge *next;
	int distance;
};
struct Graph 
{
	struct Node *vertices;
};

void RedrawWindow(Display *display, Window window, struct Graph *graph);
struct Graph *MakeGraph();
void dijsktra(struct Graph *graph, struct Node *Start, struct Node *End, Display *display, Window window);
int doesFileExist(const char *filename);
bool IsNotPointInBox(int X1, int Y1, int X2, int Y2, int Px, int Py);
int CalculateDistance(int x1, int y1, int x2, int y2);
void DrawTriangle2(Display *display, Drawable d, GC gc, struct Triangle triangle);
bool doIntersect(struct Point *p1, struct Point *q1, struct Point *p2, struct Point *q2);
int orientation(struct Point *p, struct Point *q, struct Point *r);
bool onSegment(struct Point *p, struct Point *q, struct Point *r);


//Global variables
int triangleCount=0;
int maxX=0;
int maxY=0;
int boundX = 0;
int boundY = 0;
struct Triangle Triangles[N];
struct Node *FirstClick = NULL;
struct Node *SecondClick = NULL;


int main(int argc, char **argv)
{
	
	struct Point *firstPoint;
	struct Point *secondPoint;
	struct Point *thirdPoint;
	 
	char filename[255]="/home/ravi/Desktop/test_in";
	char line[80];
	FILE *fr;	
	
	printf("Enter file name: ");
	scanf("%s", filename);

	
   if(doesFileExist(filename)==1)
   {
		fr = fopen(filename,"rt"); 
		while(fgets(line, 80, fr) != NULL && triangleCount<N)
		{
			
			firstPoint = malloc(sizeof(Point));
			secondPoint = malloc(sizeof(Point));
			thirdPoint = malloc(sizeof(Point));

			sscanf (line, "T (%d, %d) (%d, %d) (%d, %d)", &firstPoint->x, &firstPoint->y, &secondPoint->x, &secondPoint->y, &thirdPoint->x, &thirdPoint->y);

			Triangles[triangleCount].a=firstPoint;
			Triangles[triangleCount].b=secondPoint;
			Triangles[triangleCount].c=thirdPoint;
			triangleCount=triangleCount+1;
			
			if(firstPoint->x > maxX)
			{
				maxX = firstPoint->x;
			}
			if(secondPoint->x > maxX)
			{
				maxX = secondPoint->x;
			}
			if(thirdPoint->x > maxX)
			{
				maxX = thirdPoint->x;
			}
			
			if(firstPoint->y > maxY)
			{
				maxY = firstPoint->y;
			}
			if(secondPoint->y > maxY)
			{
				maxY = secondPoint->y;
			}
			if(thirdPoint->y > maxY)
			{
				maxY = thirdPoint->y;
			}
			
		}
		fclose(fr);
	}
	else
	{
		printf("Invalid file name");
		//scanf("%s", filename);
		exit(0);
	}
	
	boundX = maxX*0.1;
	boundY = maxY*0.1;
	
	
	struct Graph *graph;
	int i;
	
	
	for(i=0; i<triangleCount; i=i+1)
	{
		Triangles[i].a->x = Triangles[i].a->x + boundX;
		Triangles[i].a->y = Triangles[i].a->y + boundY;
		
		Triangles[i].b->x = Triangles[i].b->x + boundX;
		Triangles[i].b->y = Triangles[i].b->y + boundY;
		
		Triangles[i].c->x = Triangles[i].c->x + boundX;
		Triangles[i].c->y = Triangles[i].c->y + boundY;
	}
	
	maxX=maxX+boundX+boundX;
	maxY = maxY+boundY+boundY;
	
	
  /* opening display: basic connection to X Server */
  if( (display_ptr = XOpenDisplay(display_name)) == NULL )
    { printf("Could not open display. \n"); exit(-1);}
    

  screen_num = DefaultScreen( display_ptr );
  screen_ptr = DefaultScreenOfDisplay( display_ptr );
  color_map  = XDefaultColormap( display_ptr, screen_num );
  
  display_width  = DisplayWidth( display_ptr, screen_num );
  display_height = DisplayHeight( display_ptr, screen_num );

  /* creating the window */
  
  border_width = 10;
  win_x = 0; win_y = 0;


 /*bound window*/
  win_width = maxX+boundX;
  win_height = maxY+boundY;
  
  win= XCreateSimpleWindow( display_ptr, RootWindow( display_ptr, screen_num), win_x, win_y, win_width, win_height, border_width,
                            BlackPixel(display_ptr, screen_num), WhitePixel(display_ptr, screen_num) );
                            
                            
  /* now try to put it on screen, this needs cooperation of window manager */
  size_hints = XAllocSizeHints();
  wm_hints = XAllocWMHints();
  class_hints = XAllocClassHint();
  if( size_hints == NULL || wm_hints == NULL || class_hints == NULL )
    { printf("Error allocating memory for hints. \n"); exit(-1);}

  size_hints -> flags = PPosition | PSize | PMinSize  ;
  size_hints -> min_width = 60;
  size_hints -> min_height = 60;

  XStringListToTextProperty( &win_name_string,1,&win_name);
  XStringListToTextProperty( &icon_name_string,1,&icon_name);
  
  wm_hints -> flags = StateHint | InputHint ;
  wm_hints -> initial_state = NormalState;
  wm_hints -> input = False;

  class_hints -> res_name = "x_use_example";
  class_hints -> res_class = "examples";

  XSetWMProperties( display_ptr, win, &win_name, &icon_name, argv, argc,size_hints, wm_hints, class_hints );

  /* what events do we want to receive */
  XSelectInput( display_ptr, win, ExposureMask | StructureNotifyMask | ButtonPressMask );
  
  /* finally: put window on screen */
  XMapWindow( display_ptr, win );

  XFlush(display_ptr);

  /* create graphics context, so that we may draw in this window */
  gc = XCreateGC( display_ptr, win, valuemask, &gc_values);
  XSetForeground( display_ptr, gc, BlackPixel( display_ptr, screen_num ) );
  XSetLineAttributes( display_ptr, gc, 1, LineSolid, CapNotLast, JoinMiter);
  
  
  /* and three other graphics contexts, to draw in yellow and red and grey*/
  gc_yellow = XCreateGC( display_ptr, win, valuemask, &gc_yellow_values);
  XSetLineAttributes(display_ptr, gc_yellow, 2, LineSolid,CapRound, JoinRound);
  if( XAllocNamedColor( display_ptr, color_map, "yellow", 
			&tmp_color1, &tmp_color2 ) == 0 )
    {printf("failed to get color yellow\n"); exit(-1);} 
  else
    XSetForeground( display_ptr, gc_yellow, tmp_color1.pixel );
    
    
    
  gc_red = XCreateGC( display_ptr, win, valuemask, &gc_red_values);
  XSetLineAttributes( display_ptr, gc_red, 2, LineSolid, CapRound, JoinRound);
  if( XAllocNamedColor( display_ptr, color_map, "red", 
			&tmp_color1, &tmp_color2 ) == 0 )
    {printf("failed to get color red\n"); exit(-1);} 
  else
    XSetForeground( display_ptr, gc_red, tmp_color1.pixel );
    
    
  gc_grey = XCreateGC( display_ptr, win, valuemask, &gc_grey_values);
  if( XAllocNamedColor( display_ptr, color_map, "light grey", 
			&tmp_color1, &tmp_color2 ) == 0 )
    {printf("failed to get color grey\n"); exit(-1);} 
  else
    XSetForeground( display_ptr, gc_grey, tmp_color1.pixel );

  /* and now it starts: the event loop */
  while(1)
    { 
		XNextEvent( display_ptr, &report );
	    switch( report.type )
		{
		case Expose:
			graph = MakeGraph();
			RedrawWindow(display_ptr, win, graph);
			 break;
		case ConfigureNotify:
		  /* This event happens when the user changes the size of the window*/
		  win_width = report.xconfigure.width;
		  win_height = report.xconfigure.height;
		  break;
		case ButtonPress:
		  /* This event happens when the user pushes a mouse button. I draw
			a circle to show the point where it happened, but do not save 
			the position; so when the next redraw event comes, these circles
		disappear again. */
		  {  
			int x, y;
			x = report.xbutton.x;
			y = report.xbutton.y;
			if (report.xbutton.button == Button1 )
			{
				
				bool found= false;
				i=0;
				while(i<triangleCount && found == false)
				{
					if(isInside(Triangles[i].a->x, Triangles[i].a->y, Triangles[i].b->x, Triangles[i].b->y, Triangles[i].c->x, Triangles[i].c->y, x, y))
					 found=true;
					i=i+1;
					
				}
				
				if(found==false)
				{
					found = IsNotPointInBox(boundX, boundY, maxX, maxY,x,y);
				}
				
				if(found == false)
				{
					if(graph->vertices != NULL)
					{
						struct Node *node = malloc(sizeof(struct Node));
						node->element = malloc(sizeof(struct Point));
						node->element->x = x;
						node->element->y = y;
						node->edges = NULL;
						node->Cost = IN;
						node->visited=0;
						
						
						if(FirstClick == NULL)
						{
							FirstClick = node;
						}
						else
						{
							if(SecondClick == NULL)
							{
								SecondClick = node;
							}
							else
							{		
								FirstClick=NULL;
								SecondClick=NULL;
							}
						}
					}
				}
				else
				{
					if(FirstClick != NULL && SecondClick != NULL)
						{
							FirstClick=NULL;
							SecondClick=NULL;
							
						}
				}
				

				graph = MakeGraph();
				RedrawWindow(display_ptr, win, graph);
			}
			else
			{
				XCloseDisplay(display_ptr);
			}

		  }
		  break;
		default:
	  /* this is a catch-all for other events; it does not do anything.
			 One could look at the report type to see what the event was */ 
		  break;
		}

    }
  exit(0);
}


struct Graph *MakeGraph()
{
	//Construct all vertex nodes
	struct Graph *graph = malloc(sizeof(struct Graph));
	graph->vertices=NULL;
	//printf("%p\n", graph);
	int i;
	int j;
	
	int foundA=0;
	int foundB=0;
	int foundC=0;
	
	for(i=0; i<triangleCount; i=i+1)
	{
		foundA=0;
		foundB=0;
		foundC=0;
		for(j=0; j<triangleCount; j=j+1)
		{
			if(j!=i)
			{
				if(isInside(Triangles[j].a->x, Triangles[j].a->y, Triangles[j].b->x, Triangles[j].b->y, Triangles[j].c->x, Triangles[j].c->y, Triangles[i].a->x, Triangles[i].a->y))
				{
					foundA=1;
				}
				if(isInside(Triangles[j].a->x, Triangles[j].a->y, Triangles[j].b->x, Triangles[j].b->y, Triangles[j].c->x, Triangles[j].c->y, Triangles[i].b->x, Triangles[i].b->y))
				{
					foundB=1;
				}
				if(isInside(Triangles[j].a->x, Triangles[j].a->y, Triangles[j].b->x, Triangles[j].b->y, Triangles[j].c->x, Triangles[j].c->y, Triangles[i].c->x, Triangles[i].c->y))
				{
					foundC=1;
				}
			}
		}
		struct Node *node = malloc(sizeof(struct Node));
		if(foundA==0)
		{
			
			node->element = Triangles[i].a;
			node->visited = 0;
			node->next = NULL;
			node->edges = NULL;
			node->Cost = IN;
			
			
			
			if(graph->vertices == NULL)
			{
				graph->vertices = node;
			}
			else
			{
				node->next = graph->vertices;
				graph->vertices = node;
			}
		}
		
		if(foundB==0)
		{
			node = malloc(sizeof(struct Node));
			node->element = Triangles[i].b;
			node->visited = 0;
			node->next = NULL;
			node->edges = NULL;
			node->Cost = IN;
			
			if(graph->vertices == NULL)
			{
				graph->vertices = node;
			}
			else
			{
				node->next = graph->vertices;
				graph->vertices = node;
			}
		}
		
		if(foundC==0)
			{
			node = malloc(sizeof(struct Node));
			node->element = Triangles[i].c;
			node->visited = 0;
			node->next = NULL;
			node->edges = NULL;
			node->Cost = IN;
			
			if(graph->vertices == NULL)
			{
				graph->vertices = node;
			}
			else
			{
				node->next = graph->vertices;
				graph->vertices = node;
			}
		}

	}
	
	//printf("%p", graph);
	//Construct all edges
	if(graph->vertices != NULL)
	{
		struct Node *node = graph->vertices;
		while(node != NULL)
		{
			
			struct Node *innerNode = graph->vertices;
			while(innerNode != NULL)
			{
				//any triangles in between node and inner node?
				i=0;
				bool found = false;
				if(node != innerNode)
				{
					while(i<triangleCount && found == false)
					{
					
						if(doIntersect(node->element, innerNode->element, Triangles[i].a, Triangles[i].b) == true)
							found=true;
						else
							if(doIntersect(node->element, innerNode->element, Triangles[i].a, Triangles[i].c) == true)
								found=true;
							else
								if(doIntersect(node->element, innerNode->element, Triangles[i].c, Triangles[i].b) == true)
								 found=true;
						i=i+1;
					}
					

					if(found == false)
					{
						//ADD edge to node
						struct Edge *edge = malloc(sizeof(struct Edge));
						edge->connectsTo = innerNode;
						edge->distance = CalculateDistance(node->element->x, node->element->y, innerNode->element->x, innerNode->element->y);
						edge->next = NULL;
						if(node->edges == NULL)
						{
							node->edges = edge;
						}
						else
						{
							edge->next = node->edges;
							node->edges = edge;
						}
					}																
				}
				
				innerNode = innerNode->next;
			}
			node = node->next;
		}
		
		
		//CONSTRUCT ALL EDGES FROM START POINT TO ALL OTHER POINTS
		if(FirstClick != NULL)
		{
			node = graph->vertices;
			while(node != NULL)
			{
				//any triangles in between node and inner node?
				i=0;
				bool found = false;
				while(i<triangleCount && found == false)
				{
				
					if(doIntersect(node->element, FirstClick->element, Triangles[i].a, Triangles[i].b) == true)
						found=true;
					else
						if(doIntersect(node->element, FirstClick->element, Triangles[i].a, Triangles[i].c) == true)
							found=true;
						else
							if(doIntersect(node->element, FirstClick->element, Triangles[i].c, Triangles[i].b) == true)
							 found=true;
					i=i+1;
					
				}

				if(found == false)
				{
					//ADD edge to node
					struct Edge *edge = malloc(sizeof(struct Edge));
					edge->connectsTo = node;
					edge->distance = CalculateDistance(node->element->x, node->element->y, FirstClick->element->x, FirstClick->element->y);
					if(FirstClick->edges == NULL)
					{
						FirstClick->edges = edge;
					}
					else
					{
						edge->next = FirstClick->edges;
						FirstClick->edges = edge;
					}
				}
				node = node->next;
			}
		}
		
		
		
		
		//CONSTRUCT ALL EDGES FROM All POINTS TO END
		if(SecondClick != NULL)
		{
			node = graph->vertices;
			while(node != NULL)
			{
				//any triangles in between node and inner node?
				i=0;
				bool found = false;
				while(i<triangleCount && found == false)
				{
				
					if(doIntersect(node->element, SecondClick->element, Triangles[i].a, Triangles[i].b) == true)
						found=true;
					else
						if(doIntersect(node->element, SecondClick->element, Triangles[i].a, Triangles[i].c) == true)
							found=true;
						else
							if(doIntersect(node->element, SecondClick->element, Triangles[i].c, Triangles[i].b) == true)
							 found=true;
					i=i+1;
					
				}

				if(found == false)
				{
					//ADD edge to node
					struct Edge *edge = malloc(sizeof(struct Edge));
					edge->connectsTo = SecondClick;
					edge->distance = CalculateDistance(node->element->x, node->element->y, SecondClick->element->x, SecondClick->element->y);
					if(node->edges == NULL)
					{
						node->edges = edge;
					}
					else
					{
						edge->next = node->edges;
						node->edges = edge;
					}
				}
				node = node->next;
			}
			
			//make edge from start to end if exists
			bool found = false;
			i=0;
			while(i<triangleCount && found == false)
			{
			
				if(doIntersect(FirstClick->element, SecondClick->element, Triangles[i].a, Triangles[i].b) == true)
					found=true;
				else
					if(doIntersect(FirstClick->element, SecondClick->element, Triangles[i].a, Triangles[i].c) == true)
						found=true;
					else
						if(doIntersect(FirstClick->element, SecondClick->element, Triangles[i].c, Triangles[i].b) == true)
							found=true;
				i=i+1;
				//printf("%d\n", found);
			}
			
			if(found == false)
			{
				//ADD edge to node
				struct Edge *edge = malloc(sizeof(struct Edge));
				edge->connectsTo = SecondClick;
				edge->distance = CalculateDistance(FirstClick->element->x, FirstClick->element->y, SecondClick->element->x, SecondClick->element->y);
				if(FirstClick->edges == NULL)
				{
					FirstClick->edges = edge;
				}
				else
				{
					edge->next = FirstClick->edges;
					FirstClick->edges = edge;
				}
			}
			
			
			
		}
	}
	return graph;
	//printf("%p\n", graph->vertices);
} 

void RedrawWindow(Display *display, Window window, struct Graph *graph)
{
	XClearWindow(display, window);
	XDrawLine(display_ptr, win, gc, boundX, boundY, maxX, boundY);
	XDrawLine(display_ptr, win, gc, boundX, boundY, boundX, maxY);
	XDrawLine(display_ptr, win, gc, boundX, maxY, maxX, maxY);
	XDrawLine(display_ptr, win, gc, maxX, boundY, maxX, maxY);
	
	XFillRectangle(display,win,gc,0,0,maxX+boundX,boundY);
	XFillRectangle(display,win,gc,0,0,boundX,maxY+boundY);
	XFillRectangle(display,win,gc,0,maxY,boundX+maxX,maxY+boundY);
	XFillRectangle(display,win,gc,maxX,0,boundX,maxY+boundY);
	

	int i;
	
	
	
	for(i=0; i<triangleCount; i=i+1)
	{			
		DrawTriangle2(display_ptr, win, gc, Triangles[i]);
	}
	
	
	
	if(graph->vertices != NULL)
	{
		struct Node *node = graph->vertices;
		
		while(node != NULL)
		{
			
			struct Edge *edge = node->edges;
		
			while(edge != NULL)
			{
				if(SecondClick != NULL && edge->connectsTo == SecondClick)
				{
					XDrawLine(display_ptr, win, gc_grey, node->element->x, node->element->y, edge->connectsTo->element->x, edge->connectsTo->element->y);
				}
				else
				{
					XDrawLine(display_ptr, win, gc, node->element->x, node->element->y, edge->connectsTo->element->x, edge->connectsTo->element->y);
				}
				edge = edge->next;
			}
			node = node->next; 
		}
	}
	
	if(FirstClick != NULL)
	{
		struct Edge *edge = FirstClick->edges;
				
		while(edge != NULL)
		{
			
			XDrawLine(display_ptr, win, gc_yellow, FirstClick->element->x, FirstClick->element->y, edge->connectsTo->element->x, edge->connectsTo->element->y);
			edge = edge->next;
			
		}
		
		
	}

	if(SecondClick != NULL)
	{
		
		struct Edge *edge = SecondClick->edges;
				
		while(edge != NULL)
		{
			
			XDrawLine(display_ptr, win, gc_yellow, SecondClick->element->x, SecondClick->element->y, edge->connectsTo->element->x, edge->connectsTo->element->y);
			edge = edge->next;
			
		}
		dijsktra(graph, FirstClick, SecondClick, display, window);
	}
}


float area(int x1, int y1, int x2, int y2, int x3, int y3)
{
   return abs((x1*(y2-y3) + x2*(y3-y1)+ x3*(y1-y2))/2.0);
}

bool isInside(int x1, int y1, int x2, int y2, int x3, int y3, int x, int y)
{   
   /* Calculate area of triangle ABC */
   float A = area (x1, y1, x2, y2, x3, y3);
 
   /* Calculate area of triangle PBC */  
   float A1 = area (x, y, x2, y2, x3, y3);
 
   /* Calculate area of triangle PAC */  
   float A2 = area (x1, y1, x, y, x3, y3);
 
   /* Calculate area of triangle PAB */   
   float A3 = area (x1, y1, x2, y2, x, y);
   
   /* Check if sum of A1, A2 and A3 is same as A */
   return (A == A1 + A2 + A3);
}


void dijsktra(struct Graph *graph, struct Node *Start, struct Node *End, Display *display, Window window)
{
	int d, i, min, minPos;
	int currentNode=0;
	struct Node *discovered[N];
	int prev[N];
	
	discovered[currentNode] = Start;
	discovered[currentNode]->Cost = 0;	
	discovered[currentNode]->position = currentNode;	
	prev[currentNode] = -1;	
	currentNode = currentNode+1;
	
	struct Node *node = graph->vertices;
	while(node != NULL)
	{
		discovered[currentNode] = node;
		discovered[currentNode]->Cost = IN;	
		discovered[currentNode]->position = currentNode;	
		prev[currentNode] = -1;	
		currentNode = currentNode+1;
		node = node->next;
	}
	discovered[currentNode] = End;
	discovered[currentNode]->Cost = IN;	
	discovered[currentNode]->position = currentNode;	
	prev[currentNode] = -1;	
	currentNode = currentNode+1;
	
	struct Edge *edge ;
	min =0;
	minPos=0;

	while(Start != End && min < IN)
	{
		Start->visited = 1;
		edge = Start->edges;

		
		while(edge != NULL)
		{
			
			d = Start->Cost + edge->distance;
			if(d < edge->connectsTo->Cost)
			{
				edge->connectsTo->Cost = d;
				prev[edge->connectsTo->position] = minPos;
			}
				
			edge = edge->next;
			
			
		}
		
		min = IN;
		//take the minimum not visited
		for(i=0; i<currentNode; i=i+1)
		{
			
			if(discovered[i]->visited == 0 && discovered[i]->Cost < min)
			{
				Start = discovered[i];
				min = discovered[i]->Cost;
				minPos = i;
				
			}
		}
	}
	
	
	int previousElement = prev[currentNode-1];
	if(previousElement != -1) //no path found
	{
	
		while(previousElement > 0)
		{
			XDrawLine(display_ptr, win, gc_red, End->element->x, End->element->y, discovered[previousElement]->element->x, discovered[previousElement]->element->y);
			
			End = discovered[previousElement];
			
			previousElement = prev[previousElement];
		}
		XDrawLine(display_ptr, win, gc_red, End->element->x, End->element->y, discovered[0]->element->x, discovered[0]->element->y);
	}
	else
	{
		XDrawString(display, win, gc_red, 0, 0, "No path found", 14);
	}

}


int doesFileExist(const char *filename) {
    /*struct stat st;
    int result = stat(filename, &st);
    return result == 0;*/
    return 1;
}


bool IsNotPointInBox(int X1, int Y1, int X2, int Y2, int Px, int Py)
{
	if(Px > X1 && Py > Y1 && Px < X2 && Py < Y2)
	{
		return false;
	}
	else
	{
		return true;
	}
}


bool onSegment(struct Point *p, struct Point *q, struct Point *r)
{
    if (q->x <= max(p->x, r->x) && q->x >= min(p->x, r->x) &&
        q->y <= max(p->y, r->y) && q->y >= min(p->y, r->y))
       return true;
 
    return false;
}

int orientation(struct Point *p, struct Point *q, struct Point *r)
{
    int val = (q->y - p->y) * (r->x - q->x) -
              (q->x - p->x) * (r->y - q->y);
 
    if (val == 0) return 0;  
    
   
 
    return (val > 0)? 1: 2; 
}

bool doIntersect(struct Point *p1, struct Point *q1, struct Point *p2, struct Point *q2)
{
	
	if(p1 != p2 && p1 != q2 && q1 != p2 && q1 != q2)
	{		
		int o1 = orientation(p1, q1, p2);
		int o2 = orientation(p1, q1, q2);
		int o3 = orientation(p2, q2, p1);
		int o4 = orientation(p2, q2, q1);

		if (o1 != o2 && o3 != o4)
			return true;

		if (o1 == 0 && onSegment(p1, p2, q1)) return true;

		if (o2 == 0 && onSegment(p1, q2, q1)) return true;

		if (o3 == 0 && onSegment(p2, p1, q2)) return true;
	 
		if (o4 == 0 && onSegment(p2, q1, q2)) return true;
	 
		return false;
	}
	return false;
}

	
void DrawTriangle2(Display *display, Drawable d, GC gc, struct Triangle triangle)
	{
		XDrawLine(display_ptr, win, gc, triangle.a->x, triangle.a->y, triangle.b->x, triangle.b->y);
		XDrawLine(display_ptr, win, gc, triangle.a->x, triangle.a->y, triangle.c->x, triangle.c->y);
		XDrawLine(display_ptr, win, gc, triangle.c->x, triangle.c->y, triangle.b->x, triangle.b->y);
		
		
		XPoint v[3];
		v[0].x = triangle.a->x;
		v[0].y = triangle.a->y;
		
		v[1].x = triangle.b->x;
		v[1].y = triangle.b->y;
		
		v[2].x = triangle.c->x;
		v[2].y = triangle.c->y;
		
		XFillPolygon(display,d,gc,v,3,Complex,CoordModeOrigin);

	}	
	
	
int CalculateDistance(int x1, int y1, int x2, int y2)
{	
	int diffx = x1 - x2;
	int diffy = y1 - y2;
	int diffx_sqr = pow(diffx,2);
	int diffy_sqr = pow(diffy, 2);
	int distance = sqrt(diffx_sqr + diffy_sqr);

return distance;
}	

