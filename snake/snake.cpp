#include "snake.h"
#include "util.h"
#include <cstdlib>
#include <algorithm>
#include <iostream>

Direction oppositeDir(Direction d)
{
  switch (d)
  {
  case UP:
    return DOWN;
    break;
  case DOWN:
    return UP;
    break;
  case LEFT:
    return RIGHT;
    break;
  case RIGHT:
    return LEFT;
    break;
  default:
    return UP; // shouldn't happen but return something anyways
    break;
  }
}

void Game::initGame()
{
  lose = false;
  paused = false;
  dir = RIGHT;
  firstTick = true;

  pendingDirs.clear();
  snake.clear();

  std::srand(unsigned(std::time(0)));

  generateMaze();
  generateFood();

  Point head;
  do {
    head.set(std::rand() % GRID_WIDTH, std::rand() % GRID_HEIGHT);
  } while (checkCollisions(head));

  Direction maxDir = UP;
  int maxDist;
  int i;
  for (i = head.y - 1; !checkCollisions(head.x, i); --i)
    ;
  maxDist = head.y - i;

  for (i = head.y + 1; !checkCollisions(head.x, i); ++i)
    ;
  int dist = i - head.y;
  if (dist > maxDist)
  {
    maxDir = DOWN;
    maxDist = dist;
  }

  for (i = head.x - 1; !checkCollisions(i, head.y); --i)
    ;
  dist = head.x - i;
  if (dist > maxDist)
  {
    maxDir = LEFT;
    maxDist = dist;
  }

  for (i = head.x + 1; !checkCollisions(i, head.y); ++i)
    ;
  dist = i - head.x;
  if (dist > maxDist)
  {
    maxDir = RIGHT;
    maxDist = dist;
  }

  dir = maxDir;

  for (int i = 0; i < 4; ++i)
    snake.push_back(Segment{ head, dir });
}

bool Game::checkCollisions(int x, int y)
{
  if (x < 0 || x >= GRID_WIDTH) return true;
  if (y < 0 || y >= GRID_HEIGHT) return true;

  if (walls[x][y])
    return true;

  for (auto it = snake.begin(); it != snake.end(); ++it)
    if (it->pos.x == x && it->pos.y == y) return true;

  return false;
}

void Game::generateFood()
{
  do
  {
    food.set(std::rand() % GRID_WIDTH, std::rand() % GRID_HEIGHT);
  } while (checkCollisions(food));
}


inline unsigned point2Id(unsigned x, unsigned y)
{
  return x + MAZE_WIDTH * y;
}
inline unsigned point2Id(Point p) { return point2Id(p.x, p.y); }

inline Point id2Point(unsigned id)
{
  return Point(id % MAZE_WIDTH, id / MAZE_WIDTH);
}

inline int random(int i) { return std::rand() % i; }

void Game::clearCell(Point p)
{
  p *= MAZE_SIZE;

  for (int i = 0; i < MAZE_SIZE; ++i)
    for (int j = 0; j < MAZE_SIZE; ++j)
      walls[p.x + i + MAZE_OFFSET_X][p.y + j + MAZE_OFFSET_Y] = false;
}

void Game::carveEdge(Point p1, Point p2)
{
  Point diff = p2 - p1;
  clearCell(p1 * 2);
  clearCell(p1 * 2 + diff);
  clearCell(p2 * 2);
}

// randomly carve passages if points are not connected
void Game::generateMaze()
{
  UnionFind maze(MAZE_WIDTH * MAZE_HEIGHT);

  struct Edge { unsigned p1, p2; };
  std::vector<Edge> edges((MAZE_WIDTH - 1) * MAZE_HEIGHT + (MAZE_HEIGHT - 1) * MAZE_WIDTH);

  auto it = edges.begin();
  for (int row = 0; row < MAZE_HEIGHT; ++row)
    for (int col = 0; col < MAZE_WIDTH - 1; ++col)
    {
      it->p1 = point2Id(col, row);
      it->p2 = point2Id(col + 1, row);
      ++it;
    }
  for (int col = 0; col < MAZE_WIDTH; ++col)
    for (int row = 0; row < MAZE_HEIGHT - 1; ++row)
    {
      it->p1 = point2Id(col, row);
      it->p2 = point2Id(col, row + 1);
      ++it;
    }

  std::random_shuffle(edges.begin(), edges.end(), random);

  for (int i = 0; i < GRID_WIDTH; ++i)
    for (int j = 0; j < GRID_HEIGHT; ++j)
      walls[i][j] = true;

  for (auto it = edges.begin(); it != edges.end(); ++it)
  {
    if (maze.setID(it->p1) != maze.setID(it->p2))
    {
      maze.valUnion(it->p1, it->p2);

      Point p1 = id2Point(it->p1), p2 = id2Point(it->p2);
      carveEdge(p1, p2);

      if (maze.numSets() == 1) break;
    }
  }
}

SDL_HitTestResult hitTest(SDL_Window* win, const SDL_Point* area, void* data) { return SDL_HITTEST_DRAGGABLE; }

