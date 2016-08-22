/* Name: Lalchandra Rampersaud
 * Course: I96 (Fundamental algorithms)
 * Homework #3*/


#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <limits>
#include <fstream>
using namespace std;


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
char *win_name_string = "Corridor Problem";
char *icon_name_string = "Icon for Window";

XEvent report;

GC gc, gc_yellow, gc_red, gc_grey, gc_green, gc_blue;
unsigned long valuemask = 0;
XGCValues gc_values, gc_yellow_values, gc_red_values, gc_grey_values, gc_green_values, gc_blue_values;
Colormap color_map;
XColor tmp_color1, tmp_color2;


struct Line
{
    int startPoint;
    int p1;
    int p2;
    int orientation; /*0 for horizontal, 1 for vertical*/
    struct Line* next;
};

struct Node
{
    struct Line *a;
    struct Line *b;
    int X;
    int Y;
    struct Node *next;
    struct Edge *edges;
    int edgeCount;
    bool visited;
} ;



struct Edge
{
    struct Node *connectsTo;
    struct Node *from;
    struct Edge *next;
    int distance;
    int included;
};
struct Graph
{
    struct Node *nodes;
};

struct MSTEdge;
struct MSTNode
{
	Line* line;
	MSTEdge* edges;


};
struct MSTEdge
{
	int Distance;
	MSTEdge* next;
	MSTNode* mstNode;

	int X1;
	int Y1;
	int X2;
	int Y2;
};

struct Tour
{
    struct Node *nodes;
    int Cost;
};

bool doIntersect(int p1x, int p1y, int q1x, int q1y, int p2x, int p2y, int q2x, int q2y);
int CalculateDistance(int x1, int y1, int x2, int y2);
int random_at_most(int max);
bool IsSolutionValid(Tour *tempSolution);
void RedrawWindow(Display *display, Window window, Tour* solution);
void SetNeighbours(Tour *tour);
Tour* HillClimbing();
int* primMST(int** graph, int V);
void PrintMST(int* parent, std::vector<Node*> head, GC gc_colorChosen);
int GetTotalMST(int* parent, int V, int** graph);
int CalculateLowerBoundX();
int CalculateLowerBoundY();
int DrawSolution(Display *display, Window window, Tour* solution);


struct Node* NodeArray;
struct Line *LineXArray;
struct Line *LineYArray;
int NodeCount=0;



bool fexists(const char *filename)
{
  ifstream ifile(filename);
  return ifile;
}


