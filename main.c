#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
// include the map for the maze.
#include "map.h"
#include "pacman.h"
// the size of the block to draw
const int c_blockSize=25;
const float c_scale=0.08f;
// the width of the screen taking into account the maze and block
#define WIDTH (COLS-1)*c_blockSize
// the height of the screen taking into account the maze and block
#define HEIGHT (ROWS-1)*c_blockSize
// an enumeration for direction to move USE more enums!
enum DIRECTION{LEFT,RIGHT,UP,DOWN,NONE};
enum TYPE{SHADOW,SPEEDY,BASHFUL,POKEY};
enum BLOCK{BLACK,BLUE,RPILL,GATE,POWERPILL};

pacman setPacDir( pacman, int, bool*, bool* );
ghost setGhostDir( ghost, int, int, int );
ghost setRandDir(ghost , int );
int getDirOpts( ghost );
void setFrightened( bool*, ghost*, ghost*, ghost*, ghost*, struct timespec );

bool checkMove( int, int, int, bool, bool );
void checkPill( int, int, bool*, struct timespec*, int, int*);
void checkTeleport( int*, int );
int checkMazeBlock( int, int );
void checkDeaths(pacman*, ghost*, ghost*, ghost*, ghost*, int*, score*);
bool checkPac( ghost, pacman, int* );
bool checkGhost( ghost*, pacman, score*);

pacman movePac( pacman, int ,int );
ghost moveGhost(ghost, ghost, pacman, int, int);
ghost moveFrightened(ghost);
ghost moveShadow(ghost, pacman );
ghost moveSpeedy(ghost, pacman);
ghost moveBashful(ghost, pacman, ghost );
ghost movePokey(ghost, pacman);
void moveSprite( int*, int*, int, bool, bool, bool, int );

void drawScreen(SDL_Renderer*, SDL_Texture*[], pacman, int, int, struct timespec, int, int, bool,TTF_Font*, ... );
void drawStart( SDL_Renderer*, SDL_Texture*);
void drawGameOver( SDL_Renderer*, SDL_Texture*);
void drawAllEnemies(SDL_Renderer*, struct timespec, int, SDL_Texture*, va_list);
void drawGhost( SDL_Renderer*, SDL_Texture*, ghost, bool, struct timespec, int, int );
void drawPacman( SDL_Renderer*, SDL_Texture*, pacman, int );
void drawMaze( SDL_Renderer*, SDL_Texture*, SDL_Texture* );
void drawScore(SDL_Renderer*, int, TTF_Font* );

int pillCount();
int reverseDir( int );
double timeDiff( struct timespec*, struct timespec );
void reset(bool*, bool*, bool*, bool*, bool*, int*, int *, ghost*, ghost*, ghost*, ghost *io_pokey, pacman*);
SDL_Texture *createTex( const char*, SDL_Renderer*, SDL_Surface* );
void createTexArray(int, SDL_Texture*[], SDL_Renderer *, SDL_Surface *, ...);
ghost createGhost(int, int, int, int, int, void(*)(void));


