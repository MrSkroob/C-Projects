#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

// NOTE: SCREEN SIZE IS 320x240
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// BLOCK CONSTANTS
#define MATRIX_SIZE 16
#define SMALL_MATRIX_SIZE 9
#define MATRIX_WIDTH 4
#define SMALL_MATRIX_WIDTH 3
#define BLOCK_WIDTH 10

#define BOARD_WIDTH 110
#define BOARD_HEIGHT 160

#define BOARD_BLOCK_WIDTH BOARD_WIDTH / BLOCK_WIDTH
#define BOARD_BLOCK_HEIGHT BOARD_HEIGHT / BLOCK_WIDTH
#define BOARD_X 50
#define BOARD_Y (SCREEN_HEIGHT - BOARD_HEIGHT) / 2
#define NUMBER_OF_BLOCKS BOARD_BLOCK_WIDTH * BOARD_BLOCK_HEIGHT


// block definitions
int const I_BLOCK[MATRIX_SIZE] = {
  0, 0, 0, 0,
  1, 1, 1, 1,
  0, 0, 0, 0,
  0, 0, 0, 0
};
int const L_BLOCK_LEFT[SMALL_MATRIX_SIZE] = {
  1, 0, 0,
  1, 1, 1,
  0, 0, 0
};
int const L_BLOCK_RIGHT[SMALL_MATRIX_SIZE] = {
  0, 0, 1,
  1, 1, 1,
  0, 0, 0
};
int const O_BLOCK[MATRIX_SIZE] = {
  0, 0, 0, 0,
  0, 1, 1, 0,
  0, 1, 1, 0,
  0, 0, 0, 0
};
int const S_BLOCK_LEFT[SMALL_MATRIX_SIZE] = {
  0, 1, 1,
  1, 1, 0,
  0, 0, 0
};
int const S_BLOCK_RIGHT[SMALL_MATRIX_SIZE] = {
  0, 0, 0,
  1, 1, 0,
  0, 1, 1
};
int const T_BLOCK[SMALL_MATRIX_SIZE] = {
  0, 0, 0,
  0, 1, 0,
  1, 1, 1
};
typedef struct Shapes {
  int block[16];
  Int64 x;
  Int64 y;
  int matrix_width;
  int matrix_size;
  Channel sprite;
  int sprite_index;
} Shape;


void apply_changes(const int* new_array, int* block_array, int array_sizes) { // copies the new_array to 
  for (int i = 0; i < array_sizes; i++) {                               // the block array
    block_array[i] = new_array[i];
  }
}


void reverse_rows(int* block_array, int matrix_size, int matrix_width) { // reverses the rows in a 4x4 (16) id array
  int copy[matrix_size];
  for (int i = 0; i < matrix_width; i++) {
    for (int j = matrix_width; j > 0; j--) {
      int index = i * matrix_width - 1 + j; // j = collumn, i = row
      int reversed_index = i * matrix_width + (matrix_width - j);
      copy[reversed_index] = block_array[index];
    }
  }
  apply_changes(copy, block_array, MATRIX_SIZE);
}


void reverse_collumns(int* block_array, int matrix_size, int matrix_width) { // reverses the collumns in a 4x4 (16) 1d array
  int copy[matrix_size];
  for (int i = 0; i < matrix_width; i++) {
    int collumn_copy[matrix_width];
    for (int j = 0; j < matrix_width; j++) {
      int index = j * matrix_width + i;
      collumn_copy[j] = block_array[index];
    }
    for (int j = 0; j < matrix_width; j++) {
      int index = j * matrix_width + i;
      copy[index] = collumn_copy[matrix_width - 1 - j];
    };
  }
  apply_changes(copy, block_array, matrix_size);
}


void transpose(int* block_array, int matrix_size, int matrix_width) { // transposes the 4x4 (16) 1d array
  int copy[matrix_size];
  for (int i = 0; i < matrix_width; i++) {
    for (int j = 0; j < matrix_width; j++) {
      int index = i * matrix_width + j;
      int swapped_index = i + j * matrix_width;
      int temp = block_array[index];
      copy[swapped_index] = temp;
    }
  };
  apply_changes(copy, block_array, matrix_size);
};