int main(int argc, char** argv) {


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
  win_width = 800;
  win_height = 800;

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
  XSetLineAttributes( display_ptr, gc_red, 1, LineSolid, CapRound, JoinRound);
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


  gc_green = XCreateGC( display_ptr, win, valuemask, &gc_green_values);
  XSetLineAttributes( display_ptr, gc_green, 4, LineSolid, CapRound, JoinRound);
  if( XAllocNamedColor( display_ptr, color_map, "green",
			&tmp_color1, &tmp_color2 ) == 0 )
    {printf("failed to get color green\n"); exit(-1);}
  else
    XSetForeground( display_ptr, gc_green, tmp_color1.pixel );


  gc_blue = XCreateGC( display_ptr, win, valuemask, &gc_blue_values);
  XSetLineAttributes( display_ptr, gc_blue, 4, LineSolid, CapRound, JoinRound);
  if( XAllocNamedColor( display_ptr, color_map, "light blue",
			&tmp_color1, &tmp_color2 ) == 0 )
    {printf("failed to get color blue\n"); exit(-1);}
  else
    XSetForeground( display_ptr, gc_blue, tmp_color1.pixel );

  /* and now it starts: the event loop */

  struct Line *line;
      struct Line *lineHelper;
      struct Node* node;
      struct Node* nodeHelper;
      struct Edge* edge;
      int i, x, max;
      max=600;
      int P1, P2, P3;
      char orientation;

  	char filename[255]="/home/ravi/Desktop/test_in3";
  	if(argc > 2)
  		{
  			strcpy(filename, argv[1]);
  		}

  	char lineToRead[80];
  	FILE *fr;
  	if(fexists(filename))
  	{
		fr = fopen(filename,"rt");
		while(fgets(lineToRead, 80, fr) != NULL)
		{
			sscanf (lineToRead, "%c %d, %d, %d", &orientation, &P1, &P2, &P3);
			line  = new Line();

			if(orientation == 'v')
			{
				line->orientation=0;
			}
			else
			{
				line->orientation = 1;
			}
			line->startPoint=P1;
			line->p1=P2;
			line->p2=P3;
			line->next = NULL;


			if(line->orientation ==0)
			{
				if(LineXArray == NULL)
				{
					LineXArray = line;
				}
				else
				{
					line->next = LineXArray;
					LineXArray = line;
				}
			}
			else
			{
				if(LineYArray == NULL)
				{
					LineYArray = line;
				}
				else
				{
					line->next = LineYArray;
					LineYArray = line;
				}
			}
		}
	}



    srand((unsigned)time(0));

    line=LineXArray;
    while(line != NULL)
    {
        lineHelper = LineYArray;
        while(lineHelper != NULL)
        {
            if(doIntersect(line->startPoint, line->p1, line->startPoint, line->p2, lineHelper->p1, lineHelper->startPoint, lineHelper->p2, lineHelper->startPoint))
            {
                node = new Node();
                node->a = line;
                node->b = lineHelper;
                node->X = line->startPoint;
                node->Y = lineHelper->startPoint;
                node->next = NULL;
                node->edges = NULL;

                if(NodeArray == NULL)
                {
                    NodeArray = node;
                }
                else
                {
                    node->next = NodeArray;
                    NodeArray = node;
                }
                NodeCount = NodeCount+1;
            }
            lineHelper = lineHelper->next;
        }
        line = line->next;
    }
    //CalculateLowerBound();



    Tour* solution = HillClimbing();

  while(1)
    {

		XNextEvent( display_ptr, &report );
	    switch( report.type )
		{
		case Expose:

			RedrawWindow(display_ptr, win, solution);
			int total;
			total=0;
			total+=CalculateLowerBoundX();
			total+=CalculateLowerBoundY();
			printf("Lower bound: %d\n", total);
			int solQuality;
			solQuality= DrawSolution(display_ptr, win, solution);
			float ratio;
			ratio = (solQuality*1.0)/(total*1.0);
			printf("Ratio: %f\n",ratio);

			printf("----------------------------------------------------------------------\n\n");

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
			  if (report.xbutton.button == Button1 )
			  {
				  XCloseDisplay(display_ptr);
			  }
		  }
		}

    }
      exit(0);
}

int DrawSolution(Display *display, Window window, Tour* solution)
{
	std::vector<Node*> headX;
	    Node* node = solution->nodes;
	    while(node!=NULL)
	    {
	    	headX.push_back(node);
	    	node=node->next;
	    }
	    int VX = headX.size();
	    int** graphX = new int*[VX];
	    for(int i = 0; i < VX; ++i)
	    {
	        graphX[i] = new int[VX];
	    }
	    SetNeighbours(solution);

	    for(int i=0; i<VX; i++)
	    {
	        for(int j=0; j<VX; j++)
	        {
	            graphX[i][j]=0;
	            if(i != j)
	            {
	                if(headX.at(i)->a == headX.at(j)->a || headX.at(i)->b == headX.at(j)->b)
	                {
	                    graphX[i][j] = CalculateDistance(headX.at(i)->X, headX.at(i)->Y, headX.at(j)->X, headX.at(j)->Y);
	                }
	            }
	        }
	    }
	    int *PX = primMST(graphX, VX);
	    PrintMST(PX, headX, gc_red);
	    int Total = GetTotalMST(PX,VX,graphX);
	    printf("Tour Length: %d \n", Total);

	    if(PX != NULL)
	        delete PX;

	    for(int i=0; i< VX; i++)
	    {
	        delete graphX[i];
	    }
	    delete graphX;

	    return Total;
}


void RedrawWindow(Display *display, Window window, Tour* solution)
{
	struct Line *line;

    line = LineXArray;
    while(line != NULL)
    {
		XDrawLine(display_ptr, win, gc_grey, line->startPoint, line->p1, line->startPoint, line->p2);
		line = line->next;
	}

    line = LineYArray;
    while(line != NULL)
    {
		XDrawLine(display_ptr, win, gc_grey, line->p1, line->startPoint, line->p2, line->startPoint);
		line = line->next;
	}
};