int main()
{
  // initialise SDL and check that it worked otherwise exit
  // see here for details http://wiki.libsdl.org/CategoryAPI
  if ( SDL_Init(SDL_INIT_EVERYTHING) == -1 )
  {
    printf("%s\n",SDL_GetError());
    return EXIT_FAILURE;
  }
  if ( TTF_Init() == -1 )
  {
    printf("%s\n",SDL_GetError());
    return EXIT_FAILURE;
  }
  // we are now going to create an SDL window

  SDL_Window *win = 0;
  win = SDL_CreateWindow("Pacman", 100, 100, WIDTH, HEIGHT+c_blockSize, SDL_WINDOW_SHOWN);
  if ( win == 0 )
  {
    printf("%s\n",SDL_GetError());
    return EXIT_FAILURE;
  }
  // the renderer is the core element we need to draw, each call to SDL for drawing will need the
  // renderer pointer
  SDL_Renderer *ren = 0;
  ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  // check to see if this worked
  if ( ren == 0 )
  {
    printf("%s\n",SDL_GetError() );
    return EXIT_FAILURE;
  }
  SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);

  TTF_Font* Sans = TTF_OpenFont("/usr/share/fonts/gnu-free/FreeSans.ttf", 24);
  SDL_Surface * image = NULL;
  SDL_Texture* texArr[7];
  createTexArray(7,texArr,ren,image,"pacsprite.png","pills.png","ghostSprites.png","startScreen.png","gameOver.png","pacDeath.png","wallSegs.png");
  bool quit=false;

  ghost shadow, speedy, bashful, pokey;
  pacman pac;
  bool keyPressed, begin, chngDir, frightened, lifeDeduct;
  double diff;
  int frameCount, aiMode, lives = 3;
  reset(&keyPressed,&begin,&chngDir,&frightened,
        &lifeDeduct,&frameCount,&aiMode,&shadow,
        &speedy,&bashful,&pokey,&pac);
  int moveMode[7]={7,27,34,54,59,79,84};
  srand(time(NULL));
  struct timespec start, end, lEnd, frightenedClock;
  clock_gettime(CLOCK_REALTIME, &start);
  clock_gettime(CLOCK_REALTIME, &lEnd);
  score points = {0,0,0,0};

  while ( quit != true )
  {
    SDL_Event event;
    // grab the SDL event (this will be keys etc)
    while ( SDL_PollEvent(&event) )
    {
      keyPressed = false;
      // look for the x of the window being clicked and exit
      if ( event.type == SDL_QUIT )
      {
        quit = true;
      }
      // check for a key down
      else if ( event.type == SDL_KEYDOWN )
      {
        switch ( event.key.keysym.sym )
        {
          case SDLK_ESCAPE:
            quit=true;
            break;
          case SDLK_UP:
          case SDLK_w:
          {
            pac = setPacDir(pac, UP, &begin, &keyPressed);
            break;
          }
          case SDLK_DOWN:
          case SDLK_s:
          {
            pac = setPacDir(pac, DOWN, &begin, &keyPressed);
            break;
          }
          case SDLK_RIGHT:
          case SDLK_d:
          {
            pac = setPacDir(pac, RIGHT, &begin, &keyPressed);
            break;
          }
          case SDLK_LEFT:
          case SDLK_a:
          {
            pac = setPacDir(pac, LEFT, &begin, &keyPressed);
            break;
          }
        }
      }
    }

    int pills = pillCount();
    points.pills = (256 - pills)*10;

    setFrightened(&frightened, &shadow, &speedy, &bashful, &pokey, frightenedClock);
    checkDeaths(&pac,&shadow,&speedy,&bashful,&pokey,&frameCount, &points);
    checkPill(pac.x, pac.y, &frightened, &frightenedClock, aiMode, &moveMode[0]);

    if ( !begin )
    {
      clock_gettime(CLOCK_REALTIME, &start);
    }
    else if ( pac.alive )
    {
      lifeDeduct = true;
      pac = movePac(pac, keyPressed,frameCount);

      diff = timeDiff(&end,start);
      if ( (diff >= moveMode[aiMode]) && (aiMode < 7) )
      {
        aiMode++;
      }

      shadow = moveGhost(shadow,shadow,pac,aiMode,frameCount);
      speedy = moveGhost(speedy,shadow,pac,aiMode,frameCount);
      if ( (pills <= 228) && !frightened )
      {
        bashful = moveGhost(bashful,shadow,pac,aiMode,frameCount);
      }
      if ( (pills <= 172) && !frightened )
      {
        pokey = moveGhost(pokey,shadow,pac,aiMode,frameCount);
      }
    }
    else if ( lives > 0 )
    {
      if ( lifeDeduct )
      {
        clock_gettime(CLOCK_REALTIME, &lEnd);
        lives--;
      }
      diff = timeDiff(&end,lEnd);
      if ( (diff >= 2) && !lifeDeduct )
      {
        reset(&keyPressed,&begin,&chngDir,&frightened,
              &lifeDeduct,&frameCount,&aiMode,&shadow,
              &speedy,&bashful,&pokey,&pac);
      }
      lifeDeduct = false;
    }
    if(pac.alive || frameCount < 29)
    {
      frameCount++;
      if ( frameCount >= 30 )
      {
        frameCount = 0;
      }
    }
    points.total = points.ghosts + points.pills + points.lastLevel;
    drawScreen(ren,texArr,pac,frameCount,frameCount,frightenedClock,points.total,lives,begin,Sans,pokey,speedy,bashful,shadow);

  }
  SDL_Quit();
  return EXIT_SUCCESS;
}

