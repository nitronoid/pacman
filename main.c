#include <time.h>
#include <sys/time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
// include the map for the maze.
#include "map.h"
// the size of the block to draw
const int BLOCKSIZE=25;
const float scale = 0.08f;
// the width of the screen taking into account the maze and block
#define WIDTH (COLS-1)*BLOCKSIZE
// the height of the screen taking into account the maze and block
#define HEIGHT (ROWS-1)*BLOCKSIZE
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
    BOOL frightened;
    BOOL alive;
}ghost;

BOOL checkMove(int dir, int x, int y, BOOL pac, BOOL alive);
BOOL checkPac(ghost *enemy, pacman *pac);
BOOL checkGhost(ghost *enemy, pacman *pac);
void checkDeaths(pacman *pac, ghost *shadow, ghost *speedy, ghost *bashful, ghost *pokey);
void drawStart(SDL_Renderer *ren, SDL_Texture *tex);
void drawGameOver(SDL_Renderer *ren, SDL_Texture *tex);
void checkPill(int x, int y, BOOL *frightened, struct timespec *fStart, int aiMode, int *t);
void moveSprite(int *x, int *y, int dir, BOOL pac, BOOL frightened, BOOL alive, int frameCount);
void moveShadow(ghost *enemy, int pacX, int pacY, int frameCount);
void moveSpeedy(ghost *enemy,int pacX,int pacY, int pacDir, int frameCount);
void moveBashful(ghost *enemy,int pacX,int pacY, int pacDir, int shadX, int shadY, int frameCount);
void movePokey(ghost *enemy,int pacX,int pacY, int *tempX, int *tempY, BOOL *loop, int frameCount);
void moveFrightened(ghost *enemy, int frameCount);
void movePac(pacman *pac, int keyPressed,int frameCount);
void drawGhost(SDL_Renderer *ren, SDL_Texture *tex, ghost *enemy, BOOL frightened, struct timespec fClock, int ghostType, int frameCount);
void drawPacman(SDL_Renderer *ren, SDL_Texture *tex, pacman *pac, int frameCount);
void drawDeadPac(SDL_Renderer *ren, SDL_Texture *tex, pacman *pac, int frameCount);
void drawMaze(SDL_Renderer *ren, SDL_Texture *btex, SDL_Texture *wtex);
void checkTeleport(int *x, int dir);
void getGhostDir(ghost *enemy, int mhtnX, int mhtnY, int temp);
int pillCount();
int checkMazeBlock(int x, int y);
int reverseDir(int dir);
void reset( BOOL *keyPressed,
            BOOL *begin,
            BOOL *loop,
            BOOL *pokeyMove,
            BOOL *bashfulMove,
            BOOL *chngDir,
            BOOL *frightened,
            BOOL *lifeDeduct,
            int *pTempX,
            int *pTempY,
            int *frameCount,
            int *deathCount,
            int *aiMode,
            ghost *shadow,
            ghost *speedy,
            ghost *bashful,
            ghost *pokey,
            pacman *pac       );