void rotate_90_CW(Shape* shape) {
  // rotates the tetris piece 90 degrees counter clockwise
  int matrix_size = shape->matrix_size;
  int matrix_width = shape->matrix_width;
  transpose(shape->block, matrix_size, matrix_width);
  reverse_collumns(shape->block, matrix_size, matrix_width);
}


void rotate_90_CCW(Shape* shape) {
  // rotates the tetris piece 90 degrees clockwise
  rotate_90_CW(shape);
  rotate_90_CW(shape);
  rotate_90_CW(shape);
  /*
  int matrix_size = shape->matrix_size;
  int matrix_width = shape->matrix_width;
  reverse_rows(shape->block, matrix_size, matrix_width);
  transpose(shape->block, matrix_size, matrix_width);
  */
};


int get_random(int lower, int upper) {
  // returns a random integer between the lower and upper bounds, inclusive
  return (rand() % (upper - lower + 1) + (lower - 1));
}


void draw(Channel io, Channel sprite, int x, int y) {
  BeginDraw(io);
  DrawSprite(io, sprite, x, y);
  EndDraw(io);
}


Bool is_colliding(Shape shape, int* board) {
  // checks if the given shape is colliding with a block
  Bool result = False;
  int matrix_width = shape.matrix_width;
  for (int i = 0; i < matrix_width; i += 1) {
    for (int j = 0; j < matrix_width; j += 1) {
      int index = i * matrix_width + j;
      if (shape.block[index] == 1) {
        Int64 x = shape.x + i;
	Int64 y = shape.y + j;
	int board_index = y * BOARD_BLOCK_WIDTH + x;
	if (!(x > -1 && x < BOARD_BLOCK_WIDTH && y > -1)) {
	  return True;
	}
	else if (board_index > -1 && board_index < NUMBER_OF_BLOCKS && board[board_index] > 0) {
	  return True;
	}
      }
    }
  }
  return False;
}


Bool is_game_over(Shape shape, int* board) {
  // checks if the given shape is colliding with a block
  Bool result = False;
  int matrix_width = shape.matrix_width;
  for (int i = 0; i < matrix_width; i += 1) {
    for (int j = 0; j < matrix_width; j += 1) {
      int index = i * matrix_width + j;
      if (shape.block[index] == 1) {
	Int64 y = shape.y + j;
        if (y > 14) {
	  return True;
	}
      }
    }
  }
  return False;
};


void draw_shape(Channel io, Shape shape, Channel sprite, int* board, Bool override, Bool draw_anyway) {
  // draws the tetris piece with the given colour
  // shape_x starts from the right
  // shape_y starts from the bottom
  // override means deleting the area which is specified by the .block array
  BeginDraw(io);
  int matrix_width = shape.matrix_width;
  for (int i = 0; i < matrix_width; i += 1) {
    for (int j = 0; j < matrix_width; j += 1) {
      int index = i * matrix_width + j;
      if (shape.block[index] == 1) {
	// draw current position, defined by .block shape.
	Int64 x = shape.x + i;
	Int64 y = shape.y + j;
	int board_index = y * BOARD_BLOCK_WIDTH + x;
	if ((x > -1 && x < BOARD_BLOCK_WIDTH && y > -1 && y < BOARD_BLOCK_HEIGHT) || draw_anyway == True) {
	  // in boundaries
	  // we only attempt to draw if the block is within the boundaries
	  if (draw_anyway == False) {
	    if (override) {
	      board[board_index] = 0;
	    }
	    else {
	      board[board_index] = shape.sprite_index;
	    }
	  }
	  DrawSprite(io, sprite, BOARD_X + shape.x * BLOCK_WIDTH + i * BLOCK_WIDTH, BOARD_Y + shape.y * BLOCK_WIDTH + j * BLOCK_WIDTH);
	}	
      }
    }
  }
  EndDraw(io);
}


int get_int_len(int value) {
  int length = 1;
  while (value > 9) {
    length++;
    value /= 10;
  }
  return length;
}


void preview_piece(Channel io, Shape current_piece, int* board) {
  current_piece.x = 17;
  current_piece.y = 12;
  BeginDraw(io);
  FillRect(io, Black, 220, 160, 250, 200);
  EndDraw(io);
  draw_shape(io, current_piece, current_piece.sprite, board, True, True);
}


