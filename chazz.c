#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Removed MEMORY_SIZE define to allow dynamic allocation
#define BOOL_LOG 0
#define BOOL_BOARD 0
#define MAX_MEMORY_SIZE 30000  // Maximum allowed memory size

typedef struct {
  int rows;
  int cols;
  char **squares;
  char to_turn;  // 'w' or 'b'
} ChessBoard;

// Updated function signature to include memory_size
void interpretBoard(const ChessBoard* board, int* memory, int memory_size, int* pointer, int* buffer);
void printBoard(const ChessBoard* board);
void generateHTMLBoard(const ChessBoard* board, const char* filename);
const char* getUnicodePiece(char piece);
int parseFEN(const char* fen, ChessBoard* board);
char* readFile(const char* filename);

int main(int argc, char* argv[])
{
  int pointer = 0;
  unsigned int export_html = 0;
  int buffer = 0;
  int memory_size = 0;
  int *memory = NULL;

  if (argc < 2)
  {
    fprintf(stderr, "Usage: %s <filename> [true|false]\n", argv[0]);
    return EXIT_FAILURE;
  }

  char* fen = readFile(argv[1]);
  if (!fen)
  {
    fprintf(stderr, "Failed to read FEN file\n");
    return EXIT_FAILURE;
  }

  if (argc >= 3 && argv[2] != NULL)
  {
    if (!strcmp(argv[2], "true"))
      export_html = 1;

    else if (!strcmp(argv[2], "false"))
      export_html = 0;
  }

  ChessBoard board;
  if (parseFEN(fen, &board) != 0)
  {
    fprintf(stderr, "Error: Invalid FEN format\n");
    free(fen);
    return EXIT_FAILURE;
  }

  // Calculate memory_size based on board dimensions
  memory_size = board.rows * board.cols;

  if (memory_size > MAX_MEMORY_SIZE)
  {
    fprintf(stderr, "Error: Required MEMORY_SIZE (%d) exceeds the maximum allowed (%d).\n", memory_size, MAX_MEMORY_SIZE);
    free(fen);

    for (int i = 0; i < board.rows; i++)
      free(board.squares[i]);

    free(board.squares);
    return EXIT_FAILURE;
  }

  // Allocate memory dynamically and initialize to zero
  memory = (int*)calloc(memory_size, sizeof(int));
  if (!memory)
  {
    perror("Memory allocation failed");
    free(fen);

    for (int i = 0; i < board.rows; i++)
      free(board.squares[i]);

    free(board.squares);
    return EXIT_FAILURE;
  }

  if (BOOL_BOARD)
    printBoard(&board);

  if (export_html)
  {
    generateHTMLBoard(&board, "output_chess_board.html");

    #ifdef _WIN32
      system("start output_chess_board.html");
    #elif __APPLE__
      system("open output_chess_board.html");
    #elif __linux__
      system("xdg-open output_chess_board.html");
    #else
      printf("Automatic opening of the HTML file is not supported on this platform.\n");
    #endif
  }

  interpretBoard(&board, memory, memory_size, &pointer, &buffer);

  printf("\n");
  free(fen);

  for (int i = 0; i < board.rows; i++)
    free(board.squares[i]);
  free(board.squares);

  free(memory);

  return 0;
}

/// Read a .FEN file into a string
char* readFile(const char* filename)
{
  FILE *file = fopen(filename, "r");
  if (!file)
  {
    perror("Could not open file");
    return NULL;
  }

  size_t buffer_size = 256;
  char *fen = (char *)malloc(buffer_size * sizeof(char));
  if (!fen)
  {
    perror("Memory allocation failed");
    fclose(file);
    return NULL;
  }

  size_t len = 0;
  int c;
  while ((c = fgetc(file)) != EOF && c != '\n')
  {
    if (len >= buffer_size - 1)
    {
      buffer_size *= 2;
      char *temp = realloc(fen, buffer_size);
      if (!temp)
      {
        perror("Memory reallocation failed");
        free(fen);
        fclose(file);
        return NULL;
      }
      fen = temp;
    }
    fen[len++] = (char)c;
  }
  fen[len] = '\0';

  fclose(file);
  return fen;
}

