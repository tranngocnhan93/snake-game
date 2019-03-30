//Metropolia UAS
//Snake Game Project
//Name: Nhan Tran Ngoc

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdbool.h>

#define WORM_LEN_MAX 	50
#define WALL_LEN_LV1 	50
#define WALL_LEN_LV2 	140
#define WALL_LEN_LV3	240
#define WORM_SPEED 	100000
#define BOUND_BOTTOM 	35					
#define BOUND_RIGHT	100					
#define BOUND_LEFT	0
#define BOUND_TOP 	0


void MoveCursor(int col, int lin)
{
    printf("\033[%d;%dH", lin, col);
}

void ClearScreen(void)
{
	printf("\033[2J");
}

enum direction
{
    left,							//from right to left
    right,							//from left to right
    up,								//upwards
    down							//downwards
};

enum oject_type
{
	food,
	door,
	wall,
	potato							//when the worm has eaten enough it has an ability to change wall to harmless potato
};

struct object
{
	int x;
	int y;
	enum oject_type objtp;
};

struct worm_element
{
    int x;
    int y;
    enum direction dir;						//next direction of the element
};

void DrawWorm(struct worm_element worm[], int n, bool break_wall)
{
    int j;
	if(break_wall)
	{
		for(int i = 0; i < n; i++)
		{
			j = i + 31;
			if(j > 36)
				j -= 6;
			printf("\033[%d;49m", j);
			MoveCursor(worm[i].x, worm[i].y);
			printf("*\n");
		}
		printf("\033[39;49m");
	}
	else
	{
		for(int i = 0; i < n; i++)
		{
			MoveCursor(worm[i].x, worm[i].y);
			printf("*\n");
		}
	}
}
void EraseWorm(struct worm_element worm[], int n)
{
    for(int i = 0; i < n; i++)
    {
        MoveCursor(worm[i].x, worm[i].y);
        printf(" \n");
    }
}
void WormInit(struct worm_element worm[], int n, enum direction dir, int col, int lin)
{
    if(dir == right)
    {
        for(int i = 0; i < n; i++)
        {
            worm[i].x = col - i;
            worm[i].y = lin;
            worm[i].dir = right;
        }
    }
    DrawWorm(worm, n, false);
}
void GrowWorm(struct worm_element worm[], int *n, bool break_wall)
{
	(*n)++;
	switch(worm[(*n)-2].dir)
	{
	case right:
		worm[(*n)-1].x = worm[(*n)-2].x - 1;
		worm[(*n)-1].y = worm[(*n)-2].y;
		worm[(*n)-1].dir = worm[(*n)-2].dir;
		break;
	case left:
		worm[(*n)-1].x = worm[(*n)-2].x + 1;
		worm[(*n)-1].y = worm[(*n)-2].y;
		worm[(*n)-1].dir = worm[(*n)-2].dir;
		break;
	case up:
		worm[(*n)-1].x = worm[(*n)-2].x;
		worm[(*n)-1].y = worm[(*n)-2].y + 1;
		worm[(*n)-1].dir = worm[(*n)-2].dir;
		break;
	case down:
		worm[(*n)-1].x = worm[(*n)-2].x;
		worm[(*n)-1].y = worm[(*n)-2].y - 1;
		worm[(*n)-1].dir = worm[(*n)-2].dir;
		break;
	}
	DrawWorm(worm, (*n), break_wall);
}
void Move(struct worm_element worm[], int n, enum direction dir, bool break_wall)
{
    MoveCursor(worm[n-1].x, worm[n-1].y);
	printf(" ");
	for(int i = n-1; i > 0; i--)
    {
        worm[i].x = worm[i-1].x;
        worm[i].y = worm[i-1].y;
        worm[i].dir = worm[i-1].dir;
    }
	switch (dir)
	{
		case right:
			worm[0].x += 1;
			worm[0].y += 0;
			worm[0].dir = right;
			break;
		case left:
			worm[0].x -= 1;
			worm[0].y += 0;
			worm[0].dir = left;
			break;
		case up:
			worm[0].x += 0;
			worm[0].y -= 1;
			worm[0].dir = up;
			break;
		case down:
			worm[0].x += 0;
			worm[0].y += 1;
			worm[0].dir = down;
			break;
	}
    DrawWorm(worm, n, break_wall);
}
void DrawBoundary(void)
{
	MoveCursor(BOUND_LEFT, BOUND_TOP);
	for(int i = BOUND_LEFT; i < BOUND_RIGHT; i++)
	{
		printf("-");
	}
	MoveCursor(BOUND_LEFT, BOUND_BOTTOM);
	for(int i = BOUND_LEFT; i < BOUND_RIGHT; i++)
	{
		printf("-");
	}
	MoveCursor(BOUND_LEFT, BOUND_TOP + 2);
	for(int i = 1; i < (BOUND_BOTTOM - BOUND_TOP - 1); i++)
	{
		printf("|\n");
	}
	
	for(int i = 2; i < (BOUND_BOTTOM - BOUND_TOP); i++)
	{
		MoveCursor(BOUND_RIGHT, BOUND_TOP + i);
		printf("|");
	}
}
void DrawWalls(int level, struct object objects[], int m)
{
	int k = 1; 								//index to keep track of objects in objects[]
	int xterval, yterval;							//divide the x and y into 7 intervals each
	xterval = (int) (BOUND_RIGHT - BOUND_LEFT)/7;				//distance between walls is calculated based on this
	yterval = (int) (BOUND_BOTTOM - BOUND_TOP)/7;		//BOUND_LEFT ---- lv3 ---- lv2 ---- lv1 ---- lv1 ---- lv2 ---- lv3 ---- BOUND_RIGHT
	int x, y;						//BOUND_LEFT	   x	    2x	    3x	     4x       5x	6x	BOUND_RIGHT, x : xterval
	/* Set changing stage door*/
	x = (int) (BOUND_RIGHT - BOUND_LEFT)/2;
	y = (int) (BOUND_BOTTOM - BOUND_TOP)/2;
	objects[0].x = x;
	objects[0].y = y;
	objects[0].objtp = door;
	MoveCursor(x, y);
	printf("@");								//print the key symbol
	
	x = (4-level)*xterval;							//calculate x, y interval based on level
	y = (4-level)*yterval;
	MoveCursor(x, y);							//draw top wall, i.e: for level = 1, draw from (3xterval, 3yterval)
	for(int i = x; i < (7*xterval-x); i++)					//i = 3xterval; i <= 4xterval; i++
	{
		objects[k].x = i;
		objects[k].y = y;
		objects[k].objtp = wall;
		k++;
		printf("#");
	}
	
	x = 7*xterval - x;							//draw right wall, i.e: for level = 1, draw from (4xterval, 3yterval)
	for(int i = y; i < (7*yterval-y); i++)					//i = 3yterval; i < 4yterval; i++
	{
		objects[k].x = x;
		objects[k].y = i;
		objects[k].objtp = wall;
		k++;
		MoveCursor(x, i);								
		printf("#");
	}	
	
	y = 7*yterval - y;							//draw bottom wall, i.e: for level = 1, draw from (4xterval, 4yterval)
	for(int i = x; i >= (7*xterval-x); i--)					//i = 4xterval; i > 3yterval; i--
	{
		objects[k].x = i;
		objects[k].y = y;
		objects[k].objtp = wall;
		k++;
		MoveCursor(i, y);								
		printf("#");
	}
	
	x = 7*xterval - x;							//draw left wall, i.e: for level = 1, draw from (3xterval, 4yterval)
	for(int i = y; i > (7*yterval-y); i--)					//i = 4yterval; i > 3yterval; i--
	{
		objects[k].x = x;
		objects[k].y = i;
		objects[k].objtp = wall;
		k++;
		MoveCursor(x, i);
		printf("#");
	}
}
void GenerateFood(int layer, struct object *foodie)
{
	int xterval, yterval;							//divide the x and y into 7 intervals each
	xterval = (int) (BOUND_RIGHT - BOUND_LEFT)/7;				//distance between walls is calculated based on this
	yterval = (int) (BOUND_BOTTOM - BOUND_TOP)/7;		//BOUND_LEFT ---- lv3 ---- lv2 ---- lv1 ---- lv1 ---- lv2 ---- lv3 ---- BOUND_RIGHT
	int x, y;						//BOUND_LEFT	   x	    2x      3x	     4x       5x	6x	BOUND_RIGHT, x : xterval
	
	time_t t;
	srand((unsigned) time(&t));						//initialize random number generator
	x = rand() % (BOUND_RIGHT - 2) + (BOUND_LEFT + 2);			//generate x first, then generate y based on x
	switch (layer)								//layer means level, the map is divided into layers based on level
	{
/* Generate y based on x and level */		
		case 3:										//level 3
			if(x < xterval || x > 6*xterval)
			{
				y = rand() % (BOUND_BOTTOM - BOUND_TOP - 2) + (BOUND_TOP + 2);
				break;
			}
			else
			{
				if(rand() % 2)
				{	
					y = rand() % (yterval - 2) + (6*yterval + 1);
					break;
				}
				else
				{
					y = rand() % (yterval - 2) + (BOUND_TOP + 2);
					break;
				}
			}
			break;
		case 2:										//level 2
			if(x < 2*xterval || x > 5*xterval)
			{
				y = rand() % (BOUND_BOTTOM - BOUND_TOP - 2) + (BOUND_TOP + 2);
				break;
			}
			else
			{
				if(rand() % 2)
				{	
					y = rand() % (2*yterval - 2) + (5*yterval + 1);
					break;
				}
				else
				{
					y = rand() % (2*yterval - 2) + (BOUND_TOP + 2);
					break;
				}
			}
			break;
		case 1:										//level 1
			if(x < 3*xterval || x > 4*xterval)
			{
				y = rand() % (BOUND_BOTTOM - BOUND_TOP - 2) + (BOUND_TOP + 2);
				break;
			}
			else
			{
				if(rand() % 2)
				{	
					y = rand() % (3*yterval - 2) + (4*yterval + 1);
					break;
				}
				else
				{
					y = rand() % (3*yterval - 2) + (BOUND_TOP + 2);
					break;
				}
			}
			break;
	}
	/* Set up a food object and print it */
	foodie->x = x;
	foodie->y = y;
	foodie->objtp = food;
	MoveCursor(x, y);
	printf("$");
}

