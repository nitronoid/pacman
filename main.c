#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
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
enum BLOCK{BLACK,BLUE,RPILL,GATE,POWERPILL,HOME};
typedef enum {FALSE,TRUE}BOOL;

typedef struct
{
    int x;
    int Y;
    int dir;
    int temp;
    int last;
    BOOL alive;
}pacman;
typedef struct
{
    int x;
    int Y;
    int dir;
    BOOL gate;
    BOOL l;
    int tx;
    int ty;
    BOOL alive;
}ghost;

BOOL checkVictory();
BOOL checkMove(int dir, int x, int y, BOOL pac);
BOOL checkDeath(int shadX,int shadY,int speeX,int speeY,int blinX,int blinY,int pokeX,int pokeY,int pacX,int pacY);
BOOL checkGhost(int x, int y, int pacX,int pacY,BOOL alive);
void drawStart(SDL_Renderer *ren, SDL_Texture *tex);
void drawGameOver(SDL_Renderer *ren, SDL_Texture *tex);
void checkPill(int *x, int *y, BOOL *frightened, clock_t *fStart, int aiMode, int *t);
void moveSprite(int *x, int *y, int dir, BOOL pac, BOOL frightened);
void moveShadow(int *x, int *y, int *dir, int pacX, int pacY, BOOL *la);
void moveSpeedy(int *x, int *y, int *dir,int pacX,int pacY,BOOL *la, int pacDir, BOOL *gate);
void moveBashful(int *x, int *y, int *dir,int pacX,int pacY,BOOL *la, int pacDir, int shadX, int shadY, BOOL *gate);
void movePokey(int *x, int *y, int *dir,int pacX,int pacY,BOOL *la, int *tempX, int *tempY, BOOL *loop, BOOL *gate);
void moveFrightened(int *x, int *y, int *dir, int *tempX, int *tempY);
void movePac(int *dir, int *last, int *temp, int *x, int *y, int keyPressed);
void drawGhost(SDL_Renderer *ren, SDL_Texture *tex, int x, int y, int dir, BOOL frightened, clock_t fClock, int ghostType);
void drawPacman(SDL_Renderer *ren, SDL_Texture *tex, int x, int y, int dir, int frameCount);
void drawDeadPac(SDL_Renderer *ren, SDL_Texture *tex, int x, int y, int dir, int frameCount);
void drawMaze(SDL_Renderer *ren, SDL_Texture *btex);
void checkTeleport(int *x, int dir);
int pillCount();
int reverseDir(int dir);