pacman setPacDir(pacman io_pac, int _newDir, bool *io_begin, bool *io_keyPressed)
{
  *io_begin = true;
  io_pac.temp = io_pac.dir;
  io_pac.last = NONE;
  *io_keyPressed = true;
  io_pac.dir = _newDir;
  return io_pac;
}
ghost setGhostDir(ghost io_enemy, int _mhtnX, int _mhtnY, int _temp)
{
  bool check[4];
  check[RIGHT] = checkMove(RIGHT,io_enemy.x,io_enemy.y,false,io_enemy.alive);
  check[LEFT] = checkMove(LEFT,io_enemy.x,io_enemy.y,false,io_enemy.alive);
  check[UP] = checkMove(UP,io_enemy.x,io_enemy.y,false,io_enemy.alive);
  check[DOWN] = checkMove(DOWN,io_enemy.x,io_enemy.y,false,io_enemy.alive);
  io_enemy.turn = false;
  int distA = _mhtnX;
  int distB = _mhtnY;
  int directions[4] = {RIGHT,LEFT,DOWN,UP};
  if ( abs(_mhtnX) < abs(_mhtnY) )
  {
    distB = _mhtnX;
    distA = _mhtnY;
    memcpy(directions, (int const[]){DOWN,UP,RIGHT,LEFT}, sizeof(directions));
  }
  if ( (distA >= 0) && ((check[directions[0]]) && (_temp!=directions[0])) )
  {
    io_enemy.dir = directions[0];
  }
  else if ( (distA < 0) && ((check[directions[1]]) && (_temp!=directions[1])) )
  {
    io_enemy.dir = directions[1];
  }
  else if ( (distB >= 0) && ((check[directions[2]]) && (_temp!=directions[2])) )
  {
    io_enemy.dir = directions[2];
  }
  else if ( (check[directions[3]]) && (_temp!=directions[3]) )
  {
    io_enemy.dir = directions[3];
  }
  return io_enemy;
}
ghost setRandDir(ghost io_enemy, int temp)
{
  io_enemy.turn = true;
  while( (!checkMove(io_enemy.dir,io_enemy.x,io_enemy.y,false,io_enemy.alive)) || (io_enemy.dir == temp) )
  {
    io_enemy.dir = rand()%4;
  }
  return io_enemy;
}
int getDirOpts(ghost io_enemy)
{
  int a = 0;
  for(int i = 0; i<4; ++i)
  {
    if ( checkMove (i,io_enemy.x,io_enemy.y,false,io_enemy.alive) )
    {
      a++;
    }
  }
  return a;
}
void setFrightened(bool *io_frightened, ghost *io_shadow, ghost *io_speedy, ghost *io_bashful, ghost *io_pokey, struct timespec _frightenedClock)
{
  struct timespec end;
  if ( *io_frightened )
  {
    if ( io_shadow->alive )
    {
      io_shadow->frightened = true;
      io_shadow->dir = reverseDir(io_shadow->dir);
    }
    if ( io_speedy->alive )
    {
      io_speedy->frightened = true;
      io_speedy->dir = reverseDir(io_speedy->dir);
    }
    if ( io_bashful->alive )
    {
      io_bashful->frightened = true;
      io_bashful->dir = reverseDir(io_bashful->dir);
    }
    if ( io_pokey->alive )
    {
      io_pokey->frightened = true;
      io_pokey->dir = reverseDir(io_pokey->dir);
    }
    *io_frightened = false;
  }

  double diff = timeDiff(&end,_frightenedClock);
  if ( !io_shadow->alive || (diff >= 7) )
  {
    io_shadow->frightened = false;
  }
  if ( !io_speedy->alive || (diff >= 7) )
  {
    io_speedy->frightened = false;
  }
  if ( !io_bashful->alive || (diff >= 7) )
  {
    io_bashful->frightened = false;
  }
  if ( !io_pokey->alive || (diff >= 7) )
  {
    io_pokey->frightened = false;
  }
}