/* This function check the new position of the worm and react based on the object type: */
/* food -> grow the worm, door -> move to next level, wall -> break wall or die 		*/
bool AmIDead(struct worm_element worm[], int *n, struct object objects[], int m, struct object *foodie, int *level, int *eaten, bool *break_wall)
{
	for(int i = 1; i < (*n); i++)
	{
		if(worm[0].x == worm[i].x && worm[0].y == worm[i].y)
			return true;								//you bit yourself
	}
	if(worm[0].x == (BOUND_LEFT + 1) || worm[0].x == BOUND_RIGHT)				// +1 to get rid of unexplained bug
		return true;									//you ran to the boundary
	if(worm[0].y == BOUND_BOTTOM || worm[0].y == (BOUND_TOP + 1))				// +1 to get rid of unexplained bug
		return true;									//you ran to the boundary
	for(int i = 1; i < m; i++)
	{
		if(worm[0].x == objects[i].x && worm[0].y == objects[i].y && objects[i].objtp == wall)
		{
			if(*break_wall)
			{
				objects[i].objtp = potato;
				objects[i-1].objtp = potato;					//the surrounding bricks are also transformed
				objects[i+1].objtp = potato;
				MoveCursor(objects[i-1].x, objects[i-1].y);
				printf(" ");
				MoveCursor(objects[i+1].x, objects[i+1].y);
				printf(" ");
				*break_wall = false;						//worm loses its ability after breaking the bricks
			}
			else
				return true;							//you ran to the wall
		}
	}
	if(worm[0].x == objects[0].x && worm[0].y == objects[0].y)				//if door, move to next level
		(*level)++;
	if(worm[0].x == foodie->x && worm[0].y == foodie->y)					//if food, grow the worm
	{
		(*eaten)++;
		GrowWorm(worm, n, *break_wall);
		GenerateFood(*level, foodie);
	}
	if(*eaten == 3)
	{
		*break_wall = true;
		*eaten = 0;
	}
	return false;										//long live the king
}

