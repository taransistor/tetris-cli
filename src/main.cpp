#include <array>
#include <chrono>
#include <cstdlib>
#include <ncurses.h>
#include <string>

#define HEIGHT 20
#define WIDTH 10

typedef struct
{
  int color;
  int nPiece;
  int rotation;
  int x;
  int y;
  bool piece[16];
} Piece;

void renderState(WINDOW *win, std::array<std::array<int, WIDTH>, HEIGHT> &state)
{
  box(win, 0, 0);
  char c;
  for (int y = 0; y < HEIGHT; ++y)
  {
    for (int x = 0; x < WIDTH; ++x)
    {
      c = state[y][x] == 0 ? '.' : ' ';
      wattron(win, COLOR_PAIR(state[y][x] + 1));
      mvwaddch(win, y + 1, x * 2 + 1, c);
      mvwaddch(win, y + 1, x * 2 + 2, c);
      wattroff(win, COLOR_PAIR(state[y][x] + 1));
    }
  }
  wrefresh(win);
}

void renderPiece(WINDOW *win, Piece &piece)
{
  for (int j = 0; j < 4; ++j)
  {
    for (int i = 0; i < 4; ++i)
    {
      if (piece.piece[4 * j + i] == 1)
      {
        wattron(win, COLOR_PAIR(piece.color));
        mvwaddch(win, piece.y + j + 1, (piece.x + i) * 2 + 1, ' ');
        mvwaddch(win, piece.y + j + 1, (piece.x + i) * 2 + 2, ' ');
        wattroff(win, COLOR_PAIR(piece.color));
      }
    }
  }
  wrefresh(win);
}

// clang-format off
bool pieces[7][16] = {
  {1,1,0,0,//O
   1,1,0,0,
   0,0,0,0,
   0,0,0,0},
  {1,1,1,1,//I
   0,0,0,0,
   0,0,0,0,
   0,0,0,0},
  {1,1,1,0,//L
   1,0,0,0,
   0,0,0,0,
   0,0,0,0},
  {1,0,0,0,//J
   1,1,1,0,
   0,0,0,0,
   0,0,0,0},
  {1,1,1,0,//T
   0,1,0,0,
   0,0,0,0,
   0,0,0,0},
  {0,1,1,0,//S
   1,1,0,0,
   0,0,0,0,
   0,0,0,0},
  {1,1,0,0,//Z
   0,1,1,0,
   0,0,0,0,
   0,0,0,0},
};
// clang-format on

bool isValid(std::array<std::array<int, WIDTH>, HEIGHT> &state, int y, int x, bool piece[16])
{
  for (int j = 0; j < 4; ++j)
  {
    for (int i = 0; i < 4; ++i)
    {
      if (piece[4 * j + i] == 1 && !(x + i < WIDTH && y + j < HEIGHT && x + i >= 0 && y + j >= 0 &&
                                     state[y + j][x + i] == 0))
      {
        return false;
      }
    }
  }
  return true;
}

void renderHardDropPreview(WINDOW *win, std::array<std::array<int, WIDTH>, HEIGHT> &state,
                           Piece &piece)
{
  int y = 0;
  while (isValid(state, y, piece.x, piece.piece))
  {
    ++y;
  }
  --y;
  for (int j = 0; j < 4; ++j)
  {
    for (int i = 0; i < 4; ++i)
    {
      if (piece.piece[4 * j + i] == 1)
      {
        wattron(win, COLOR_PAIR(9));
        mvwaddch(win, y + j + 1, (piece.x + i) * 2 + 1, '#');
        mvwaddch(win, y + j + 1, (piece.x + i) * 2 + 2, '#');
        wattroff(win, COLOR_PAIR(9));
      }
    }
  }
  wrefresh(win);
}

void putPiece(std::array<std::array<int, WIDTH>, HEIGHT> &state, Piece &piece)
{
  for (int j = 0; j < 4; ++j)
  {
    for (int i = 0; i < 4; ++i)
    {
      if (piece.piece[4 * j + i] == 1)
      {
        state[piece.y + j][piece.x + i] = piece.color - 1;
      }
    }
  }
}

void renewActivePiece(Piece &piece)
{
  piece.color = (rand() % 6) + 3;
  piece.nPiece = rand() % 7;
  int start = -1;
  for (int i = 0; i < 16; ++i)
  {
    if (start == -1 && pieces[piece.nPiece][i] == 1)
    {
      start = i;
    }
    piece.piece[i] = pieces[piece.nPiece][i];
  }
  piece.x = 3 - start % 4;
  piece.y = 0 - start / 4;
  piece.rotation = 0;
}

void delFullRows(WINDOW *win, std::array<std::array<int, WIDTH>, HEIGHT> &state, int y)
{
  bool isDeleted = false;
  for (int i = y; i < HEIGHT; ++i)
  {
    bool isFull = true;
    for (int j = 0; j < WIDTH; ++j)
    {
      if (state[i][j] == 0)
      {
        isFull = false;
        break;
      }
    }
    if (isFull)
    {
      isDeleted = true;
      for (int row = i; row > 0; --row)
      {
        state[row] = state[row - 1];
        wattron(win, COLOR_PAIR(2));
        mvwprintw(win, i + 1, 1, "                    ");
        wattroff(win, COLOR_PAIR(2));
      }
      state[0] = std::array<int, WIDTH>{};
    }
  }
  if (isDeleted)
  {
    wrefresh(win);
    napms(150);
  }
}

