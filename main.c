#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<time.h>
// include the map for the maze.
#include "map.h"
// the size of the block to draw
const int BLOCKSIZE=25;
const float scale = 0.08f;
// the width of the screen taking into account the maze and block
#define WIDTH COLS*BLOCKSIZE
// the height of the screen taking into account the maze and block
#define HEIGHT ROWS*BLOCKSIZE
// an enumeration for direction to move USE more enums!
enum DIRECTION{LEFT,RIGHT,UP,DOWN,NONE};
enum BLOCK{BLACK,BLUE,RPILL,GATE,POWERPILL};
typedef enum {FALSE,TRUE}BOOL;

typedef struct
{
    int x;
    int Y;
    int dir;
    int temp;
    int last;
}pacman;
typedef struct
{
    int x;
    int Y;
    int dir;
    BOOL gate;
    BOOL l;
}ghost;

BOOL checkVictory();
BOOL checkMove(int dir, int x, int y, BOOL pac);
void checkPill(int *x, int *y);
void moveSprite(int *x, int *y, int dir);
void moveShadow(int *x, int *y, int *dir, int pacX, int pacY, BOOL *la);
void moveSpeedy(int *x, int *y, int *dir,int pacX,int pacY,BOOL *la, int pacDir);
void moveBashful(int *x, int *y, int *dir,int pacX,int pacY,BOOL *la, int pacDir, int shadX, int shadY);
void movePokey(int *x, int *y, int *dir,int pacX,int pacY,BOOL *la, BOOL scatter);
void moveFrightened(int *x, int *y, int *dir);
void movePac(int *dir, int *last, int *temp, int *x, int *y, int keyPressed);
void drawShadow(SDL_Renderer *ren, SDL_Texture *tex, int x, int y, int dir);
void drawSpeedy(SDL_Renderer *ren, SDL_Texture *tex, int x, int y, int dir);
void drawBashful(SDL_Renderer *ren, SDL_Texture *tex, int x, int y, int dir);
void drawPokey(SDL_Renderer *ren, SDL_Texture *tex, int x, int y, int dir);
void drawPacman(SDL_Renderer *ren, SDL_Texture *tex, int x, int y, int dir, int frameCount);
void drawMaze(SDL_Renderer *ren);
void checkTeleport(int *x, int dir);
int pillCount();


