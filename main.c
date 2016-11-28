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
    BOOL alive;
}ghost;

BOOL checkVictory();
BOOL checkMove(int dir, int x, int y, BOOL pac);
BOOL checkDeath(int ghostX,int ghostY,int pacX,int pacY);
BOOL checkGhost(int x, int y, int pacX,int pacY,BOOL alive);
void drawStart(SDL_Renderer *ren, SDL_Texture *tex);
void drawGameOver(SDL_Renderer *ren, SDL_Texture *tex);
void checkPill(int *x, int *y, BOOL *frightened, struct timespec *fStart, int aiMode, int *t);
void moveSprite(int *x, int *y, int dir, BOOL pac, BOOL frightened, int frameCount);
void moveShadow(ghost *enemy, int pacX, int pacY, int frameCount);
void moveSpeedy(ghost *enemy,int pacX,int pacY, int pacDir, int frameCount);
void moveBashful(ghost *enemy,int pacX,int pacY, int pacDir, int shadX, int shadY, int frameCount);
void movePokey(ghost *enemy,int pacX,int pacY, int *tempX, int *tempY, BOOL *loop, int frameCount);
void moveFrightened(ghost *enemy, int frameCount);
void movePac(int *dir, int *last, int *temp, int *x, int *y, int keyPressed,int frameCount);
void drawGhost(SDL_Renderer *ren, SDL_Texture *tex, int x, int y, int dir, BOOL frightened, struct timespec fClock, int ghostType, int frameCount);
void drawPacman(SDL_Renderer *ren, SDL_Texture *tex, int x, int y, int dir, int frameCount);
void drawDeadPac(SDL_Renderer *ren, SDL_Texture *tex, int x, int y, int dir, int frameCount);
void drawMaze(SDL_Renderer *ren, SDL_Texture *btex, SDL_Texture *wtex);
void checkTeleport(int *x, int dir);
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

    ghost shadow = {14*BLOCKSIZE,11*BLOCKSIZE,LEFT,TRUE,TRUE,0,0,TRUE};
    ghost speedy = {13*BLOCKSIZE,13*BLOCKSIZE,UP,TRUE,TRUE,0,0,TRUE};
    ghost bashful = {14*BLOCKSIZE,13*BLOCKSIZE,LEFT,TRUE,TRUE,0,0,TRUE};
    ghost pokey = {13*BLOCKSIZE,14*BLOCKSIZE,RIGHT,TRUE,TRUE,0,0,TRUE};
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

        if(!frightened)
        {
            if(pac.alive&&shadow.alive)
                pac.alive = checkDeath(shadow.x,shadow.Y,pac.x,pac.Y);
            if(pac.alive&&speedy.alive)
                pac.alive = checkDeath(speedy.x,speedy.Y,pac.x,pac.Y);
            if(pac.alive&&bashful.alive)
                pac.alive = checkDeath(bashful.x,bashful.Y,pac.x,pac.Y);
            if(pac.alive&&pokey.alive)
                pac.alive = checkDeath(pokey.x,pokey.Y,pac.x,pac.Y);
        }
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
            movePac(&pac.dir,&pac.last,&pac.temp,&pac.x,&pac.Y, keyPressed,frameCount);

            if((pillCount() <= 228)&&!frightened)
                bashfulMove = TRUE;
            if((pillCount() <= 172)&&!frightened)
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
                    moveFrightened(&shadow,frameCount);
                else
                    moveShadow(&shadow, 13*BLOCKSIZE,11*BLOCKSIZE,frameCount);
                if(speedy.alive)
                    moveFrightened(&speedy,frameCount);
                else
                    moveShadow(&speedy, 13*BLOCKSIZE,11*BLOCKSIZE,frameCount);
                if(!bashful.gate)
                {
                    if(bashful.alive)
                        moveFrightened(&bashful,frameCount);
                    else
                        moveShadow(&bashful, 13*BLOCKSIZE,11*BLOCKSIZE,frameCount);
                }
                if(!pokey.gate)
                {
                    if(pokey.alive)
                        moveFrightened(&pokey,frameCount);
                    else
                        moveShadow(&pokey, 13*BLOCKSIZE,11*BLOCKSIZE,frameCount);
                }
                clock_gettime(CLOCK_REALTIME, &end);
                diff = ( end.tv_sec - frightenedClock.tv_sec ) + ( end.tv_nsec - frightenedClock.tv_nsec )/1E9;

                if(diff >= 7)
                    frightened = FALSE;
            }
            else
            {
                chngDir = TRUE;
                clock_gettime(CLOCK_REALTIME, &end);
                diff = ( end.tv_sec - start.tv_sec ) + ( end.tv_nsec - start.tv_nsec )/1E9;
                if((diff >= moveMode[aiMode])&&(aiMode < 7))
                {
                    aiMode++;
                }
                if(aiMode%2 == 0)
                {
                    if(shadow.alive)
                        moveShadow(&shadow, 30*BLOCKSIZE,0,frameCount);
                    else
                        moveShadow(&shadow, 13*BLOCKSIZE,11*BLOCKSIZE,frameCount);
                    if(speedy.alive)
                        moveSpeedy(&speedy,0,0, pac.dir,frameCount);
                    else
                        moveShadow(&speedy, 13*BLOCKSIZE,11*BLOCKSIZE,frameCount);
                    if(bashfulMove)
                    {
                        if(bashful.alive)
                            moveBashful(&bashful,30*BLOCKSIZE,28*BLOCKSIZE, pac.dir, shadow.x, shadow.Y,frameCount);
                        else
                            moveShadow(&bashful, 13*BLOCKSIZE,11*BLOCKSIZE,frameCount);
                    }
                    if(pokeyMove)
                    {
                        if(pokey.alive)
                            movePokey(&pokey, 0,28*BLOCKSIZE,&pTempX, &pTempY, &loop,frameCount);
                        else
                            moveShadow(&pokey, 13*BLOCKSIZE,11*BLOCKSIZE,frameCount);
                    }
                }
                else
                {
                    if(shadow.alive)
                        moveShadow(&shadow, pac.x,pac.Y,frameCount);
                    else
                        moveShadow(&shadow, 13*BLOCKSIZE,11*BLOCKSIZE,frameCount);
                    if(speedy.alive)
                        moveSpeedy(&speedy,pac.x,pac.Y, pac.dir,frameCount);
                    else
                        moveShadow(&speedy, 13*BLOCKSIZE,11*BLOCKSIZE,frameCount);
                    if(bashfulMove)
                    {
                        if(bashful.alive)
                            moveBashful(&bashful,pac.x,pac.Y, pac.dir, shadow.x, shadow.Y,frameCount);
                        else
                            moveShadow(&bashful, 13*BLOCKSIZE,11*BLOCKSIZE,frameCount);
                    }
                    if(pokeyMove)
                    {
                        if(pokey.alive)
                            movePokey(&pokey, pac.x,pac.Y,&pTempX, &pTempY, &loop,frameCount);
                        else
                            moveShadow(&pokey, 13*BLOCKSIZE,11*BLOCKSIZE,frameCount);
                    }
                }
            }
        }


        checkPill(&pac.x, &pac.Y, &frightened, &frightenedClock, aiMode, &moveMode[0]);
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
                drawGhost(ren, gtex, shadow.x, shadow.Y, shadow.dir, frightened, frightenedClock,4,frameCount);
            else
                drawGhost(ren, gtex, shadow.x, shadow.Y, shadow.dir, FALSE, frightenedClock,0,0);
            if(speedy.alive)
                drawGhost(ren, gtex, speedy.x, speedy.Y, speedy.dir, frightened, frightenedClock,2,frameCount);
            else
                drawGhost(ren, gtex, speedy.x, speedy.Y, speedy.dir, FALSE, frightenedClock,0,0);
            if(bashful.alive)
                drawGhost(ren, gtex, bashful.x, bashful.Y, bashful.dir, frightened, frightenedClock,3,frameCount);
            else
                drawGhost(ren, gtex, bashful.x, bashful.Y, bashful.dir, FALSE, frightenedClock,0,0);
            if(pokey.alive)
                drawGhost(ren,gtex,pokey.x,pokey.Y,pokey.dir, frightened, frightenedClock,1,frameCount);
            else
                drawGhost(ren,gtex,pokey.x,pokey.Y,pokey.dir, FALSE, frightenedClock,0,0);
            lifeDeduct = TRUE;
            drawPacman(ren, ptex, pac.x, pac.Y, pac.dir, frameCount);
        }
        else if (lives > 0)
        {
            clock_gettime(CLOCK_REALTIME, &end);
            diff = ( end.tv_sec - lEnd.tv_sec ) + ( end.tv_nsec - lEnd.tv_nsec )/1E9;
            printf("%lf\n",diff);

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
            drawDeadPac(ren, dtex, pac.x, pac.Y, pac.dir, deathCount);
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
            drawDeadPac(ren, dtex, pac.x, pac.Y, pac.dir, deathCount);
        }

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
    for(int i = 1; i < ROWS+1; ++i)
    {
        for(int j = 1; j < COLS+1; ++j)
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

        a = (int)round((y - step-dim)/((float)BLOCKSIZE))+1;
        b = (int)round((x-dim)/((float)BLOCKSIZE))+1;
        c = (int)round((x+dim)/((float)BLOCKSIZE))+1;
        if((map[a][b] == BLUE)||(map[a][c] == BLUE)||(map[a][b] == GATE)||(map[a][c] == GATE))
            valid = FALSE;
        break;
    case DOWN:
        a = (int)round((y + step + dim)/((float)BLOCKSIZE))+1;
        b = (int)round((x+dim)/((float)BLOCKSIZE))+1;
        c = (int)round((x-dim)/((float)BLOCKSIZE))+1;
        if((map[a][b] == BLUE)||(map[a][c] == BLUE)||(map[a][b] == GATE)||(map[a][c] == GATE))
            valid = FALSE;
        break;
    case LEFT:
        a = (int)round((y+dim)/((float)BLOCKSIZE))+1;
        b = (int)round((y-dim)/((float)BLOCKSIZE))+1;
        c = (int)round((x - step-dim)/((float)BLOCKSIZE))+1;
        if((map[a][c] == BLUE)||(map[b][c] == BLUE)||(map[a][c] == GATE)||(map[b][c] == GATE))
            valid = FALSE;
        break;
    case RIGHT:
        a = (int)round((y+dim)/((float)BLOCKSIZE))+1;
        b = (int)round((y-dim)/((float)BLOCKSIZE))+1;
        c = (int)round((x + step+dim)/((float)BLOCKSIZE))+1;
        if((map[a][c] == BLUE)||(map[b][c] == BLUE)||(map[a][c] == GATE)||(map[b][c] == GATE))
            valid = FALSE;
        break;
    case NONE:
        valid = TRUE;
        break;

    }
    return valid;
}