int main()
{
    //fixMap();
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

    image=IMG_Load("pills.png");
    if(!image)
    {
        printf("IMG_Load: %s\n", IMG_GetError());
        return EXIT_FAILURE;
    }

    SDL_Texture *btex = 0;
    btex = SDL_CreateTextureFromSurface(ren, image);
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

    image=IMG_Load("startScreen.png");
    if(!image)
    {
        printf("IMG_Load: %s\n", IMG_GetError());
        return EXIT_FAILURE;
    }

    SDL_Texture *stex = 0;
    stex = SDL_CreateTextureFromSurface(ren, image);
    // free the image
    SDL_FreeSurface(image);

    image=IMG_Load("gameOver.png");
    if(!image)
    {
        printf("IMG_Load: %s\n", IMG_GetError());
        return EXIT_FAILURE;
    }

    SDL_Texture *etex = 0;
    etex = SDL_CreateTextureFromSurface(ren, image);
    // free the image
    SDL_FreeSurface(image);

    image=IMG_Load("pacDeath.png");
    if(!image)
    {
        printf("IMG_Load: %s\n", IMG_GetError());
        return EXIT_FAILURE;
    }

    SDL_Texture *dtex = 0;
    dtex = SDL_CreateTextureFromSurface(ren, image);
    // free the image
    SDL_FreeSurface(image);


    BOOL quit=FALSE;
    // now we are going to loop forever, process the keys then draw

    ghost shadow = {14*BLOCKSIZE,11*BLOCKSIZE,LEFT,TRUE,TRUE,0,0,TRUE};
    ghost speedy = {13*BLOCKSIZE,13*BLOCKSIZE,UP,TRUE,TRUE,0,0,TRUE};
    ghost bashful = {14*BLOCKSIZE,13*BLOCKSIZE,LEFT,TRUE,TRUE,0,0,TRUE};
    ghost pokey = {13*BLOCKSIZE,14*BLOCKSIZE,RIGHT,TRUE,TRUE,0,0,TRUE};
    pacman pac = {13*BLOCKSIZE,17*BLOCKSIZE+1,NONE,NONE,NONE,TRUE};
    BOOL keyPressed = FALSE;
    BOOL begin = FALSE;
    BOOL loop = FALSE;
    BOOL pokeyMove = FALSE;
    BOOL bashfulMove = FALSE;
    BOOL chngDir = TRUE;
    BOOL frightened = FALSE;
    clock_t frightenedClock;
    int pTempX = 0;
    int pTempY = 0;
    int frameCount = 0;
    int deathCount = 0;
    int aiMode = 0;
    int msec;
    int moveMode[7]={7,27,34,54,59,79,84};

    while (quit !=TRUE)
    {

        SDL_Event event;
        // grab the SDL event (this will be keys etc)
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
                    begin = TRUE;
                    pac.temp = pac.dir;
                    pac.last = NONE;
                    keyPressed = TRUE;
                    pac.dir = UP;
                    break;
                case SDLK_DOWN :
                case SDLK_s :
                    begin = TRUE;
                    pac.temp = pac.dir;
                    pac.last = NONE;
                    keyPressed = TRUE;
                    pac.dir = DOWN;
                    break;
                case SDLK_RIGHT :
                case SDLK_d :
                    begin = TRUE;
                    pac.temp = pac.dir;
                    pac.last = NONE;
                    keyPressed = TRUE;
                    pac.dir = RIGHT;
                    break;
                case SDLK_LEFT :
                case SDLK_a :
                    begin = TRUE;
                    pac.temp = pac.dir;
                    pac.last = NONE;
                    keyPressed = TRUE;
                    pac.dir = LEFT;
                    break;

                }
            }
        }

        if(!frightened)
            pac.alive = checkDeath(shadow.x,shadow.Y,speedy.x,speedy.Y,bashful.x,bashful.Y,pokey.x,pokey.Y,pac.x,pac.Y);
        else
        {
            shadow.alive = checkGhost(shadow.x,shadow.Y,pac.x,pac.Y,shadow.alive);
            speedy.alive = checkGhost(speedy.x,speedy.Y,pac.x,pac.Y,speedy.alive);
            bashful.alive = checkGhost(bashful.x,bashful.Y,pac.x,pac.Y, bashful.alive);
            pokey.alive = checkGhost(pokey.x,pokey.Y,pac.x,pac.Y, pokey.alive);

        }
        if(!shadow.alive)
            shadow.alive = checkGhost(shadow.x,shadow.Y,pac.x,pac.Y,shadow.alive);
        if(!speedy.alive)
            speedy.alive = checkGhost(speedy.x,speedy.Y,pac.x,pac.Y,speedy.alive);
        if(!bashful.alive)
            bashful.alive = checkGhost(bashful.x,bashful.Y,pac.x,pac.Y, bashful.alive);
        if(!pokey.alive)
            pokey.alive = checkGhost(pokey.x,pokey.Y,pac.x,pac.Y, pokey.alive);
        if(begin&&pac.alive)
        {
            movePac(&pac.dir,&pac.last,&pac.temp,&pac.x,&pac.Y, keyPressed);

            if((pillCount() <= 270)&&!frightened)
                bashfulMove = TRUE;
            if((pillCount() <= 200)&&!frightened)
                pokeyMove = TRUE;


            if(frightened)
            {
                if(chngDir)
                {
                    shadow.dir = reverseDir(shadow.dir);
                    speedy.dir = reverseDir(speedy.dir);
                    bashful.dir = reverseDir(bashful.dir);
                    pokey.dir = reverseDir(pokey.dir);
                }
                chngDir = FALSE;
                if(shadow.alive)
                    moveFrightened(&shadow.x,&shadow.Y,&shadow.dir,&shadow.tx,&shadow.ty);
                else
                    moveShadow(&shadow.x,&shadow.Y,&shadow.dir, 13*BLOCKSIZE,11*BLOCKSIZE,&shadow.l);
                if(speedy.alive)
                    moveFrightened(&speedy.x,&speedy.Y,&speedy.dir,&speedy.tx,&speedy.ty);
                else
                    moveShadow(&speedy.x,&speedy.Y,&speedy.dir, 13*BLOCKSIZE,11*BLOCKSIZE,&speedy.l);
                if(!bashful.gate)
                {
                    if(bashful.alive)
                        moveFrightened(&bashful.x,&bashful.Y,&bashful.dir,&bashful.tx,&bashful.ty);
                    else
                        moveShadow(&bashful.x,&bashful.Y,&bashful.dir, 13*BLOCKSIZE,11*BLOCKSIZE,&bashful.l);
                }
                if(!pokey.gate)
                {
                    if(pokey.alive)
                        moveFrightened(&pokey.x,&pokey.Y,&pokey.dir,&pokey.tx,&pokey.ty);
                    else
                        moveShadow(&pokey.x,&pokey.Y,&pokey.dir, 13*BLOCKSIZE,11*BLOCKSIZE,&pokey.l);
                }
                diff = clock() - frightenedClock;
                msec = (diff*1000/CLOCKS_PER_SEC);
                if(msec >= 700)
                    frightened = FALSE;
            }
            else
            {
                chngDir = TRUE;
                diff = clock() - start;
                msec = (diff*1000/CLOCKS_PER_SEC);
                if((msec >= (moveMode[aiMode]*100)*2)&&(aiMode < 7))
                {
                    aiMode++;
                }
                if(aiMode%2 == 0)
                {
                    if(shadow.alive)
                        moveShadow(&shadow.x,&shadow.Y,&shadow.dir, 30*BLOCKSIZE,0,&shadow.l);
                    else
                        moveShadow(&shadow.x,&shadow.Y,&shadow.dir, 13*BLOCKSIZE,11*BLOCKSIZE,&shadow.l);
                    if(speedy.alive)
                        moveSpeedy(&speedy.x,&speedy.Y,&speedy.dir,0,0,&speedy.l, pac.dir,&speedy.gate);
                    else
                        moveShadow(&speedy.x,&speedy.Y,&speedy.dir, 13*BLOCKSIZE,11*BLOCKSIZE,&speedy.l);
                    if(bashfulMove)
                    {
                        if(bashful.alive)
                            moveBashful(&bashful.x,&bashful.Y,&bashful.dir,30*BLOCKSIZE,28*BLOCKSIZE,&bashful.l, pac.dir, shadow.x, shadow.Y,&bashful.gate);
                        else
                            moveShadow(&bashful.x,&bashful.Y,&bashful.dir, 13*BLOCKSIZE,11*BLOCKSIZE,&bashful.l);
                    }
                    if(pokeyMove)
                    {
                        if(pokey.alive)
                            movePokey(&pokey.x,&pokey.Y,&pokey.dir, 0,28*BLOCKSIZE,&pokey.l,&pTempX, &pTempY, &loop,&pokey.gate);
                        else
                            moveShadow(&pokey.x,&pokey.Y,&pokey.dir, 13*BLOCKSIZE,11*BLOCKSIZE,&pokey.l);
                    }
                }
                else
                {
                    if(shadow.alive)
                        moveShadow(&shadow.x,&shadow.Y,&shadow.dir, pac.x,pac.Y,&shadow.l);
                    else
                        moveShadow(&shadow.x,&shadow.Y,&shadow.dir, 13*BLOCKSIZE,11*BLOCKSIZE,&shadow.l);
                    if(speedy.alive)
                        moveSpeedy(&speedy.x,&speedy.Y,&speedy.dir,pac.x,pac.Y,&speedy.l, pac.dir,&speedy.gate);
                    else
                        moveShadow(&speedy.x,&speedy.Y,&speedy.dir, 13*BLOCKSIZE,11*BLOCKSIZE,&speedy.l);
                    if(bashfulMove)
                    {
                        if(bashful.alive)
                            moveBashful(&bashful.x,&bashful.Y,&bashful.dir,pac.x,pac.Y,&bashful.l, pac.dir, shadow.x, shadow.Y,&bashful.gate);
                        else
                            moveShadow(&bashful.x,&bashful.Y,&bashful.dir, 13*BLOCKSIZE,11*BLOCKSIZE,&bashful.l);
                    }
                    if(pokeyMove)
                    {
                        if(pokey.alive)
                            movePokey(&pokey.x,&pokey.Y,&pokey.dir, pac.x,pac.Y,&pokey.l,&pTempX, &pTempY, &loop,&pokey.gate);
                        else
                            moveShadow(&pokey.x,&pokey.Y,&pokey.dir, 13*BLOCKSIZE,11*BLOCKSIZE,&pokey.l);
                    }
                }
            }
        }
        //printf("%d\n",aiMode);


        checkPill(&pac.x, &pac.Y, &frightened, &frightenedClock, aiMode, &moveMode[0]);
        checkTeleport(&pac.x, pac.dir);
        checkTeleport(&shadow.x, shadow.dir);
        checkTeleport(&speedy.x, speedy.dir);
        checkTeleport(&bashful.x, bashful.dir);
        checkTeleport(&pokey.x, pokey.dir);

        // now we clear the screen (will use the clear colour set previously)
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);

        drawMaze(ren, btex);

        if(shadow.alive)
            drawGhost(ren, gtex, shadow.x, shadow.Y, shadow.dir, frightened, frightenedClock,4);
        else
            drawGhost(ren, gtex, shadow.x, shadow.Y, shadow.dir, FALSE, frightenedClock,0);
        if(speedy.alive)
            drawGhost(ren, gtex, speedy.x, speedy.Y, speedy.dir, frightened, frightenedClock,2);
        else
            drawGhost(ren, gtex, speedy.x, speedy.Y, speedy.dir, FALSE, frightenedClock,0);
        if(bashful.alive)
            drawGhost(ren, gtex, bashful.x, bashful.Y, bashful.dir, frightened, frightenedClock,3);
        else
            drawGhost(ren, gtex, bashful.x, bashful.Y, bashful.dir, FALSE, frightenedClock,0);
        if(pokey.alive)
            drawGhost(ren,gtex,pokey.x,pokey.Y,pokey.dir, frightened, frightenedClock,1);
        else
            drawGhost(ren,gtex,pokey.x,pokey.Y,pokey.dir, FALSE, frightenedClock,0);
        if(pac.alive)
            drawPacman(ren, ptex, pac.x, pac.Y, pac.dir, frameCount);
        else
        {
            drawDeadPac(ren, dtex, pac.x, pac.Y, pac.dir, deathCount);
            drawGameOver(ren,etex);
        }
        if(!begin)
            drawStart(ren,stex);

        if(quit != TRUE)
            quit = checkVictory();
        // Up until now everything was drawn behind the scenes.
        // This will show the new, red contents of the window.
        SDL_RenderPresent(ren);
        if(!pac.alive&&(deathCount<33))
            deathCount++;
        else
        {
            frameCount++;
            if(frameCount > 28)
                frameCount = 0;
        }

    }
    SDL_Quit();
    return EXIT_SUCCESS;
}
void drawStart(SDL_Renderer *ren, SDL_Texture *tex)
{
    SDL_Rect screenBlock;
    screenBlock.h = 775;
    screenBlock.w = 700;
    screenBlock.x=0;
    screenBlock.y=0;
    SDL_Rect img;
    img.x=0;
    img.y=0;
    img.w = 700;
    img.h = 775;
    SDL_RenderCopy(ren,tex,&img,&screenBlock);
}
void drawGameOver(SDL_Renderer *ren, SDL_Texture *tex)
{
    SDL_Rect screenBlock;
    screenBlock.h = 775;
    screenBlock.w = 700;
    screenBlock.x=0;
    screenBlock.y=0;
    SDL_Rect img;
    img.x=0;
    img.y=0;
    img.w = 700;
    img.h = 775;
    SDL_RenderCopy(ren,tex,&img,&screenBlock);
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
            if ((map[i][j] == RPILL)||map[i][j] == POWERPILL)
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
void checkPill(int *x, int *y, BOOL *frightened, clock_t *fStart, int aiMode, int *t)
{

    int a = (int)round(*y/((float)BLOCKSIZE));
    int b = (int)round(*x/((float)BLOCKSIZE));
    if(map[a][b] == RPILL)
        map[a][b] = BLACK;
    else if(map[a][b] == POWERPILL)
    {
        map[a][b] = BLACK;
        *frightened = TRUE;
        *fStart = clock();
        for(int i = aiMode; i < 8; i++)
        {
            *(t+i)+=7;
        }
    }

}
void checkTeleport(int *x, int dir)
{
    int endBound = (sizeof(map[0])/sizeof(map[0][0]))*BLOCKSIZE-BLOCKSIZE;
    if((*x-BLOCKSIZE*0.5 <= 0)&&(dir == LEFT))
        *x = endBound-BLOCKSIZE*0.25;
    else if((*x+BLOCKSIZE*0.5 >= endBound)&&(dir == RIGHT))
        *x = 0;
}

void moveSprite(int *x, int *y, int dir, BOOL pac, BOOL frightened)
{

    int step = BLOCKSIZE*scale;
    if(!pac)
    {
        if(frightened)
            step-=1;
    }
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
    temp = reverseDir(*dir);
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
    moveSprite(x,y,*dir, FALSE, FALSE);
}

void movePokey(int *x, int *y, int *dir,int pacX,int pacY,BOOL *la, int *tempX, int *tempY, BOOL *loop, BOOL *gate)
{
    if(*y <= 11*BLOCKSIZE)
        *gate = FALSE;
    else if (*gate)
        *dir = UP;
    if(!(*gate))
    {
        int distX = abs(pacX-*x);
        int distY = abs(pacY-*y);
        int dist = distX + distY;
        if((dist < 8*BLOCKSIZE)&&!(*loop))
        {
            *loop = TRUE;
            *tempX = *x;
            *tempY = *y;
        }
        if(*loop)
        {
            *loop = FALSE;
            pacX = *tempX;
            pacY = *tempY;
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
        temp = reverseDir(*dir);
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
    }
    moveSprite(x,y,*dir, FALSE, FALSE);
}


void moveBashful(int *x, int *y, int *dir,int pacX,int pacY,BOOL *la, int pacDir, int shadX, int shadY, BOOL *gate)
{

    if(*y <= 11*BLOCKSIZE)
        *gate = FALSE;
    else if (*gate)
        *dir = UP;
    if(!(*gate))
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
        temp = reverseDir(*dir);
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
    }
    moveSprite(x,y,*dir, FALSE, FALSE);
}

void moveSpeedy(int *x, int *y, int *dir,int pacX,int pacY,BOOL *la, int pacDir, BOOL *gate)
{
    if(*y <= 11*BLOCKSIZE)
        *gate = FALSE;
    else if (*gate)
        *dir = UP;
    if(!(*gate))
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
        temp = reverseDir(*dir);
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
    }
    moveSprite(x,y,*dir, FALSE, FALSE);
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
        moveSprite(x, y, *dir, TRUE, FALSE);
    }
    else if(move == TRUE)
    {
        moveSprite(x, y, *dir, TRUE, FALSE);
    }
    else if(moveBckUp == TRUE)
    {
        *last = *dir;
        *dir = *temp;
        moveSprite(x, y, *dir, TRUE, FALSE);
    }

}

