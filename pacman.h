#ifndef PACMAN_H
#define PACMAN_H

typedef struct
{
  int pills;
  int ghosts;
  int lastLevel;
  int total;
}score;

typedef struct
{
  int x;
  int y;
  int dir;
  int temp;
  int last;
  bool alive;
}pacman;

typedef struct
{
  int type;
  int x;
  int y;
  int dir;
  bool gate;
  bool turn;
  int tempX;
  int tempY;
  int scatX;
  int scatY;
  bool frightened;
  bool alive;
  bool loop;
  void(*move)(void);
}ghost;

#endif // PACMAN_H