int main()
{
    srand(time(NULL));
    clock_t start = clock(), diff;
    // initialise SDL and check that it worked otherwise exit
    // see here for details http://wiki.libsdl.org/CategoryAPI
    if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
    {
        printf("%s\n",SDL_GetError());
        return EXIT_FAILURE;
    }
    // we are now going to create an SDL window

    SDL_Window *win = 0;
    win = SDL_CreateWindow("Pacman", 100, 100, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (win == 0)
    {
        printf("%s\n",SDL_GetError());
        return EXIT_FAILURE;
    }
    // the renderer is the core element we need to draw, each call to SDL for drawing will need the
    // renderer pointer
    SDL_Renderer *ren = 0;
    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    // check to see if this worked
    if (ren == 0)
    {
        printf("%s\n",SDL_GetError() );
        return EXIT_FAILURE;
    }
    // this will set the background colour to white.
    // however we will overdraw all of this so only for reference
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);

    SDL_Surface *image;
    image=IMG_Load("pacsprite.png");
    if(!image)
    {
        printf("IMG_Load: %s\n", IMG_GetError());
        return EXIT_FAILURE;
    }

    SDL_Texture *ptex = 0;
    ptex = SDL_CreateTextureFromSurface(ren, image);
    // free the image
    SDL_FreeSurface(image);

    image=IMG_Load("ghostsprite.png");
    if(!image)
    {
        printf("IMG_Load: %s\n", IMG_GetError());
        return EXIT_FAILURE;
    }

    SDL_Texture *gtex = 0;
    gtex = SDL_CreateTextureFromSurface(ren, image);
    // free the image
    SDL_FreeSurface(image);

    BOOL quit=FALSE;
    // now we are going to loop forever, process the keys then draw

    ghost shadow = {14*BLOCKSIZE,11*BLOCKSIZE,LEFT,TRUE,TRUE};
    ghost speedy = {14*BLOCKSIZE,14*BLOCKSIZE,LEFT,TRUE,TRUE};
    ghost bashful = {14*BLOCKSIZE,14*BLOCKSIZE,LEFT,TRUE,TRUE};
    ghost pokey = {14*BLOCKSIZE,14*BLOCKSIZE,LEFT,TRUE,TRUE};
    pacman pac = {13*BLOCKSIZE,17*BLOCKSIZE+1,NONE,NONE,NONE};
    BOOL keyPressed = FALSE;
    int frameCount = 0;
    int aiMode = 0;
    int msec;
    int moveMode[7]={7,27,34,54,59,79,84};


    while (quit !=TRUE)
    {

        SDL_Event event;
        // grab the SDL even (this will be keys etc)
        while (SDL_PollEvent(&event))
        {
            // look for the x of the window being clicked and exit
            if (event.type == SDL_QUIT)
                quit = TRUE;//quit =
            // check for a key down
            keyPressed = FALSE;

            if (event.type == SDL_KEYDOWN)
            {

                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE :
                    quit=TRUE;
                    break;
                case SDLK_UP :
                case SDLK_w :
                    pac.temp = pac.dir;
                    pac.last = NONE;
                    keyPressed = TRUE;
                    pac.dir = UP;
                    break;
                case SDLK_DOWN :
                case SDLK_s :
                    pac.temp = pac.dir;
                    pac.last = NONE;
                    keyPressed = TRUE;
                    pac.dir = DOWN;
                    break;
                case SDLK_RIGHT :
                case SDLK_d :
                    pac.temp = pac.dir;
                    pac.last = NONE;
                    keyPressed = TRUE;
                    pac.dir = RIGHT;
                    break;
                case SDLK_LEFT :
                case SDLK_a :
                    pac.temp = pac.dir;
                    pac.last = NONE;
                    keyPressed = TRUE;
                    pac.dir = LEFT;
                    break;

                }
            }
        }

        movePac(&pac.dir,&pac.last,&pac.temp,&pac.x,&pac.Y, keyPressed);

        if(aiMode < 7)
        {
            diff = clock() - start;
            msec = (diff*1000/CLOCKS_PER_SEC);
            if(msec == (moveMode[aiMode]*100)/2)
            {
                aiMode++;
            }
            if(aiMode%2 == 0)
            {
                moveShadow(&shadow.x,&shadow.Y,&shadow.dir, 30*BLOCKSIZE,0,&shadow.l);
                moveSpeedy(&speedy.x,&speedy.Y,&speedy.dir,0,0,&speedy.l, pac.dir);
                moveBashful(&bashful.x,&bashful.Y,&bashful.dir,30*BLOCKSIZE,28*BLOCKSIZE,&bashful.l, pac.dir, shadow.x, shadow.Y);
                movePokey(&pokey.x,&pokey.Y,&pokey.dir, 0,28*BLOCKSIZE,&pokey.l,TRUE);
            }
            else
            {
                moveShadow(&shadow.x,&shadow.Y,&shadow.dir, pac.x,pac.Y,&shadow.l);
                moveSpeedy(&speedy.x,&speedy.Y,&speedy.dir,pac.x,pac.Y,&speedy.l, pac.dir);
                moveBashful(&bashful.x,&bashful.Y,&bashful.dir,pac.x,pac.Y,&bashful.l, pac.dir, shadow.x, shadow.Y);
                movePokey(&pokey.x,&pokey.Y,&pokey.dir, pac.x,pac.Y,&pokey.l,FALSE);
            }
        }
        //moveFrightened(&shadow.x,&shadow.Y,&shadow.dir);

        checkPill(&pac.x, &pac.Y);
        checkTeleport(&pac.x, pac.dir);
        checkTeleport(&shadow.x, shadow.dir);
        checkTeleport(&speedy.x, speedy.dir);
        checkTeleport(&bashful.x, bashful.dir);
        checkTeleport(&pokey.x, pokey.dir);

        // now we clear the screen (will use the clear colour set previously)
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);

        drawMaze(ren);
        drawPacman(ren, ptex, pac.x, pac.Y, pac.dir, frameCount);
        drawShadow(ren, gtex, shadow.x, shadow.Y, shadow.dir);
        drawSpeedy(ren, gtex, speedy.x, speedy.Y, speedy.dir);
        drawBashful(ren, gtex, bashful.x, bashful.Y, bashful.dir);
        drawPokey(ren,gtex,pokey.x,pokey.Y,pokey.dir);

        if(quit != TRUE)
            quit = checkVictory();
        // Up until now everything was drawn behind the scenes.
        // This will show the new, red contents of the window.
        SDL_RenderPresent(ren);
        frameCount++;
        if(frameCount > 28)
            frameCount = 0;

    }
    SDL_Quit();
    return EXIT_SUCCESS;
}