int main()
{
    //fixMap();
    srand(time(NULL));
    struct timespec start, end, lEnd, frightenedClock;
    clock_gettime(CLOCK_REALTIME, &start);
    clock_gettime(CLOCK_REALTIME, &lEnd);
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


    image=IMG_Load("ghostSprites.png");
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

    image=IMG_Load("wallSegs.png");
    if(!image)
    {
        printf("IMG_Load: %s\n", IMG_GetError());
        return EXIT_FAILURE;
    }

    SDL_Texture *wtex = 0;
    wtex = SDL_CreateTextureFromSurface(ren, image);
    // free the image
    SDL_FreeSurface(image);


    BOOL quit=FALSE;
    // now we are going to loop forever, process the keys then draw

    ghost shadow = {14*BLOCKSIZE,11*BLOCKSIZE,LEFT,TRUE,TRUE,0,0,FALSE,TRUE};
    ghost speedy = {13*BLOCKSIZE,13*BLOCKSIZE,UP,TRUE,TRUE,0,0,FALSE,TRUE};
    ghost bashful = {14*BLOCKSIZE,13*BLOCKSIZE,LEFT,TRUE,TRUE,0,0,FALSE,TRUE};
    ghost pokey = {13*BLOCKSIZE,14*BLOCKSIZE,RIGHT,TRUE,TRUE,0,0,FALSE,TRUE};
    pacman pac = {13.5*BLOCKSIZE,23*BLOCKSIZE+1,NONE,NONE,NONE,TRUE};
    BOOL keyPressed = FALSE;
    BOOL begin = FALSE;
    BOOL loop = FALSE;
    BOOL pokeyMove = FALSE;
    BOOL bashfulMove = FALSE;
    BOOL chngDir = TRUE;
    BOOL frightened = FALSE;
    BOOL lifeDeduct = FALSE;
    double diff;
    int pTempX = 0;
    int pTempY = 0;
    int frameCount = 0;
    int deathCount = 0;
    int aiMode = 0;
    int moveMode[7]={7,27,34,54,59,79,84};
    int lives = 3;

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

        if(frightened)
        {
            if(shadow.alive)
                shadow.frightened = TRUE;
            else
                shadow.frightened = FALSE;
            if(speedy.alive)
                speedy.frightened = TRUE;
            else
                speedy.frightened = FALSE;
            if(bashful.alive)
                bashful.frightened = TRUE;
            else
                bashful.frightened = FALSE;
            if(pokey.alive)
                pokey.frightened = TRUE;
            else
                pokey.frightened = FALSE;

            shadow.dir = reverseDir(shadow.dir);
            speedy.dir = reverseDir(speedy.dir);
            bashful.dir = reverseDir(bashful.dir);
            pokey.dir = reverseDir(pokey.dir);

            frightened = FALSE;
        }
        checkDeaths(&pac,&shadow,&speedy,&bashful,&pokey);

        if(!shadow.alive)
            shadow.frightened = FALSE;
        if(!speedy.alive)
            speedy.frightened = FALSE;
        if(!bashful.alive)
            bashful.frightened = FALSE;
        if(!pokey.alive)
            pokey.frightened = FALSE;

        if(begin&&pac.alive)
        {
            movePac(&pac, keyPressed,frameCount);

            if((pillCount() <= 228)&&!frightened)
                bashfulMove = TRUE;
            if((pillCount() <= 172)&&!frightened)
                pokeyMove = TRUE;

            clock_gettime(CLOCK_REALTIME, &end);
            diff = ( end.tv_sec - start.tv_sec ) + ( end.tv_nsec - start.tv_nsec )/1E9;
            if((diff >= moveMode[aiMode])&&(aiMode < 7))
            {
                aiMode++;
            }

            clock_gettime(CLOCK_REALTIME, &end);
            diff = ( end.tv_sec - frightenedClock.tv_sec ) + ( end.tv_nsec - frightenedClock.tv_nsec )/1E9;
            if(diff >= 7)
            {
                shadow.frightened = FALSE;
                speedy.frightened = FALSE;
                bashful.frightened = FALSE;
                pokey.frightened = FALSE;
            }



            if(shadow.alive)
            {
                if(shadow.frightened)
                    moveFrightened(&shadow,frameCount);
                else if (aiMode%2 == 0)
                    moveShadow(&shadow, 30*BLOCKSIZE,0,frameCount);
                else
                    moveShadow(&shadow, pac.x,pac.Y,frameCount);
            }
            else
                moveShadow(&shadow, 13*BLOCKSIZE,13*BLOCKSIZE,frameCount);

            if(speedy.alive)
            {
                if(speedy.frightened)
                    moveFrightened(&speedy,frameCount);
                else if (aiMode%2 == 0)
                    moveSpeedy(&speedy,0,0, pac.dir,frameCount);
                else
                    moveSpeedy(&speedy,pac.x,pac.Y, pac.dir,frameCount);
            }
            else
                moveShadow(&speedy, 13*BLOCKSIZE,13*BLOCKSIZE,frameCount);

            if(bashfulMove)
            {
                if(bashful.alive)
                {
                    if(bashful.frightened&&!bashful.gate)
                        moveFrightened(&bashful,frameCount);
                    else if (aiMode%2 == 0)
                        moveBashful(&bashful,30*BLOCKSIZE,28*BLOCKSIZE, pac.dir, shadow.x, shadow.Y,frameCount);
                    else
                        moveBashful(&bashful,pac.x,pac.Y, pac.dir, shadow.x, shadow.Y,frameCount);
                }
                else
                    moveShadow(&bashful, 13*BLOCKSIZE,13*BLOCKSIZE,frameCount);
            }

            if(pokeyMove)
            {
                if(pokey.alive)
                {
                    if(pokey.frightened&&!pokey.gate)
                        moveFrightened(&pokey,frameCount);
                    else if (aiMode%2 == 0)
                        movePokey(&pokey, 0,28*BLOCKSIZE,&pTempX, &pTempY, &loop,frameCount);
                    else
                        movePokey(&pokey, pac.x,pac.Y,&pTempX, &pTempY, &loop,frameCount);
                }
                else
                    moveShadow(&pokey, 13*BLOCKSIZE,13*BLOCKSIZE,frameCount);
            }
        }



        checkPill(pac.x, pac.Y, &frightened, &frightenedClock, aiMode, &moveMode[0]);
        checkTeleport(&pac.x, pac.dir);
        checkTeleport(&shadow.x, shadow.dir);
        checkTeleport(&speedy.x, speedy.dir);
        checkTeleport(&bashful.x, bashful.dir);
        checkTeleport(&pokey.x, pokey.dir);

        // now we clear the screen (will use the clear colour set previously)
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);

        drawMaze(ren, btex, wtex);


        if(pac.alive)
        {
            if(shadow.alive)
                drawGhost(ren, gtex, &shadow, shadow.frightened, frightenedClock,4,frameCount);
            else
                drawGhost(ren, gtex, &shadow, FALSE, frightenedClock,0,0);
            if(speedy.alive)
                drawGhost(ren, gtex, &speedy, speedy.frightened, frightenedClock,2,frameCount);
            else
                drawGhost(ren, gtex, &speedy, FALSE, frightenedClock,0,0);
            if(bashful.alive)
                drawGhost(ren, gtex, &bashful, bashful.frightened, frightenedClock,3,frameCount);
            else
                drawGhost(ren, gtex, &bashful, FALSE, frightenedClock,0,0);
            if(pokey.alive)
                drawGhost(ren, gtex, &pokey, pokey.frightened, frightenedClock,1,frameCount);
            else
                drawGhost(ren, gtex, &pokey, FALSE, frightenedClock,0,0);
            lifeDeduct = TRUE;
            drawPacman(ren, ptex, &pac, frameCount);
        }
        else if (lives > 0)
        {
            clock_gettime(CLOCK_REALTIME, &end);
            diff = ( end.tv_sec - lEnd.tv_sec ) + ( end.tv_nsec - lEnd.tv_nsec )/1E9;

            if((diff >= 2)&&!lifeDeduct)
            {
                reset(&keyPressed,
                      &begin,
                      &loop,
                      &pokeyMove,
                      &bashfulMove,
                      &chngDir,
                      &frightened,
                      &lifeDeduct,
                      &pTempX,
                      &pTempY,
                      &frameCount,
                      &deathCount,
                      &aiMode,
                      &shadow,
                      &speedy,
                      &bashful,
                      &pokey,
                      &pac);
                clock_gettime(CLOCK_REALTIME, &start);
            }
            drawDeadPac(ren, dtex, &pac, deathCount);
            if(lifeDeduct)
            {
                clock_gettime(CLOCK_REALTIME, &lEnd);
                lives--;
            }
            lifeDeduct = FALSE;
        }
        if(!begin)
            drawStart(ren,stex);

        if(lives <= 0)
        {
            drawGameOver(ren,etex);
            drawDeadPac(ren, dtex, &pac, deathCount);
        }

        if((quit != TRUE)&&(pillCount() <= 0))
            quit = TRUE;
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