void draw_square(Channel io, Channel sprite, int x, int y) {
  // similar to draw_shape x and y is grid based.
  BeginDraw(io);
  DrawSprite(io, sprite, BOARD_X + x * BLOCK_WIDTH, BOARD_Y + y * BLOCK_WIDTH);
  EndDraw(io);
}


void draw_space(Channel io, Channel sprite, int x1, int y1, int x2, int y2) {
  // note that this method uses precise x and y coordinates.
  BeginDraw(io);
  for (int i = x1; i < x2; i += BLOCK_WIDTH) {
    for (int j = y1; j < y2; j += BLOCK_WIDTH) {
      DrawSprite(io, sprite, i, j);
    }
  }
  EndDraw(io);
}

void draw_text(Channel io, Color colour, int x, int y, StaticText text) {
  BeginDraw(io);
  FillRect(io, Black, x, y, x + strlen(text) * 6, y - 10);
  BeginDrawText(io, Mono11, colour, x, y);
  PutText(io, text);
  EndDrawText(io);
  EndDraw(io);
}


void draw_number(Channel io, Color colour, int x, int y, int score) {
  BeginDraw(io);
  // int length = get_int_len(score);
  FillRect(io, Black, x, y, x + 100, y - 10);
  BeginDrawText(io, Mono11, colour, x, y);
  PutInt32AsText(io, score);
  EndDrawText(io);
  EndDraw(io);
}