BOOL checkVictory()
{
    BOOL victory = TRUE;
    int mazeY = (sizeof(map)/sizeof(map[0]));
    int mazeX = (sizeof(map[0])/sizeof(map[0][0]));
    for(int i = 0; i < mazeY; ++i)
    {
        for(int j = 0; j < mazeX; ++j)
        {
            if (map[i][j] == RPILL)
                victory = FALSE;

        }
    }
    return victory;
}

BOOL checkMove(int dir, int x, int y, BOOL pac)
{
    BOOL valid = TRUE;
    int a,b,c;
    int step = BLOCKSIZE*scale;
    if(!pac)
        step+=1;
    int dim = (BLOCKSIZE-1)/2-1;

    switch (dir)
    {
    case UP:

        a = (int)round((y - step-dim)/((float)BLOCKSIZE));
        b = (int)round((x-dim)/((float)BLOCKSIZE));
        c = (int)round((x+dim)/((float)BLOCKSIZE));
        if((map[a][b] == BLUE)||(map[a][c] == BLUE)||(map[a][b] == GATE)||(map[a][c] == GATE))
            valid = FALSE;
        break;
    case DOWN:
        a = (int)round((y + step + dim)/((float)BLOCKSIZE));
        b = (int)round((x+dim)/((float)BLOCKSIZE));
        c = (int)round((x-dim)/((float)BLOCKSIZE));
        if((map[a][b] == BLUE)||(map[a][c] == BLUE)||(map[a][b] == GATE)||(map[a][c] == GATE))
            valid = FALSE;
        break;
    case LEFT:
        a = (int)round((y+dim)/((float)BLOCKSIZE));
        b = (int)round((y-dim)/((float)BLOCKSIZE));
        c = (int)round((x - step-dim)/((float)BLOCKSIZE));
        if((map[a][c] == BLUE)||(map[b][c] == BLUE)||(map[a][c] == GATE)||(map[b][c] == GATE))
            valid = FALSE;
        break;
    case RIGHT:
        a = (int)round((y+dim)/((float)BLOCKSIZE));
        b = (int)round((y-dim)/((float)BLOCKSIZE));
        c = (int)round((x + step+dim)/((float)BLOCKSIZE));
        if((map[a][c] == BLUE)||(map[b][c] == BLUE)||(map[a][c] == GATE)||(map[b][c] == GATE))
            valid = FALSE;
        break;
    case NONE:
        valid = TRUE;
        break;

    }
    return valid;
}
void checkPill(int *x, int *y)
{

    int a = (int)round(*y/((float)BLOCKSIZE));
    int b = (int)round(*x/((float)BLOCKSIZE));
    if(map[a][b] == RPILL)
        map[a][b] = 0;
}
void checkTeleport(int *x, int dir)
{
    int endBound = (sizeof(map[0])/sizeof(map[0][0]))*BLOCKSIZE-BLOCKSIZE;
    if((*x-BLOCKSIZE*0.5 <= 0)&&(dir == LEFT))
        *x = endBound-BLOCKSIZE*0.25;
    else if((*x+BLOCKSIZE*0.5 >= endBound)&&(dir == RIGHT))
        *x = 0;
}

