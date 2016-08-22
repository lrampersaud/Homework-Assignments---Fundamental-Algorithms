/* Name: Lalchandra Rampersaud
 * Course: I96 (Fundamental algorithms)
 * Homework #4*/

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <vector>
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
char *win_name_string = "Traveling Salesman Problem";
char *icon_name_string = "Icon for TSP Window";

XEvent report;

GC gc, gc_yellow, gc_red, gc_grey;
unsigned long valuemask = 0;
XGCValues gc_values, gc_yellow_values, gc_red_values, gc_grey_values;
Colormap color_map;
XColor tmp_color1, tmp_color2;


struct Point
{
    int x;
    int y;
    Point* next;
    bool connected;
};


struct Node
{
    int Value;
    bool Selected;
    std::vector<Node> Nodes;
    int CountNodes;
};

unsigned int **Dist;
int **g, **p;

void SetMatrix(int numOfVertices);
void ClearMatrix(int numOfVertices);
void AddDistances(int numOfVertices, int Offset);
void InitializeMaskPath(int nPow, int numOfVertices);
int compute(int start, int set, int nPow, int numOfVertices);
void getpath(int start, int set, int nPow);
void HeldKarp(int nPow, int numOfVertices);
int CalculateDistance(int x1, int y1, int x2, int y2);

int Vertices[20];
Point* pointArray;
Point* Points;
int CountPoints;
int CountRecursive=0;
bool fileChosen = false;
std::vector<int> solution;
std::vector<Point*> solutionSet[10];
int CountSolutions=0;

void SortArray()
{
	Point *outer = pointArray;
	Point *inner;
	int x,y;
	while(outer!=NULL)
	{
		inner = outer->next;
		while(inner != NULL)
		{
			if(outer->x > inner->x)
			{
				x = outer->x;
				y = outer->y;
				outer->x = inner->x;
				outer->y = inner->y;
				inner->x=x;
				inner->y=y;
			}
			inner=inner->next;
		}
		outer = outer->next;
	}
}

void JoinPaths()
{
	int minPos1=0;
	int minPos2=0;
	int minPos1_S=0;
	int minPos2_S=0;

	for(int x=0; x<CountSolutions-1; x++)
	{
		minPos1=-1;
		minPos2=-1;
		minPos1_S=-1;
		minPos2_S=-1;
		int min=numeric_limits<int>::max();


		for(int i=0; i<solutionSet[x].size(); i++)
		{
			int x1 = solutionSet[x].at(i)->x;
			int y1 = solutionSet[x].at(i)->y;
			for(int j=0; j<solutionSet[x+1].size(); j++)
			{
				int x2 = solutionSet[x+1].at(j)->x;
				int y2 = solutionSet[x+1].at(j)->y;
				int d = CalculateDistance(x1, y1, x2, y2);
				if(d < min)
				{
					min = d;
					minPos1=i;
					minPos2=j;
				}
			}
		}
		solutionSet[x].at(minPos1)->connected=true;
		solutionSet[x+1].at(minPos2)->connected=true;
		XDrawLine(display_ptr, win, gc_red, solutionSet[x].at(minPos1)->x, solutionSet[x].at(minPos1)->y, solutionSet[x+1].at(minPos2)->x, solutionSet[x+1].at(minPos2)->y);

		min=numeric_limits<int>::max();


		for(int i=0; i<solutionSet[x].size(); i++)
		{
			if(i!=minPos1)
			{
				int x1 = solutionSet[x].at(i)->x;
				int y1 = solutionSet[x].at(i)->y;
				for(int j=0; j<solutionSet[x+1].size(); j++)
				{
					if(j!=minPos2)
					{
						int x2 = solutionSet[x+1].at(j)->x;
						int y2 = solutionSet[x+1].at(j)->y;
						int d = CalculateDistance(x1, y1, x2, y2);
						if(d < min)
						{
							if(((i+1)==minPos1 || (i-1)==minPos1)&& ((j+1)==minPos2 || (j-1)==minPos2))
							{
								min = d;
								minPos1_S=i;
								minPos2_S=j;
							}
						}
					}
				}
			}
		}
		solutionSet[x].at(minPos1_S)->connected=true;
		solutionSet[x+1].at(minPos2_S)->connected=true;
		XDrawLine(display_ptr, win, gc_red, solutionSet[x].at(minPos1_S)->x, solutionSet[x].at(minPos1_S)->y, solutionSet[x+1].at(minPos2_S)->x, solutionSet[x+1].at(minPos2_S)->y);
	}


	for(int x=0; x<CountSolutions; x++)
	{
		for(int i=0; i<solutionSet[x].size()-1; i++)
		{
			if(!(solutionSet[x].at(i)->connected && solutionSet[x].at(i+1)->connected))
				XDrawLine(display_ptr, win, gc_red, solutionSet[x].at(i)->x, solutionSet[x].at(i)->y, solutionSet[x].at(i+1)->x, solutionSet[x].at(i+1)->y);
			else
			{
				XDrawLine(display_ptr, win, gc_grey, solutionSet[x].at(i)->x, solutionSet[x].at(i)->y, solutionSet[x].at(i+1)->x, solutionSet[x].at(i+1)->y);
				solutionSet[x].at(i)->connected = false;
				solutionSet[x].at(i+1)->connected = false;
			}
		}

		//join first and last if they are both not connected
		if(!(solutionSet[x].front()->connected && solutionSet[x].back()->connected))
		{
			XDrawLine(display_ptr, win, gc_red, solutionSet[x].front()->x, solutionSet[x].front()->y, solutionSet[x].back()->x, solutionSet[x].back()->y);
		}

	}
}