/// Get unicode character for piece (for HTML)
const char* getUnicodePiece(char piece)
{
  switch (piece)
  {
    case 'P': return "&#9817;"; // White Pawn
    case 'N': return "&#9816;"; // White Knight
    case 'B': return "&#9815;"; // White Bishop
    case 'R': return "&#9814;"; // White Rook
    case 'Q': return "&#9813;"; // White Queen
    case 'K': return "&#9812;"; // White King
    case 'p': return "&#9823;"; // Black Pawn
    case 'n': return "&#9822;"; // Black Knight
    case 'b': return "&#9821;"; // Black Bishop
    case 'r': return "&#9820;"; // Black Rook
    case 'q': return "&#9819;"; // Black Queen
    case 'k': return "&#9818;"; // Black King
    default:  return "";
  }
}

/// Generate HTML visualization of the board
void generateHTMLBoard(const ChessBoard* board, const char* filename)
{
  FILE *file = fopen(filename, "w");
  if (!file)
  {
    perror("Could not open HTML file for writing");
    return;
  }

  fprintf(file, "<!DOCTYPE html>\n<html>\n<head>\n");
  fprintf(file, "<meta charset=\"UTF-8\">\n");
  fprintf(file, "<title>Chess Board</title>\n");
  fprintf(file, "<style>\n");
  fprintf(file, "table { border-collapse: collapse; }\n");
  fprintf(file, "td { width: 60px; height: 60px; text-align: center; vertical-align: middle; font-size: 48px; }\n");
  // Updated colors to match chess.com
  fprintf(file, ".white { background-color: #eeeed2; }\n");
  fprintf(file, ".black { background-color: #769656; }\n");
  fprintf(file, "</style>\n");
  fprintf(file, "</head>\n<body>\n");
  fprintf(file, "<table>\n");

  for (int row = 0; row < board->rows; row++)
  {
    fprintf(file, "<tr>\n");
    for (int col = 0; col < board->cols; col++)
    {
      const char* color = ((row + col) % 2 == 0) ? "white" : "black";
      fprintf(file, "<td class=\"%s\">", color);
      char piece = board->squares[row][col];
      const char *unicode_piece = getUnicodePiece(piece);
      fprintf(file, "%s", unicode_piece);
      fprintf(file, "</td>\n");
    }
    fprintf(file, "</tr>\n");
  }

  fprintf(file, "</table>\n");
  fprintf(file, "</body>\n</html>\n");

  fclose(file);
}

/// Print board to console
void printBoard(const ChessBoard *board)
{
  printf("   ");
  for (int c = 0; c < board->cols; c++)
    printf("%2d", c + 1);
  printf("\n");

  printf("  +");
  for (int c = 0; c < board->cols; c++)
    printf("--");
  printf("+\n");

  for (int row = 0; row < board->rows; row++)
  {
    printf("%2d|", board->rows - row);
    for (int col = 0; col < board->cols; col++)
    {
      printf("%c ", board->squares[row][col]);
    }
    printf("|\n");
  }

  printf("  +");
  for (int c = 0; c < board->cols; c++)
    printf("--");
  printf("+\n");
}