void checkPill(int *x, int *y, BOOL *frightened, struct timespec *fStart, int aiMode, int *t)
{
    int a = (int)round(*y/((float)BLOCKSIZE))+1;
    int b = (int)round(*x/((float)BLOCKSIZE))+1;
    if(map[a][b] == RPILL)
        map[a][b] = BLACK;
    else if(map[a][b] == POWERPILL)
    {
        map[a][b] = BLACK;
        *frightened = TRUE;
        clock_gettime(CLOCK_REALTIME, fStart);
        for(int i = aiMode; i < 8; i++)
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

void moveSprite(int *x, int *y, int dir, BOOL pac, BOOL frightened, int frameCount)
{

    int step = BLOCKSIZE*scale;
    if(!pac)
    {
        if(frightened||(frameCount%2 == 0))
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

void moveShadow(ghost *enemy,int pacX,int pacY, int frameCount)
{
    int a = 0;
    int temp;
    int mhtnX = pacX-enemy->x;
    int mhtnY = pacY-enemy->Y;
    for(int i = 0; i<4; ++i)
    {
        if(checkMove(i,enemy->x,enemy->Y,FALSE))
            a++;
    }
    temp = reverseDir(enemy->dir);
    if(a == 2)
    {
        enemy->l = TRUE;
        while((!checkMove(enemy->dir,enemy->x,enemy->Y,FALSE))||(enemy->dir==temp))
        {
            enemy->dir = rand()%4;
        }
    }
    else if((a>2)&&enemy->l)
    {
        enemy->l = FALSE;
        if(abs(mhtnX)>=abs(mhtnY))
        {
            if(mhtnX >= 0)
            {
                if((checkMove(RIGHT,enemy->x,enemy->Y,FALSE))&&(temp!=RIGHT))
                    enemy->dir = RIGHT;
                else if(mhtnY >= 0)
                {
                    if((checkMove(DOWN,enemy->x,enemy->Y,FALSE))&&(temp!=DOWN))
                        enemy->dir = DOWN;
                }
                else
                {
                    if((checkMove(UP,enemy->x,enemy->Y,FALSE))&&(temp!=UP))
                        enemy->dir = UP;
                }

            }
            else
            {
                if((checkMove(LEFT,enemy->x,enemy->Y,FALSE))&&(temp!=LEFT))
                    enemy->dir = LEFT;
                else if(mhtnY >= 0)
                {
                    if((checkMove(DOWN,enemy->x,enemy->Y,FALSE))&&(temp!=DOWN))
                        enemy->dir = DOWN;
                }
                else
                {
                    if((checkMove(UP,enemy->x,enemy->Y,FALSE))&&(temp!=UP))
                        enemy->dir = UP;
                }

            }
        }
        else
        {
            if(mhtnY>=0)
            {
                if((checkMove(DOWN,enemy->x,enemy->Y,FALSE))&&(temp!=DOWN))
                    enemy->dir = DOWN;
                else if(mhtnX>=0)
                {
                    if((checkMove(RIGHT,enemy->x,enemy->Y,FALSE))&&(temp!=RIGHT))
                        enemy->dir = RIGHT;
                }
                else
                {
                    if((checkMove(LEFT,enemy->x,enemy->Y,FALSE))&&(temp!=LEFT))
                        enemy->dir = LEFT;
                }
            }
            else
            {
                if((checkMove(UP,enemy->x,enemy->Y,FALSE))&&(temp!=UP))
                    enemy->dir = UP;
                else if(mhtnX>=0)
                {
                    if((checkMove(RIGHT,enemy->x,enemy->Y,FALSE))&&(temp!=RIGHT))
                        enemy->dir = RIGHT;
                }
                else
                {
                    if((checkMove(LEFT,enemy->x,enemy->Y,FALSE))&&(temp!=LEFT))
                        enemy->dir = LEFT;
                }
            }
        }
    }
    moveSprite(&enemy->x,&enemy->Y,enemy->dir, FALSE, FALSE, frameCount);
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
            if(checkMove(i,enemy->x,enemy->Y,FALSE))
                a++;
        }
        temp = reverseDir(enemy->dir);
        if(a == 2)
        {
            enemy->l = TRUE;
            while((!checkMove(enemy->dir,enemy->x,enemy->Y,FALSE))||(enemy->dir==temp))
            {
                enemy->dir = rand()%4;
            }
        }
        else if((a>2)&&enemy->l)
        {
            enemy->l = FALSE;
            if(abs(mhtnX)>=abs(mhtnY))
            {
                if(mhtnX >= 0)
                {
                    if((checkMove(RIGHT,enemy->x,enemy->Y,FALSE))&&(temp!=RIGHT))
                        enemy->dir = RIGHT;
                    else if(mhtnY >= 0)
                    {
                        if((checkMove(DOWN,enemy->x,enemy->Y,FALSE))&&(temp!=DOWN))
                            enemy->dir = DOWN;
                    }
                    else
                    {
                        if((checkMove(UP,enemy->x,enemy->Y,FALSE))&&(temp!=UP))
                            enemy->dir = UP;
                    }

                }
                else
                {
                    if((checkMove(LEFT,enemy->x,enemy->Y,FALSE))&&(temp!=LEFT))
                        enemy->dir = LEFT;
                    else if(mhtnY >= 0)
                    {
                        if((checkMove(DOWN,enemy->x,enemy->Y,FALSE))&&(temp!=DOWN))
                            enemy->dir = DOWN;
                    }
                    else
                    {
                        if((checkMove(UP,enemy->x,enemy->Y,FALSE))&&(temp!=UP))
                            enemy->dir = UP;
                    }

                }
            }
            else
            {
                if(mhtnY>=0)
                {
                    if((checkMove(DOWN,enemy->x,enemy->Y,FALSE))&&(temp!=DOWN))
                        enemy->dir = DOWN;
                    else if(mhtnX>=0)
                    {
                        if((checkMove(RIGHT,enemy->x,enemy->Y,FALSE))&&(temp!=RIGHT))
                            enemy->dir = RIGHT;
                    }
                    else
                    {
                        if((checkMove(LEFT,enemy->x,enemy->Y,FALSE))&&(temp!=LEFT))
                            enemy->dir = LEFT;
                    }
                }
                else
                {
                    if((checkMove(UP,enemy->x,enemy->Y,FALSE))&&(temp!=UP))
                        enemy->dir = UP;
                    else if(mhtnX>=0)
                    {
                        if((checkMove(RIGHT,enemy->x,enemy->Y,FALSE))&&(temp!=RIGHT))
                            enemy->dir = RIGHT;
                    }
                    else
                    {
                        if((checkMove(LEFT,enemy->x,enemy->Y,FALSE))&&(temp!=LEFT))
                            enemy->dir = LEFT;
                    }
                }
            }
        }
    }
    moveSprite(&enemy->x,&enemy->Y,enemy->dir, FALSE, FALSE,frameCount);
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
            if(checkMove(i,enemy->x,enemy->Y,FALSE))
                a++;
        }
        temp = reverseDir(enemy->dir);
        if(a == 2)
        {
            enemy->l = TRUE;
            while((!checkMove(enemy->dir,enemy->x,enemy->Y,FALSE))||(enemy->dir==temp))
            {
                enemy->dir = rand()%4;
            }
        }
        else if((a>2)&&enemy->l)
        {
            enemy->l = FALSE;
            if(abs(mhtnX)>=abs(mhtnY))
            {
                if(mhtnX >= 0)
                {
                    if((checkMove(RIGHT,enemy->x,enemy->Y,FALSE))&&(temp!=RIGHT))
                        enemy->dir = RIGHT;
                    else if(mhtnY >= 0)
                    {
                        if((checkMove(DOWN,enemy->x,enemy->Y,FALSE))&&(temp!=DOWN))
                            enemy->dir = DOWN;
                    }
                    else
                    {
                        if((checkMove(UP,enemy->x,enemy->Y,FALSE))&&(temp!=UP))
                            enemy->dir = UP;
                    }

                }
                else
                {
                    if((checkMove(LEFT,enemy->x,enemy->Y,FALSE))&&(temp!=LEFT))
                        enemy->dir = LEFT;
                    else if(mhtnY >= 0)
                    {
                        if((checkMove(DOWN,enemy->x,enemy->Y,FALSE))&&(temp!=DOWN))
                            enemy->dir = DOWN;
                    }
                    else
                    {
                        if((checkMove(UP,enemy->x,enemy->Y,FALSE))&&(temp!=UP))
                            enemy->dir = UP;
                    }

                }
            }
            else
            {
                if(mhtnY>=0)
                {
                    if((checkMove(DOWN,enemy->x,enemy->Y,FALSE))&&(temp!=DOWN))
                        enemy->dir = DOWN;
                    else if(mhtnX>=0)
                    {
                        if((checkMove(RIGHT,enemy->x,enemy->Y,FALSE))&&(temp!=RIGHT))
                            enemy->dir = RIGHT;
                    }
                    else
                    {
                        if((checkMove(LEFT,enemy->x,enemy->Y,FALSE))&&(temp!=LEFT))
                            enemy->dir = LEFT;
                    }
                }
                else
                {
                    if((checkMove(UP,enemy->x,enemy->Y,FALSE))&&(temp!=UP))
                        enemy->dir = UP;
                    else if(mhtnX>=0)
                    {
                        if((checkMove(RIGHT,enemy->x,enemy->Y,FALSE))&&(temp!=RIGHT))
                            enemy->dir = RIGHT;
                    }
                    else
                    {
                        if((checkMove(LEFT,enemy->x,enemy->Y,FALSE))&&(temp!=LEFT))
                            enemy->dir = LEFT;
                    }
                }
            }
        }
    }
    moveSprite(&enemy->x,&enemy->Y,enemy->dir, FALSE, FALSE,frameCount);
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
            if(checkMove(i,enemy->x,enemy->Y,FALSE))
                a++;
        }
        temp = reverseDir(enemy->dir);
        if(a == 2)
        {
            enemy->l = TRUE;
            while((!checkMove(enemy->dir,enemy->x,enemy->Y,FALSE))||(enemy->dir==temp))
            {
                enemy->dir = rand()%4;
            }
        }
        else if((a>2)&&enemy->l)
        {
            enemy->l = FALSE;
            if(abs(mhtnX)>=abs(mhtnY))
            {
                if(mhtnX >= 0)
                {
                    if((checkMove(RIGHT,enemy->x,enemy->Y,FALSE))&&(temp!=RIGHT))
                        enemy->dir = RIGHT;
                    else if(mhtnY >= 0)
                    {
                        if((checkMove(DOWN,enemy->x,enemy->Y,FALSE))&&(temp!=DOWN))
                            enemy->dir = DOWN;
                    }
                    else
                    {
                        if((checkMove(UP,enemy->x,enemy->Y,FALSE))&&(temp!=UP))
                            enemy->dir = UP;
                    }

                }
                else
                {
                    if((checkMove(LEFT,enemy->x,enemy->Y,FALSE))&&(temp!=LEFT))
                        enemy->dir = LEFT;
                    else if(mhtnY >= 0)
                    {
                        if((checkMove(DOWN,enemy->x,enemy->Y,FALSE))&&(temp!=DOWN))
                            enemy->dir = DOWN;
                    }
                    else
                    {
                        if((checkMove(UP,enemy->x,enemy->Y,FALSE))&&(temp!=UP))
                            enemy->dir = UP;
                    }

                }
            }
            else
            {
                if(mhtnY>=0)
                {
                    if((checkMove(DOWN,enemy->x,enemy->Y,FALSE))&&(temp!=DOWN))
                        enemy->dir = DOWN;
                    else if(mhtnX>=0)
                    {
                        if((checkMove(RIGHT,enemy->x,enemy->Y,FALSE))&&(temp!=RIGHT))
                            enemy->dir = RIGHT;
                    }
                    else
                    {
                        if((checkMove(LEFT,enemy->x,enemy->Y,FALSE))&&(temp!=LEFT))
                            enemy->dir = LEFT;
                    }
                }
                else
                {
                    if((checkMove(UP,enemy->x,enemy->Y,FALSE))&&(temp!=UP))
                        enemy->dir = UP;
                    else if(mhtnX>=0)
                    {
                        if((checkMove(RIGHT,enemy->x,enemy->Y,FALSE))&&(temp!=RIGHT))
                            enemy->dir = RIGHT;
                    }
                    else
                    {
                        if((checkMove(LEFT,enemy->x,enemy->Y,FALSE))&&(temp!=LEFT))
                            enemy->dir = LEFT;
                    }
                }
            }
        }
    }
    moveSprite(&enemy->x,&enemy->Y,enemy->dir, FALSE, FALSE,frameCount);
}