bool checkMove(int _dir, int _x, int _y, bool _slow, bool _alive)
{
  bool valid = true;
  int a,b,c;
  int step = c_blockSize*c_scale;
  if ( !_slow )
  {
    step+=1;
  }
  int dim = (c_blockSize - 1) / 2 - 1;
  switch (_dir)
  {
    case UP:
    {
      a = round((_y - (step + dim)) / ((float)c_blockSize)) + 1;
      b = round((_x - dim) / ((float)c_blockSize)) + 1;
      c = round((_x + dim) / ((float)c_blockSize)) + 1;
      valid =!( ((map[a][b] == BLUE) || (map[a][c] == BLUE)) ||
              ( (_slow || _alive) && ((map[a][b] == GATE) || (map[a][c] == GATE)))
              );
      break;
    }
    case DOWN:
    {
      a = round((_y + (step + dim)) / ((float)c_blockSize)) + 1;
      b = round((_x - dim) / ((float)c_blockSize)) + 1;
      c = round((_x + dim) / ((float)c_blockSize)) + 1;
      valid =!( ((map[a][b] == BLUE) || (map[a][c] == BLUE)) ||
              ( (_slow || _alive) && ((map[a][b] == GATE) || (map[a][c] == GATE)))
              );
      break;
    }
    case LEFT:
    {
      a = round((_y + dim) / ((float)c_blockSize)) + 1;
      b = round((_y - dim) / ((float)c_blockSize)) + 1;
      c = round((_x - (step + dim)) / ((float)c_blockSize)) + 1;
      valid =!( ((map[a][c] == BLUE) || (map[b][c] == BLUE)) ||
              ( (_slow || _alive) && ((map[a][c] == GATE) || (map[b][c] == GATE)))
              );
      break;
    }
    case RIGHT:
    {
      a = round((_y + dim) / ((float)c_blockSize)) + 1;
      b = round((_y - dim) / ((float)c_blockSize)) + 1;
      c = round((_x + (step + dim)) / ((float)c_blockSize)) + 1;
      valid =!( ((map[a][c] == BLUE) || (map[b][c] == BLUE)) ||
              ( (_slow || _alive) && ((map[a][c] == GATE) || (map[b][c] == GATE)))
              );
      break;
    }
    case NONE:
    {
      valid = false;
      break;
    }
  }
  return valid;
}
void checkPill(int _x, int _y, bool *io_frightened, struct timespec *io_fStart, int _aiMode, int *io_t)
{
  int a = (int)round(_y/((float)c_blockSize))+1;
  int b = (int)round(_x/((float)c_blockSize))+1;
  if ( map[a][b] == RPILL )
  {
    map[a][b] = BLACK;
  }
  else if ( map[a][b] == POWERPILL )
  {
    map[a][b] = BLACK;
    *io_frightened = true;
    clock_gettime(CLOCK_REALTIME, io_fStart);
    for(int i = _aiMode; i < 7; i++)
    {
      *(io_t+i)+=7;
    }
  }
}
void checkTeleport(int *io_x, int _dir)
{
  int startBound = -(c_blockSize / 5);
  int endBound = (COLS - 1) * c_blockSize + (3 * startBound);
  if ( (*io_x - c_blockSize * 0.5 <= startBound) && (_dir == LEFT) )
  {
    *io_x = endBound - c_blockSize * 0.25;
  }
  else if ( (*io_x + c_blockSize  *0.5 >= endBound) && (_dir == RIGHT) )
  {
    *io_x = 0;
  }
}
int checkMazeBlock(int _x, int _y)
{
  int type;
  int left = map[_y][_x-1];
  int right = map[_y][_x+1];
  int up = map[_y-1][_x];
  int down = map[_y+1][_x];
  int upperLeft = map[_y-1][_x-1];
  int lowerLeft = map[_y+1][_x-1];
  int upperRight = map[_y-1][_x+1];
  int lowerRight = map[_y+1][_x+1];

  if ( (right != BLUE) || (left != BLUE) )
  {
    type = 0;
  }
  if ( (up != BLUE) || (down != BLUE) )
  {
    type = 1;
  }
  if ( (upperRight != BLUE) && (up == BLUE) && (right == BLUE) )
  {
    type = 5;
  }
  if ( (lowerRight != BLUE) && (down == BLUE) && (right == BLUE) )
  {
    type = 3;
  }
  if ( (upperLeft != BLUE) && (up == BLUE) && (left == BLUE) )
  {
    type = 4;
  }
  if ( (lowerLeft != BLUE) && (down == BLUE) && (left == BLUE) )
  {
    type = 2;
  }
  if ( (right != BLUE) && (up != BLUE) && (upperRight != BLUE) )
  {
    type = 2;
  }
  if ( (left != BLUE) && (up != BLUE) && (upperLeft != BLUE) )
  {
    type = 3;
  }
  if ( (right != BLUE) && (down != BLUE) && (lowerRight != BLUE) )
  {
    type = 4;
  }
  if ( (left != BLUE) && (down != BLUE) && (lowerLeft != BLUE) )
  {
    type = 5;
  }
  if ( (right == GATE) || (left == GATE) )
  {
    type = 1;
  }

  return type;
}
void checkDeaths(pacman *io_pac, ghost *io_shadow, ghost *io_speedy, ghost *io_bashful, ghost *io_pokey,int *io_frameCount, score *points)
{
  if ( io_shadow->frightened || !io_shadow->alive )
  {
    io_shadow->alive = checkGhost(io_shadow, *io_pac, points);
  }
  else if ( io_pac->alive && io_shadow->alive )
  {
    io_pac->alive = checkPac(*io_shadow, *io_pac, io_frameCount);
  }
  if ( io_speedy->frightened || !io_speedy->alive )
  {
    io_speedy->alive = checkGhost(io_speedy, *io_pac, points);
  }
  else if ( io_pac->alive && io_speedy->alive )
  {
    io_pac->alive = checkPac(*io_speedy, *io_pac, io_frameCount);
  }
  if ( io_bashful->frightened || !io_bashful->alive )
  {
    io_bashful->alive = checkGhost(io_bashful, *io_pac, points);
  }
  else if ( io_pac->alive && io_bashful->alive )
  {
    io_pac->alive = checkPac(*io_bashful, *io_pac, io_frameCount);
  }
  if ( io_pokey->frightened || !io_pokey->alive )
  {
    io_pokey->alive = checkGhost(io_pokey, *io_pac, points);
  }
  else if ( io_pac->alive && io_pokey->alive )
  {
    io_pac->alive = checkPac(*io_pokey, *io_pac, io_frameCount);
  }
}
bool checkPac(ghost _enemy, pacman _pac, int *io_frameCount)
{
  int tlrnce = 3*c_blockSize/4;
  if ( (abs(_pac.x - _enemy.x) <= tlrnce) && (abs(_pac.y - _enemy.y) <= tlrnce) )
  {
    *io_frameCount = 0;
    return false;
  }
  else
  {
    return true;
  }
}
bool checkGhost(ghost *io_enemy, pacman _pac, score *points)
{
  int tlrnce = 3*c_blockSize/4;
  if ( io_enemy->alive )
  {
    if ( (abs(_pac.x - io_enemy->x) <= tlrnce) && (abs(_pac.y - io_enemy->y) <= tlrnce) )
    {
      points->ghosts+=100;
      return false;
    }
    else
    {
      return true;
    }
  }
  else
  {
    if ( (abs(io_enemy->y - 14*c_blockSize) <= tlrnce) &&
         ((abs(io_enemy->x - 14*c_blockSize) <= tlrnce) ||
         (abs(io_enemy->x - 13*c_blockSize) <= tlrnce))
       )
    {
      io_enemy->dir = reverseDir(io_enemy->dir);
      return true;
    }
    else
    {
      return false;
    }
  }

}