void Game::init()
{
  SDL_Init(SDL_INIT_EVERYTHING);
  window = SDL_CreateWindow(
    "Snake",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    GRID_WIDTH * CELL_PX + WINDOW_PADDING * 2,
    GRID_HEIGHT * CELL_PX + WINDOW_PADDING * 2,
    0);
  renderer = SDL_CreateRenderer(window, -1, 0);

  SDL_SetWindowHitTest(window, hitTest, 0);

  initGame();
}

void Game::handleEvents()
{
  SDL_Event event;

  while (SDL_PollEvent(&event))
    switch (event.type)
    {
    case SDL_QUIT:
      running = false;
      break;
    case SDL_KEYDOWN:
      switch (event.key.keysym.sym)
      {
      case SDLK_w: case SDLK_UP:
        pendingDirs.push_back(UP);
        break;
      case SDLK_s: case SDLK_DOWN:
        pendingDirs.push_back(DOWN);
        break;
      case SDLK_a: case SDLK_LEFT:
        pendingDirs.push_back(LEFT);
        break;
      case SDLK_d: case SDLK_RIGHT:
        pendingDirs.push_back(RIGHT);
        break;
      case SDLK_ESCAPE:
        running = false;
        break;
      case SDLK_p:
        paused = !paused;
        firstTick = true;
        break;
      case SDLK_r:
        if (lose)
          initGame();
        break;
      default:
        break;
      }
    }
}

inline long long msElapsed(std::chrono::time_point<std::chrono::steady_clock> time)
{
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - time).count();
}

void Game::update()
{
  if (lose || paused) return;

  if (firstTick)
    firstTick = false;
  else if (msElapsed(lastTick) < DELAY)
    return;

  while (!pendingDirs.empty() && (pendingDirs.front() == dir || oppositeDir(pendingDirs.front()) == dir))
    pendingDirs.pop_front();
  if (!pendingDirs.empty())
  {
    dir = pendingDirs.front();
    pendingDirs.pop_front();
  }
  if (pendingDirs.size() > 1) pendingDirs.resize(1);

  Segment next = snake.front();
  next.dir = dir;
  switch (dir)
  {
  case UP:
    --next.pos.y;
    break;
  case DOWN:
    ++next.pos.y;
    break;
  case LEFT:
    --next.pos.x;
    break;
  case RIGHT:
    ++next.pos.x;
    break;
  default:
    break;
  }

  Segment tail = snake.back();
  if (next.pos != food)
    snake.pop_back();

  if (checkCollisions(next.pos))
  {
    lose = true;
    snake.push_back(tail);
    return;
  }

  snake.push_front(next);

  if (next.pos == food)
    generateFood();

  lastTick = std::chrono::steady_clock::now();
}

void Game::render()
{
  SDL_SetRenderDrawColor(renderer, 49, 48, 50, 255);
  SDL_RenderClear(renderer);

  SDL_Rect rect;

  rect.w = CELL_PX;
  rect.h = CELL_PX;

  if (lose)
    SDL_SetRenderDrawColor(renderer, 174, 187, 126, 255);
  else
    SDL_SetRenderDrawColor(renderer, 179, 229, 125, 255);
  for (auto it = snake.begin(); it != snake.end(); ++it)
  {
    rect.x = (it->pos.x) * CELL_PX + WINDOW_PADDING;
    rect.y = (it->pos.y) * CELL_PX + WINDOW_PADDING;
    SDL_RenderFillRect(renderer, &rect);
  }

  rect.x = food.x * CELL_PX + WINDOW_PADDING;
  rect.y = food.y * CELL_PX + WINDOW_PADDING;
  SDL_SetRenderDrawColor(renderer, 248, 88, 62, 255);
  SDL_RenderFillRect(renderer, &rect);

  SDL_SetRenderDrawColor(renderer, 102, 103, 119, 255);

  for (int i = 0; i < GRID_WIDTH; ++i)
    for (int j = 0; j < GRID_HEIGHT; ++j)
      if (walls[i][j])
      {
        rect.x = i * CELL_PX + WINDOW_PADDING;
        rect.y = j * CELL_PX + WINDOW_PADDING;
        SDL_RenderFillRect(renderer, &rect);
      }

  rect.x = 0;
  rect.y = 0;
  rect.w = WINDOW_PADDING;
  rect.h = GRID_HEIGHT * CELL_PX + 2 * WINDOW_PADDING;
  SDL_RenderFillRect(renderer, &rect);
  rect.x = GRID_WIDTH * CELL_PX + WINDOW_PADDING;
  SDL_RenderFillRect(renderer, &rect);

  rect.x = WINDOW_PADDING;
  rect.w = GRID_WIDTH * CELL_PX;
  rect.h = WINDOW_PADDING;
  SDL_RenderFillRect(renderer, &rect);
  rect.y = GRID_HEIGHT * CELL_PX + WINDOW_PADDING;
  SDL_RenderFillRect(renderer, &rect);

  SDL_RenderPresent(renderer);
}

void Game::clean()
{
  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  SDL_Quit();
}