void movePac(int *dir, int *last, int *temp, int *x, int *y, int keyPressed,int frameCount)
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
        moveSprite(x, y, *dir, TRUE, FALSE,frameCount);
    }
    else if(move == TRUE)
    {
        moveSprite(x, y, *dir, TRUE, FALSE,frameCount);
    }
    else if(moveBckUp == TRUE)
    {
        *last = *dir;
        *dir = *temp;
        moveSprite(x, y, *dir, TRUE, FALSE,frameCount);
    }
    else
    {
        *dir = NONE;
        *last = NONE;
        *temp = NONE;
    }

}

void drawGhost(SDL_Renderer *ren, SDL_Texture *tex, int x, int y, int dir, BOOL frightened, struct timespec fClock, int ghostType, int frameCount)
{
    struct timespec start;
    clock_gettime(CLOCK_REALTIME, &start);
    double diff = ( start.tv_sec - fClock.tv_sec ) + ( start.tv_nsec - fClock.tv_nsec )/1E9;
    //printf("%lf\n",diff);
    int desc = 0;
    int anim = 5*(BLOCKSIZE*2);
    if((frameCount/15) == 0)
        anim = 0;
    SDL_Rect block;
    block.x=x;
    block.y=y;
    block.w=BLOCKSIZE-1;
    block.h=BLOCKSIZE-1;
    SDL_Rect ghost;
    ghost.w=BLOCKSIZE*2;
    ghost.h=BLOCKSIZE*2;
    ghost.x=ghostType*(BLOCKSIZE*2)+anim;
    ghost.y=dir*(BLOCKSIZE*2);
    if(frightened)
    {
        if(diff >= 5)
        {

           desc = (int)(diff*10)%2;
           if(desc == 1)
           {
               ghost.x=5*(BLOCKSIZE*2)+anim;
               ghost.y=BLOCKSIZE*2;
           }
           else
           {
               ghost.x=5*(BLOCKSIZE*2)+anim;
               ghost.y=0;
           }
        }
        else
        {
        ghost.x=5*(BLOCKSIZE*2)+anim;
        ghost.y=BLOCKSIZE*2;
        }
    }
    SDL_RenderCopy(ren, tex,&ghost, &block);
}