void moveSprite(int *x, int *y, int dir)
{
    int step = BLOCKSIZE*scale;
    switch (dir)
    {

    case UP :
        *y-=step;
        break;
    case DOWN :
        *y+=step;
        break;
    case RIGHT :
        *x+=step;
        break;
    case LEFT :
        *x-=step;
        break;

    }
}
void moveShadow(int *x, int *y, int *dir,int pacX,int pacY,BOOL *la)
{
    int a = 0;
    int temp;
    int mhtnX = pacX-*x;
    int mhtnY = pacY-*y;
    for(int i = 0; i<4; ++i)
    {
        if(checkMove(i,*x,*y,FALSE))
            a++;
    }
    if(*dir == LEFT)
        temp = RIGHT;
    else if(*dir == RIGHT)
        temp = LEFT;
    else if(*dir == UP)
        temp = DOWN;
    else
        temp = UP;
    if(a == 2)
    {
        *la = TRUE;
        while((!checkMove(*dir,*x,*y,FALSE))||(*dir==temp))
        {
            *dir = rand()%4;
        }
    }
    else if((a>2)&&*la)
    {
        *la = FALSE;
        if(abs(mhtnX)>=abs(mhtnY))
        {
            if(mhtnX >= 0)
            {
                if((checkMove(RIGHT,*x,*y,FALSE))&&(temp!=RIGHT))
                    *dir = RIGHT;
                else if(mhtnY >= 0)
                {
                    if((checkMove(DOWN,*x,*y,FALSE))&&(temp!=DOWN))
                        *dir = DOWN;
                }
                else
                {
                    if((checkMove(UP,*x,*y,FALSE))&&(temp!=UP))
                        *dir = UP;
                }

            }
            else
            {
                if((checkMove(LEFT,*x,*y,FALSE))&&(temp!=LEFT))
                    *dir = LEFT;
                else if(mhtnY >= 0)
                {
                    if((checkMove(DOWN,*x,*y,FALSE))&&(temp!=DOWN))
                        *dir = DOWN;
                }
                else
                {
                    if((checkMove(UP,*x,*y,FALSE))&&(temp!=UP))
                        *dir = UP;
                }

            }
        }
        else
        {
            if(mhtnY>=0)
            {
                if((checkMove(DOWN,*x,*y,FALSE))&&(temp!=DOWN))
                    *dir = DOWN;
                else if(mhtnX>=0)
                {
                    if((checkMove(RIGHT,*x,*y,FALSE))&&(temp!=RIGHT))
                        *dir = RIGHT;
                }
                else
                {
                    if((checkMove(LEFT,*x,*y,FALSE))&&(temp!=LEFT))
                        *dir = LEFT;
                }
            }
            else
            {
                if((checkMove(UP,*x,*y,FALSE))&&(temp!=UP))
                    *dir = UP;
                else if(mhtnX>=0)
                {
                    if((checkMove(RIGHT,*x,*y,FALSE))&&(temp!=RIGHT))
                        *dir = RIGHT;
                }
                else
                {
                    if((checkMove(LEFT,*x,*y,FALSE))&&(temp!=LEFT))
                        *dir = LEFT;
                }
            }
        }
    }
    moveSprite(x,y,*dir);
}

void movePokey(int *x, int *y, int *dir,int pacX,int pacY,BOOL *la, BOOL scatter)
{
    if(scatter)
    {
        int distX = pacX-*x;
        int distY = pacY-*y;
        if(distX+distY > 8*BLOCKSIZE)
        {
            pacX = *x;
            pacY = *y;
        }
    }
    int a = 0;
    int temp;
    int mhtnX = pacX-*x;
    int mhtnY = pacY-*y;
    for(int i = 0; i<4; ++i)
    {
        if(checkMove(i,*x,*y,FALSE))
            a++;
    }
    if(*dir == LEFT)
        temp = RIGHT;
    else if(*dir == RIGHT)
        temp = LEFT;
    else if(*dir == UP)
        temp = DOWN;
    else
        temp = UP;
    if(a == 2)
    {
        *la = TRUE;
        while((!checkMove(*dir,*x,*y,FALSE))||(*dir==temp))
        {
            *dir = rand()%4;
        }
    }
    else if((a>2)&&*la)
    {
        *la = FALSE;
        if(abs(mhtnX)>=abs(mhtnY))
        {
            if(mhtnX >= 0)
            {
                if((checkMove(RIGHT,*x,*y,FALSE))&&(temp!=RIGHT))
                    *dir = RIGHT;
                else if(mhtnY >= 0)
                {
                    if((checkMove(DOWN,*x,*y,FALSE))&&(temp!=DOWN))
                        *dir = DOWN;
                }
                else
                {
                    if((checkMove(UP,*x,*y,FALSE))&&(temp!=UP))
                        *dir = UP;
                }

            }
            else
            {
                if((checkMove(LEFT,*x,*y,FALSE))&&(temp!=LEFT))
                    *dir = LEFT;
                else if(mhtnY >= 0)
                {
                    if((checkMove(DOWN,*x,*y,FALSE))&&(temp!=DOWN))
                        *dir = DOWN;
                }
                else
                {
                    if((checkMove(UP,*x,*y,FALSE))&&(temp!=UP))
                        *dir = UP;
                }

            }
        }
        else
        {
            if(mhtnY>=0)
            {
                if((checkMove(DOWN,*x,*y,FALSE))&&(temp!=DOWN))
                    *dir = DOWN;
                else if(mhtnX>=0)
                {
                    if((checkMove(RIGHT,*x,*y,FALSE))&&(temp!=RIGHT))
                        *dir = RIGHT;
                }
                else
                {
                    if((checkMove(LEFT,*x,*y,FALSE))&&(temp!=LEFT))
                        *dir = LEFT;
                }
            }
            else
            {
                if((checkMove(UP,*x,*y,FALSE))&&(temp!=UP))
                    *dir = UP;
                else if(mhtnX>=0)
                {
                    if((checkMove(RIGHT,*x,*y,FALSE))&&(temp!=RIGHT))
                        *dir = RIGHT;
                }
                else
                {
                    if((checkMove(LEFT,*x,*y,FALSE))&&(temp!=LEFT))
                        *dir = LEFT;
                }
            }
        }
    }
    moveSprite(x,y,*dir);
}