bool IsSolutionValid(Tour *tempSolution)
{

    bool Stop;
    int XValid, YValid, EValid;
    struct Line *line;
    struct Node *node;
    struct Edge *edge;

    line = LineXArray;
    Stop=false;
    XValid=0;

    /*checks that all corridors are covered*/
    while(line != NULL && !Stop)
    {
        node = tempSolution->nodes;
        while(node != NULL && XValid == 0)
        {
            if(node->a == line || node->b == line)
            {
                XValid = 1;
            }
            node = node->next;
        }
        if(XValid == 0)
            Stop = true;
        line = line->next;
    }


    line = LineYArray;
    YValid=0;

    while(line != NULL && !Stop)
    {
        YValid=0;
        node = tempSolution->nodes;
        while(node != NULL && YValid == 0)
        {
            if(node->a == line || node->b == line)
            {
                YValid = 1;
            }
            node = node->next;
        }
        if(YValid == 0)
            Stop = true;
        line = line->next;
    }


SetNeighbours(tempSolution);

if(!Stop)
{
	//BFS
	std::vector<Node*> queue;
	queue.insert(queue.begin(),tempSolution->nodes);

	while(queue.size()>0)
	{
		Node* current = queue.back();
		queue.pop_back();
		Edge* nodeEdge = current->edges;
		while(nodeEdge!=NULL)
		{
			//std::find(queue.begin(), queue.end(),nodeEdge->connectsTo)
			if(nodeEdge->connectsTo->visited == false)
			{
				queue.insert(queue.begin(),nodeEdge->connectsTo);
			}
			nodeEdge = nodeEdge->next;
		}
		current->visited=true;
	}

	EValid=1;
	node = tempSolution->nodes;
	while(node != NULL && !Stop)
	{
		if(!node->visited)
		{
			Stop = true;
		}
		node = node->next;
	}
}

node = tempSolution->nodes;
while(node!=NULL)
{
	node->visited=false;
	node=node->next;
}


    return !Stop;
};

bool IsNodeInSolution(Tour* tour, Node* newNode)
{
    bool NodeInSolution = false;
    struct Node *node = tour->nodes;
    while(node != NULL && !NodeInSolution)
    {
        if(node->X == newNode->X &&  node->Y == newNode->Y)
        {
            NodeInSolution = true;
        }
        node=node->next;
    }
    return NodeInSolution;

}

Tour* generateInitialSolution()
{
    Tour *tempSolution = new Tour();
    tempSolution->nodes = NULL;

    struct Node *node = NodeArray;
    struct Node *tempNode;

    struct Node *tempNodeMin1;
    struct Node *tempNodeMin2;
    int min=1000;
    int position=0;
    int currentPos=0;


    bool IsValid=false;


    while(!IsValid)
    {
        position=random_at_most(NodeCount);
        currentPos = 0;
        node = NodeArray;
        while(node != NULL && currentPos<position)
        {
            currentPos=currentPos+1;
            if(currentPos+1 != position)
            	node=node->next;
        }


        tempNode = new Node();
        tempNode->a = node->a;
        tempNode->b = node->b;
        tempNode->X = node->X;
        tempNode->Y = node->Y;
        tempNode->next = NULL;
        tempNode->edges = NULL;
        tempNode->visited=false;

        if(!IsNodeInSolution(tempSolution, tempNode))
        {
            if(tempSolution->nodes == NULL)
            {
                tempSolution->nodes = tempNode;
            }
            else
            {
                tempNode->next = tempSolution->nodes;
                tempSolution->nodes = tempNode;
            }
        }


        IsValid=IsSolutionValid(tempSolution);
    }
    return tempSolution;

};