/// The Interpreter
void interpretBoard(const ChessBoard* board, int* memory, int memory_size, int* pointer, int* buffer)
{
    typedef struct {
        int row;
        int col;
    } LoopPosition;

    LoopPosition loop_stack[256];
    int stack_ptr = -1;

    if (BOOL_LOG)
        printf("\nStarting interpretation...\n");

    // Determine which color to look for first
    int looking_for_white = (board->to_turn == 'w');

    // Find the first piece of the correct color in top-left to bottom-right order
    int start_found = 0;
    int start_row = 0;
    int start_col = 0;

    for (int row = 0; row < board->rows && !start_found; row++)
    {
        for (int col = 0; col < board->cols && !start_found; col++)
        {
            char piece = board->squares[row][col];
            if (piece == ' ')
                continue;
            int is_white_piece = (piece >= 'A' && piece <= 'Z');
            if ((looking_for_white && is_white_piece) ||
                (!looking_for_white && !is_white_piece))
            {
                start_found = 1;
                start_row = row;
                start_col = col;
            }
        }
    }

    if (!start_found)
    {
        // If no piece of correct color found, start at top-left anyway
        start_row = 0;
        start_col = 0;
    }

    int started = 0;
    for (int row = 0; row < board->rows; row++)
    {
        for (int col = 0; col < board->cols; col++)
        {
            if (!started)
            {
                if (row == start_row && col == start_col)
                    started = 1;
                else
                    continue;
            }

            char piece = board->squares[row][col];

            if (piece == ' ')
                continue;

            if (BOOL_LOG)
                printf("Interpreting piece at [%d][%d]: %c\n", row, col, piece);

            switch (piece)
            {
                case 'P': // horizontally relative increment
                    memory[*pointer] += (1 << col);
                    break;

                case 'p': // horizontally relative decrement
                    memory[*pointer] -= (1 << col);
                    break;

                case 'N': // pointer increment
                    *pointer = (*pointer + 1) % memory_size;
                    break;

                case 'n': // pointer decrement
                    *pointer = (*pointer - 1 + memory_size) % memory_size;
                    break;

                case 'B': // Begin loop
                    if (memory[*pointer] == 0)
                    {
                        int loop = 1;
                        int search_row = row;
                        int search_col = col;
                        while (loop > 0)
                        {
                            search_col++;
                            if (search_col >= board->cols)
                            {
                                search_col = 0;
                                search_row++;
                                if (search_row >= board->rows)
                                {
                                    printf("Error: Loop start not found\n");
                                    exit(1);
                                }
                            }
                            char c = board->squares[search_row][search_col];
                            if (c == 'B') loop++;
                            else if (c == 'b') loop--;
                        }
                        row = search_row;
                        col = search_col;
                    }
                    else
                    {
                        if (stack_ptr >= 255)
                        {
                            printf("Error: Loop stack overflow\n");
                            exit(1);
                        }
                        loop_stack[++stack_ptr].row = row;
                        loop_stack[stack_ptr].col = col;
                    }
                    break;

                case 'b': // End loop
                    if (memory[*pointer] != 0)
                    {
                        if (stack_ptr < 0)
                        {
                            printf("Error: Loop stack underflow\n");
                            exit(1);
                        }
                        int jump_row = loop_stack[stack_ptr].row;
                        int jump_col = loop_stack[stack_ptr].col;
                        row = jump_row;
                        col = jump_col; // Set to the position of 'B'
                    }
                    else
                    {
                        if (stack_ptr < 0)
                        {
                            printf("Error: Loop stack underflow\n");
                            exit(1);
                        }
                        stack_ptr--;
                    }
                    break;

                case 'R': // Output memory cell
                    printf("%c", memory[*pointer]);
                    break;

                case 'r': // Get character and store in memory cell
                    printf("\nInput a character: ");
                    {
                        int ch = getchar();
                        // getchar(); // consume newline
                        memory[*pointer] = ch;
                    }
                    break;

                case 'Q': // "push" current memory value to buffer
                    *buffer = memory[*pointer];
                    break;

                case 'q': // "pop" buffer's content to memory
                    memory[*pointer] = *buffer;
                    break;

                case 'K': // Set memory cell to 1 (true)
                    memory[*pointer] = 1;
                    break;

                case 'k': // Set memory cell to 0 (false)
                    memory[*pointer] = 0;
                    break;

                default:
                    // Unrecognized piece
                    break;
            }
        }
    }

    if (BOOL_LOG)
        printf("\nInterpretation complete.\n");
}