void drawGhost(SDL_Renderer *ren, SDL_Texture *tex, int x, int y, int dir, BOOL frightened, clock_t fClock, int ghostType)
{
    clock_t diff = clock() - fClock;
    int msec = (diff*1000/CLOCKS_PER_SEC);
    int desc = 0;
    SDL_Rect block;
    block.x=x;
    block.y=y;
    block.w=BLOCKSIZE-1;
    block.h=BLOCKSIZE-1;
    SDL_Rect ghost;
    ghost.w=22;
    ghost.h=20;
    ghost.x=ghostType*22;
    ghost.y=dir*20;
    if(frightened)
    {
        if(msec >= 400)
        {
           desc = msec-((int)(msec/50))*50;
           if(desc > 25)
           {
               ghost.x=5*22+5;
               ghost.y=1*20;
           }
           else
           {
               ghost.x=5*22+5;
               ghost.y=0*20;
           }
        }
        else
        {
        ghost.x=5*22+5;
        ghost.y=1*20;
        }
    }
    SDL_RenderCopy(ren, tex,&ghost, &block);
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
    if(dir == NONE)
    {
        pacman.x=0;
        pacman.y=0;
    }
    SDL_RenderCopy(ren, tex,&pacman, &block);
}
void drawMaze(SDL_Renderer *ren, SDL_Texture *tex)
{
    SDL_Rect block;
    SDL_Rect pill;
    pill.w = 25;
    pill.h = 25;

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
            case (BLACK):
                SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
                SDL_RenderFillRect(ren,&block);
                break;
            case (BLUE):
                SDL_SetRenderDrawColor(ren, 0, 0, 255, 255);
                SDL_RenderDrawRect(ren,&block);
                break;
            case (RPILL):
                pill.x = 0;
                pill.y = 0;
                SDL_RenderCopy(ren, tex, &pill, &block);
                break;
            case (GATE):
                block.h=BLOCKSIZE/8;
                block.y+=((BLOCKSIZE/2)-block.h/2);
                SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
                SDL_RenderFillRect(ren,&block);
                break;
            case(POWERPILL):
                pill.x = BLOCKSIZE;
                SDL_RenderCopy(ren, tex, &pill, &block);
                break;
            }


        }
    }
}
void moveFrightened(int *x, int *y, int *dir, int *tempX, int *tempY)
{
    int a = 0;
    int temp;
    for(int i = 0; i<4; ++i)
    {
        if(checkMove(i,*x,*y,FALSE))
            a++;
    }

    temp = reverseDir(*dir);
    if((a > 1)&&((abs(*tempX-*x)>2)||(abs(*tempY-*y)>2)))
    {
        *tempX = *x;
        *tempY = *y;
        *dir = rand()%4;
        while((!checkMove(*dir,*x,*y,FALSE))||(*dir==temp))
        {
            (*dir)++;
            if(*dir > 3)
                *dir = 0;
        }
    }

    moveSprite(x, y, *dir, FALSE, TRUE);
}
int pillCount()
{
    int count = 0;
    for(int i = 0; i < ROWS; ++i)
    {
        for(int j = 0; j < COLS; ++j)
        {
           if((map[i][j] == RPILL)||(map[i][j] == POWERPILL))
               count++;
        }
    }
    return count;
}
int reverseDir(int dir)
{
    switch (dir) {
    case UP:
        return DOWN;
    case RIGHT:
        return LEFT;
    case DOWN:
        return UP;
    case LEFT:
        return RIGHT;

    }
    return NONE;
}
BOOL checkDeath(int shadX,int shadY,int speeX,int speeY,int blinX,int blinY,int pokeX,int pokeY,int pacX,int pacY)
{
    int diff = 3*BLOCKSIZE/4;
    if((abs(pacX-shadX)<=diff)&&(abs(pacY-shadY)<=diff))
        return FALSE;
    else if((abs(pacX-speeX)<=diff)&&(abs(pacY-speeY)<=diff))
        return FALSE;
    else if((abs(pacX-blinX)<=diff)&&(abs(pacY-blinY)<=diff))
        return FALSE;
    else if((abs(pacX-pokeX)<=diff)&&(abs(pacY-pokeY)<=diff))
        return FALSE;
    else
        return TRUE;
}
BOOL checkGhost(int x, int y, int pacX,int pacY, BOOL alive)
{
    int diff = 3*BLOCKSIZE/4;
    if(alive)
    {
        if((abs(pacX-x)<=diff)&&(abs(pacY-y)<=diff))
            return FALSE;
        else
            return TRUE;
    }
    else
    {
        if((abs(y-11*BLOCKSIZE)<=diff)&&(abs(x-13*BLOCKSIZE)<=diff))
            return TRUE;
        else
            return FALSE;
    }

}
void drawDeadPac(SDL_Renderer *ren, SDL_Texture *tex, int x, int y, int dir, int frameCount)
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
    pacman.y=(frameCount/4)*20;
    if(dir == NONE)
    {
        pacman.x=0;
    }
    SDL_RenderCopy(ren, tex,&pacman, &block);
}