BOOL checkMove(int dir, int x, int y, BOOL pac, BOOL alive)
{
    BOOL valid = TRUE;
    BOOL condition;
    int a,b,c;
    int step = BLOCKSIZE*scale;
    if(!pac)
        step+=1;
    int dim = (BLOCKSIZE-1)/2-1;

    switch (dir)
    {
    case UP:

        a = round((y - step-dim)/((float)BLOCKSIZE))+1;
        b = round((x-dim)/((float)BLOCKSIZE))+1;
        c = round((x+dim)/((float)BLOCKSIZE))+1;
        condition = ((map[a][b] == BLUE)||(map[a][c] == BLUE));
        if(pac||alive)
            condition = (condition||(map[a][b] == GATE)||(map[a][c] == GATE));
        if(condition)
            valid = FALSE;
        break;
    case DOWN:
        a = round((y + step + dim)/((float)BLOCKSIZE))+1;
        b = round((x+dim)/((float)BLOCKSIZE))+1;
        c = round((x-dim)/((float)BLOCKSIZE))+1;
        condition = ((map[a][b] == BLUE)||(map[a][c] == BLUE));
        if(pac||alive)
            condition = (condition||(map[a][b] == GATE)||(map[a][c] == GATE));
        if(condition)
            valid = FALSE;
        break;
    case LEFT:
        a = round((y+dim)/((float)BLOCKSIZE))+1;
        b = round((y-dim)/((float)BLOCKSIZE))+1;
        c = round((x - step-dim)/((float)BLOCKSIZE))+1;
        condition = ((map[a][c] == BLUE)||(map[b][c] == BLUE));
        if(pac||alive)
            condition = (condition||(map[a][c] == GATE)||(map[b][c] == GATE));
        if(condition)
            valid = FALSE;
        break;
    case RIGHT:
        a = round((y+dim)/((float)BLOCKSIZE))+1;
        b = round((y-dim)/((float)BLOCKSIZE))+1;
        c = round((x + step+dim)/((float)BLOCKSIZE))+1;
        condition = ((map[a][c] == BLUE)||(map[b][c] == BLUE));
        if(pac||alive)
            condition = (condition||(map[a][c] == GATE)||(map[b][c] == GATE));
        if(condition)
            valid = FALSE;
        break;
    case NONE:
        valid = TRUE;
        break;

    }
    return valid;
}