/// Parse FEN (only piece placement and side to move), supports multi-digit empty squares
int parseFEN(const char *fen, ChessBoard *board)
{
  char *fen_copy = strdup(fen);
  if (!fen_copy)
    return -1;

  char *space_ptr = strchr(fen_copy, ' ');
  char *board_part = fen_copy;
  char *side_part = NULL;

  if (space_ptr)
  {
    *space_ptr = '\0';
    side_part = space_ptr + 1;
    while (*side_part == ' ')
      side_part++;
  }

  // Determine rows and max_cols
  {
    char *fen_rank_str = strdup(board_part);
    if (!fen_rank_str)
    {
      free(fen_copy);
      return -1;
    }

    int rows = 0;
    int max_cols = 0;
    char *saveptr;
    char *rank = strtok_r(fen_rank_str, "/", &saveptr);
    while (rank)
    {
      rows++;
      int col_count = 0;
      for (int i = 0; rank[i] != '\0'; i++)
      {
        if (rank[i] >= '0' && rank[i] <= '9')
        {
          // Accumulate digits for multi-digit numbers
          int j = i;
          while (rank[j] >= '0' && rank[j] <= '9')
            j++;
          int length = j - i;
          if (length >= 16)
          {
            // Number too large
            free(fen_rank_str);
            free(fen_copy);
            return -1;
          }
          char num_str[16];
          strncpy(num_str, &rank[i], length);
          num_str[length] = '\0';
          int empty_spaces = atoi(num_str);
          col_count += empty_spaces;
          i = j - 1;
        }
        else if (strchr("PNBRQKpnbrqk", rank[i]))
        {
          col_count += 1;
        }
        else
        {
          // Invalid character
          free(fen_rank_str);
          free(fen_copy);
          return -1;
        }
      }
      if (col_count > max_cols)
        max_cols = col_count;
      rank = strtok_r(NULL, "/", &saveptr);
    }

    board->rows = rows;
    board->cols = max_cols;
    free(fen_rank_str);
  }

  board->squares = (char**)malloc(sizeof(char*) * board->rows);
  if (!board->squares)
  {
    free(fen_copy);
    return -1;
  }

  for (int i = 0; i < board->rows; i++)
  {
    board->squares[i] = (char*)malloc(sizeof(char) * board->cols);
    if (!board->squares[i])
    {
      for (int j = 0; j < i; j++)
        free(board->squares[j]);
      free(board->squares);
      free(fen_copy);
      return -1;
    }
    for (int c = 0; c < board->cols; c++)
      board->squares[i][c] = ' ';
  }

  // Fill the board with data
  {
    char *fen_rank_str2 = strdup(board_part);
    if (!fen_rank_str2)
    {
      for (int i = 0; i < board->rows; i++)
        free(board->squares[i]);
      free(board->squares);
      free(fen_copy);
      return -1;
    }

    char *saveptr;
    char *rank = strtok_r(fen_rank_str2, "/", &saveptr);
    int r = 0;
    while (rank)
    {
      int col = 0;
      for (int i = 0; rank[i] != '\0'; i++)
      {
        if (rank[i] >= '0' && rank[i] <= '9')
        {
          // Accumulate digits for multi-digit numbers
          int j = i;
          while (rank[j] >= '0' && rank[j] <= '9')
            j++;
          int length = j - i;
          if (length >= 16)
          {
            // Number too large
            free(fen_rank_str2);
            free(fen_copy);
            for (int x = 0; x < board->rows; x++)
              free(board->squares[x]);
            free(board->squares);
            return -1;
          }

          char num_str[16];
          strncpy(num_str, &rank[i], length);
          num_str[length] = '\0';
          int empty_spaces = atoi(num_str);
          for (int k = 0; k < empty_spaces && col < board->cols; k++)
          {
            board->squares[r][col++] = ' ';
          }
          i = j - 1;
        }
        else if (strchr("PNBRQKpnbrqk", rank[i]))
        {
          if (col < board->cols)
            board->squares[r][col++] = rank[i];
        }
        else
        {
          // Invalid character
          free(fen_rank_str2);
          free(fen_copy);
          for (int x = 0; x < board->rows; x++)
            free(board->squares[x]);
          free(board->squares);
          return -1;
        }
      }
      rank = strtok_r(NULL, "/", &saveptr);
      r++;
    }
    free(fen_rank_str2);
  }

  // Parse "to turn"
  if (side_part && (*side_part == 'w' || *side_part == 'b'))
    board->to_turn = *side_part;
  else
    board->to_turn = 'w'; // default

  free(fen_copy);
  return 0;
}