void moveBashful(int *x, int *y, int *dir,int pacX,int pacY,BOOL *la, int pacDir, int shadX, int shadY)
{
    int vectorX;
    int vectorY;
    switch(pacDir)
    {
        case UP:
            pacY-=(BLOCKSIZE*2);
            break;
        case DOWN:
            pacY+=(BLOCKSIZE*2);
            break;
        case LEFT:
            pacY-=(BLOCKSIZE*2);
            break;
        case RIGHT:
            pacY+=(BLOCKSIZE*2);
            break;
    }
    vectorX = (pacX-shadX)*2;
    vectorY = (pacY-shadY)*2;
    pacX=shadX+vectorX;
    pacY=shadY+vectorY;

    int a = 0;
    int temp;
    int mhtnX = pacX-*x;
    int mhtnY = pacY-*y;
    for(int i = 0; i<4; ++i)
    {
        if(checkMove(i,*x,*y,FALSE))
            a++;
    }
    if(*dir == LEFT)
        temp = RIGHT;
    else if(*dir == RIGHT)
        temp = LEFT;
    else if(*dir == UP)
        temp = DOWN;
    else
        temp = UP;
    if(a == 2)
    {
        *la = TRUE;
        while((!checkMove(*dir,*x,*y,FALSE))||(*dir==temp))
        {
            *dir = rand()%4;
        }
    }
    else if((a>2)&&*la)
    {
        *la = FALSE;
        if(abs(mhtnX)>=abs(mhtnY))
        {
            if(mhtnX >= 0)
            {
                if((checkMove(RIGHT,*x,*y,FALSE))&&(temp!=RIGHT))
                    *dir = RIGHT;
                else if(mhtnY >= 0)
                {
                    if((checkMove(DOWN,*x,*y,FALSE))&&(temp!=DOWN))
                        *dir = DOWN;
                }
                else
                {
                    if((checkMove(UP,*x,*y,FALSE))&&(temp!=UP))
                        *dir = UP;
                }

            }
            else
            {
                if((checkMove(LEFT,*x,*y,FALSE))&&(temp!=LEFT))
                    *dir = LEFT;
                else if(mhtnY >= 0)
                {
                    if((checkMove(DOWN,*x,*y,FALSE))&&(temp!=DOWN))
                        *dir = DOWN;
                }
                else
                {
                    if((checkMove(UP,*x,*y,FALSE))&&(temp!=UP))
                        *dir = UP;
                }

            }
        }
        else
        {
            if(mhtnY>=0)
            {
                if((checkMove(DOWN,*x,*y,FALSE))&&(temp!=DOWN))
                    *dir = DOWN;
                else if(mhtnX>=0)
                {
                    if((checkMove(RIGHT,*x,*y,FALSE))&&(temp!=RIGHT))
                        *dir = RIGHT;
                }
                else
                {
                    if((checkMove(LEFT,*x,*y,FALSE))&&(temp!=LEFT))
                        *dir = LEFT;
                }
            }
            else
            {
                if((checkMove(UP,*x,*y,FALSE))&&(temp!=UP))
                    *dir = UP;
                else if(mhtnX>=0)
                {
                    if((checkMove(RIGHT,*x,*y,FALSE))&&(temp!=RIGHT))
                        *dir = RIGHT;
                }
                else
                {
                    if((checkMove(LEFT,*x,*y,FALSE))&&(temp!=LEFT))
                        *dir = LEFT;
                }
            }
        }
    }
    moveSprite(x,y,*dir);
}