void checkPill(int x, int y, BOOL *frightened, struct timespec *fStart, int aiMode, int *t)
{
    int a = (int)round(y/((float)BLOCKSIZE))+1;
    int b = (int)round(x/((float)BLOCKSIZE))+1;
    if(map[a][b] == RPILL)
        map[a][b] = BLACK;
    else if(map[a][b] == POWERPILL)
    {
        map[a][b] = BLACK;
        *frightened = TRUE;
        clock_gettime(CLOCK_REALTIME, fStart);
        for(int i = aiMode; i < 7; i++)
        {
            *(t+i)+=7;
        }
    }
}

void checkTeleport(int *x, int dir)
{
    int endBound = (COLS-1)*BLOCKSIZE-BLOCKSIZE;
    if((*x-BLOCKSIZE*0.5 <= 0)&&(dir == LEFT))
        *x = endBound-BLOCKSIZE*0.25;
    else if((*x+BLOCKSIZE*0.5 >= endBound)&&(dir == RIGHT))
        *x = 0;
}

void moveSprite(int *x, int *y, int dir, BOOL pac, BOOL frightened, BOOL alive, int frameCount)
{

    int step = BLOCKSIZE*scale;
    if(!pac)
    {
        if(frightened||(frameCount%2 == 0))
            step-=1;
    }
    if(!alive)
        step++;
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

void moveShadow(ghost *enemy,int pacX,int pacY, int frameCount)
{
    int a = 0;
    int temp;
    int mhtnX = pacX-enemy->x;
    int mhtnY = pacY-enemy->Y;
    for(int i = 0; i<4; ++i)
    {
        if(checkMove(i,enemy->x,enemy->Y,FALSE,enemy->alive))
            a++;
    }
    temp = reverseDir(enemy->dir);
    if(a == 2)
    {
        enemy->l = TRUE;
        while((!checkMove(enemy->dir,enemy->x,enemy->Y,FALSE,enemy->alive))||(enemy->dir==temp))
        {
            enemy->dir = rand()%4;
        }
    }
    else if((a>2)&&enemy->l)
    {
        getGhostDir(enemy, mhtnX, mhtnY, temp);
    }
    moveSprite(&enemy->x,&enemy->Y,enemy->dir, FALSE, FALSE, enemy->alive, frameCount);
}

void movePokey(ghost *enemy,int pacX,int pacY, int *tempX, int *tempY, BOOL *loop, int frameCount)
{
    if(enemy->Y <= 11*BLOCKSIZE)
        enemy->gate = FALSE;
    else if (enemy->gate)
        enemy->dir = UP;
    if(!(enemy->gate))
    {
        int distX = abs(pacX-enemy->x);
        int distY = abs(pacY-enemy->Y);
        int dist = distX + distY;
        if((dist < 8*BLOCKSIZE)&&!(*loop))
        {
            *loop = TRUE;
            *tempX = enemy->x;
            *tempY = enemy->Y;
        }
        if(*loop)
        {
            *loop = FALSE;
            pacX = *tempX;
            pacY = *tempY;
        }
        int a = 0;
        int temp;
        int mhtnX = pacX-enemy->x;
        int mhtnY = pacY-enemy->Y;
        for(int i = 0; i<4; ++i)
        {
            if(checkMove(i,enemy->x,enemy->Y,FALSE,enemy->alive))
                a++;
        }
        temp = reverseDir(enemy->dir);
        if(a == 2)
        {
            enemy->l = TRUE;
            while((!checkMove(enemy->dir,enemy->x,enemy->Y,FALSE,enemy->alive))||(enemy->dir==temp))
            {
                enemy->dir = rand()%4;
            }
        }
        else if((a>2)&&enemy->l)
        {
            getGhostDir(enemy, mhtnX, mhtnY, temp);
        }
    }
    moveSprite(&enemy->x,&enemy->Y,enemy->dir, FALSE, FALSE, enemy->alive, frameCount);
}

void moveBashful( ghost *enemy,int pacX,int pacY, int pacDir, int shadX, int shadY, int frameCount)
{

    if(enemy->Y <= 11*BLOCKSIZE)
        enemy->gate = FALSE;
    else if (enemy->gate)
        enemy->dir = UP;
    if(!(enemy->gate))
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
        int mhtnX = pacX-enemy->x;
        int mhtnY = pacY-enemy->Y;
        for(int i = 0; i<4; ++i)
        {
            if(checkMove(i,enemy->x,enemy->Y,FALSE,enemy->alive))
                a++;
        }
        temp = reverseDir(enemy->dir);
        if(a == 2)
        {
            enemy->l = TRUE;
            while((!checkMove(enemy->dir,enemy->x,enemy->Y,FALSE,enemy->alive))||(enemy->dir==temp))
            {
                enemy->dir = rand()%4;
            }
        }
        else if((a>2)&&enemy->l)
        {
            getGhostDir(enemy, mhtnX, mhtnY, temp);
        }
    }
    moveSprite(&enemy->x,&enemy->Y,enemy->dir, FALSE, FALSE, enemy->alive, frameCount);
}