pacman movePac( pacman io_pac, int _keyPressed,int _frameCount)
{
  bool move, moveBckUp, movePrdct = false;
  move = checkMove(io_pac.dir,io_pac.x,io_pac.y,true,true);

  if ( io_pac.last != NONE )
  {
    movePrdct = checkMove(io_pac.last,io_pac.x,io_pac.y,true,true);
  }
  else
  {
    movePrdct = false;
  }

  moveBckUp = checkMove(io_pac.temp,io_pac.x,io_pac.y,true,true);

  if ( (movePrdct == true) && (_keyPressed == false) )
  {
    io_pac.dir = io_pac.last;
    io_pac.last = NONE;
    io_pac.temp = NONE;
    moveSprite(&io_pac.x, &io_pac.y, io_pac.dir, true, false,true,_frameCount);
  }
  else if ( move == true )
  {
    moveSprite(&io_pac.x, &io_pac.y, io_pac.dir, true, false,true,_frameCount);
  }
  else if ( moveBckUp == true )
  {
    io_pac.last = io_pac.dir;
    io_pac.dir = io_pac.temp;
    moveSprite(&io_pac.x, &io_pac.y, io_pac.dir, true, false,true,_frameCount);
  }
  else
  {
    io_pac.dir = NONE;
    //io_pac.last = NONE;
    io_pac.temp = NONE;
  }
  checkTeleport(&io_pac.x, io_pac.dir);
  return io_pac;
}
ghost moveGhost(ghost io_enemy, ghost _shad, pacman _target, int _aiMode, int _frameCount )
{
  if ( io_enemy.alive && (_aiMode%2 == 0) )
  {
    _target.x = io_enemy.scatX;
    _target.y = io_enemy.scatY;
    _target.dir = NONE;
    _shad.x = 0;
    _shad.y = 0;
  }
  if ( !io_enemy.alive )
  {
    _target.x = 13*c_blockSize;
    _target.y = 13*c_blockSize;
    io_enemy = moveShadow(io_enemy, _target);
  }
  else if ( io_enemy.frightened && !io_enemy.gate )
  {
    io_enemy = moveFrightened(io_enemy);
  }
  else if (io_enemy.type == BASHFUL)
  {
    ghost(*moved)(ghost,pacman,ghost);
    moved = (ghost(*)(ghost,pacman,ghost))io_enemy.move;
    io_enemy = moved(io_enemy, _target, _shad);
  }
  else
  {
    ghost(*moved)(ghost,pacman);
    moved = (ghost(*)(ghost,pacman))io_enemy.move;
    io_enemy = moved(io_enemy, _target);
  }
  moveSprite(&io_enemy.x,&io_enemy.y,io_enemy.dir, false, io_enemy.frightened, io_enemy.alive, _frameCount);
  checkTeleport(&io_enemy.x, io_enemy.dir);
  return io_enemy;
}
ghost moveFrightened(ghost io_enemy)
{
  int a = 0;
  int temp;
  for(int i = 0; i<4; ++i)
  {
    if ( checkMove(i,io_enemy.x,io_enemy.y,false,io_enemy.alive) )
    {
      a++;
    }
  }

  temp = reverseDir(io_enemy.dir);
  if ( (a > 1) && ((abs(io_enemy.tempX - io_enemy.x) > 2) || (abs(io_enemy.tempY - io_enemy.y) > 2)) )
  {
    io_enemy.tempX = io_enemy.x;
    io_enemy.tempY = io_enemy.y;
    io_enemy.dir = rand()%4;
    while( (!checkMove(io_enemy.dir,io_enemy.x,io_enemy.y,false,io_enemy.alive)) || (io_enemy.dir == temp) )
    {
      (io_enemy.dir)++;
      if ( io_enemy.dir > 3 )
      {
        io_enemy.dir = 0;
      }
    }
  }
  return io_enemy;
}
ghost moveShadow(ghost io_enemy, pacman _target)
{
  int a = getDirOpts(io_enemy);
  int temp;
  int mhtnX = _target.x-io_enemy.x;
  int mhtnY = _target.y-io_enemy.y;

  temp = reverseDir(io_enemy.dir);
  if ( a == 2 )
  {
    io_enemy = setRandDir(io_enemy, temp);
  }
  else if ( (a>2) && io_enemy.turn )
  {
    io_enemy = setGhostDir(io_enemy, mhtnX, mhtnY, temp);
  }
  return io_enemy;
}
ghost movePokey(ghost io_enemy, pacman _target)
{
  if ( io_enemy.y <= 11*c_blockSize )
  {
    io_enemy.gate = false;
  }
  else if ( io_enemy.gate )
  {
    io_enemy.dir = UP;
  }
  if ( !(io_enemy.gate) )
  {
    int dist = abs(_target.x-io_enemy.x) + abs(_target.y-io_enemy.y);
    if ( (dist < 8*c_blockSize) && !io_enemy.loop )
    {
      io_enemy.loop = true;
      io_enemy.tempX = io_enemy.x;
      io_enemy.tempY = io_enemy.y;
    }
    if ( io_enemy.loop )
    {
      io_enemy.loop = false;
      _target.x = io_enemy.tempX;
      _target.y = io_enemy.tempY;
    }
    io_enemy = moveShadow(io_enemy, _target);
  }
  return io_enemy;
}
ghost moveBashful( ghost io_enemy, pacman _target, ghost _shad)
{

  if ( io_enemy.y <= 11*c_blockSize )
  {
    io_enemy.gate = false;
  }
  else if ( io_enemy.gate )
  {
    io_enemy.dir = UP;
  }
  if ( !io_enemy.gate )
  {
    int vectorX;
    int vectorY;
    switch (_target.dir)
    {
      case UP:
      {
        _target.y-=(c_blockSize*2);
        break;
      }
      case DOWN:
      {
        _target.y+=(c_blockSize*2);
        break;
      }
      case LEFT:
      {
        _target.y-=(c_blockSize*2);
        break;
      }
      case RIGHT:
      {
        _target.y+=(c_blockSize*2);
        break;
      }
    }
    vectorX = (_target.x-_shad.x)*2;
    vectorY = (_target.y-_shad.y)*2;
    _target.x=_shad.x+vectorX;
    _target.y=_shad.y+vectorY;

    io_enemy = moveShadow(io_enemy, _target);
  }
  return io_enemy;
}
ghost moveSpeedy(ghost io_enemy, pacman _target)
{
  if ( io_enemy.y <= 11*c_blockSize )
  {
    io_enemy.gate = false;
  }
  else if ( io_enemy.gate )
  {
    io_enemy.dir = UP;
  }
  if ( !(io_enemy.gate) )
  {
    switch(_target.dir)
    {
      case UP:
      {
        _target.y-=(c_blockSize*4);
        break;
      }
      case DOWN:
      {
        _target.y+=(c_blockSize*4);
        break;
      }
      case LEFT:
      {
        _target.y-=(c_blockSize*4);
        break;
      }
      case RIGHT:
      {
        _target.y+=(c_blockSize*4);
        break;
      }
    }

    io_enemy = moveShadow(io_enemy, _target);
  }
  return io_enemy;
}
void moveSprite(int *io_x, int *io_y, int _dir, bool _slow, bool _frightened, bool _alive, int _frameCount)
{
  int step = c_blockSize*c_scale;
  if ( !_slow )
  {
    if ( _frightened || (_frameCount%2 == 0) )
      step-=1;
  }
  printf("%d\n",step);
  if ( !_alive )
  {
    step++;
  }
  switch (_dir)
  {
    case UP:
    {
      *io_y-=step;
      break;
    }
    case DOWN:
    {
      *io_y+=step;
      break;
    }
    case RIGHT:
    {
      *io_x+=step;
      break;
    }
    case LEFT:
    {
      *io_x-=step;
      break;
    }
  }
}