int main()
{
    ClearScreen();
	int level = 1, food_eaten = 0;
	int n = 10;										//worm initial length
    struct worm_element worm[WORM_LEN_MAX];							//the worm is an array of worm elements
    enum direction dir = right;									//next direction of th worm
	enum direction cur_dir = right;								//current direction of the worm
	bool break_wall = false;
	bool temp = false;
	
	
	char input_key[5];
	fd_set rfds;
    struct timeval tv;
    int retval;
    tv.tv_sec = 0;										//time out for select()
    tv.tv_usec = 200000;
	
	struct object wall1[WALL_LEN_LV1];
	struct object wall2[WALL_LEN_LV2];	
	struct object wall3[WALL_LEN_LV3];
	struct object food;
	
	DrawBoundary();
	DrawWalls(level, wall1, WALL_LEN_LV1);
	WormInit(worm, n, right, 40, 5);
	GenerateFood(level, &food);
    system("stty raw -echo");									//set the terminal to raw mode
    while(1)
    {	
		FD_ZERO(&rfds);									//watch stdin (fd 0) to see when it has input
		FD_SET(0, &rfds);		
		retval = select(1, &rfds, NULL, NULL, &tv);					//read single arrow key from input
		
        if (retval)
		{
			fgets(input_key, 4, stdin);						//terminate reading process after 3 characters
			if(strcmp(input_key, "\033[A") == 0)					//up
			{
				Move(worm, n, up, break_wall);
				usleep(WORM_SPEED);
				cur_dir = up;
			}
			else if(strcmp(input_key, "\033[B") == 0)				//down
			{
				Move(worm, n, down, break_wall);
				usleep(WORM_SPEED);
				cur_dir = down;
			}
			else if(strcmp(input_key, "\033[D") == 0)				//left
			{
				Move(worm, n, left, break_wall);
				usleep(WORM_SPEED);
				cur_dir = left;
			}
			else if(strcmp(input_key, "\033[C") == 0)				//right
			{
				Move(worm, n, right, break_wall);
				usleep(WORM_SPEED);
				cur_dir = right;
			}
			else if(*input_key == 'q')						//press escape to quit
				break;
		}
        else											//if no new direction read, continue moving in current direction
        {
			Move(worm, n, cur_dir, break_wall);
			usleep(WORM_SPEED);
		}
		if(AmIDead(worm, &n, wall1, WALL_LEN_LV1, &food, &level, &food_eaten, &break_wall))
			break;
		if(level == 2 && temp == false)
		{
			EraseWorm(worm, n);							//earse worm of previous level
			cur_dir = right;
			WormInit(worm, n, cur_dir, 50, 3);					//initalize a new worm
			DrawWalls(1, wall1, WALL_LEN_LV1);
			DrawWalls(2, wall2, WALL_LEN_LV2);
			temp = true;								//this block is run only once
		}
		if(level == 2)
			if(AmIDead(worm, &n, wall2, WALL_LEN_LV2, &food, &level, &food_eaten, &break_wall))
				break;
		if(level == 3 && temp == true)
		{
			EraseWorm(worm, n);							//earse worm of previous level
			cur_dir = right;
			WormInit(worm, n, cur_dir, 50, 3);					//initalize a new worm
			DrawWalls(1, wall1, WALL_LEN_LV1);
			DrawWalls(2, wall2, WALL_LEN_LV2);
			DrawWalls(3, wall3, WALL_LEN_LV3);
			temp = false;								//this block is run only once
		}
		if(level == 3)
		{
			if(AmIDead(worm, &n, wall2, WALL_LEN_LV2, &food, &level, &food_eaten, &break_wall))
				break;
			if(AmIDead(worm, &n, wall3, WALL_LEN_LV3, &food, &level, &food_eaten, &break_wall))
				break;
		}
    }
	system("stty cooked");									//reset terminal back to cooked mode
	system("stty echo");
	

    return 0;
}