void moveSpeedy(int *x, int *y, int *dir,int pacX,int pacY,BOOL *la, int pacDir)
{
    switch(pacDir)
    {
        case UP:
            pacY-=(BLOCKSIZE*4);
            break;
        case DOWN:
            pacY+=(BLOCKSIZE*4);
            break;
        case LEFT:
            pacY-=(BLOCKSIZE*4);
            break;
        case RIGHT:
            pacY+=(BLOCKSIZE*4);
            break;
    }

    int a = 0;
    int temp;
    int mhtnX = pacX-*x;
    int mhtnY = pacY-*y;
    for(int i = 0; i<4; ++i)
    {
        if(checkMove(i,*x,*y,FALSE))
            a++;
    }
    if(*dir == LEFT)
        temp = RIGHT;
    else if(*dir == RIGHT)
        temp = LEFT;
    else if(*dir == UP)
        temp = DOWN;
    else
        temp = UP;
    if(a == 2)
    {
        *la = TRUE;
        while((!checkMove(*dir,*x,*y,FALSE))||(*dir==temp))
        {
            *dir = rand()%4;
        }
    }
    else if((a>2)&&*la)
    {
        *la = FALSE;
        if(abs(mhtnX)>=abs(mhtnY))
        {
            if(mhtnX >= 0)
            {
                if((checkMove(RIGHT,*x,*y,FALSE))&&(temp!=RIGHT))
                    *dir = RIGHT;
                else if(mhtnY >= 0)
                {
                    if((checkMove(DOWN,*x,*y,FALSE))&&(temp!=DOWN))
                        *dir = DOWN;
                }
                else
                {
                    if((checkMove(UP,*x,*y,FALSE))&&(temp!=UP))
                        *dir = UP;
                }

            }
            else
            {
                if((checkMove(LEFT,*x,*y,FALSE))&&(temp!=LEFT))
                    *dir = LEFT;
                else if(mhtnY >= 0)
                {
                    if((checkMove(DOWN,*x,*y,FALSE))&&(temp!=DOWN))
                        *dir = DOWN;
                }
                else
                {
                    if((checkMove(UP,*x,*y,FALSE))&&(temp!=UP))
                        *dir = UP;
                }

            }
        }
        else
        {
            if(mhtnY>=0)
            {
                if((checkMove(DOWN,*x,*y,FALSE))&&(temp!=DOWN))
                    *dir = DOWN;
                else if(mhtnX>=0)
                {
                    if((checkMove(RIGHT,*x,*y,FALSE))&&(temp!=RIGHT))
                        *dir = RIGHT;
                }
                else
                {
                    if((checkMove(LEFT,*x,*y,FALSE))&&(temp!=LEFT))
                        *dir = LEFT;
                }
            }
            else
            {
                if((checkMove(UP,*x,*y,FALSE))&&(temp!=UP))
                    *dir = UP;
                else if(mhtnX>=0)
                {
                    if((checkMove(RIGHT,*x,*y,FALSE))&&(temp!=RIGHT))
                        *dir = RIGHT;
                }
                else
                {
                    if((checkMove(LEFT,*x,*y,FALSE))&&(temp!=LEFT))
                        *dir = LEFT;
                }
            }
        }
    }
    moveSprite(x,y,*dir);
}


void movePac(int *dir, int *last, int *temp, int *x, int *y, int keyPressed)
{
    BOOL move, moveBckUp, movePrdct = FALSE;
    move = checkMove(*dir,*x,*y,TRUE);

    if(*last!=NONE)
        movePrdct = checkMove(*last,*x,*y,TRUE);
    else
        movePrdct = FALSE;

    moveBckUp = checkMove(*temp,*x,*y,TRUE);

    if((movePrdct == TRUE)&&(keyPressed == FALSE))
    {
        *dir = *last;
        *last = NONE;
        *temp = NONE;
        moveSprite(x, y, *dir);
    }
    else if(move == TRUE)
    {
        moveSprite(x, y, *dir);
    }
    else if(moveBckUp == TRUE)
    {
        *last = *dir;
        *dir = *temp;
        moveSprite(x, y, *dir);
    }

}