void drawPacman(SDL_Renderer *ren, SDL_Texture *tex, int x, int y, int dir, int frameCount)
{
    SDL_Rect block;
    block.x=x;
    block.y=y;
    block.w=BLOCKSIZE;
    block.h=BLOCKSIZE;
    SDL_Rect pacman;
    pacman.w=BLOCKSIZE*2;
    pacman.h=BLOCKSIZE*2;
    pacman.x=dir*(BLOCKSIZE*2);
    pacman.y=(frameCount/7)*(BLOCKSIZE*2);
    if(dir == NONE)
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
        if(checkMove(i,enemy->x,enemy->Y,FALSE))
            a++;
    }

    temp = reverseDir(enemy->dir);
    if((a > 1)&&((abs(enemy->tx-enemy->x)>2)||(abs(enemy->ty-enemy->Y)>2)))
    {
        enemy->tx = enemy->x;
        enemy->ty = enemy->Y;
        enemy->dir = rand()%4;
        while((!checkMove(enemy->dir,enemy->x,enemy->Y,FALSE))||(enemy->dir==temp))
        {
            (enemy->dir)++;
            if(enemy->dir > 3)
                enemy->dir = 0;
        }
    }

    moveSprite(&enemy->x, &enemy->Y, enemy->dir, FALSE, TRUE,frameCount);
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

BOOL checkDeath(int ghostX,int ghostY,int pacX,int pacY)
{
    int diff = 3*BLOCKSIZE/4;
    if((abs(pacX-ghostX)<=diff)&&(abs(pacY-ghostY)<=diff))
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

    speedy->x=13*BLOCKSIZE;
    speedy->Y=13*BLOCKSIZE;
    speedy->dir=UP;
    speedy->alive=TRUE;
    speedy->gate=TRUE;
    speedy->l=TRUE;
    speedy->tx=0;
    speedy->ty=0;

    bashful->x=14*BLOCKSIZE;
    bashful->Y=13*BLOCKSIZE;
    bashful->dir=LEFT;
    bashful->alive=TRUE;
    bashful->gate=TRUE;
    bashful->l=TRUE;
    bashful->tx=0;
    bashful->ty=0;

    pokey->x=14*BLOCKSIZE;
    pokey->Y=13*BLOCKSIZE;
    pokey->dir=LEFT;
    pokey->alive=TRUE;
    pokey->gate=TRUE;
    pokey->l=TRUE;
    pokey->tx=0;
    pokey->ty=0;

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