void SetNeighbours(Tour *tour)
{
    Node* node = tour->nodes;
    Edge* edge = NULL;
    Edge* tempEdge = NULL;

    //remove any existing edges
    while(node!=NULL)
    {
        node->visited=false;
        edge = node->edges;
        while(edge != NULL)
        {
            tempEdge = edge->next;
            delete edge;
            edge = tempEdge;
        }
        node->edges=NULL;
        node=node->next;
    }


    //Add all edges for current nodes

    struct Node *nodeHelper;
    node = tour->nodes;
    while(node != NULL)
    {
        nodeHelper = node->next;
        while(nodeHelper != NULL)
        {
            //check if we can move from one node to another
            if(node != nodeHelper)
            {
                if(node->a == nodeHelper->a || node->a == nodeHelper->b || node->b == nodeHelper->a || node->b == nodeHelper->b)
                {
                    edge = new Edge();
                    edge->connectsTo=nodeHelper;
                    edge->from = node;
                    edge->distance = CalculateDistance(node->X, node->Y, nodeHelper->X, nodeHelper->Y);
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

                    edge = new Edge();
                    edge->connectsTo=node;
                    edge->from = nodeHelper;
                    edge->distance = CalculateDistance(nodeHelper->X, nodeHelper->Y, node->X, node->Y);
                    edge->next = NULL;

                    if(nodeHelper->edges == NULL)
                    {
                        nodeHelper->edges = edge;
                    }
                    else
                    {
                        edge->next = nodeHelper->edges;
                        nodeHelper->edges = edge;
                    }
                }
            }
            nodeHelper = nodeHelper->next;
        }


        edge = node->edges;
        node->edgeCount=0;
        while(edge!=NULL)
        {
			node->edgeCount++;
			edge = edge->next;
		}

        node = node->next;
    }



}

bool onSegment(int px, int py, int qx, int qy, int rx, int ry)
{
    if (qx <= max(px, rx) && qx >= min(px, rx) && qy <= max(py, ry) && qy >= min(py, ry))
       return true;
    return false;
}

int orientation(int px, int py, int qx, int qy, int rx, int ry)
{
    int val = (qy - py) * (rx - qx) - (qx - px) * (ry - qy);
    if (val == 0) return 0;
    return (val > 0)? 1: 2;
}

bool doIntersect(int p1x, int p1y, int q1x, int q1y, int p2x, int p2y, int q2x, int q2y)
{
    int o1 = orientation(p1x, p1y, q1x, q1y, p2x, p2y);
    int o2 = orientation(p1x, p1y, q1x, q1y, q2x, q2y);
    int o3 = orientation(p2x, p2y, q2x, q2y, p1x, p1y);
    int o4 = orientation(p2x, p2y, q2x, q2y, q1x, q1y);
    if (o1 != o2 && o3 != o4)
        return true;
    if (o1 == 0 && onSegment(p1x, p1y, p2x, p2y, q1x, q1y)) return true;
    if (o2 == 0 && onSegment(p1x, p1y, q2x, q2y, q1x, q1y)) return true;
    if (o3 == 0 && onSegment(p2x, p2y, p1x, p1y, q2x, q2y)) return true;
    if (o4 == 0 && onSegment(p2x, p2y, q1x, q1y, q2x, q2y)) return true;
    return false;
}

int CalculateDistance(int x1, int y1, int x2, int y2)
{
    int diffx = x1 - x2;
    int diffy = y1 - y2;
    int diffx_sqr = pow(diffx,2);
    int diffy_sqr = pow(diffy, 2);
    int distance = (int)sqrt(diffx_sqr + diffy_sqr);
    return distance;
}	;
int random_at_most(int max)
{

  int r = rand() % (max+1);
  return r;
}


void CalculateCost(Tour* solution)
{
    std::vector<Node*> headX;
    Node* node = solution->nodes;
    while(node!=NULL)
    {
    	headX.push_back(node);
    	node=node->next;
    }
    int VX = headX.size();
    int** graphX = new int*[headX.size()];
    for(int i = 0; i < VX; ++i)
    {
        graphX[i] = new int[VX];
    }
    SetNeighbours(solution);

    for(int i=0; i<VX; i++)
    {
        for(int j=0; j<VX; j++)
        {
            graphX[i][j]=0;
            if(i != j)
            {
            	if(headX.at(i)->a == headX.at(j)->a || headX.at(i)->a == headX.at(j)->b || headX.at(i)->b == headX.at(j)->a || headX.at(i)->b == headX.at(j)->b)
            	{
                    graphX[i][j] = CalculateDistance(headX.at(i)->X, headX.at(i)->Y, headX.at(j)->X, headX.at(j)->Y);
                }
            }


        }
    }
    int *PX = primMST(graphX, VX);
    int Total = GetTotalMST(PX,VX,graphX);

    delete PX;

    for(int i=0; i< VX; i++)
    {
        delete graphX[i];
    }
    delete graphX;


    solution->Cost = Total;

}