void drawShadow(SDL_Renderer *ren, SDL_Texture *tex, int x, int y, int dir)
{
    SDL_Rect block;
    block.x=x;
    block.y=y;
    block.w=BLOCKSIZE-1;
    block.h=BLOCKSIZE-1;
    SDL_Rect shadow;
    shadow.w=22;
    shadow.h=20;
    shadow.x=4*22;
    shadow.y=dir*20;
    SDL_RenderCopy(ren, tex,&shadow, &block);
}
void drawSpeedy(SDL_Renderer *ren, SDL_Texture *tex, int x, int y, int dir)
{
    SDL_Rect block;
    block.x=x;
    block.y=y;
    block.w=BLOCKSIZE-1;
    block.h=BLOCKSIZE-1;
    SDL_Rect shadow;
    shadow.w=22;
    shadow.h=20;
    shadow.x=2*22;
    shadow.y=dir*20;
    SDL_RenderCopy(ren, tex,&shadow, &block);
}
void drawBashful(SDL_Renderer *ren, SDL_Texture *tex, int x, int y, int dir)
{
    SDL_Rect block;
    block.x=x;
    block.y=y;
    block.w=BLOCKSIZE-1;
    block.h=BLOCKSIZE-1;
    SDL_Rect shadow;
    shadow.w=22;
    shadow.h=20;
    shadow.x=3*22;
    shadow.y=dir*20;
    SDL_RenderCopy(ren, tex,&shadow, &block);
}
void drawPokey(SDL_Renderer *ren, SDL_Texture *tex, int x, int y, int dir)
{
    SDL_Rect block;
    block.x=x;
    block.y=y;
    block.w=BLOCKSIZE-1;
    block.h=BLOCKSIZE-1;
    SDL_Rect shadow;
    shadow.w=22;
    shadow.h=20;
    shadow.x=1*22;
    shadow.y=dir*20;
    SDL_RenderCopy(ren, tex,&shadow, &block);
}
void drawPacman(SDL_Renderer *ren, SDL_Texture *tex, int x, int y, int dir, int frameCount)
{
    SDL_Rect block;
    block.x=x;
    block.y=y;
    block.w=BLOCKSIZE-1;
    block.h=BLOCKSIZE-1;
    SDL_Rect pacman;
    pacman.w=22;
    pacman.h=20;
    pacman.x=dir*22;
    pacman.y=(frameCount/7)*20;
    printf("%d\t%d\n",dir,frameCount);
    if(dir == NONE)
    {
        pacman.x=0;
        pacman.y=0;
    }
    SDL_RenderCopy(ren, tex,&pacman, &block);
}
void drawMaze(SDL_Renderer *ren)
{
    SDL_Rect block;
    for(int i = 0; i < ROWS; ++i)
    {
        for(int j = 0; j < COLS; ++j)
        {
            block.x=j*BLOCKSIZE;
            block.y=i*BLOCKSIZE;
            block.w=BLOCKSIZE;
            block.h=BLOCKSIZE;
            switch(map[i][j])
            {
            case (0):
                SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
                break;
            case (1):
                SDL_SetRenderDrawColor(ren, 0, 0, 255, 255);
                break;
            case (2):
                block.w=BLOCKSIZE/4;
                block.h=BLOCKSIZE/4;
                block.x+=((BLOCKSIZE/2)-block.w/4);
                block.y+=((BLOCKSIZE/2)-block.h/4);
                SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
                break;
            case (3):
                block.h=BLOCKSIZE/4;
                block.y+=((BLOCKSIZE/2)-block.h/2);
                SDL_SetRenderDrawColor(ren, 255, 185, 247, 191);

            }
            SDL_RenderFillRect(ren,&block);

        }
    }
}
void moveFrightened(int *x, int *y, int *dir)
{
    int a = 0;
    int temp;
    for(int i = 0; i<4; ++i)
    {
        if(checkMove(i,*x,*y,FALSE))
            a++;
    }
    if(*dir == LEFT)
        temp = RIGHT;
    else if(*dir == RIGHT)
        temp = LEFT;
    else if(*dir == UP)
        temp = DOWN;
    else
        temp = UP;
    if(a > 1)
    {
        *dir = rand()%4;
        while((!checkMove(*dir,*x,*y,FALSE))||(*dir==temp))
        {
            (*dir)++;
            if(*dir > 3)
                *dir = 0;
        }
    }
    moveSprite(x,y,*dir);
}
int pillCount()
{
    int count = 0;
    for(int i = 0; i < ROWS; ++i)
    {
        for(int j = 0; j < COLS; ++j)
        {
           if(map[i][j] == 2)
               count++;
        }
    }
    return count;
}
