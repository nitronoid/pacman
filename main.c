///
///  @file main.c
///  @brief This file holds the main body of code, containing all of the functions used for the pacman game.
///  @author Jack Diver
///  @version 7.2
///  @date Last revision 13 January 2017 updated to NCCA coding standard
///  Initial version 3 November 2016

#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
// include the map for the maze.
#include "map.h"
// include the structs for sprites and score
#include "pacman.h"
// the size of the block to draw
const int g_c_blockSize=25;
// the blocks per frame to move sprites
const float g_c_scale=0.08f;
// the width of the screen taking into account the maze and block
#define WIDTH (COLS-1)*g_c_blockSize
// the height of the screen taking into account the maze and block
#define HEIGHT (ROWS-1)*g_c_blockSize
// an enumeration for directions
enum DIRECTION{LEFT,RIGHT,UP,DOWN,NONE};
// an enumeration for ghost sprite types
enum TYPE{SHADOW,SPEEDY,BASHFUL,POKEY};
// an enumeration for the different map areas
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

void drawScreen(SDL_Renderer*, SDL_Texture*[], pacman, int, struct timespec, int, int, int, bool,TTF_Font*, ... );
void drawStart( SDL_Renderer*, SDL_Texture*);
void drawGameOver( SDL_Renderer*, SDL_Texture*);
void drawAllEnemies(SDL_Renderer*, struct timespec, int, SDL_Texture*, va_list);
void drawGhost( SDL_Renderer*, SDL_Texture*, ghost, bool, struct timespec, int, int );
void drawPacman( SDL_Renderer*, SDL_Texture*, pacman, int );
void drawMaze( SDL_Renderer*, SDL_Texture*, SDL_Texture* );
void drawScore(SDL_Renderer*, int, int, const char*, TTF_Font* );

int pillCount();
int reverseDir( int );
double timeDiff( struct timespec*, struct timespec );
void reset(bool*, bool*, bool*, bool*, int*, int *, ghost*, ghost*, ghost*, ghost *io_pokey, pacman*);
void resetMap();
SDL_Texture *createTex( const char*, SDL_Renderer*, SDL_Surface* );
void createTexArray(int, SDL_Texture*[], SDL_Renderer *, SDL_Surface *, ...);
ghost createGhost(int, int, int, int, int, void(*)(void));
void saveScore(int);
int getScore();

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
  win = SDL_CreateWindow("Pacman", 100, 100, WIDTH, HEIGHT+g_c_blockSize, SDL_WINDOW_SHOWN);
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

  //load the font to draw text
  TTF_Font* Sans = TTF_OpenFont("/usr/share/fonts/gnu-free/FreeSans.ttf", 24);
  //create an image to draw textures
  SDL_Surface * image = NULL;
  //initialise the texture array
  SDL_Texture* texArr[7];
  createTexArray(7,texArr,ren,image,"pacsprite.png","pills.png","ghostSprites.png","startScreen.png","gameOver.png","pacDeath.png","wallSegs.png");
  bool quit=false;

  //creat ghosts and pacman
  ghost shadow, speedy, bashful, pokey;
  pacman pac;
  bool keyPressed, begin, frightened, lifeDeduct;
  double diff;
  int frameCount, aiMode, lives = 3, level = 1;
  //use reset function to assign default values
  reset(&keyPressed,&begin,&frightened,
        &lifeDeduct,&frameCount,&aiMode,&shadow,
        &speedy,&bashful,&pokey,&pac);
  int moveMode[7]={7,27,34,54,59,79,84};
  //load times into clocks
  srand(time(NULL));
  struct timespec start, end, lEnd, frightenedClock;
  clock_gettime(CLOCK_REALTIME, &start);
  clock_gettime(CLOCK_REALTIME, &lEnd);
  score points = {0,0,0,0};
  int highScore = 0;
  //start loop until the user quits
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
        //quit if esc key pressed
          case SDLK_ESCAPE:
            quit=true;
            break;
        //look for wasd and arrow keys to change pacmans direction
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
    //load highscore
    highScore = getScore();
    //count pills
    int pills = pillCount();
    //calculate current score from pills
    points.pills = (256 - pills)*10;
    //if there are no pills move to next level
    if(pills <= 0)
    {
      level++;
      points.lastLevel = points.ghosts + points.pills + points.lastLevel;
      points.ghosts = 0;
      reset(&keyPressed,&begin,&frightened,
            &lifeDeduct,&frameCount,&aiMode,&shadow,
            &speedy,&bashful,&pokey,&pac);
      resetMap();
    }
    //check if ghosts are frightened
    setFrightened(&frightened, &shadow, &speedy, &bashful, &pokey, frightenedClock);
    //check for any dead sprites
    checkDeaths(&pac,&shadow,&speedy,&bashful,&pokey,&frameCount, &points);
    //check whether pacman has eaten a pill
    checkPill(pac.x, pac.y, &frightened, &frightenedClock, aiMode, &moveMode[0]);
    //if a key has not been pressed reset the start clock
    if ( !begin )
    {
      clock_gettime(CLOCK_REALTIME, &start);
    }
    //if a key has been pressed we begin
    else if ( pac.alive )
    {
      lifeDeduct = true;
      //move pacman
      pac = movePac(pac, keyPressed,frameCount);
      //calculate the time since the game started
      diff = timeDiff(&end,start);
      //choose a movement mode for the ghosts
      if ( (diff >= moveMode[aiMode]) && (aiMode < 7) )
      {
        aiMode++;
      }
      //move all ghosts
      shadow = moveGhost(shadow,shadow,pac,aiMode,frameCount);
      speedy = moveGhost(speedy,shadow,pac,aiMode,frameCount);
      //only move blue and orange if the pills have dropped to a certain limit
      if ( (pills <= 228) && !frightened )
      {
        bashful = moveGhost(bashful,shadow,pac,aiMode,frameCount);
      }
      if ( (pills <= 172) && !frightened )
      {
        pokey = moveGhost(pokey,shadow,pac,aiMode,frameCount);
      }
    }
    //if pacman is dead deduct a life
    else if ( lives > 0 )
    {
      //after two seconds we reset
      if ( lifeDeduct )
      {
        clock_gettime(CLOCK_REALTIME, &lEnd);
        lives--;
      }
      diff = timeDiff(&end,lEnd);
      if ( (diff >= 2) && !lifeDeduct )
      {
        reset(&keyPressed,&begin,&frightened,
              &lifeDeduct,&frameCount,&aiMode,&shadow,
              &speedy,&bashful,&pokey,&pac);
      }
      lifeDeduct = false;
    }
    //update frame count
    if(pac.alive || frameCount < 29)
    {
      frameCount++;
      if ( frameCount >= 30 )
      {
        frameCount = 0;
      }
    }
    //calculate score
    points.total = points.ghosts + points.pills + points.lastLevel;
    if(points.total > highScore)
    {
      //save current score as new high score if it is larger than the old high score
      saveScore(points.total);
    }
    //draw the screen
    drawScreen(ren,texArr,pac,frameCount,frightenedClock,points.total,highScore,lives,begin,Sans,pokey,speedy,bashful,shadow);
  }
  //quit the program
  SDL_Quit();
  return EXIT_SUCCESS;
}