Tour* CopySolution(Tour* solution)
{
    Tour* candidate = new Tour();
    candidate->Cost = 0;
    Node *tempNode;
    //get a copy of the solution
    Node* node = solution->nodes;
    while(node != NULL)
    {
        tempNode = new Node();
        tempNode->a = node->a;
        tempNode->b = node->b;
        tempNode->X = node->X;
        tempNode->Y = node->Y;
        tempNode->next = NULL;
        tempNode->edges = NULL;
        tempNode->visited=false;

        if(!IsNodeInSolution(candidate, tempNode))
        {
            if(candidate->nodes == NULL)
            {
                candidate->nodes = tempNode;
            }
            else
            {
                tempNode->next = candidate->nodes;
                candidate->nodes = tempNode;
            }
        }
        node=node->next;
    }
    return candidate;
}

int GetSolutionLength(Tour* tour)
{
	Node* node=tour->nodes;
	int count=0;
	while(node!=NULL)
	{
		count++;
		node=node->next;
	}
	return count;
}

Tour* GenerateNeighbourSolution(Tour* solution)
{
    Tour* candidate = CopySolution(solution);
    Node* node;
    Node* toRemove;
    Node* toSubstitute;
    int position=0;
    int currentPos=0;
    int count=0;
    int solSize=0;

    bool IsValid=false;
    bool RemovalSuccess=false;
    bool SubstitutionSuccess=false;

    //make 400 attempts to remove a random node
    while(!IsValid && count<400)
    {
    	solSize = GetSolutionLength(candidate);
        position=random_at_most(solSize);
        currentPos = 0;
        node = candidate->nodes;
        while(node != NULL && currentPos<position)
        {
            if(currentPos+1==position)
            {
                toRemove = node->next;
                if(toRemove!=NULL)
                {
                	toRemove=node;
                	node->next=toRemove->next;

					IsValid=IsSolutionValid(candidate);
					if(!IsValid)
					{
						toRemove->next=node->next;
						node->next = toRemove;
					}
					else
					{
						RemovalSuccess=true;
					}
				}
            }
            currentPos=currentPos+1;
            node=node->next;
        }
        count++;
    }

    //make 10 attempts to substitute a node
    count=0;
    while(!IsValid && count<10)
    {
    	solSize = GetSolutionLength(candidate);
        position=random_at_most(solSize);
        currentPos = 0;
        node = candidate->nodes;
        toRemove = NULL;
        while(node != NULL && currentPos<position)
        {
            if(currentPos+1==position)
            {
            	if(node->next != NULL)
            	{
					toRemove = node->next;
					node->next=toRemove->next;
            	}
            }
            currentPos=currentPos+1;
            node=node->next;
        }

        position=random_at_most(NodeCount);
        node = NodeArray;
        currentPos = 0;
        toSubstitute = NULL;
        while(node != NULL && currentPos<position)
        {
            if(currentPos+1==position)
            {
                toSubstitute = node->next;
            }
            currentPos=currentPos+1;
            node=node->next;
        }



        if(toRemove != NULL && toSubstitute!= NULL)
        {
            if(!IsNodeInSolution(candidate, toSubstitute))
            {
                node = new Node();
                node->a = toSubstitute->a;
                node->b = toSubstitute->b;
                node->X = toSubstitute->X;
                node->Y = toSubstitute->Y;
                node->next = NULL;
                node->edges = NULL;
                node->visited=false;
                node->next = candidate->nodes;
                candidate->nodes = node;

                IsValid=IsSolutionValid(candidate);

                if(!IsValid)
                {
                    toRemove->next = candidate->nodes->next;
                    candidate->nodes = toRemove;
                }
                else
                {
                    SubstitutionSuccess = true;
                }

            }
        }
        count++;
    }
    //add a random node to the solution: 50 attempt
    count=60;
    position=random_at_most(NodeCount);
    node = NodeArray;
    currentPos = 0;
    toSubstitute = NULL;
    while(count<50 && !IsValid)
    {
        position=random_at_most(NodeCount);
        node = NodeArray;
        currentPos = 0;
        toSubstitute = NULL;
        while(node != NULL && currentPos<position)
        {
            if(currentPos+1==position)
            {
                toSubstitute = node->next;
            }
            currentPos=currentPos+1;
            node=node->next;
        }


        if(toSubstitute!=NULL && !IsNodeInSolution(candidate, toSubstitute))
        {
            if(candidate->nodes == NULL)
            {
                candidate->nodes = toSubstitute;
            }
            else
            {
                toSubstitute->next = candidate->nodes;
                candidate->nodes = toSubstitute;
            }
            IsValid=true;
        }
        count++;
    }

    return candidate;
}