void Main(Channel io) {
  // load in sprites to draw
  Channel cyan_block = LoadNewSprite("sprite", io, "cyan_block");
  Channel pink_block = LoadNewSprite("sprite", io, "pink_block");
  Channel red_block = LoadNewSprite("sprite", io, "red_block");
  Channel blue_block = LoadNewSprite("sprite", io, "blue_block");
  Channel green_block = LoadNewSprite("sprite", io, "green_block");
  Channel orange_block = LoadNewSprite("sprite", io, "orange_block");
  Channel yellow_block = LoadNewSprite("sprite", io, "yellow_block");
  Channel black_block = LoadNewSprite("sprite", io, "black_block");

  srand(time(NULL));

  int board[NUMBER_OF_BLOCKS];
  memset(board, 0, sizeof(board));

  // setup the blocks to use later
  int i_block[MATRIX_SIZE];
  int l_block_left[MATRIX_SIZE];
  int l_block_right[MATRIX_SIZE];
  int o_block[MATRIX_SIZE];
  int s_block_left[MATRIX_SIZE];
  int s_block_right[MATRIX_SIZE];
  int t_block[MATRIX_SIZE];

  apply_changes(I_BLOCK, i_block, MATRIX_SIZE); // copy the array to tetris_piece
  apply_changes(L_BLOCK_LEFT, l_block_left, SMALL_MATRIX_SIZE);
  apply_changes(L_BLOCK_RIGHT, l_block_right, SMALL_MATRIX_SIZE);
  apply_changes(O_BLOCK, o_block, MATRIX_SIZE);
  apply_changes(S_BLOCK_LEFT, s_block_left, SMALL_MATRIX_SIZE);
  apply_changes(S_BLOCK_RIGHT, s_block_right, SMALL_MATRIX_SIZE);
  apply_changes(T_BLOCK, t_block, SMALL_MATRIX_SIZE);
  
  Shape i_shape;
  i_shape.sprite = cyan_block; // assign attributes to each struct
  i_shape.matrix_size = MATRIX_SIZE;
  i_shape.matrix_width = MATRIX_WIDTH;
  i_shape.sprite_index = 1; // we start index 1 because 0 is reserved by empty space
  apply_changes(i_block, i_shape.block, MATRIX_SIZE);

  Shape l_shape_left;
  l_shape_left.sprite = red_block;
  l_shape_left.matrix_size = SMALL_MATRIX_SIZE;
  l_shape_left.matrix_width = SMALL_MATRIX_WIDTH;
  l_shape_left.sprite_index = 2;
  apply_changes(l_block_left, l_shape_left.block, MATRIX_SIZE);
  
  Shape l_shape_right;
  l_shape_right.sprite = blue_block;
  l_shape_right.matrix_size = SMALL_MATRIX_SIZE;
  l_shape_right.matrix_width = SMALL_MATRIX_WIDTH;
  l_shape_right.sprite_index = 3;
  apply_changes(l_block_right, l_shape_right.block, MATRIX_SIZE);
  
  Shape o_shape;
  o_shape.sprite = green_block;
  o_shape.matrix_size = MATRIX_SIZE;
  o_shape.matrix_width = MATRIX_WIDTH;
  o_shape.sprite_index = 4;
  apply_changes(o_block, o_shape.block, MATRIX_SIZE);
  
  Shape s_shape_left;
  s_shape_left.sprite = pink_block;
  s_shape_left.matrix_size = SMALL_MATRIX_SIZE;
  s_shape_left.matrix_width = SMALL_MATRIX_WIDTH;
  s_shape_left.sprite_index = 5;
  apply_changes(s_block_left, s_shape_left.block, MATRIX_SIZE);
  
  Shape s_shape_right;
  s_shape_right.sprite = orange_block;
  s_shape_right.matrix_size = SMALL_MATRIX_SIZE;
  s_shape_right.matrix_width = SMALL_MATRIX_WIDTH;
  s_shape_right.sprite_index = 6;
  apply_changes(s_block_right, s_shape_right.block, MATRIX_SIZE);

  Shape t_shape;
  t_shape.sprite = yellow_block;
  t_shape.matrix_size = SMALL_MATRIX_SIZE;
  t_shape.matrix_width = SMALL_MATRIX_WIDTH;
  t_shape.sprite_index = 7;
  apply_changes(t_block, t_shape.block, MATRIX_SIZE);

  // array of all the shapes
  Shape all_shapes[7] = {
    i_shape,
    l_shape_left,
    l_shape_right,
    o_shape,
    s_shape_left,
    s_shape_right,
    t_shape
  };

  // text positions
  int title_y = 220;
  int score_y = 140;
  int clear_y = 120;
  int combo_y = 100;
  int place_y = 80;
  int total_y = 60;

  // draw the board
  draw_space(io, black_block, BOARD_X, BOARD_Y, BOARD_X + BOARD_WIDTH, BOARD_Y + BOARD_HEIGHT);
  draw_text(io, White, 180, title_y, "NOT TETRIS");
  draw_text(io, White, 80, title_y, "LEVEL: 1");
  
  draw_text(io, White, 180, 180, "next:");
  draw_text(io, White, 216, score_y, "score:");
  draw_text(io, White, 180, clear_y, "clear bonus:");
  draw_text(io, White, 180, combo_y, "clear combo:");
  draw_text(io, White, 192, place_y, "placement:");

  draw_text(io, Green, 256, score_y, "0");
  draw_text(io, Green, 256, clear_y, "+0");
  draw_text(io, Green, 256, combo_y, "x0");
  draw_text(io, Green, 256, place_y, "+0");
  draw_text(io, Green, 256, total_y, "+0");

  // level
  int MAX_LEVEL = 10;
  int level = 0;
  int threshhold[10] = { // number of lines to clear to advance to the next level
    10,
    15,
    20,
    25,
    30,
    40,
    50,
    60,
    80,
    100 // not needed, technically
  };
  int levels[10] = { // ticks game waits before lowering the block
    20,
    18,
    15,
    13,
    12,
    11,
    10,
    7,
    6,
    5,
  };

  // score vars
  int score = 0;
  int rows_cleared = 0; // not shown
  int row_clear = 0;
  int combo = 0; // each successive clear increases the combo by 1;
  int fall_bonus = 0;
  int place_score = 0;

  // control vars
  int tick = 0; // movement downwards is based off of this tick
  int keydown_time[5] = {0};

  int move_left_index = 0;
  int move_right_index = 1;
  int rotate_cw_index = 2;
  int rotate_ccw_index = 3;
  int soft_fall_index = 4;
  
  int x_offset = 0; // direction to move the piece
  int rotation_direction = 0; // which direction to rotate
  Bool skip = False; // go down

  Bool paused = False;

  int shape_index = get_random(1, 7); // next piece
  
  while (True) {
    // random piece chosen:

    Shape current_piece = all_shapes[shape_index];
    
    current_piece = all_shapes[shape_index];
    current_piece.y = BOARD_BLOCK_HEIGHT;
    current_piece.x = BOARD_BLOCK_WIDTH - 7;

    Bool input_cooldown = False;
    Bool keydown[5] = {False};

    shape_index = get_random(1, 7);
    preview_piece(io, all_shapes[shape_index], board);

    while (True) { 
      // input handling
      memset(keydown, False, sizeof(keydown));

      // pausing
      if (IsKeyDown(io, KeyP)) {
	paused = True;
      }
      else if (IsKeyDown(io, KeyO)) {
	paused = False;
      }
      
      if (IsKeyDown(io, KeyLeft)) {
	x_offset = -1;
	keydown[move_left_index] = True;
	keydown_time[move_left_index] += 1;
	if (keydown_time[move_left_index] > 5) {
	  keydown_time[move_left_index] = 0;
	}
      }
      else {
	keydown_time[move_left_index] = 0;
      };
      if (IsKeyDown(io, KeyRight)) {
	x_offset = 1;
	keydown[move_right_index] = True;
	keydown_time[move_right_index] += 1;
	if (keydown_time[move_right_index] > 5) {
	  keydown_time[move_right_index] = 0;
	}
      }
      else {
	keydown_time[move_right_index] = 0;
      }
      if (IsKeyDown(io, KeyUp)) {
	rotation_direction = 2;
	keydown[rotate_cw_index] = True;
	keydown_time[rotate_cw_index] += 1;
	if (keydown_time[rotate_cw_index] > 10) {
	  keydown_time[rotate_cw_index] = 0;
	}
      }
      else {
	keydown_time[rotate_cw_index] = 0;
      }
      if (IsKeyDown(io, KeyDown)) {
	rotation_direction = 1;
	keydown[rotate_ccw_index] = True;
	keydown_time[rotate_ccw_index] += 1;
	if (keydown_time[rotate_ccw_index] > 10) {
	  keydown_time[rotate_ccw_index] = 0;
	}
      }
      else {
	keydown_time[rotate_ccw_index] = 0;
      }
      if (IsKeyDown(io, KeySpace)) {
	skip = True;
	keydown[soft_fall_index] = True;
	keydown_time[soft_fall_index] += 1;
	if (keydown_time[soft_fall_index] > 3) {
	  keydown_time[soft_fall_index] = 0;
	}
      }
      else {
	keydown_time[soft_fall_index] = 0;
	skip = False;
      }
      // input handling END

      // we clear the previous drawing
      draw_shape(io, current_piece, black_block, board, True, False);
      
      // matrix orientation prior to movement changes
      int before[MATRIX_SIZE];
      int before_x = current_piece.x;
      apply_changes(current_piece.block, before, MATRIX_SIZE);
      
      // apply movement changes
      if (keydown_time[move_left_index] == 1 || keydown_time[move_right_index] == 1) {
	current_piece.x += x_offset;
      }
      if (keydown_time[rotate_cw_index] == 1 || keydown_time[rotate_ccw_index] == 1) {
	if (rotation_direction == 1) {
	  rotate_90_CW(&current_piece);
	}
	else if (rotation_direction == 2) {
	  rotate_90_CCW(&current_piece);
	}
      }

      // check if movement changes are valid
      if (is_colliding(current_piece, board)) {
	// collision! invalid movement
	Bool success = False;
	// we don't need to do this work if
	// the only reason it is invalid is
	// when we move it left or right
	    
	// try going right
	for (int i = 0; i < 3; i++) {
	  current_piece.x += 1;
	  if (!is_colliding(current_piece, board)) {
	    success = True;
	    break;
	  }
	}

	// try going left
	if (success == False) {
	  // return to previous position
	  current_piece.x = before_x + x_offset;
	  for (int i = 0; i < 3; i++) {
	    current_piece.x -= 1;
	    if (!is_colliding(current_piece, board)) {
	      success = True;
	      break;
	    }
	  }
	  if (success == False) {
	    apply_changes(before, current_piece.block, MATRIX_SIZE);
	    current_piece.x = before_x;
	  }
	}
	// we finally draw the actual piece
      }
      
      // validation END

      if (paused == False) {
	draw_shape(io, current_piece, current_piece.sprite, board, True, False);
	if (skip == True) {
	  // move down faster (skipping ticks) if we're holding down
	  tick += 5;
	  place_score += 1;
	  draw_number(io, Green, 262, total_y, row_clear * combo + place_score);
	  draw_number(io, Green, 262, place_y, place_score);
	}
	else {
	  tick += 1;
	}
      }
      
      // if 20 ticks have passed
      if (tick > levels[level]) {
	tick = 0;
	// move down
	draw_shape(io, current_piece, black_block, board, True, False);
	current_piece.y -= 1;
	if (is_colliding(current_piece, board)) {
	  current_piece.y += 1;
	  draw_shape(io, current_piece, current_piece.sprite, board, False, False);
	  place_score += 40;
	  draw_number(io, Green, 262, total_y, row_clear * combo + place_score);
	  draw_number(io, Green, 262, place_y, place_score);
	  break;
	}
	draw_shape(io, current_piece, current_piece.sprite, board, False, False);
      }
      WaitForTick(io);
    }
    
    // handle board clearing

    Bool rows_to_clear[BOARD_BLOCK_HEIGHT];
    int move_count = 0;
    memset(rows_to_clear, False, sizeof(rows_to_clear));
    
    for (int i = 0; i < BOARD_BLOCK_HEIGHT; i++) {
      // x will be the row number
      int consecutive_blocks = 0;
      for (int j = 0; j < BOARD_BLOCK_WIDTH; j++) {
	int board_index = i * BOARD_BLOCK_WIDTH + j;
	if (board[board_index] == 0) {
	  // unsuccessful
	  if (move_count > 0) {
	    for (int j = 0; j < BOARD_BLOCK_WIDTH; j++) {
	      int board_index = i * BOARD_BLOCK_WIDTH + j;
	      if (board[board_index] > 0) {
		draw_square(io, black_block, j, i);
		draw_square(io, all_shapes[board[board_index] - 1].sprite, j, i - move_count);
	      }
	      board[board_index - move_count * BOARD_BLOCK_WIDTH] = board[board_index];
	      board[board_index] = 0;
	    }
	  }
	  break;
	}
	consecutive_blocks += 1;
      }
      if (consecutive_blocks == BOARD_BLOCK_WIDTH) {
	for (int j = 0; j < BOARD_BLOCK_WIDTH; j++) {
	  int board_index = i * BOARD_BLOCK_WIDTH + j;
	  board[board_index] = 0;
	  int y = BOARD_Y + BLOCK_WIDTH * i;
	  draw_square(io, black_block, j, i);
	  // draw_space(io, black_block, BOARD_X, y, BOARD_X + BOARD_WIDTH, y + BLOCK_WIDTH);
	}
	move_count += 1;
	rows_to_clear[i] = True;
	row_clear += 1000;
	draw_number(io, Green, 262, total_y, row_clear * combo + place_score);
	draw_number(io, Green, 262, clear_y, row_clear);
	WaitForTime(io, 20);
      }
    }
    
    if (move_count == 0) {
      combo = 0;
    }
    else {
      combo += 1;
    }

    // handle text
    
    draw_number(io, Green, 262, combo_y, combo);
    draw_number(io, Green, 262, total_y, row_clear * combo + place_score);

    WaitForTime(io, 20);
    
    score += row_clear * combo + place_score;
    
    draw_number(io, Green, 256, score_y, score);
    draw_number(io, Green, 262, total_y, 0);
    draw_number(io, Green, 262, clear_y, 0);
    draw_number(io, Green, 262, place_y, 0);

    fall_bonus = 0;
    place_score = 0;
    row_clear = 0;
    
    if (is_game_over(current_piece, board)) {
      // game over!
      PutText(Trace, "game over!\n");
      draw_text(io, White, 180, title_y, "GAME OVER!"); 
      WaitForKeyPress(io);

      // reset for next game
      score = 0;
      level = 0;
      rows_cleared = 0;
      draw_number(io, Green, 256, score_y, 0);
      draw_text(io, White, 180, title_y, "NOT TETRIS");
      draw_space(io, black_block, BOARD_X, BOARD_Y, BOARD_X + BOARD_WIDTH, BOARD_Y + BOARD_HEIGHT);
      memset(board, 0, sizeof(board));
    };
    
  }
    
  WaitForKeyPress(io);
}
