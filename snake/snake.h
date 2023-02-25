#ifndef _SNAKE_H
#define _SNAKE_H

// prevent SDL from defining main
// https://stackoverflow.com/a/48723846/11389823
#define SDL_MAIN_HANDLED
#include "SDL.h"

#include <list>
#include <chrono>

#define WINDOW_PADDING 8
#define GRID_WIDTH 50
#define GRID_HEIGHT 50
#define CELL_PX 6
#define MAZE_SIZE 3 // width of maze passages

#define DELAY 150

const unsigned MAZE_WIDTH = (GRID_WIDTH) / (MAZE_SIZE * 2);
const unsigned MAZE_HEIGHT = (GRID_HEIGHT) / (MAZE_SIZE * 2);
const unsigned MAZE_OFFSET_X = (GRID_WIDTH - (MAZE_WIDTH * 2 - 1) * MAZE_SIZE) / 2;
const unsigned MAZE_OFFSET_Y = (GRID_HEIGHT - (MAZE_HEIGHT * 2 - 1) * MAZE_SIZE) / 2;

enum Direction { UP, DOWN, LEFT, RIGHT };

Direction oppositeDir(Direction d);

// represents a point in 2D space
struct Point
{
  int x, y;

  Point() : x(0), y(0) {}
  Point(int x, int y) : x(x), y(y) {}
  Point(Direction dir): x(0), y(0) {
    switch (dir) {
    case UP:
      x = 0;
      y = 1;
      break;
    case DOWN:
      x = 0;
      y = -1;
      break;
    }
  }

  Point(const Point& p) : x(p.x), y(p.y) {}
  inline Point& operator=(Point& p)
  {
    x = p.x; y = p.y;
  }
  inline void set(int x, int y) { this->x = x; this->y = y; }

  inline Point operator-() const { return Point(-x, -y); }

  inline Point operator+(Point& p) const { return Point(x + p.x, y + p.y); }
  inline Point operator-(Point& p) const { return Point(x - p.x, y - p.y); }

  inline Point operator*(int k) const { return Point(k * x, k * y); }
  inline friend Point operator*(int k, Point& p) { return Point(k * p.x, k * p.y); }

  inline Point& operator+=(Point& p) { x += p.x; y += p.y; return *this; }
  inline Point& operator-=(Point& p) { x -= p.x; y -= p.y; return *this; }
  inline Point& operator*=(int k) { x *= k; y *= k; return *this; }

  inline bool operator==(Point& p) const { return x == p.x && y == p.y; }
  inline bool operator!=(Point& p) const { return !(operator==(p)); }
};

class Game
{
protected:
  SDL_Window* window;
  SDL_Renderer* renderer;

  // represents the segments of the snake
  struct Segment
  {
    Point pos;
    Direction dir; // to next segment
  };
  std::list<Segment> snake;

  // the direction of the snake's travel
  Direction dir;
  std::list<Direction> pendingDirs;

  Point food;

  bool walls[GRID_WIDTH][GRID_HEIGHT];

  bool running, lose, paused;

  bool firstTick;
  std::chrono::time_point<std::chrono::steady_clock> lastTick;

  void initGame();

  bool checkCollisions(int x, int y);
  inline bool checkCollisions(Point p) { return checkCollisions(p.x, p.y); }

  void generateFood();

private:
  void clearCell(Point p);
  void carveEdge(Point p1, Point p2);
protected:
  void generateMaze();

public:
  Game() :
    running(true),
    window(nullptr), renderer(nullptr)
  {
  }

  void init();
  void handleEvents();
  void update();
  void render();
  void clean();

  inline bool isRunning() { return running; }
  inline unsigned getDelayMs() { return DELAY; }
};

#endif // _SNAKE_H
