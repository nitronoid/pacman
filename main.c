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
enum DIRECTION{UP,DOWN,LEFT,RIGHT,NONE};

int checkMove(int dir, int x, int y)
{
    int valid = 1;
    int a,b,c;
    int step = BLOCKSIZE*scale*2;

    switch (dir)
    {
    case UP:

        a = (int)round((y - step - (BLOCKSIZE*scale))/((float)BLOCKSIZE));
        b = (int)round((x+step)/((float)BLOCKSIZE));
        c = (int)round((x-step)/((float)BLOCKSIZE));
        if((map[a][b] == 1)||(map[a][c] == 1))
            valid = 0;
        break;
    case DOWN:
        a = (int)round((y + step + (BLOCKSIZE*scale))/((float)BLOCKSIZE));
        b = (int)round((x+step)/((float)BLOCKSIZE));
        c = (int)round((x-step)/((float)BLOCKSIZE));
        if((map[a][b] == 1)||(map[a][c] == 1))
            valid = 0;
        break;
    case LEFT:
        a = (int)round((y+step)/((float)BLOCKSIZE));
        b = (int)round((y-step)/((float)BLOCKSIZE));
        c = (int)round((x - step - (BLOCKSIZE*scale))/((float)BLOCKSIZE));
        if((map[a][c] == 1)||(map[b][c] == 1))
            valid = 0;
        break;
    case RIGHT:
        a = (int)round((y+step)/((float)BLOCKSIZE));
        b = (int)round((y-step)/((float)BLOCKSIZE));
        c = (int)round((x + step + (BLOCKSIZE*scale))/((float)BLOCKSIZE));
        if((map[a][c] == 1)||(map[b][c] == 1))
            valid = 0;
        break;

    }
    return valid;
}

void movePac(int *pacposX, int *pacposY, int pacDir)
{
    int valid = 1;
    switch (pacDir)
    {

    case UP :
        *pacposY-=(BLOCKSIZE*scale);
        break;
    case DOWN :
        *pacposY+=(BLOCKSIZE*scale);
        break;
    case RIGHT :
        *pacposX+=(BLOCKSIZE*scale);
        break;
    case LEFT :
        *pacposX-=(BLOCKSIZE*scale);
        break;

    }
}

void drawPacman(SDL_Rect block,SDL_Renderer *ren, SDL_Texture *tex, int pacposX, int pacposY, int pacDir)
{
    block.x=pacposX;
    block.y=pacposY;
    SDL_Rect pacman;
    pacman.w=22;
    pacman.h=20;
    pacman.x=2*22;
    pacman.y=3*20;
    SDL_RenderCopy(ren, tex,&pacman, &block);
}
void drawMaze(SDL_Rect block,SDL_Renderer *ren, SDL_Texture *tex, int pacposX, int pacposY, int pacDir)
{
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
                block.x+=(BLOCKSIZE/5);
                block.y+=(BLOCKSIZE/5);
                block.w=BLOCKSIZE/2;
                block.h=BLOCKSIZE/2;
                SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
                break;

            }
            SDL_RenderFillRect(ren,&block);

        }
    }
    drawPacman(block, ren, tex, pacposX, pacposY, pacDir);
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

    // SDL image is an abstraction for all images
    SDL_Surface *image;
    // we are going to use the extension SDL_image library to load a png, documentation can be found here
    // http://www.libsdl.org/projects/SDL_image/
    image=IMG_Load("pacsprite.png");
    if(!image)
    {
        printf("IMG_Load: %s\n", IMG_GetError());
        return EXIT_FAILURE;
    }
    // SDL texture converts the image to a texture suitable for SDL rendering  / blitting
    // once we have the texture it will be store in hardware and we don't need the image data anymore

    SDL_Texture *tex = 0;
    tex = SDL_CreateTextureFromSurface(ren, image);
    // free the image
    SDL_FreeSurface(image);



    int quit=0;
    // now we are going to loop forever, process the keys then draw

    int pacposX = 13*BLOCKSIZE;
    int pacposY = 17*BLOCKSIZE;
    int pacDir;
    int move = 0;
    while (quit !=1)
    {

        SDL_Event event;
        // grab the SDL even (this will be keys etc)
        while (SDL_PollEvent(&event))
        {
            // look for the x of the window being clicked and exit
            if (event.type == SDL_QUIT)
                quit = 1;
            // check for a key down
            if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE :
                    quit=1;
                    break;
                case SDLK_UP :
                    pacDir = UP;
                    break;
                case SDLK_w :
                    pacDir = UP;
                    break;
                case SDLK_DOWN :
                    pacDir = DOWN;
                    break;
                case SDLK_s :
                    pacDir = DOWN;
                    break;
                case SDLK_RIGHT :
                    pacDir = RIGHT;
                    break;
                case SDLK_d :
                    pacDir = RIGHT;
                    break;
                case SDLK_LEFT :
                    pacDir = LEFT;
                    break;
                case SDLK_a :
                    pacDir = LEFT;
                    break;

                }

            }
        }

        move = checkMove(pacDir,pacposX,pacposY);
        if(move == 1)
        {
            //usleep(20000);
            movePac(&pacposX, &pacposY, pacDir);
        }
        // now we clear the screen (will use the clear colour set previously)
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);
        // we will create an SDL_Rect structure and draw a block
        SDL_Rect block;
        // set the block position
        drawMaze(block, ren, tex, pacposX, pacposY, pacDir);
        // Up until now everything was drawn behind the scenes.
        // This will show the new, red contents of the window.
        SDL_RenderPresent(ren);

    }


    SDL_Quit();
    return EXIT_SUCCESS;
}
