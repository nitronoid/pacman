#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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
enum BLOCK{BLACK,BLUE,RPILL,};
enum BOOL{FALSE,TRUE};
typedef struct
{
    int x;
    int Y;
    int dir;
    int temp;
    int last;
}pacman;


int checkVictory()
{
    int victory = TRUE;
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

int checkMove(int dir, int x, int y)
{
    int valid = TRUE;
    int a,b,c;
    int step = BLOCKSIZE*scale*5;

    switch (dir)
    {
    case UP:

        a = (int)round((y - step - (BLOCKSIZE*scale))/((float)BLOCKSIZE));
        b = (int)round((x+step)/((float)BLOCKSIZE));
        c = (int)round((x-step)/((float)BLOCKSIZE));
        if((map[a][b] == BLUE)||(map[a][c] == BLUE))
            valid = FALSE;
        break;
    case DOWN:
        a = (int)round((y + step + (BLOCKSIZE*scale))/((float)BLOCKSIZE));
        b = (int)round((x+step)/((float)BLOCKSIZE));
        c = (int)round((x-step)/((float)BLOCKSIZE));
        if((map[a][b] == BLUE)||(map[a][c] == BLUE))
            valid = FALSE;
        break;
    case LEFT:
        a = (int)round((y+step)/((float)BLOCKSIZE));
        b = (int)round((y-step)/((float)BLOCKSIZE));
        c = (int)round((x - step - (BLOCKSIZE*scale))/((float)BLOCKSIZE));
        if((map[a][c] == BLUE)||(map[b][c] == BLUE))
            valid = FALSE;
        break;
    case RIGHT:
        a = (int)round((y+step)/((float)BLOCKSIZE));
        b = (int)round((y-step)/((float)BLOCKSIZE));
        c = (int)round((x + step + (BLOCKSIZE*scale))/((float)BLOCKSIZE));
        if((map[a][c] == BLUE)||(map[b][c] == BLUE))
            valid = FALSE;
        break;
    case NONE:
        valid = TRUE;
        break;

    }
    return valid;
}
void checkPill(int *x, int *y, int pacDir)
{
    int endBound = (sizeof(map[0])/sizeof(map[0][0]))*BLOCKSIZE-BLOCKSIZE;
    int a = (int)round(*y/((float)BLOCKSIZE));
    int b = (int)round(*x/((float)BLOCKSIZE));
    if(map[a][b] == RPILL)
        map[a][b] = 0;
    if((*x-BLOCKSIZE*0.5 <= 0)&&(pacDir == LEFT))
        *x = endBound-BLOCKSIZE*0.25;
    else if((*x+BLOCKSIZE*0.5 >= endBound)&&(pacDir == RIGHT))
        *x = 0;
}

void movePac(int *pacposX, int *pacposY, int pacDir)
{
    int step = BLOCKSIZE*scale;
    switch (pacDir)
    {

    case UP :
        *pacposY-=step;
        break;
    case DOWN :
        *pacposY+=step;
        break;
    case RIGHT :
        *pacposX+=step;
        break;
    case LEFT :
        *pacposX-=step;
        break;

    }
}

void drawPacman(SDL_Renderer *ren, SDL_Texture *tex, int x, int y, int dir, int frameCount)
{
    SDL_Rect block;
    block.x=x;
    block.y=y;
    SDL_Rect pacman;
    pacman.w=22;
    pacman.h=20;
    pacman.x=dir*22;
    pacman.y=(frameCount/7)*20;
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
    int mazeY = (sizeof(map)/sizeof(map[0]));
    int mazeX = (sizeof(map[0])/sizeof(map[0][0]));
    for(int i = 0; i < mazeY; ++i)
    {
        for(int j = 0; j < mazeX; ++j)
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
                block.x+=(BLOCKSIZE/2);
                block.y+=(BLOCKSIZE/2);
                block.w=BLOCKSIZE/4;
                block.h=BLOCKSIZE/4;
                SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
                break;

            }
            SDL_RenderFillRect(ren,&block);

        }
    }
    ;
}


int main()
{
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

    SDL_Texture *tex = 0;
    tex = SDL_CreateTextureFromSurface(ren, image);
    // free the image
    SDL_FreeSurface(image);


    int quit=FALSE;
    // now we are going to loop forever, process the keys then draw

    pacman pac = {13*BLOCKSIZE,17*BLOCKSIZE,NONE,NONE,NONE};
    int move, moveBckUp, movePrdct, frameCount, keyPressed = 0;


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
                pac.temp = pac.dir;
                pac.last = NONE;
                keyPressed = TRUE;
                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE :
                    quit=TRUE;
                    break;
                case SDLK_UP :
                case SDLK_w :
                    pac.dir = UP;
                    break;
                case SDLK_DOWN :
                case SDLK_s :
                    pac.dir = DOWN;
                    break;
                case SDLK_RIGHT :
                case SDLK_d :
                    pac.dir = RIGHT;
                    break;
                case SDLK_LEFT :
                case SDLK_a :
                    pac.dir = LEFT;
                    break;

                }

            }
        }

        checkPill(&pac.x, &pac.Y, pac.dir);

        move = checkMove(pac.dir,pac.x,pac.Y);

        if(pac.last!=NONE)
            movePrdct = checkMove(pac.last,pac.x,pac.Y);

        moveBckUp = checkMove(pac.temp,pac.x,pac.Y);

        if((movePrdct == TRUE)&&(keyPressed == FALSE))
        {
            pac.dir = pac.last;
            movePac(&pac.x, &pac.Y, pac.dir);
        }
        else if(move == TRUE)
        {
            movePac(&pac.x, &pac.Y, pac.dir);
        }
        else if((moveBckUp == TRUE)&&(keyPressed == TRUE))
        {
            pac.last = pac.dir;
            pac.dir = pac.temp;
            movePac(&pac.x, &pac.Y, pac.dir);
        }

        // now we clear the screen (will use the clear colour set previously)
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);
        // we will create an SDL_Rect structure and draw a block

        // set the block position
        drawMaze(ren);
        drawPacman(ren, tex, pac.x, pac.Y, pac.dir, frameCount);
        // Up until now everything was drawn behind the scenes.
        // This will show the new, red contents of the window.

        /*
        if(quit != TRUE)
            quit = checkVictory();
        */
        SDL_RenderPresent(ren);
        frameCount++;
        if(frameCount > 28)
            frameCount = 0;

    }


    SDL_Quit();
    return EXIT_SUCCESS;
}