void drawScreen(SDL_Renderer* io_ren, SDL_Texture* _texArr[], pacman _pac, int _deathCount,
                int _frameCount, struct timespec _frightenedClock,int _points, int _lives, bool _begin,TTF_Font* Sans, ...)
{
  va_list ghostArgs;
  va_start ( ghostArgs, Sans );
  SDL_SetRenderDrawColor(io_ren, 0, 0, 0, 255);
  SDL_RenderClear(io_ren);
  drawMaze(io_ren, _texArr[1], _texArr[6]);
  if ( _pac.alive )
  {
    drawAllEnemies(io_ren,_frightenedClock, _frameCount, _texArr[2],ghostArgs);
    drawPacman(io_ren, _texArr[0], _pac, _frameCount);
  }
  else if ( _lives > 0 )
  {
    drawPacman(io_ren, _texArr[5], _pac, _deathCount);
  }
  if ( _lives <= 0 )
  {
    drawGameOver(io_ren,_texArr[4]);
    drawPacman(io_ren, _texArr[5], _pac, _deathCount);
  }
  if ( !_begin )
  {
    drawStart(io_ren,_texArr[3]);
  }
  drawScore(io_ren, _points, Sans);
  SDL_RenderPresent(io_ren);
  va_end ( ghostArgs );
}
void drawStart(SDL_Renderer *io_ren, SDL_Texture *io_tex)
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
  SDL_RenderCopy(io_ren,io_tex,&img,&screenBlock);
}
void drawGameOver(SDL_Renderer *io_ren, SDL_Texture *io_tex)
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
  SDL_RenderCopy(io_ren,io_tex,&img,&screenBlock);
}
void drawAllEnemies(SDL_Renderer *io_ren, struct timespec _frightenedClock, int _frameCount, SDL_Texture *io_tex, va_list ghosts )
{
  for ( int x = 0; x < 4; x++ )
  {
    ghost temp = va_arg ( ghosts, ghost );
    if ( temp.alive )
    {
      drawGhost(io_ren, io_tex, temp, temp.frightened, _frightenedClock,(x+1),_frameCount);
    }
    else
    {
      drawGhost(io_ren, io_tex, temp, false, _frightenedClock,0,0);
    }
  }
}
void drawGhost(SDL_Renderer *io_ren, SDL_Texture *io_tex, ghost _enemy, bool _frightened, struct timespec _fClock, int _ghostType, int _frameCount)
{
  struct timespec start;
  double diff = timeDiff(&start,_fClock);
  int desc = 0;
  int anim = 5*(c_blockSize*2);
  if ( (_frameCount/15) == 0 )
    anim = 0;
  SDL_Rect block;
  block.x=_enemy.x;
  block.y=_enemy.y;
  block.w=c_blockSize-1;
  block.h=c_blockSize-1;
  SDL_Rect ghosty;
  ghosty.w=c_blockSize*2;
  ghosty.h=c_blockSize*2;
  ghosty.x=_ghostType*(c_blockSize*2)+anim;
  ghosty.y=_enemy.dir*(c_blockSize*2);
  if ( _frightened )
  {
    desc = (int)(diff*10)%2;
    ghosty.x=5*(c_blockSize*2)+anim;
    if ( (diff >= 5) && (desc != 1) )
    {
      ghosty.y=0;
    }
    else
    {
      ghosty.y=c_blockSize*2;
    }
  }
  SDL_RenderCopy(io_ren, io_tex,&ghosty, &block);
}
void drawPacman(SDL_Renderer *io_ren, SDL_Texture *io_tex, pacman _pac, int _frameCount)
{
  int playSpeed;
  if(_pac.alive)
  {
    playSpeed = 5;
  }
  else
  {
    playSpeed = 3;
  }
  SDL_Rect copyBlock;
  copyBlock.x=_pac.x;
  copyBlock.y=_pac.y;
  copyBlock.w=c_blockSize;
  copyBlock.h=c_blockSize;
  SDL_Rect pacBlock;
  pacBlock.w=c_blockSize*2;
  pacBlock.h=c_blockSize*2;
  if ( _pac.dir == NONE )
  {
    pacBlock.x=0;
    pacBlock.y=0;
  }
  else
  {
    pacBlock.x=_pac.dir*(c_blockSize*2);
    pacBlock.y=(_frameCount/playSpeed)*(c_blockSize*2);
  }
  SDL_RenderCopy(io_ren, io_tex,&pacBlock, &copyBlock);
}
void drawMaze(SDL_Renderer *io_ren, SDL_Texture *io_tex, SDL_Texture *io_wtex)
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
      block.x=(j-1)*c_blockSize;
      block.y=(i-1)*c_blockSize;
      block.w=c_blockSize;
      block.h=c_blockSize;
      switch(map[i][j])
      {
        case (BLACK):
        {
          SDL_SetRenderDrawColor(io_ren, 0, 0, 0, 255);
          SDL_RenderFillRect(io_ren,&block);
          break;
        }
        case (BLUE):
        {
          wall.x = c_blockSize*checkMazeBlock(j,i);
          SDL_RenderCopy(io_ren, io_wtex, &wall, &block);
          break;
        }
        case (RPILL):
        {
          pill.x = 0;
          pill.y = 0;
          SDL_RenderCopy(io_ren, io_tex, &pill, &block);
          break;
        }
        case (GATE):
        {
          block.h=c_blockSize/8;
          block.y+=((c_blockSize/2)-block.h/2);
          SDL_SetRenderDrawColor(io_ren, 255, 255, 255, 255);
          SDL_RenderFillRect(io_ren,&block);
          break;
        }
        case(POWERPILL):
        {
          pill.x = c_blockSize;
          pill.y = 0;
          SDL_RenderCopy(io_ren, io_tex, &pill, &block);
          break;
        }
      }


    }
  }
}
void drawScore(SDL_Renderer* io_ren, int _points, TTF_Font* Sans)
{
  char texPoints[1000];
  sprintf(texPoints, "%d", _points);
  int len = strlen(texPoints);
  SDL_Color White = {255, 255, 255, 255};
  SDL_Surface* surfaceMessage = TTF_RenderText_Solid(Sans, "Score: ", White);
  SDL_Texture* Message = SDL_CreateTextureFromSurface(io_ren, surfaceMessage);
  SDL_Rect Message_rect;
  Message_rect.x = 10;
  Message_rect.y = HEIGHT-5;
  Message_rect.w = c_blockSize*3.5;
  Message_rect.h = c_blockSize*1.2;
  SDL_RenderCopy(io_ren, Message, NULL, &Message_rect);
  Message_rect.x += 3.5*c_blockSize;
  Message_rect.w = c_blockSize*len / 2;
  surfaceMessage = TTF_RenderText_Solid(Sans, texPoints, White);
  Message = SDL_CreateTextureFromSurface(io_ren, surfaceMessage);
  SDL_RenderCopy(io_ren, Message, NULL, &Message_rect);
  SDL_FreeSurface(surfaceMessage);
}