//--------------------------------------------------------------------------------------------------------
/// @brief this function assigns the value input from the keyboard to pacman
/// @param[out] o_pac is a copy of pacmans current state to be modified and returned
/// @param[in] _newDir this holds the value entered through the keyboard
/// @param[out] o_begin this is a pointer which will start the game
/// @param[out] o_keyPressed this value is changed to true when the user presses a key this frame
//--------------------------------------------------------------------------------------------------------
pacman setPacDir(pacman o_pac, int _newDir, bool *o_begin, bool *o_keyPressed)
{
  //o_begin set to true so that game will start
  *o_begin = true;
  //direction saved in temp so that it can be reverted if the move is invalid
  o_pac.temp = o_pac.dir;
  o_pac.last = NONE;
  //key pressed set to true for this frame
  *o_keyPressed = true;
  o_pac.dir = _newDir;
  return o_pac;
}

//--------------------------------------------------------------------------------------------------------
/// @brief this function deduces the direction that will move the ghost closer to pacman
/// @param[io] io_enemy is a copy of the current state of the ghost to be modified and returned
/// @param[in] _mhtnX holds the X Manhattan distance from the ghost to its target
/// @param[in] _mhtnY holds the Y Manhattan distance from the ghost to its target
/// @param[in] _prev holds the previous direction of the ghost
//--------------------------------------------------------------------------------------------------------
ghost setGhostDir(ghost io_enemy, int _mhtnX, int _mhtnY, int _prev)
{
  //check all directions for the ghost
  bool check[4];
  check[RIGHT] = checkMove(RIGHT,io_enemy.x,io_enemy.y,false,io_enemy.alive);
  check[LEFT] = checkMove(LEFT,io_enemy.x,io_enemy.y,false,io_enemy.alive);
  check[UP] = checkMove(UP,io_enemy.x,io_enemy.y,false,io_enemy.alive);
  check[DOWN] = checkMove(DOWN,io_enemy.x,io_enemy.y,false,io_enemy.alive);
  //ghost has not chnaged direction yet
  io_enemy.turn = false;
  //find distance in x and y axis from ghost to pacman
  int distA = _mhtnX;
  int distB = _mhtnY;
  //organise directions in the order to be checked
  int directions[4] = {RIGHT,LEFT,DOWN,UP};
  if ( abs(_mhtnX) < abs(_mhtnY) )
  {
    //if the distance in Y is greater than X, the order must be modified
    distB = _mhtnX;
    distA = _mhtnY;
    memcpy(directions, (int const[]){DOWN,UP,RIGHT,LEFT}, sizeof(directions));
  }
  //for each direction, check that it is both valid and is not the previous direction
  //do this until the best valid direction is found
  if ( (distA >= 0) && ((check[directions[0]]) && (_prev!=directions[0])) )
  {
    io_enemy.dir = directions[0];
  }
  else if ( (distA < 0) && ((check[directions[1]]) && (_prev!=directions[1])) )
  {
    io_enemy.dir = directions[1];
  }
  else if ( (distB >= 0) && ((check[directions[2]]) && (_prev!=directions[2])) )
  {
    io_enemy.dir = directions[2];
  }
  else if ( (check[directions[3]]) && (_prev!=directions[3]) )
  {
    io_enemy.dir = directions[3];
  }
  return io_enemy;
}

//--------------------------------------------------------------------------------------------------------
/// @brief this function assigns a random valid direction to a ghost
/// @param[io] io_enemy is a copy of the current state of the ghost to be modified and returned
/// @param[in] _prev holds the previous direction of the ghost
//--------------------------------------------------------------------------------------------------------
ghost setRandDir(ghost io_enemy, int _prev)
{
  //the ghost will turn
  io_enemy.turn = true;
  //checks that the direction is valid and not the previous direction
  while( (!checkMove(io_enemy.dir,io_enemy.x,io_enemy.y,false,io_enemy.alive)) || (io_enemy.dir == _prev) )
  {
    io_enemy.dir = rand()%4;
  }
  return io_enemy;
}

//--------------------------------------------------------------------------------------------------------
/// @brief this function counts the ammount of valid directions that the ghost can move in
/// @param[io] io_enemy is a copy of the current state of the ghost
//--------------------------------------------------------------------------------------------------------
int getDirOpts(ghost _enemy)
{
  int options = 0;
  for(int i = 0; i<4; ++i)
  {
    if ( checkMove (i,_enemy.x,_enemy.y,false,_enemy.alive) )
    {
      options++;
    }
  }
  return options;
}