bool movePiece(Piece &piece, char c, std::array<std::array<int, WIDTH>, HEIGHT> &state)
{
  int x = piece.x;
  int y = piece.y;

  switch (c)
  {
    case ('a'):
      --x;
      break;
    case ('s'):
      ++y;
      break;
    case ('d'):
      ++x;
      break;
    case (' '):
      while (isValid(state, y, x, piece.piece))
      {
        ++y;
      }
      piece.y = --y;
      return false;
      break;
  }
  if (isValid(state, y, x, piece.piece))
  {
    piece.x = x;
    piece.y = y;
    return true;
  }
  else
  {
    return false;
  }
}

void moveToTopLeft(bool array[16])
{
  int minRow = 4;
  int minCol = 4;
  for (int i = 0; i < 16; ++i)
  {
    if (array[i])
    {
      int row = i >> 2; // i / 4
      int col = i & 3;  // i % 4
      minRow = std::min(minRow, row);
      minCol = std::min(minCol, col);
    }
  }
  // Shift rows up
  if (minRow)
  {
    for (int i = 0; i < (4 - minRow) * 4; ++i)
      array[i] = array[i + minRow * 4];
    for (int i = (4 - minRow) * 4; i < 16; ++i)
      array[i] = false;
  }
  // Shift columns left
  if (minCol)
  {
    for (int row = 0; row < 4; ++row)
    {
      for (int col = 0; col < 4 - minCol; ++col)
        array[row * 4 + col] = array[row * 4 + col + minCol];

      for (int col = 4 - minCol; col < 4; ++col)
        array[row * 4 + col] = false;
    }
  }
}

void rotatePiece(std::array<std::array<int, WIDTH>, HEIGHT> &state, Piece &piece)
{
  piece.rotation += 90;
  piece.rotation %= 360;
  bool rotatedPiece[16];
  switch (piece.rotation)
  {
    case (0):
      for (int i = 0; i < 16; ++i)
      {
        rotatedPiece[i] = pieces[piece.nPiece][i];
      }
      break;
    case (90):
      for (int i = 0; i < 16; ++i)
      {
        int y = i / 4;
        int x = i % 4;
        rotatedPiece[i] = pieces[piece.nPiece][(3 - x) * 4 + y];
      }
      break;
    case (180):
      for (int i = 0; i < 16; ++i)
      {
        int y = i / 4;
        int x = i % 4;
        rotatedPiece[i] = pieces[piece.nPiece][(3 - y) * 4 + (3 - x)];
      }
      break;
    case (270):
      for (int i = 0; i < 16; ++i)
      {
        int y = i / 4;
        int x = i % 4;
        rotatedPiece[i] = pieces[piece.nPiece][x * 4 + (3 - y)];
      }
      break;
  }
  // move to left;
  moveToTopLeft(rotatedPiece);
  if (isValid(state, piece.y, piece.x, rotatedPiece))
  {
    for (int i = 0; i < 16; ++i)
    {
      piece.piece[i] = rotatedPiece[i];
    }
  }
}

bool isValidInp(char c)
{
  std::string validInp = "asd ";
  for (int i = 0; i < validInp.size(); ++i)
  {
    if (validInp[i] == c)
    {
      return true;
    }
  }
  return false;
}

int main()
{
  initscr();
  cbreak();
  noecho();
  curs_set(0);
  nodelay(stdscr, TRUE);
  if (has_colors())
  {
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_WHITE, COLOR_WHITE);
    init_pair(3, COLOR_WHITE, COLOR_RED);
    init_pair(4, COLOR_WHITE, COLOR_GREEN);
    init_pair(5, COLOR_WHITE, COLOR_YELLOW);
    init_pair(6, COLOR_WHITE, COLOR_BLUE);
    init_pair(7, COLOR_WHITE, COLOR_MAGENTA);
    init_pair(8, COLOR_WHITE, COLOR_CYAN);
    init_pair(9, COLOR_BLACK, COLOR_WHITE);
  }
  bkgd(COLOR_PAIR(2));

  int winPosY;
  int winPosX;
  getmaxyx(stdscr, winPosY, winPosX);
  winPosY = (winPosY - (HEIGHT + 2)) / 2;
  winPosX = (winPosX - (WIDTH * 2 + 2)) / 2;
  auto win = newwin(HEIGHT + 2, WIDTH * 2 + 2, winPosY, winPosX);

  srand(time(NULL));

  std::array<std::array<int, WIDTH>, HEIGHT> state{};

  Piece piece;
  renewActivePiece(piece);

  char c;

  bool gameOver = false;

  auto last = std::chrono::steady_clock::now();
  auto interval = std::chrono::milliseconds(350);

  while (!gameOver)
  {
    // get inp
    c = getch();

    // change state/piece
    if (isValidInp(c) && !movePiece(piece, c, state) && (c == 's' || c == ' '))
    {
      putPiece(state, piece);
      delFullRows(win, state, piece.y);
      renewActivePiece(piece);
      if (!isValid(state, piece.y, piece.x, piece.piece))
      {
        // gameOver()
        gameOver = true;
      }
    }

    // rotation
    if (c == 'z')
    {
      rotatePiece(state, piece);
    }

    if (std::chrono::steady_clock::now() - last >= interval)
    {
      // down
      if (!movePiece(piece, 's', state))
      {
        putPiece(state, piece);
        delFullRows(win, state, piece.y);
        renewActivePiece(piece);
        if (!isValid(state, piece.y, piece.x, piece.piece))
        {
          // gameOver()
          gameOver = true;
        }
      }

      last += interval;
    }

    // render shi
    renderState(win, state);
    renderHardDropPreview(win, state, piece);
    renderPiece(win, piece);

    napms(10);
  }

  endwin();
  return 0;
}