void SetMatrix(unsigned int numOfVertices) {
	Dist = new unsigned int*[numOfVertices];
	for (unsigned int i = 0; i < numOfVertices; i++)
		Dist[i] = new unsigned int[numOfVertices];
}

void ClearMatrix(int numOfVertices) {
	for (unsigned int i = 0; i < numOfVertices; i++)
		delete[] Dist[i];
	delete Dist;
}

void AddDistances(const unsigned int numOfVertices, int Offset)
{
	SetMatrix(numOfVertices);

	Point* starting = pointArray;

	int j=0;
	while(starting != NULL && j<Offset)
	{
		starting = starting->next;
		j++;
	}


	Point *tempOuter = starting;
		int i=0;
		while(tempOuter != NULL && i<numOfVertices)
		{
			int h = 15, w = 15;
			XFillArc(display_ptr, win, gc, tempOuter->x-(w/2), tempOuter->y-(h/2),w, h, 0, 360 * 64);

			Points[i].x = tempOuter->x;
			Points[i].y = tempOuter->y;
			Point *tempInner = starting;
			int j=0;
			while(tempInner!=NULL && j<numOfVertices)
			{
				if(i!=j)
				{
					Dist[i][j]=CalculateDistance(tempOuter->x,tempOuter->y, tempInner->x, tempInner->y);
				}
				else
				{
					Dist[i][j]=0;
				}
				j++;
				tempInner = tempInner->next;
			}
			tempOuter = tempOuter->next;
			i++;
		}
}

void InitializeMaskPath(int nPow, unsigned int numOfVertices) {
	g = new int *[nPow];
	p = new int *[nPow];
	for (unsigned int i = 0; i < numOfVertices; i++) {
		g[i] = new int[nPow];
		p[i] = new int[nPow];
	}
}


int compute(int start, int set, int nPow, unsigned int numOfVertices) {
	int masked, mask, result = numeric_limits<int>::max(), temp;

	if (g[start][set] != -1)
		return g[start][set];
	for (unsigned int i = 0; i < numOfVertices; i++) {
		mask = (nPow - 1) - (1 << i);
		masked = set & mask;
		if (masked != set) {
			temp = Dist[start][i]
			        + compute(i, masked, nPow, numOfVertices);
			if (temp < result)
				result = temp, p[start][set] = i;
		}
	}
	return g[start][set] = result;
}

void getpath(int start, int set, int nPow) {
	if (p[start][set] == -1)
		return;
	int x = p[start][set];
	int mask = (nPow - 1) - (1 << x); 
	int masked = set & mask;
	solution.push_back(x);
	getpath(x, masked, nPow);
}