Tour* HillClimbing()
{
    Tour* best = new Tour();
    best->Cost=99999;

    Tour* candidate = NULL;
    Tour* Initial = NULL;
    int Restarts=0;


    while(Restarts < 5)
    {
        Initial = generateInitialSolution();
        CalculateCost(Initial);

        for(int i=0; i<400;i++)
        {
            candidate = GenerateNeighbourSolution(Initial);
            CalculateCost(candidate);

            if(candidate->Cost < Initial->Cost)
            {
                Initial = candidate;
            }
        }

        if(Initial->Cost < best->Cost)
        {
            best = Initial;
        }

        Restarts = Restarts + 1;
    }

    return best;
}
int minKey(int key[], bool mstSet[], int V)
{
   int min = std::numeric_limits<int>::max();
   int min_index=0;

   for (int v = 0; v < V; v++)
   {
     if (mstSet[v] == false && key[v] < min)
     {
         min = key[v];
         min_index = v;
     }
   }
   return min_index;
}

int* primMST(int** graph, int V)
{
     int* parent = new int[V];
     int key[V];
     bool mstSet[V];

     for (int i = 0; i < V; i++)
     {
        key[i] = std::numeric_limits<int>::max();
        mstSet[i] = false;
     }


     key[0] = 0;
     parent[0] = -1;

     for (int count = 0; count < V-1; count++)
     {
        int u = minKey(key, mstSet, V);
        mstSet[u] = true;
        for (int v = 0; v < V; v++)
        {
          if (graph[u][v] && mstSet[v] == false && graph[u][v] <  key[v])
          {
             parent[v]  = u, key[v] = graph[u][v];
          }
        }
     }

     return parent;
}

void PrintMST(int* parent, std::vector<Node*> head, GC gc_colorChosen)
{
    Node* startPoint;
    Node* endPoint;
    for (int i = 1; i < head.size(); i++)
     {
        startPoint = head.at(parent[i]);
        endPoint = head.at(i);
        XDrawLine(display_ptr, win, gc_colorChosen, startPoint->X, startPoint->Y, endPoint->X, endPoint->Y);
     }
}

int GetTotalMST(int* parent, int V, int** graph)
{
    int Total=0;
    for (int i = 1; i < V; i++)
     {
        Total += graph[i][parent[i]];
     }
    return Total;
}