void moveSpeedy(ghost *enemy,int pacX,int pacY, int pacDir, int frameCount)
{
    if(enemy->Y <= 11*BLOCKSIZE)
        enemy->gate = FALSE;
    else if (enemy->gate)
        enemy->dir = UP;
    if(!(enemy->gate))
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
        int mhtnX = pacX-enemy->x;
        int mhtnY = pacY-enemy->Y;
        for(int i = 0; i<4; ++i)
        {
            if(checkMove(i,enemy->x,enemy->Y,FALSE,enemy->alive))
                a++;
        }
        temp = reverseDir(enemy->dir);
        if(a == 2)
        {
            enemy->l = TRUE;
            while((!checkMove(enemy->dir,enemy->x,enemy->Y,FALSE,enemy->alive))||(enemy->dir==temp))
            {
                enemy->dir = rand()%4;
            }
        }
        else if((a>2)&&enemy->l)
        {
            getGhostDir(enemy, mhtnX, mhtnY, temp);
        }
    }
    moveSprite(&enemy->x,&enemy->Y,enemy->dir, FALSE, FALSE, enemy->alive, frameCount);
}

void getGhostDir(ghost *enemy, int mhtnX, int mhtnY, int temp)
{
    enemy->l = FALSE;
    if(abs(mhtnX)>=abs(mhtnY))
    {
        if(mhtnX >= 0)
        {
            if((checkMove(RIGHT,enemy->x,enemy->Y,FALSE,enemy->alive))&&(temp!=RIGHT))
                enemy->dir = RIGHT;
            else if(mhtnY >= 0)
            {
                if((checkMove(DOWN,enemy->x,enemy->Y,FALSE,enemy->alive))&&(temp!=DOWN))
                    enemy->dir = DOWN;
            }
            else
            {
                if((checkMove(UP,enemy->x,enemy->Y,FALSE,enemy->alive))&&(temp!=UP))
                    enemy->dir = UP;
            }

        }
        else
        {
            if((checkMove(LEFT,enemy->x,enemy->Y,FALSE,enemy->alive))&&(temp!=LEFT))
                enemy->dir = LEFT;
            else if(mhtnY >= 0)
            {
                if((checkMove(DOWN,enemy->x,enemy->Y,FALSE,enemy->alive))&&(temp!=DOWN))
                    enemy->dir = DOWN;
            }
            else
            {
                if((checkMove(UP,enemy->x,enemy->Y,FALSE,enemy->alive))&&(temp!=UP))
                    enemy->dir = UP;
            }

        }
    }
    else
    {
        if(mhtnY>=0)
        {
            if((checkMove(DOWN,enemy->x,enemy->Y,FALSE,enemy->alive))&&(temp!=DOWN))
                enemy->dir = DOWN;
            else if(mhtnX>=0)
            {
                if((checkMove(RIGHT,enemy->x,enemy->Y,FALSE,enemy->alive))&&(temp!=RIGHT))
                    enemy->dir = RIGHT;
            }
            else
            {
                if((checkMove(LEFT,enemy->x,enemy->Y,FALSE,enemy->alive))&&(temp!=LEFT))
                    enemy->dir = LEFT;
            }
        }
        else
        {
            if((checkMove(UP,enemy->x,enemy->Y,FALSE,enemy->alive))&&(temp!=UP))
                enemy->dir = UP;
            else if(mhtnX>=0)
            {
                if((checkMove(RIGHT,enemy->x,enemy->Y,FALSE,enemy->alive))&&(temp!=RIGHT))
                    enemy->dir = RIGHT;
            }
            else
            {
                if((checkMove(LEFT,enemy->x,enemy->Y,FALSE,enemy->alive))&&(temp!=LEFT))
                    enemy->dir = LEFT;
            }
        }
    }
}