int pillCount()
{
  int count = 0;
  for(int i = 0; i < ROWS; ++i)
  {
    for(int j = 0; j < COLS; ++j)
    {
      if ( (map[i][j] == RPILL) || (map[i][j] == POWERPILL) )
      {
        count++;
      }
    }
  }
  return count;
}
int reverseDir(int _dir)
{
  int rev = NONE;
  switch (_dir)
  {
    case UP:
    {
      rev = DOWN;
      break;
    }
    case RIGHT:
    {
      rev = LEFT;
      break;
    }
    case DOWN:
    {
      rev = UP;
      break;
    }
    case LEFT:
    {
      rev = RIGHT;
      break;
    }
  }
  return rev;
}
double timeDiff(struct timespec *io_a, struct timespec _b)
{
  clock_gettime(CLOCK_REALTIME, io_a);
  return (((*io_a).tv_sec - _b.tv_sec) + ( (*io_a).tv_nsec - _b.tv_nsec )/1E9);
}
void reset( bool *io_keyPressed, bool *io_begin, bool *io_chngDir, bool *io_frightened, bool *io_lifeDeduct,
            int *io_frameCount, int *io_aiMode, ghost *io_shadow,
            ghost *io_speedy, ghost *io_bashful, ghost *io_pokey, pacman *io_pac)
{

  *io_shadow = createGhost(SHADOW,14,11,30,0,(void(*)(void))moveShadow);
  *io_speedy = createGhost(SPEEDY,13,13,0,0,(void(*)(void))moveSpeedy);
  *io_bashful = createGhost(BASHFUL,14,13,30,28,(void(*)(void))moveBashful);
  *io_pokey = createGhost(POKEY,13,14,0,28,(void(*)(void))movePokey);

  io_pac->x=13*c_blockSize;
  io_pac->y=23*c_blockSize+1;
  io_pac->dir=NONE;
  io_pac->last=NONE;
  io_pac->temp=NONE;
  io_pac->alive=true;

  *io_keyPressed = false;
  *io_begin = false;
  *io_chngDir = true;
  *io_frightened = false;
  *io_lifeDeduct = false;
  *io_frameCount = 0;
  *io_aiMode = 0;

}
SDL_Texture *createTex(const char *c_path, SDL_Renderer *io_ren, SDL_Surface *io_image)
{
  io_image=IMG_Load(c_path);
  if ( !io_image )
  {
    printf("IMG_Load: %s\n", IMG_GetError());
    //return EXIT_FAILURE;
  }

  SDL_Texture *tex = 0;
  tex = SDL_CreateTextureFromSurface(io_ren, io_image);
  SDL_FreeSurface(io_image);
  return tex;
}
void createTexArray(int pathCount, SDL_Texture* texArr[], SDL_Renderer *io_ren, SDL_Surface *io_image, ...)
{
  va_list paths;
  va_start(paths, io_image);
  for(int i = 0; i < pathCount; ++i)
  {
    const char *path = va_arg(paths, const char *);
    texArr[i] = createTex(path, io_ren, io_image);
  }
  va_end(paths);

}
ghost createGhost(int type, int startX, int startY, int scatX, int scatY, void(*moveFunc)(void))
{
  bool gate;
  if (  ((10 < startX) && (startX < 19)) && ((12 < startY) && (startY < 18)) )
  {
    gate = true;
  }
  else
  {
    gate = false;
  }
  startX = startX * c_blockSize;
  startY = startY * c_blockSize;
  scatX = scatX * c_blockSize;
  scatY = scatY * c_blockSize;
  ghost newGhost = {type,startX,startY,UP,gate,true,0,0,scatX,scatY,false,true,false,(void(*)(void))(moveFunc)};
  return newGhost;

}