int CalculateLowerBoundX()
{
	std::vector<MSTNode*> nodeList;
	MSTNode* node;
	Line* line = LineXArray;
	while(line!=NULL)
	{
		node = new MSTNode();
		node->line = line;
		nodeList.push_back(node);

		line=line->next;
	}

	int V = nodeList.size();

	int** graph = new int*[V];
	for(int i = 0; i < V; ++i)
	{
		graph[i] = new int[V];
	}

	for(int i=0; i<nodeList.size();i++)
	{
		MSTNode* line1 = nodeList.at(i);
		for(int j=0; j<nodeList.size(); j++)
		{
			graph[i][j]=0;
			MSTNode* line2 = nodeList.at(j);
			if(i!=j)
			{
				Line *lineHelper = LineYArray;
				while(lineHelper != NULL)
				{
					if(doIntersect(line1->line->startPoint, line1->line->p1, line1->line->startPoint, line1->line->p2, lineHelper->p1, lineHelper->startPoint, lineHelper->p2, lineHelper->startPoint) &&
							doIntersect(line2->line->startPoint, line2->line->p1, line2->line->startPoint, line2->line->p2, lineHelper->p1, lineHelper->startPoint, lineHelper->p2, lineHelper->startPoint))
					{
						MSTEdge* edge = new MSTEdge();
						edge->X1 = line1->line->startPoint;
						edge->Y1 = lineHelper->startPoint;

						edge->X2 = line2->line->startPoint;
						edge->Y2 = lineHelper->startPoint;
						edge->mstNode = line2;

						edge->Distance = CalculateDistance(edge->X1, edge->Y1,edge->X2, edge->Y2);

						if(line1->edges != NULL)
						{
							edge->next = line1->edges;
							line1->edges = edge;
						}
						else
						{
							line1->edges=edge;
						}

						graph[i][j] = edge->Distance;
					}
					lineHelper=lineHelper->next;
				}
			}
			//printf("%d ", graph[i][j]);
		}
		//printf("\n");
	}


	int *PX = primMST(graph, V);

	int total=0;

	MSTNode* startPoint;
	MSTNode* endPoint;
	for (int i = 1; i < V; i++)
	 {
		startPoint = nodeList.at(PX[i]);
		endPoint = nodeList.at(i);
		MSTEdge* edge = startPoint->edges;
		while(edge != NULL)
		{
			if(edge->mstNode == endPoint)
			{
				XDrawLine(display_ptr, win, gc_blue, edge->X1, edge->Y1, edge->X2, edge->Y2);
				total+=edge->Distance;
			}
			edge = edge->next;
		}

	 }
return total;


}

int CalculateLowerBoundY()
{
	std::vector<MSTNode*> nodeList;
	MSTNode* node;
	Line* line = LineYArray;
	while(line!=NULL)
	{
		node = new MSTNode();
		node->line = line;
		nodeList.push_back(node);

		line=line->next;
	}

	int V = nodeList.size();

	int** graph = new int*[V];
	for(int i = 0; i < V; ++i)
	{
		graph[i] = new int[V];
	}

	for(int i=0; i<nodeList.size();i++)
	{
		MSTNode* line1 = nodeList.at(i);
		for(int j=0; j<nodeList.size(); j++)
		{
			graph[i][j]=0;
			MSTNode* line2 = nodeList.at(j);
			if(i!=j)
			{
				Line *lineHelper = LineXArray;
				while(lineHelper != NULL)
				{
					if(doIntersect(line1->line->p1,line1->line->startPoint, line1->line->p2, line1->line->startPoint,  lineHelper->startPoint,lineHelper->p1,  lineHelper->startPoint, lineHelper->p2) &&
							doIntersect(line2->line->p1,line2->line->startPoint,  line2->line->p2,line2->line->startPoint,  lineHelper->startPoint,lineHelper->p1,  lineHelper->startPoint, lineHelper->p2))
					{
						MSTEdge* edge = new MSTEdge();
						edge->X1 = lineHelper->startPoint;
						edge->Y1 = line1->line->startPoint;

						edge->X2 = lineHelper->startPoint;
						edge->Y2 = line2->line->startPoint;
						edge->mstNode = line2;

						edge->Distance = CalculateDistance(edge->X1, edge->Y1,edge->X2, edge->Y2);

						if(line1->edges != NULL)
						{
							edge->next = line1->edges;
							line1->edges = edge;
						}
						else
						{
							line1->edges=edge;
						}

						graph[i][j] = edge->Distance;
					}
					lineHelper=lineHelper->next;
				}
			}
			//printf("%d ", graph[i][j]);
		}
		//printf("\n");
	}


	int *PX = primMST(graph, V);

	int total=0;

	MSTNode* startPoint;
	MSTNode* endPoint;
	for (int i = 1; i < V; i++)
	 {
		startPoint = nodeList.at(PX[i]);
		endPoint = nodeList.at(i);
		MSTEdge* edge = startPoint->edges;
		while(edge != NULL)
		{
			if(edge->mstNode == endPoint)
			{
				XDrawLine(display_ptr, win, gc_green, edge->X1, edge->Y1, edge->X2, edge->Y2);
				total+=edge->Distance;
			}
			edge = edge->next;
		}

	 }
return total;


}