void movePac( pacman *pac, int keyPressed,int frameCount)
{
    BOOL move, moveBckUp, movePrdct = FALSE;
    move = checkMove(pac->dir,pac->x,pac->Y,TRUE,TRUE);

    if(pac->last!=NONE)
        movePrdct = checkMove(pac->last,pac->x,pac->Y,TRUE,TRUE);
    else
        movePrdct = FALSE;

    moveBckUp = checkMove(pac->temp,pac->x,pac->Y,TRUE,TRUE);

    if((movePrdct == TRUE)&&(keyPressed == FALSE))
    {
        pac->dir = pac->last;
        pac->last = NONE;
        pac->temp = NONE;
        moveSprite(&pac->x, &pac->Y, pac->dir, TRUE, FALSE,TRUE,frameCount);
    }
    else if(move == TRUE)
    {
        moveSprite(&pac->x, &pac->Y, pac->dir, TRUE, FALSE,TRUE,frameCount);
    }
    else if(moveBckUp == TRUE)
    {
        pac->last = pac->dir;
        pac->dir = pac->temp;
        moveSprite(&pac->x, &pac->Y, pac->dir, TRUE, FALSE,TRUE,frameCount);
    }
    else
    {
        pac->dir = NONE;
        pac->last = NONE;
        pac->temp = NONE;
    }

}

void drawGhost(SDL_Renderer *ren, SDL_Texture *tex, ghost *enemy, BOOL frightened, struct timespec fClock, int ghostType, int frameCount)
{
    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);
    double diff = ( start.tv_sec - fClock.tv_sec ) + ( start.tv_nsec - fClock.tv_nsec )/1E9;
    int desc = 0;
    int anim = 5*(BLOCKSIZE*2);
    if((frameCount/15) == 0)
        anim = 0;
    SDL_Rect block;
    block.x=enemy->x;
    block.y=enemy->Y;
    block.w=BLOCKSIZE-1;
    block.h=BLOCKSIZE-1;
    SDL_Rect ghosty;
    ghosty.w=BLOCKSIZE*2;
    ghosty.h=BLOCKSIZE*2;
    ghosty.x=ghostType*(BLOCKSIZE*2)+anim;
    ghosty.y=enemy->dir*(BLOCKSIZE*2);
    if(frightened)
    {
        if(diff >= 5)
        {

           desc = (int)(diff*10)%2;
           if(desc == 1)
           {
               ghosty.x=5*(BLOCKSIZE*2)+anim;
               ghosty.y=BLOCKSIZE*2;
           }
           else
           {
               ghosty.x=5*(BLOCKSIZE*2)+anim;
               ghosty.y=0;
           }
        }
        else
        {
        ghosty.x=5*(BLOCKSIZE*2)+anim;
        ghosty.y=BLOCKSIZE*2;
        }
    }
    SDL_RenderCopy(ren, tex,&ghosty, &block);
}

void drawPacman(SDL_Renderer *ren, SDL_Texture *tex, pacman *pac, int frameCount)
{
    SDL_Rect block;
    block.x=pac->x;
    block.y=pac->Y;
    block.w=BLOCKSIZE;
    block.h=BLOCKSIZE;
    SDL_Rect pacman;
    pacman.w=BLOCKSIZE*2;
    pacman.h=BLOCKSIZE*2;
    pacman.x=pac->dir*(BLOCKSIZE*2);
    pacman.y=(frameCount/7)*(BLOCKSIZE*2);
    if(pac->dir == NONE)
    {
        pacman.x=0;
        pacman.y=0;
    }
    SDL_RenderCopy(ren, tex,&pacman, &block);
}