void HeldKarp(int nPow, unsigned int numOfVertices)
{

	for (unsigned int i = 0; i < numOfVertices; i++)
		for (int j = 0; j < nPow; j++)
			g[i][j] = p[i][j] = -1;
	for (unsigned int i = 0; i < numOfVertices; i++)
		g[i][0] = Dist[i][0];
	int result = compute(0, nPow - 2, nPow, numOfVertices);
	printf("Path length: %d\n", result);
	solution.clear();
	solution.push_back(0);
	getpath(0, nPow - 2, nPow);


	std::vector<Point*> solPoints;


	for(int ia = 0; ia<solution.size(); ia++)
	{
		Point* toStore = new Point();
		toStore->x=Points[solution.at(ia)].x;
		toStore->y = Points[solution.at(ia)].y;
		toStore->connected = false;
		solPoints.push_back(toStore);


	}
	solutionSet[CountSolutions++] = solPoints;

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

void DrawPoints()
{
	XClearWindow(display_ptr, win);
	Point *tempInner = pointArray;
	int j=0;
	while(tempInner!=NULL && j<CountPoints)
	{
		XDrawPoint(display_ptr, win, gc, tempInner->x, tempInner->y);
		j++;
		tempInner = tempInner->next;
	}
	SortArray();
	int limit=20;

	if(CountPoints<limit)
	{
		Points = new Point[CountPoints];
		unsigned int numOfVertices = CountPoints;
		int nPow = pow(2, numOfVertices); //bit number required to represent all possible sets
		InitializeMaskPath(nPow, numOfVertices);
		AddDistances(numOfVertices, 0);
		solution.clear();
		HeldKarp(nPow, numOfVertices);
		ClearMatrix(numOfVertices);


		for(int x=0; x<CountSolutions; x++)
		{
			for(int i=0; i<solutionSet[x].size()-1; i++)
			{
				XDrawLine(display_ptr, win, gc_red, solutionSet[x].at(i)->x, solutionSet[x].at(i)->y, solutionSet[x].at(i+1)->x, solutionSet[x].at(i+1)->y);
			}
			XDrawLine(display_ptr, win, gc_red, solutionSet[x].front()->x, solutionSet[x].front()->y, solutionSet[x].back()->x, solutionSet[x].back()->y);
		}
	}
	else
	{
		int division = CountPoints / limit+1;
		int offset=limit;
		offset = CountPoints/division;
		for(int i=0; i<division; i++)
		{
			unsigned int numOfVertices = offset;
			if(i==division-1)
			{
				if(CountPoints%division!=0)
				{
					numOfVertices+=CountPoints%division;
				}
			}

			Points = new Point[numOfVertices];

			int nPow = pow(2, numOfVertices);
			InitializeMaskPath(nPow, numOfVertices);
			AddDistances(numOfVertices, i*offset);

			HeldKarp(nPow, numOfVertices);
			ClearMatrix(numOfVertices);
		}
		JoinPaths();
	}

}

bool fexists(const char *filename)
{
  ifstream ifile(filename);
  return ifile;
}

int main(int argc, char* argv[])
{
	struct Point *firstPoint;
	char filename[255]="/home/ravi/Desktop/testset2";
	if(argc > 2)
	{
		strcpy(filename, argv[1]);
	}
	char line[80];
	FILE *fr;
	//printf("Enter file name: ");
	//scanf("%s", filename);

	if(fexists(filename))
	{
		pointArray = NULL;
		CountPoints=0;
		fr = fopen(filename,"rt");
		while(fgets(line, 80, fr) != NULL)
		{
			firstPoint = new Point();

			sscanf (line, "%d %d", &firstPoint->x, &firstPoint->y);
			if(pointArray == NULL)
			{
				pointArray = firstPoint;
			}
			else
			{
				firstPoint->next = pointArray;
				pointArray = firstPoint;
			}
			CountPoints++;
		}
		fileChosen = true;
	}
	else
	{
		fileChosen = false;
	}

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
			if(fileChosen)
			{
				DrawPoints();
				fileChosen=false;
				CountPoints=0;
				delete[] pointArray;
				pointArray = NULL;

				CountSolutions=0;

			}

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
			  if(fileChosen)
			{
				DrawPoints();
				fileChosen=false;
				CountPoints=0;
				delete[] pointArray;
				pointArray = NULL;
				CountSolutions=0;
			}
			  else
			  {
				int x, y;
				x = report.xbutton.x;
				y = report.xbutton.y;
				if (report.xbutton.button == Button1 )
				{
					firstPoint = new Point();

					firstPoint->x = x;
					firstPoint->y = y;
					if(pointArray == NULL)
					{
						pointArray = firstPoint;
					}
					else
					{
						firstPoint->next = pointArray;
						pointArray = firstPoint;
					}
					CountPoints++;
					int h = 15, w = 15;
					XFillArc(display_ptr, win, gc, x-(w/2), y-(h/2),w, h, 0, 360 * 64);

				}
				else
				{
					if(CountPoints>0)
					{
						DrawPoints();
						CountPoints=0;
						delete[] pointArray;
						pointArray = NULL;
						CountSolutions=0;
					}
					else
					{
						XClearWindow(display_ptr, win);
						//XCloseDisplay(display_ptr);
					}
				}
			  }
		  }
		  break;
		default:
	  /* this is a catch-all for other events; it does not do anything.
			 One could look at the report type to see what the event was */
		  break;
		}
    }
    return 0;
}