//--------------------------------------------------------------------------------------------------------
/// @brief this function checks whether a ghost is frightened and modifies its state
/// @param[io] io_frightened tells us whether pacman has eaten a powerpill
/// @param[io] io_shadow holds the current state of the red ghost to be modified and returned
/// @param[io] io_speedy holds the current state of the pink ghost to be modified and returned
/// @param[io] io_bashful holds the current state of the blue ghost to be modified and returned
/// @param[io] io_pokey holds the current state of the orange ghost to be modified and returned
/// @param[in] _frightenedClock holds the time that has elapsed since pacman ate the power pill
//--------------------------------------------------------------------------------------------------------
void setFrightened(bool *io_frightened, ghost *io_shadow, ghost *io_speedy, ghost *io_bashful, ghost *io_pokey, struct timespec _frightenedClock)
{
  //checks whether pacman has eaten the powerpill
  if ( *io_frightened )
  {
    //if a ghost is alive then it is set to frightened
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
    //this variable is set to false so that the ghosts are only set to frightened once
    *io_frightened = false;
  }

  //the ghosts are only set to frightened for a period of 7 seconds
  struct timespec end;
  double diff = timeDiff(&end,_frightenedClock);
  //if more than 7 seconds have elapsed or the ghost has died it is no longer frightened
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

//--------------------------------------------------------------------------------------------------------
/// @brief this function checks whether the sprite can continue moving in its current direction
/// @param[in] _dir holds the direction of the sprite
/// @param[in] _x holds the x co-ordinate of the sprite
/// @param[in] _y holds the y co-ordinate of the sprite
/// @param[in] _slow tells the function whether to check 2 or 3 pixels ahead
/// @param[in] _alive tells the function whether the sprite is alive as it moves faster when dead
//--------------------------------------------------------------------------------------------------------
bool checkMove(int _dir, int _x, int _y, bool _pac, bool _alive)
{
  //we assume that the move is valid until we find otherwise
  bool valid = true;
  int a,b,c;
  //the step is the ammount of pixels we are checking ahead of the sprite
  int step = g_c_blockSize*g_c_scale;
  if ( !_pac )
  {
    //we check one pixel further for pacman as this sprite moves faster
    step+=1;
  }
  //this variable holds the distance from the centre of the pacman block to its edges
  int dim = (g_c_blockSize - 1) / 2 - 1;
  //dependant on the direction the sprite is moving we check diffrent corners of the block
  switch (_dir)
  {
    case UP:
    {
      //if the sprite moves up we only need to check the two upper corners of the block
      //to access a part of the map array we scale the pixel co-ordinates down by blocksize
      a = round((_y - (step + dim)) / ((float)g_c_blockSize)) + 1;
      b = round((_x - dim) / ((float)g_c_blockSize)) + 1;
      c = round((_x + dim) / ((float)g_c_blockSize)) + 1;
      //we check whether the next position of the block is in an area that is not allowed
      valid =!( ((map[a][b] == BLUE) || (map[a][c] == BLUE)) ||
              ( (_pac || _alive) && ((map[a][b] == GATE) || (map[a][c] == GATE)))
              );
      break;
    }
    case DOWN:
    {
      //if the sprite moves up we only need to check the two lower corners of the block
      //to access a part of the map array we scale the pixel co-ordinates down by blocksize
      a = round((_y + (step + dim)) / ((float)g_c_blockSize)) + 1;
      b = round((_x - dim) / ((float)g_c_blockSize)) + 1;
      c = round((_x + dim) / ((float)g_c_blockSize)) + 1;
      //we check whether the next position of the block is in an area that is not allowed
      valid =!( ((map[a][b] == BLUE) || (map[a][c] == BLUE)) ||
              ( (_pac || _alive) && ((map[a][b] == GATE) || (map[a][c] == GATE)))
              );
      break;
    }
    case LEFT:
    {
      //if the sprite moves left we only need to check the two left hand side corners of the block
      //to access a part of the map array we scale the pixel co-ordinates down by blocksize
      a = round((_y + dim) / ((float)g_c_blockSize)) + 1;
      b = round((_y - dim) / ((float)g_c_blockSize)) + 1;
      c = round((_x - (step + dim)) / ((float)g_c_blockSize)) + 1;
      //we check whether the next position of the block is in an area that is not allowed
      valid =!( ((map[a][c] == BLUE) || (map[b][c] == BLUE)) ||
              ( (_pac || _alive) && ((map[a][c] == GATE) || (map[b][c] == GATE)))
              );
      break;
    }
    case RIGHT:
    {
      //if the sprite moves up we only need to check the two right hand side corners of the block
      //to access a part of the map array we scale the pixel co-ordinates down by blocksize
      a = round((_y + dim) / ((float)g_c_blockSize)) + 1;
      b = round((_y - dim) / ((float)g_c_blockSize)) + 1;
      c = round((_x + (step + dim)) / ((float)g_c_blockSize)) + 1;
      //we check whether the next position of the block is in an area that is not allowed
      valid =!( ((map[a][c] == BLUE) || (map[b][c] == BLUE)) ||
              ( (_pac || _alive) && ((map[a][c] == GATE) || (map[b][c] == GATE)))
              );
      break;
    }
    case NONE:
    {
      //if the direction is none then we don't want the sprite to move
      valid = false;
      break;
    }
  }
  return valid;
}

//--------------------------------------------------------------------------------------------------------
/// @brief this function checks whether pacman is currently eating a pill
/// @param[in] _x holds the x co-ordinate of the sprite
/// @param[in] _y holds the y co-ordinate of the sprite
/// @param[io] io_frightened may need to be modified if pacman collides and eats a power pill
/// @param[io] io_frightened start is a pointer to the frightened clock which will need to be started if a power pill is eaten
/// @param[in] _aiMode holds the current movement mode for the ghosts
/// @param[io] io_moveModes is the array of times for which the movement modes will change
//--------------------------------------------------------------------------------------------------------
void checkPill(int _x, int _y, bool *io_frightened, struct timespec *io_frightenedStart, int _aiMode, int *io_moveModes)
{
  //to access a part of the map array we scale the pixel co-ordinates down by blocksize
  int a = (int)round(_y/((float)g_c_blockSize))+1;
  int b = (int)round(_x/((float)g_c_blockSize))+1;
  //if pacman eats a pill we replace that block with an empty one
  if ( map[a][b] == RPILL )
  {
    map[a][b] = BLACK;
  }
  //if pacman eats a power pill we replace the block with an empty one
  //we also set the state of the ghosts to frightned and start the clock timer
  else if ( map[a][b] == POWERPILL )
  {
    map[a][b] = BLACK;
    *io_frightened = true;
    clock_gettime(CLOCK_REALTIME, io_frightenedStart);
    //we also add 7 seconds to each movement mode change for the ghosts to account for the time spent frightned
    for(int i = _aiMode; i < 7; i++)
    {
      *(io_moveModes+i)+=7;
    }
  }
}

//--------------------------------------------------------------------------------------------------------
/// @brief this function checks whether a sprite is in the teleport tunnel and moves it accordingly
/// @param[io] io_x is a pointer to the x co-ordinate of the sprite
/// @param[in] _dir is the current direction of the sprite
//--------------------------------------------------------------------------------------------------------
void checkTeleport(int *io_x, int _dir)
{
  //first we make the boundaries which when crossed will teleport the sprite
  int startBound = -(g_c_blockSize / 5);
  int endBound = (COLS - 1) * g_c_blockSize + (3 * startBound);
  //then we check both and if one is met the sprite is teleported using its x co-ordinate
  if ( (*io_x - g_c_blockSize * 0.5 <= startBound) && (_dir == LEFT) )
  {
    *io_x = endBound - g_c_blockSize * 0.25;
  }
  else if ( (*io_x + g_c_blockSize  *0.5 >= endBound) && (_dir == RIGHT) )
  {
    *io_x = 0;
  }
}

//--------------------------------------------------------------------------------------------------------
/// @brief this function analyses the map and assigns a texture from a sprite sheet to create the original pacman maze
/// @param[in] _x holds the x co-ordinate of the sprite
/// @param[in] _y holds the y co-ordinate of the sprite
//--------------------------------------------------------------------------------------------------------
int checkMazeBlock(int _x, int _y)
{
  //using the x and y co-ordinates of the block we obtain the values of all the surrounding blocks
  int type;
  int left = map[_y][_x-1];
  int right = map[_y][_x+1];
  int up = map[_y-1][_x];
  int down = map[_y+1][_x];
  int upperLeft = map[_y-1][_x-1];
  int lowerLeft = map[_y+1][_x-1];
  int upperRight = map[_y-1][_x+1];
  int lowerRight = map[_y+1][_x+1];

  //through a series of checks to the surrounding blocks we find correct texture
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

//--------------------------------------------------------------------------------------------------------
/// @brief this function checks all of the ghosts for collision with pacman and updates their alive status
/// @param[io] io_pac is a pointer to the pacman sprite
/// @param[io] io_shadow is a pointer to the red ghost sprite
/// @param[io] io_speedy is a pointer to the pink ghost sprite
/// @param[io] io_bashful is a pointer to the blue ghost sprite
/// @param[io] io_pokey is a pointer to the orange ghost sprite
/// @param[io] io_frameCount is a pointer to current frame count
/// @param[io] io_points is a pointer to the current ammount of points that the player has
//--------------------------------------------------------------------------------------------------------
void checkDeaths(pacman *io_pac, ghost *io_shadow, ghost *io_speedy, ghost *io_bashful, ghost *io_pokey,int *io_frameCount, score *io_points)
{
  //if a ghost collides with pacman and it is not frightened, pacman is set to dead, if the ghost is frightened then it is set to dead
  if ( io_shadow->frightened || !io_shadow->alive )
  {
    io_shadow->alive = checkGhost(io_shadow, *io_pac, io_points);
  }
  else if ( io_pac->alive && io_shadow->alive )
  {
    io_pac->alive = checkPac(*io_shadow, *io_pac, io_frameCount);
  }
  if ( io_speedy->frightened || !io_speedy->alive )
  {
    io_speedy->alive = checkGhost(io_speedy, *io_pac, io_points);
  }
  else if ( io_pac->alive && io_speedy->alive )
  {
    io_pac->alive = checkPac(*io_speedy, *io_pac, io_frameCount);
  }
  if ( io_bashful->frightened || !io_bashful->alive )
  {
    io_bashful->alive = checkGhost(io_bashful, *io_pac, io_points);
  }
  else if ( io_pac->alive && io_bashful->alive )
  {
    io_pac->alive = checkPac(*io_bashful, *io_pac, io_frameCount);
  }
  if ( io_pokey->frightened || !io_pokey->alive )
  {
    io_pokey->alive = checkGhost(io_pokey, *io_pac, io_points);
  }
  else if ( io_pac->alive && io_pokey->alive )
  {
    io_pac->alive = checkPac(*io_pokey, *io_pac, io_frameCount);
  }
}

//--------------------------------------------------------------------------------------------------------
/// @brief this function checks whether pacman is colliding with a ghost, and so if it is alive
/// @param[in] _enemy holds a copy of the ghosts current state
/// @param[in] _pac holds a copy of pacmans current state
/// @param[io] io_frameCount is a pointer to current frame count
//--------------------------------------------------------------------------------------------------------
bool checkPac(ghost _enemy, pacman _pac, int *io_frameCount)
{
  //set a value for the distance between the two sprites centres which will count as a collision
  int tlrnce = 3*g_c_blockSize/4;
  //if the distance is less than the above value then pacman is set to dead
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

//--------------------------------------------------------------------------------------------------------
/// @brief this function checks whether a ghost is colliding with pacman, and so if it is alive
/// @param[io] io_enemy is a pointer to the ghost
/// @param[in] _pac is a copy of pacmans current state
/// @param[io] io_points is a pointer to the current score
//--------------------------------------------------------------------------------------------------------
bool checkGhost(ghost *io_enemy, pacman _pac, score *io_points)
{
  //set a value for the distance between the two sprites centres which will count as a collision
  int tlrnce = 3*g_c_blockSize/4;
  //if the distance is less than the above value and the ghost is alive, then the ghost is set to dead
  if ( io_enemy->alive )
  {
    if ( (abs(_pac.x - io_enemy->x) <= tlrnce) && (abs(_pac.y - io_enemy->y) <= tlrnce) )
    {
      //the players score is updated for killing the ghost
      io_points->ghosts+=100;
      return false;
    }
    else
    {
      return true;
    }
  }
  else
  {
    //if the ghost is dead we check to see if it has reached the home area
    if ( (abs(io_enemy->y - 14*g_c_blockSize) <= tlrnce) &&
         ((abs(io_enemy->x - 14*g_c_blockSize) <= tlrnce) ||
         (abs(io_enemy->x - 13*g_c_blockSize) <= tlrnce))
       )
    {
      //if the ghost has reached the home area we set it back to alive
      io_enemy->dir = reverseDir(io_enemy->dir);
      return true;
    }
    else
    {
      return false;
    }
  }

}

//--------------------------------------------------------------------------------------------------------
/// @brief this function calculates the new co-ordinates for pacman to move
/// @param[io] io_pac is a copy of pacmans current state to be modified and returned
/// @param[in] _keyPressed tells us whether a key ahs been pressed this frame
/// @param[in] _frameCount holds the current frame
//--------------------------------------------------------------------------------------------------------
pacman movePac( pacman io_pac, int _keyPressed,int _frameCount)
{

  bool moveCurrent, moveBackUp, movePredict = false;
  //first we check whether the current direction is valid
  moveCurrent = checkMove(io_pac.dir,io_pac.x,io_pac.y,true,true);
  //next we check the requested direction
  if ( io_pac.last != NONE )
  {
    movePredict = checkMove(io_pac.last,io_pac.x,io_pac.y,true,true);
  }
  else
  {
    movePredict = false;
  }
  //finally we check the back up direction
  moveBackUp = checkMove(io_pac.temp,io_pac.x,io_pac.y,true,true);
  //if the requested direction is valid then we set the current direction to that and make the move
  if ( (movePredict == true) && (_keyPressed == false) )
  {
    io_pac.dir = io_pac.last;
    io_pac.last = NONE;
    io_pac.temp = NONE;
    moveSprite(&io_pac.x, &io_pac.y, io_pac.dir, true, false,true,_frameCount);
  }
  // if requested is invalid but the current direction is valid then we use that to move
  else if ( moveCurrent == true )
  {
    moveSprite(&io_pac.x, &io_pac.y, io_pac.dir, true, false,true,_frameCount);
  }
  //if the requested and current are both invalid we use the back up direction to move
  else if ( moveBackUp == true )
  {
    io_pac.last = io_pac.dir;
    io_pac.dir = io_pac.temp;
    moveSprite(&io_pac.x, &io_pac.y, io_pac.dir, true, false,true,_frameCount);
  }
  //if all of the directions are invalid we don't move pacman
  else
  {
    io_pac.dir = NONE;
    //io_pac.last = NONE;
    io_pac.temp = NONE;
  }
  //finally we check to see if the sprite has reached the end of the tunnel and then teleport it
  checkTeleport(&io_pac.x, io_pac.dir);
  return io_pac;
}

//--------------------------------------------------------------------------------------------------------
/// @brief this function chooses which function to call and modifies the target before moving the ghost
/// @param[io] io_enemy is a copy of the ghosts current state to be modified and returned
/// @param[in] _shad is a copy of the red ghosts current state
/// @param[in] _target holds the position and state of where the ghosts are targeting
/// @param[in] _aiMode tells the ghosts whether to scatter or aim for the target
/// @param[in] _frameCount holds the current frame which will be used to slow down movement
//--------------------------------------------------------------------------------------------------------
ghost moveGhost(ghost io_enemy, ghost _shad, pacman _target, int _aiMode, int _frameCount )
{
  //if the ghost is alive and and move mode is even we scatter it
  if ( io_enemy.alive && (_aiMode%2 == 0) )
  {
    _target.x = io_enemy.scatX;
    _target.y = io_enemy.scatY;
    _target.dir = NONE;
    _shad.x = 0;
    _shad.y = 0;
  }
  //if the ghost is dead we set its target to home
  if ( !io_enemy.alive )
  {
    _target.x = 13*g_c_blockSize;
    _target.y = 13*g_c_blockSize;
    io_enemy = moveShadow(io_enemy, _target);
  }
  //if the ghost is frightened we move it randomly
  else if ( io_enemy.frightened && !io_enemy.gate )
  {
    io_enemy = moveFrightened(io_enemy);
  }
  //if the ghost is blue then we must cast its function differently as it has an extra parameter
  else if (io_enemy.type == BASHFUL)
  {
    ghost(*moved)(ghost,pacman,ghost);
    moved = (ghost(*)(ghost,pacman,ghost))io_enemy.move;
    io_enemy = moved(io_enemy, _target, _shad);
  }
  //if the ghost is not blue we call its move function
  else
  {
    ghost(*moved)(ghost,pacman);
    moved = (ghost(*)(ghost,pacman))io_enemy.move;
    io_enemy = moved(io_enemy, _target);
  }
  //now that we've altered the state of the sprite we move it
  moveSprite(&io_enemy.x,&io_enemy.y,io_enemy.dir, false, io_enemy.frightened, io_enemy.alive, _frameCount);
  //finally we check to see if the sprite has reached the end of the tunnel and then teleport it
  checkTeleport(&io_enemy.x, io_enemy.dir);
  return io_enemy;
}

//--------------------------------------------------------------------------------------------------------
/// @brief this function calculates the new co-ordinates for the ghost to move while frightened
/// @param[io] io_enemy holds the current state of the ghost to be modified and returned
//--------------------------------------------------------------------------------------------------------
ghost moveFrightened(ghost io_enemy)
{
  //first we calculate how many directions the ghost can move in
  int options = getDirOpts(io_enemy);
  int temp;

  //then we store the opposite direction
  temp = reverseDir(io_enemy.dir);
  //we randomly select directions until one that is not opposite and is also valid has been assigned
  if ( (options > 1) && ((abs(io_enemy.tempX - io_enemy.x) > 2) || (abs(io_enemy.tempY - io_enemy.y) > 2)) )
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

//--------------------------------------------------------------------------------------------------------
/// @brief this function is the basis of all ghost movements, but is used alone to move the red ghost
/// @param[io] io_enemy holds the current state of the ghost to be modified and returned
/// @param[in] _target holds the position and state of where the ghosts are targeting
//--------------------------------------------------------------------------------------------------------
ghost moveShadow(ghost io_enemy, pacman _target)
{
  //first we calculate how many directions the ghost can move in
  int options = getDirOpts(io_enemy);
  int temp;
  //then we calculate the distance in both axis from the ghost to its target
  int mhtnX = _target.x-io_enemy.x;
  int mhtnY = _target.y-io_enemy.y;

  temp = reverseDir(io_enemy.dir);
  //if there are 2 directions we randomly select one that is valid and not opposite
  if ( options == 2 )
  {
    io_enemy = setRandDir(io_enemy, temp);
  }
  //if there are more than two options we use the ghost logic to select the best direction
  else if ( (options>2) && io_enemy.turn )
  {
    io_enemy = setGhostDir(io_enemy, mhtnX, mhtnY, temp);
  }
  return io_enemy;
}

//--------------------------------------------------------------------------------------------------------
/// @brief this function builds on the red ghost movement but switches the target to itself if it gets within 8 tiles of pacman
/// @param[io] io_enemy holds the current state of the ghost to be modified and returned
/// @param[in] _target holds the position and state of where the ghosts are targeting
//--------------------------------------------------------------------------------------------------------
ghost movePokey(ghost io_enemy, pacman _target)
{
  //This function first must check that the ghost has left its home zone
  if ( io_enemy.y <= 11*g_c_blockSize )
  {
    io_enemy.gate = false;
  }
  else if ( io_enemy.gate )
  {
    io_enemy.dir = UP;
  }
  //if it has left the home we calculate its distance from the target
  if ( !(io_enemy.gate) )
  {
    int dist = abs(_target.x-io_enemy.x) + abs(_target.y-io_enemy.y);
    //if it is less than 8 tiles away we move it randomly
    if ( (dist < 8*g_c_blockSize) && !io_enemy.loop )
    {
      io_enemy.loop = true;
      io_enemy.tempX = io_enemy.x;
      io_enemy.tempY = io_enemy.y;
    }
    //if it is more than 8 tiles away we move it the same as the red ghost
    if ( io_enemy.loop )
    {
      io_enemy.loop = false;
      _target.x = io_enemy.tempX;
      _target.y = io_enemy.tempY;
    }
    //once the target tile has been chosen we move it the same as the red ghost
    io_enemy = moveShadow(io_enemy, _target);
  }
  return io_enemy;
}

//--------------------------------------------------------------------------------------------------------
/// @brief this function uses a vector from the red ghost to pacman and doubles that to give a target tile
/// @param[io] io_enemy holds the current state of the ghost to be modified and returned
/// @param[in] _target holds the position and state of where the ghosts are targeting
/// @param[in] _shad holds the current state of the red ghost
//--------------------------------------------------------------------------------------------------------
ghost moveBashful( ghost io_enemy, pacman _target, ghost _shad)
{
  //This function first must check that the ghost has left its home zone
  if ( io_enemy.y <= 11*g_c_blockSize )
  {
    io_enemy.gate = false;
  }
  else if ( io_enemy.gate )
  {
    io_enemy.dir = UP;
  }
  //if the ghost has left home we calculate a vector from the red ghost to pacman and double it
  if ( !io_enemy.gate )
  {
    int vectorX;
    int vectorY;
    switch (_target.dir)
    {
      case UP:
      {
        _target.y-=(g_c_blockSize*2);
        break;
      }
      case DOWN:
      {
        _target.y+=(g_c_blockSize*2);
        break;
      }
      case LEFT:
      {
        _target.y-=(g_c_blockSize*2);
        break;
      }
      case RIGHT:
      {
        _target.y+=(g_c_blockSize*2);
        break;
      }
    }
    vectorX = (_target.x-_shad.x)*2;
    vectorY = (_target.y-_shad.y)*2;
    // we find the tile at the end of this vector
    _target.x=_shad.x+vectorX;
    _target.y=_shad.y+vectorY;
    // then with the new target we move like the red ghost
    io_enemy = moveShadow(io_enemy, _target);
  }
  return io_enemy;
}

//--------------------------------------------------------------------------------------------------------
/// @brief this function builds on the red ghost movement but modifies the target to 4 tiles ahead of pacman
/// @param[io] io_enemy holds the current state of the ghost to be modified and returned
/// @param[in] _target holds the position and state of where the ghosts are targeting
//--------------------------------------------------------------------------------------------------------
ghost moveSpeedy(ghost io_enemy, pacman _target)
{
  //This function first must check that the ghost has left its home zone
  if ( io_enemy.y <= 11*g_c_blockSize )
  {
    io_enemy.gate = false;
  }
  else if ( io_enemy.gate )
  {
    io_enemy.dir = UP;
  }
  //if the ghost has left home we find the tile 4 ahead of pacman in his current direction
  if ( !(io_enemy.gate) )
  {
    switch(_target.dir)
    {
      case UP:
      {
        _target.y-=(g_c_blockSize*4);
        break;
      }
      case DOWN:
      {
        _target.y+=(g_c_blockSize*4);
        break;
      }
      case LEFT:
      {
        _target.y-=(g_c_blockSize*4);
        break;
      }
      case RIGHT:
      {
        _target.y+=(g_c_blockSize*4);
        break;
      }
    }
    //with the new target tile we move like the red ghost
    io_enemy = moveShadow(io_enemy, _target);
  }
  return io_enemy;
}

//--------------------------------------------------------------------------------------------------------
/// @brief this function takes the direction and co-ordinates of a sprite then modifies the values to move it
/// @param[io] io_x is a pointer to the x co-ordinate of the sprite
/// @param[io] io_y is a pointer to the y co-ordinate of the sprite
/// @param[in] _dir holds the current direction of the sprite
/// @param[in] _ghost tells us whether the sprite is a ghost or pacman
/// @param[in] _frightened tells us whether the sprite is frightened and so how fast to move it
/// @param[in] _alive tells us whether the sprite is alive and so how fast to move it
/// @param[in] _frameCount holds the current frame which we use to alternate movement speed
//--------------------------------------------------------------------------------------------------------
void moveSprite(int *io_x, int *io_y, int _dir, bool _pac, bool _frightened, bool _alive, int _frameCount)
{
  // we calculate the step from out two constants
  int step = g_c_blockSize*g_c_scale;
  //if it is a ghost we modify the step
  if ( !_pac )
  {
    if ( _frightened || (_frameCount%2 == 0) )
      step-=1;
  }
  //if the sprite is dead it moves faster
  if ( !_alive )
  {
    step++;
  }
  //add or subtract the step from the appropriate co-ordinate based on the direction
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


//--------------------------------------------------------------------------------------------------------
/// @brief this function is used to call all other draw functions to assemble and present the screen
/// @param[io] io_ren is the SDL_Renderer used to draw our graphics
/// @param[in] _texArray is the array that holds all the textures loaded from images
/// @param[in] _pac holds the current state of pacman
/// @param[in] _frameCount holds the current frame to time animations
/// @param[in] _frightenedClock holds the time spent in frightened mode
/// @param[in] _points holds the points to be displayed on screen
/// @param[in] _highScore holds the highest score to be displayed on screen
/// @param[in] _lives holds the current ammount of lives to be displayed
/// @param[in] _begin decides whether the start screen is drawn
/// @param[io] io_sans is the font used to draw the scores
//--------------------------------------------------------------------------------------------------------
void drawScreen(SDL_Renderer* io_ren, SDL_Texture* _texArr[], pacman _pac, int _frameCount,
                struct timespec _frightenedClock, int _points, int _highScore, int _lives, bool _begin,TTF_Font* io_sans, ...)
{
  //this is the list of ghosts to draw
  va_list ghostArgs;
  va_start ( ghostArgs, io_sans );
  //clear the screen
  SDL_SetRenderDrawColor(io_ren, 0, 0, 0, 255);
  SDL_RenderClear(io_ren);
  //draw the maze
  drawMaze(io_ren, _texArr[1], _texArr[6]);
  //if pacman is alive draw using the normal texture and draw ghosts
  if ( _pac.alive )
  {
    drawAllEnemies(io_ren,_frightenedClock, _frameCount, _texArr[2],ghostArgs);
    drawPacman(io_ren, _texArr[0], _pac, _frameCount);
  }
  //if pacman is dead and with remaining lives we draw the death animation
  else if ( _lives > 0 )
  {
    drawPacman(io_ren, _texArr[5], _pac, _frameCount);
  }
  //if there are no remaining lives we draw game over and the death animation
  if ( _lives <= 0 )
  {
    drawGameOver(io_ren,_texArr[4]);
    drawPacman(io_ren, _texArr[5], _pac, _frameCount);
  }
  //if the game has not yet started we draw the start screen
  if ( !_begin )
  {
    drawStart(io_ren,_texArr[3]);
  }
  //draw the scores
  drawScore(io_ren, 10, _points, "Score: ", io_sans);
  drawScore(io_ren, 200, _highScore, "High score: ", io_sans);
  //present the render
  SDL_RenderPresent(io_ren);
  //end the list of ghosts
  va_end ( ghostArgs );
}

//--------------------------------------------------------------------------------------------------------
/// @brief this function is used to draw the start screen
/// @param[io] io_ren is the SDL_Renderer used to draw our graphics
/// @param[io] io_tex is the start screen texture
//--------------------------------------------------------------------------------------------------------
void drawStart(SDL_Renderer *io_ren, SDL_Texture *io_tex)
{
  //create a block to contain the image
  SDL_Rect screenBlock;
  screenBlock.h = 775;
  screenBlock.w = 700;
  screenBlock.x=0;
  screenBlock.y=0;
  //create a block to copy this image into
  SDL_Rect img;
  img.x=0;
  img.y=0;
  img.w = 700;
  img.h = 775;
  //render the image
  SDL_RenderCopy(io_ren,io_tex,&img,&screenBlock);
}

//--------------------------------------------------------------------------------------------------------
/// @brief this function is used to draw the game over screen
/// @param[io] io_ren is the SDL_Renderer used to draw our graphics
/// @param[io] io_tex is the game over screen texture
//--------------------------------------------------------------------------------------------------------
void drawGameOver(SDL_Renderer *io_ren, SDL_Texture *io_tex)
{
  //create a block to contain the image
  SDL_Rect screenBlock;
  screenBlock.h = 775;
  screenBlock.w = 700;
  screenBlock.x=0;
  screenBlock.y=0;
  //create a block to copy this image into
  SDL_Rect img;
  img.x=0;
  img.y=0;
  img.w = 700;
  img.h = 775;
  //render the image
  SDL_RenderCopy(io_ren,io_tex,&img,&screenBlock);
}

//--------------------------------------------------------------------------------------------------------
/// @brief this function is used to call the drawGhost function for all given ghosts
/// @param[io] io_ren is the SDL_Renderer used to draw our graphics
/// @param[in] _frightenedClock holds the current time spent frightened
/// @param[in] _frameCount holds the current frame
/// @param[io] io_tex is the ghost texture
/// @param[in] _ghosts is a variable argument list that holds all ghosts to be drawn
//--------------------------------------------------------------------------------------------------------
void drawAllEnemies(SDL_Renderer *io_ren, struct timespec _frightenedClock, int _frameCount, SDL_Texture *io_tex, va_list _ghosts )
{
  //for all ghosts in the list we draw them
  for ( int x = 0; x < 4; x++ )
  {
    //get next ghost
    ghost temp = va_arg ( _ghosts, ghost );
    //choose the living or dead texture
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

//--------------------------------------------------------------------------------------------------------
/// @brief this function is used to draw a ghost
/// @param[io] io_ren is the SDL_Renderer used to draw our graphics
/// @param[io] io_tex is the ghost texture
/// @param[in] _enemy is a copy of the current ghost to be drawn
/// @param[in] _frightened tells us whether to draw the ghost as frightened or not
/// @param[in] _frightenedClock holds the time spent frightend used to alternate firghtened textures
/// @param[in] _ghostType tells the function which ghost to draw from the texture
/// @param[io] _frameCount holds the current frame used to animate the ghost wiggle
//--------------------------------------------------------------------------------------------------------
void drawGhost(SDL_Renderer *io_ren, SDL_Texture *io_tex, ghost _enemy, bool _frightened, struct timespec _frightenedClock, int _ghostType, int _frameCount)
{
  //obtain the time since being frightened started
  struct timespec start;
  double diff = timeDiff(&start,_frightenedClock);
  int desc = 0;
  //anim is a value that is used to wiggle the ghosts on alternating frames
  int anim = 5*(g_c_blockSize*2);
  if ( (_frameCount/15) == 0 )
  {
    //half the time this value is zero the other half
    anim = 0;
  }
  //create a block to contain the image
  SDL_Rect block;
  block.x=_enemy.x;
  block.y=_enemy.y;
  block.w=g_c_blockSize-1;
  block.h=g_c_blockSize-1;
  //create a block to copy this image into
  SDL_Rect ghosty;
  //obtain the correct sprite
  ghosty.w=g_c_blockSize*2;
  ghosty.h=g_c_blockSize*2;
  ghosty.x=_ghostType*(g_c_blockSize*2)+anim;
  ghosty.y=_enemy.dir*(g_c_blockSize*2);
  //if the ghost is frightened we need to choose a frightened sprite texture
  if ( _frightened )
  {
    //if the time is in the last 3 seconds of being frightened we alternate between blue and white
    desc = (int)(diff*10)%2;
    ghosty.x=5*(g_c_blockSize*2)+anim;
    if ( (diff >= 5) && (desc != 1) )
    {
      ghosty.y=0;
    }
    else
    {
      ghosty.y=g_c_blockSize*2;
    }
  }
  //render the image
  SDL_RenderCopy(io_ren, io_tex,&ghosty, &block);
}

//--------------------------------------------------------------------------------------------------------
/// @brief this function is used to draw pacman
/// @param[io] io_ren is the SDL_Renderer used to draw our graphics
/// @param[io] io_tex is the pacman texture
/// @param[in] _pac is a copy of pacman to be drawn
/// @param[in] _frameCount is holds the current frame used to animate the pacman mouth
//--------------------------------------------------------------------------------------------------------
void drawPacman(SDL_Renderer *io_ren, SDL_Texture *io_tex, pacman _pac, int _frameCount)
{
  //the dead and alive pacman play animations at different speeds
  int playSpeed;
  if(_pac.alive)
  {
    playSpeed = 5;
  }
  else
  {
    playSpeed = 3;
  }
  //create a block to contain the image
  SDL_Rect copyBlock;
  copyBlock.x=_pac.x;
  copyBlock.y=_pac.y;
  copyBlock.w=g_c_blockSize;
  copyBlock.h=g_c_blockSize;
  //create a block to copy this image into
  SDL_Rect pacBlock;
  pacBlock.w=g_c_blockSize*2;
  pacBlock.h=g_c_blockSize*2;
  //if the pacman isn't moving we set it to the top left sprite
  if ( _pac.dir == NONE )
  {
    pacBlock.x=0;
    pacBlock.y=0;
  }
  //if it is we use the fram count and direction to choose the correct sprite
  else
  {
    pacBlock.x=_pac.dir*(g_c_blockSize*2);
    pacBlock.y=(_frameCount/playSpeed)*(g_c_blockSize*2);
  }
  //render the image
  SDL_RenderCopy(io_ren, io_tex,&pacBlock, &copyBlock);
}

//--------------------------------------------------------------------------------------------------------
/// @brief this function is used to draw the maze
/// @param[io] io_ren is the SDL_Renderer used to draw our graphics
/// @param[io] io_pillTex is the pills texture
/// @param[io] io_wallTex is the walls texture
//--------------------------------------------------------------------------------------------------------
void drawMaze(SDL_Renderer *io_ren, SDL_Texture *io_pillTex, SDL_Texture *io_wallTex)
{
  //create a rect for each object to be drawn
  SDL_Rect block;
  SDL_Rect pill;
  SDL_Rect wall;
  pill.w = 25;
  pill.h = 25;
  wall.w = 25;
  wall.h = 25;
  wall.y = 0;
  wall.x = 0;
  //traverse the entire map array and select the correct texture to be drawn
  for(int i = 1; i < ROWS; ++i)
  {
    for(int j = 1; j < COLS; ++j)
    {
      block.x=(j-1)*g_c_blockSize;
      block.y=(i-1)*g_c_blockSize;
      block.w=g_c_blockSize;
      block.h=g_c_blockSize;
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
          wall.x = g_c_blockSize*checkMazeBlock(j,i);
          SDL_RenderCopy(io_ren, io_wallTex, &wall, &block);
          break;
        }
        case (RPILL):
        {
          pill.x = 0;
          pill.y = 0;
          SDL_RenderCopy(io_ren, io_pillTex, &pill, &block);
          break;
        }
        case (GATE):
        {
          block.h=g_c_blockSize/8;
          block.y+=((g_c_blockSize/2)-block.h/2);
          SDL_SetRenderDrawColor(io_ren, 255, 255, 255, 255);
          SDL_RenderFillRect(io_ren,&block);
          break;
        }
        case(POWERPILL):
        {
          pill.x = g_c_blockSize;
          pill.y = 0;
          SDL_RenderCopy(io_ren, io_pillTex, &pill, &block);
          break;
        }
      }
    }
  }
}

//--------------------------------------------------------------------------------------------------------
/// @brief this function is used to draw the scores
/// @param[io] io_ren is the SDL_Renderer used to draw our graphics
/// @param[in] _position holds the x co-ordinate to start drawing from
/// @param[in] _points holds the points to draw
/// @param[io] io_text is a pointer to a char array containing the words to be drawn
/// @param[io] io_sans is the font used to draw the score and words
//--------------------------------------------------------------------------------------------------------
void drawScore(SDL_Renderer* io_ren, int _position, int _points, const char *io_text, TTF_Font* io_sans)
{
  //create a char array to hold the score
  char texPoints[1000];
  //insert the score into the array
  sprintf(texPoints, "%d", _points);
  //get the number of digits in the score
  int len = strlen(texPoints);
  //get the number of chars in the given word
  int textLen = strlen(io_text)/2;
  //set the text colour
  SDL_Color white = {255, 255, 255, 255};
  //creat a surface from the word
  SDL_Surface* surfaceWord = TTF_RenderText_Solid(io_sans, io_text, white);
  //create a message from the surface
  SDL_Texture* word = SDL_CreateTextureFromSurface(io_ren, surfaceWord);
  //create a rect to hold the message
  SDL_Rect text;
  text.x = _position;
  text.y = HEIGHT-5;
  text.w = g_c_blockSize*textLen;
  text.h = g_c_blockSize*1.2;
  //render the word
  SDL_RenderCopy(io_ren, word, NULL, &text);
  //move the block to a new position dependent on the lenght of the word
  text.x += textLen*g_c_blockSize;
  text.w = g_c_blockSize*len / 2;
  //modify the message to hold the score
  surfaceWord = TTF_RenderText_Solid(io_sans, texPoints, white);
  word = SDL_CreateTextureFromSurface(io_ren, surfaceWord);
  //draw the score
  SDL_RenderCopy(io_ren, word, NULL, &text);
  SDL_FreeSurface(surfaceWord);
}


//--------------------------------------------------------------------------------------------------------
/// @brief this function traverses the map array and counts how many pills are left inside it
//--------------------------------------------------------------------------------------------------------
int pillCount()
{
  //traverse the whole map array and count the number of pills
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

//--------------------------------------------------------------------------------------------------------
/// @brief this function reverses the input direction
/// @param[in] _dir is used to find its opposite
//--------------------------------------------------------------------------------------------------------
int reverseDir(int _dir)
{
  //using the given direction, return the opposite direction
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

//--------------------------------------------------------------------------------------------------------
/// @brief this function takes calculates the difference between the current time and a given time
/// @param[io] io_currentTime is used to store the time this difference was requested
/// @param[in] _givenTime is the timer which the current time is compared against
//--------------------------------------------------------------------------------------------------------
double timeDiff(struct timespec *io_currentTime, struct timespec _givenTime)
{
  //assign the current time
  clock_gettime(CLOCK_REALTIME, io_currentTime);
  //calculate and return the difference
  return (((*io_currentTime).tv_sec - _givenTime.tv_sec) + ( (*io_currentTime).tv_nsec - _givenTime.tv_nsec )/1E9);
}

//--------------------------------------------------------------------------------------------------------
/// @brief This function is used to reset all variables to their default values when a level is complete or pacman dies
/// @param[o] o_keyPressed tells us whether the user pressed a key this frame
/// @param[o] o_begin will start the game when true
/// @param[o] o_frightened tells us whether pacman has eaten a super pill
/// @param[o] o_lifeDeduct tells the program to deduct a life
/// @param[o] o_frameCount holds the current frame number
/// @param[o] o_aiMode tells the ghosts whether to scatter or aim for their target
/// @param[o] o_shadow is a pointer to the red ghost
/// @param[o] o_speedy is a pointer to the pink ghost
/// @param[o] o_bashful is a pointer to the blue ghost
/// @param[o] o_pokey is a pointer to the orange ghost
/// @param[o] o_pac is a pointer to pacman
//--------------------------------------------------------------------------------------------------------
void reset( bool *o_keyPressed, bool *o_begin, bool *o_frightened, bool *o_lifeDeduct,
            int *o_frameCount, int *o_aiMode, ghost *o_shadow,
            ghost *o_speedy, ghost *o_bashful, ghost *o_pokey, pacman *o_pac)
{
  //reset all of the variables to their default values
  *o_shadow = createGhost(SHADOW,14,11,30,0,(void(*)(void))moveShadow);
  *o_speedy = createGhost(SPEEDY,13,13,0,0,(void(*)(void))moveSpeedy);
  *o_bashful = createGhost(BASHFUL,14,13,30,28,(void(*)(void))moveBashful);
  *o_pokey = createGhost(POKEY,13,14,0,28,(void(*)(void))movePokey);

  o_pac->x=13*g_c_blockSize;
  o_pac->y=23*g_c_blockSize+1;
  o_pac->dir=NONE;
  o_pac->last=NONE;
  o_pac->temp=NONE;
  o_pac->alive=true;

  *o_keyPressed = false;
  *o_begin = false;
  *o_frightened = false;
  *o_lifeDeduct = false;
  *o_frameCount = 0;
  *o_aiMode = 0;

}

//--------------------------------------------------------------------------------------------------------
/// @brief This function is used to restore the map to its original state
//--------------------------------------------------------------------------------------------------------
void resetMap()
{
  //create a new char array with the original values to be reassigned to the map
  char const originalMap[ROWS+1][COLS+1]={
      {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,},
      {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,},
      {1,1,2,2,2,2,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,2,2,2,2,1,1,},
      {1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,},
      {1,1,4,1,0,0,1,2,1,0,0,0,1,2,1,1,2,1,0,0,0,1,2,1,0,0,1,4,1,1,},
      {1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,},
      {1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,},
      {1,1,2,1,1,1,1,2,1,1,2,1,1,1,1,1,1,1,1,2,1,1,2,1,1,1,1,2,1,1,},
      {1,1,2,1,1,1,1,2,1,1,2,1,1,1,1,1,1,1,1,2,1,1,2,1,1,1,1,2,1,1,},
      {1,1,2,2,2,2,2,2,1,1,2,2,2,2,1,1,2,2,2,2,1,1,2,2,2,2,2,2,1,1,},
      {1,1,1,1,1,1,1,2,1,1,1,1,1,0,1,1,0,1,1,1,1,1,2,1,1,1,1,1,1,1,},
      {1,0,0,0,0,0,1,2,1,1,1,1,1,0,1,1,0,1,1,1,1,1,2,1,0,0,0,0,0,1,},
      {1,0,0,0,0,0,1,2,1,1,0,0,0,0,0,0,0,0,0,0,1,1,2,1,0,0,0,0,0,1,},
      {1,0,0,0,0,0,1,2,1,1,0,1,1,1,3,3,1,1,1,0,1,1,2,1,0,0,0,0,0,1,},
      {1,1,1,1,1,1,1,2,1,1,0,1,0,0,0,0,0,0,1,0,1,1,2,1,1,1,1,1,1,1,},
      {1,2,2,2,2,2,2,2,0,0,0,1,0,0,0,0,0,0,1,0,0,0,2,2,2,2,2,2,2,1,},
      {1,1,1,1,1,1,1,2,1,1,0,1,0,0,0,0,0,0,1,0,1,1,2,1,1,1,1,1,1,1,},
      {1,0,0,0,0,0,1,2,1,1,0,1,1,1,1,1,1,1,1,0,1,1,2,1,0,0,0,0,0,1,},
      {1,0,0,0,0,0,1,2,1,1,0,0,0,0,0,0,0,0,0,0,1,1,2,1,0,0,0,0,0,1,},
      {1,0,0,0,0,0,1,2,1,1,0,1,1,1,1,1,1,1,1,0,1,1,2,1,0,0,0,0,0,1,},
      {1,1,1,1,1,1,1,2,1,1,0,1,1,1,1,1,1,1,1,0,1,1,2,1,1,1,1,1,1,1,},
      {1,1,2,2,2,2,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,2,2,2,2,1,1,},
      {1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,},
      {1,1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1,1,},
      {1,1,4,2,2,1,1,2,2,2,2,2,2,2,0,0,2,2,2,2,2,2,2,1,1,2,2,4,1,1,},
      {1,1,1,1,2,1,1,2,1,1,2,1,1,1,1,1,1,1,1,2,1,1,2,1,1,2,1,1,1,1,},
      {1,1,1,1,2,1,1,2,1,1,2,1,1,1,1,1,1,1,1,2,1,1,2,1,1,2,1,1,1,1,},
      {1,1,2,2,2,2,2,2,1,1,2,2,2,2,1,1,2,2,2,2,1,1,2,2,2,2,2,2,1,1,},
      {1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,},
      {1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,},
      {1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,},
      {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,},
      {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,},
  };
  //copy the values into the map array
  memcpy(map, originalMap, sizeof(map));
}

//--------------------------------------------------------------------------------------------------------
/// @brief this function creates and returns an SDL_Texture from a given file path
/// @param[const] c_path is a pointer to a char array that holds the file path
/// @param[io] io_ren is the SDL_Renderer used to draw our graphics
/// @param[io] io_image is the image used to create our texture
//--------------------------------------------------------------------------------------------------------
SDL_Texture *createTex(const char *c_path, SDL_Renderer *io_ren, SDL_Surface *io_image)
{
  //load the image from the given path
  io_image=IMG_Load(c_path);
  if ( !io_image )
  {
    printf("IMG_Load: %s\n", IMG_GetError());
  }
  //create a new texture from the image
  SDL_Texture *tex;
  tex = SDL_CreateTextureFromSurface(io_ren, io_image);
  //free the image for later use
  SDL_FreeSurface(io_image);
  return tex;
}

//--------------------------------------------------------------------------------------------------------
/// @brief this function takes multiple file paths and creates an array of textures from them
/// @param[in] _pathCount holds the number of paths and hence the size of the array
/// @param[io] io_texArray is the array for the textures to be stored in
/// @param[io] io_ren is the SDL_Renderer used to draw our graphics
/// @param[io] io_image is the image used to create our texture
//--------------------------------------------------------------------------------------------------------
void createTexArray(int _pathCount, SDL_Texture* io_texArr[], SDL_Renderer *io_ren, SDL_Surface *io_image, ...)
{
  //get the list of paths
  va_list paths;
  va_start(paths, io_image);
  //for every path create a new texture
  for(int i = 0; i < _pathCount; ++i)
  {
    const char *path = va_arg(paths, const char *);
    io_texArr[i] = createTex(path, io_ren, io_image);
  }
  //end the list
  va_end(paths);
}

//--------------------------------------------------------------------------------------------------------
/// @brief this function creates a new ghost
/// @param[in] _type tells us which ghost type to create
/// @param[in] _startX is the starting x co-ordinate for the ghost
/// @param[in] _startY is the starting y co-ordinate for the ghost
/// @param[in] _scatX is the scatter x co-ordinate for the ghost
/// @param[in] _scatY is the scatter y co-ordinate for the ghost
/// @param[io] io_moveFunc is a function pointer to the move function of the ghost
//--------------------------------------------------------------------------------------------------------
ghost createGhost(int _type, int _startX, int _startY, int _scatX, int _scatY, void(*io_moveFunc)(void))
{
  //if the ghost starts in the home area set gate to true
  bool gate;
  if ( ((10 < _startX) && (_startX < 19)) && ((12 < _startY) && (_startY < 18)) )
  {
    gate = true;
  }
  else
  {
    gate = false;
  }
  //scale the values up by blocksize to get pixels
  _startX = _startX * g_c_blockSize;
  _startY = _startY * g_c_blockSize;
  _scatX = _scatX * g_c_blockSize;
  _scatY = _scatY * g_c_blockSize;
  //assign the given values
  ghost newGhost = {_type,_startX,_startY,UP,gate,true,0,0,_scatX,_scatY,false,true,false,(void(*)(void))(io_moveFunc)};
  return newGhost;

}

//--------------------------------------------------------------------------------------------------------
/// @brief this function saves the score to a text file so that it can be displayed as the high score
/// @param[in] _points are the points to be written to the text file
//--------------------------------------------------------------------------------------------------------
void saveScore(int _points)
{
  //open the file
  FILE *f = fopen("highScore.txt", "w");
  if (f == NULL)
  {
      printf("Error opening file!\n");
      exit(EXIT_FAILURE);
  }
  //print the values to the file
  fprintf(f,"%d",_points);
  //close the file
  fclose(f);
}

//--------------------------------------------------------------------------------------------------------
/// @brief this function retrieves the high score from the text file
//--------------------------------------------------------------------------------------------------------
int getScore()
{
  //open the file
  FILE *f = fopen("highScore.txt", "r");
  int points = 0;
  if (f == NULL)
  {
      printf("Error opening file!\n");
      exit(EXIT_FAILURE);
  }
  //read the values from the file
  fscanf(f,"%d",&points);
  //close the file
  fclose(f);
  return points;
}