void drawMaze(SDL_Renderer *ren, SDL_Texture *tex, SDL_Texture *wtex)
{
    SDL_Rect block;
    SDL_Rect pill;
    SDL_Rect wall;
    pill.w = 25;
    pill.h = 25;
    wall.w = 25;
    wall.h = 25;
    wall.y = 0;
    wall.x = 0;

    for(int i = 1; i < ROWS; ++i)
    {
        for(int j = 1; j < COLS; ++j)
        {
            block.x=(j-1)*BLOCKSIZE;
            block.y=(i-1)*BLOCKSIZE;
            block.w=BLOCKSIZE;
            block.h=BLOCKSIZE;
            switch(map[i][j])
            {
            case (BLACK):
                SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
                SDL_RenderFillRect(ren,&block);
                break;
            case (BLUE):
                wall.x = BLOCKSIZE*checkMazeBlock(j,i);
                SDL_RenderCopy(ren, wtex, &wall, &block);
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
                pill.y = 0;
                SDL_RenderCopy(ren, tex, &pill, &block);
                break;
            }


        }
    }
}

void moveFrightened(ghost *enemy, int frameCount)
{
    int a = 0;
    int temp;
    for(int i = 0; i<4; ++i)
    {
        if(checkMove(i,enemy->x,enemy->Y,FALSE,enemy->alive))
            a++;
    }

    temp = reverseDir(enemy->dir);
    if((a > 1)&&((abs(enemy->tx-enemy->x)>2)||(abs(enemy->ty-enemy->Y)>2)))
    {
        enemy->tx = enemy->x;
        enemy->ty = enemy->Y;
        enemy->dir = rand()%4;
        while((!checkMove(enemy->dir,enemy->x,enemy->Y,FALSE,enemy->alive))||(enemy->dir==temp))
        {
            (enemy->dir)++;
            if(enemy->dir > 3)
                enemy->dir = 0;
        }
    }

    moveSprite(&enemy->x, &enemy->Y, enemy->dir, FALSE, TRUE, enemy->alive, frameCount);
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

BOOL checkPac(ghost *enemy, pacman *pac)
{
    int diff = 3*BLOCKSIZE/4;
    if((abs((pac->x)-(enemy->x))<=diff)&&(abs((pac->Y)-(enemy->Y))<=diff))
        return FALSE;
    else
        return TRUE;
}

BOOL checkGhost(ghost *enemy,pacman *pac)
{
    int diff = 3*BLOCKSIZE/4;
    if(enemy->alive)
    {
        if((abs(pac->x-enemy->x)<=diff)&&(abs(pac->Y-enemy->Y)<=diff))
            return FALSE;
        else
            return TRUE;
    }
    else
    {
        if((abs(enemy->Y-13*BLOCKSIZE)<=diff)&&((abs(enemy->x-14*BLOCKSIZE)<=diff)||(abs(enemy->x-13*BLOCKSIZE)<=diff)))
        {
            printf("%d\n",enemy->dir);
            enemy->dir = reverseDir(enemy->dir);
            printf("%d\n",enemy->dir);
            return TRUE;
        }
        else
            return FALSE;
    }

}

void drawDeadPac(SDL_Renderer *ren, SDL_Texture *tex, pacman *pac, int frameCount)
{
    SDL_Rect block;
    block.x=pac->x;
    block.y=pac->Y;
    block.w=BLOCKSIZE-1;
    block.h=BLOCKSIZE-1;
    SDL_Rect pacman;
    pacman.w=22;
    pacman.h=20;
    pacman.x=pac->dir*22;
    pacman.y=(frameCount/4)*20;
    if(pac->dir == NONE)
    {
        pacman.x=0;
    }
    SDL_RenderCopy(ren, tex,&pacman, &block);
}

void reset( BOOL *keyPressed,
            BOOL *begin,
            BOOL *loop,
            BOOL *pokeyMove,
            BOOL *bashfulMove,
            BOOL *chngDir,
            BOOL *frightened,
            BOOL *lifeDeduct,
            int *pTempX,
            int *pTempY,
            int *frameCount,
            int *deathCount,
            int *aiMode,
            ghost *shadow,
            ghost *speedy,
            ghost *bashful,
            ghost *pokey,
            pacman *pac       )
{

    shadow->x=13*BLOCKSIZE;
    shadow->Y=11*BLOCKSIZE;
    shadow->dir=LEFT;
    shadow->alive=TRUE;
    shadow->gate=TRUE;
    shadow->l=TRUE;
    shadow->tx=0;
    shadow->ty=0;
    shadow->frightened=FALSE;

    speedy->x=14*BLOCKSIZE;
    speedy->Y=11*BLOCKSIZE;
    speedy->dir=UP;
    speedy->alive=TRUE;
    speedy->gate=TRUE;
    speedy->l=TRUE;
    speedy->tx=0;
    speedy->ty=0;
    speedy->frightened=FALSE;

    bashful->x=14*BLOCKSIZE;
    bashful->Y=13*BLOCKSIZE;
    bashful->dir=LEFT;
    bashful->alive=TRUE;
    bashful->gate=TRUE;
    bashful->l=TRUE;
    bashful->tx=0;
    bashful->ty=0;
    bashful->frightened=FALSE;

    pokey->x=13*BLOCKSIZE;
    pokey->Y=13*BLOCKSIZE;
    pokey->dir=LEFT;
    pokey->alive=TRUE;
    pokey->gate=TRUE;
    pokey->l=TRUE;
    pokey->tx=0;
    pokey->ty=0;
    pokey->frightened=FALSE;

    pac->x=13*BLOCKSIZE;
    pac->Y=17*BLOCKSIZE+1;
    pac->dir=NONE;
    pac->last=NONE;
    pac->temp=NONE;
    pac->alive=TRUE;

    *keyPressed = FALSE;
    *begin = FALSE;
    *loop = FALSE;
    *pokeyMove = FALSE;
    *bashfulMove = FALSE;
    *chngDir = TRUE;
    *frightened = FALSE;
    *lifeDeduct = FALSE;
    *pTempX = 0;
    *pTempY = 0;
    *frameCount = 0;
    *deathCount = 0;
    *aiMode = 0;

}

int checkMazeBlock(int x, int y)
{
    int left,right,up,down,upperLeft,lowerLeft,upperRight,lowerRight,type;
    left = map[y][x-1];
    right = map[y][x+1];
    up = map[y-1][x];
    down = map[y+1][x];
    upperLeft = map[y-1][x-1];
    lowerLeft = map[y+1][x-1];
    upperRight = map[y-1][x+1];
    lowerRight = map[y+1][x+1];

    if((right != BLUE)||(left != BLUE))
        type = 0;
    if((up != BLUE)||(down != BLUE))
        type = 1;
    if((upperRight != BLUE)&&(up == BLUE)&&(right == BLUE))
        type = 5;
    if((lowerRight != BLUE)&&(down == BLUE)&&(right == BLUE))
        type = 3;
    if((upperLeft != BLUE)&&(up == BLUE)&&(left == BLUE))
        type = 4;
    if((lowerLeft != BLUE)&&(down == BLUE)&&(left == BLUE))
        type = 2;
    if((right != BLUE)&&(up != BLUE)&&(upperRight != BLUE))
        type = 2;
    if((left != BLUE)&&(up != BLUE)&&(upperLeft != BLUE))
        type = 3;
    if((right != BLUE)&&(down != BLUE)&&(lowerRight != BLUE))
        type = 4;
    if((left != BLUE)&&(down != BLUE)&&(lowerLeft != BLUE))
        type = 5;
    if((right == GATE)||(left == GATE))
        type = 1;

    return type;
}

void checkDeaths(pacman *pac, ghost *shadow, ghost *speedy, ghost *bashful, ghost *pokey)
{
    if(shadow->frightened||!shadow->alive)
        shadow->alive = checkGhost(shadow, pac);
    else if (pac->alive&&shadow->alive)
        pac->alive = checkPac(shadow, pac);
    if(speedy->frightened||!speedy->alive)
        speedy->alive = checkGhost(speedy, pac);
    else if (pac->alive&&speedy->alive)
        pac->alive = checkPac(speedy, pac);
    if(bashful->frightened||!bashful->alive)
        bashful->alive = checkGhost(bashful, pac);
    else if (pac->alive&&bashful->alive)
        pac->alive = checkPac(bashful, pac);
    if(pokey->frightened||!pokey->alive)
        pokey->alive = checkGhost(pokey, pac);
    else if (pac->alive&&pokey->alive)
        pac->alive = checkPac(pokey, pac);
}
