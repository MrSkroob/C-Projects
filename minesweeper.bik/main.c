#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

const int MINE_SIZE = 10; // size of each square DEFAULT 10
const int MINE_FREQUENCY = 85; // 0 is all mines, 100 is no mine. 
const int SCREEN_CENTRE_X = 160; // NOTE, SCREEN SIZE IS 320
const int SCREEN_CENTRE_Y = 120; // NOTE, SCREEN SIZE IS 240
const int OFFSETS[9][2] = {// array which stores offsets 
  {-1, -1}, // top left
  {0, -1}, // top
  {1, -1}, // top right
  {-1, 0}, // left
  {1, 0}, // right
  {-1, 1}, // bottom left
  {0, 1}, // bottom
  {1, 1}
}; // bottom right


struct mine_piece {
  Float64 x1;
  Float64 y1;
  Float64 x2;
  Float64 y2;
  int i;
  int j;
  Float64 size;
  int mines_surrounded; // only used if the mine is not a mine.
  Bool is_marked; // if user has flagged it!
  Bool is_revealed; // has been revealed, basically.
  Bool is_mine; // self explanatory
};


int within_mine(Float64 mouse_x, Float64 mouse_y, struct mine_piece mines[], int mine_count) {
  // returns the index of the mine the mouse is intercepting. Returns -1 if nothing found.
  for (int i = 0; i < mine_count; i++) {
    struct mine_piece mine = mines[i];
    if ((mouse_x > mine.x1 && mouse_x < mine.x2) && (mouse_y > mine.y1 && mouse_y < mine.y2)) {
      return i;
    }
  }
  return -1;
}


int get_index_from_position(int collumn, int row, int board_size) {
  if (collumn < 0 || row < 0) {
    return -1;
  }
  if (collumn > board_size - 1 || row > board_size - 1) {
    return -1;
  }
  int index = (row * board_size) + collumn;
  return index;
}


int surrounding_mines(int mine_index, struct mine_piece mines[], int board_size) {
  // returns the number of mines that surround the current piece.
  int current_collumn = mine_index % board_size; // collumn
  int current_row = (int) floor(mine_index / board_size); // row
  int number_of_mines = 0;

  for (int i = 0; i < 9; i++) {
    int collumn = OFFSETS[i][0];
    int row = OFFSETS[i][1];

    int adj_index = get_index_from_position(current_collumn + collumn, current_row + row, board_size);
    if (adj_index != -1) {
      struct mine_piece mine = mines[adj_index];
      if (mine.is_mine == True) {
	number_of_mines++;
      }
    }
  }
  
  return number_of_mines;
}


void draw_space(Channel io, struct mine_piece mine) {
  BeginDraw(io);
  FillRect(io, White, mine.x1, mine.y1, mine.x2, mine.y2);
  FillRect(io, Gray(200), mine.x1 + 1, mine.y1 + 1, mine.x2 - 1, mine.y2 - 1);
  // DrawRect(io, White, mine.x1, mine.y1, mine.x2, mine.y2);
  EndDraw(io);
}


void draw_flag(Channel io, struct mine_piece mine) {
  BeginDraw(io);
  FillRect(io, White, mine.x1, mine.y1, mine.x2, mine.y2);
  FillRect(io, Green, mine.x1 + 1, mine.y1 + 1, mine.x2 - 1, mine.y2 - 1);
  // DrawRect(io, White, mine.x1, mine.y1, mine.x2, mine.y2);
  EndDraw(io);
}


void draw_empty_space(Channel io, struct mine_piece mine) {
  BeginDraw(io);
  if (mine.is_mine == False) {
    FillRect(io, Black, mine.x1, mine.y1, mine.x2, mine.y2);
    if (mine.mines_surrounded > 0) {
      BeginDrawText(io, Mono11, White, (mine.x1 + mine.x2) / 2 - 3, (mine.y1 + mine.y2) / 2 + 4);
      PutInt64AsText(io, mine.mines_surrounded);
      EndDrawText(io);
    }
  }
  else {
    FillRect(io, Red, mine.x1, mine.y1, mine.x2, mine.y2);
  }
  EndDraw(io);
}


void reveal_board(Channel io, struct mine_piece mines[], int mine_count) {
  for (int i = 0; i < mine_count; i++) {
    struct mine_piece mine = mines[i];
    draw_empty_space(io, mine);
    mines[i].is_revealed = True;
  }
}


void win_text(Channel io) {
  BeginDraw(io);
  BeginDrawText(io, Mono11, Green, SCREEN_CENTRE_X - 3, 12);
  PutText(io, "You win!");
  EndDrawText(io);
  EndDraw(io);
}


void score_text(Channel io, int score) {
  BeginDraw(io);
  FillRect(io, Gray(128), 5, SCREEN_CENTRE_Y - 9, 25, SCREEN_CENTRE_Y + 5);
  BeginDrawText(io, Mono11, White, 5, SCREEN_CENTRE_Y);
  PutInt32AsText(io, score);
  EndDrawText(io);
  EndDraw(io);
}


void mines_left_text(Channel io, int mines_left) {
  BeginDraw(io);
  FillRect(io, Gray(128), 5, SCREEN_CENTRE_Y - 24, 25, SCREEN_CENTRE_Y -10);
  BeginDrawText(io, Mono11, Green, 5, SCREEN_CENTRE_Y - 15);
  PutInt32AsText(io, mines_left);
  EndDrawText(io);
  EndDraw(io);
}


int clear_area(Channel io, struct mine_piece *mines, int mine_index, int board_size) {
  // recursively clears adjacent empty spaces.
  int area_cleared = 1;
  struct mine_piece mine = mines[mine_index];
  if (mine.is_marked == True) {return 0;}
  if (mine.is_revealed == True) {return 0;}
  if (mine.is_mine == True) {return -1;} // lose state!!
  mines[mine_index].is_revealed = True;
  if (mine.mines_surrounded > 0) {
    draw_empty_space(io, mine);
    return 1;
  }
  int current_collumn = mine_index % board_size; // collumn
  int current_row = (int) floor(mine_index / board_size); // row
  draw_empty_space(io, mine);
  for (int i = 0; i < 9; i++) {
    int collumn = OFFSETS[i][0];
    int row = OFFSETS[i][1];
    int index = get_index_from_position(current_collumn + collumn, current_row + row, board_size);
    if (index != -1) {
      area_cleared += clear_area(io, mines, index, board_size);
    }
  }
  return area_cleared;
}


int clear_surroundings(Channel io, struct mine_piece *mines, int mine_index, int board_size) {
  // behavior of clicking number, which will clear the surrounding area.
  int area_cleared = 0;
  int current_collumn = mine_index % board_size;
  int current_row = (int) floor(mine_index / board_size);
  for (int i = 0; i < 9; i++) {
    int collumn = OFFSETS[i][0];
    int row = OFFSETS[i][1];
    int index = get_index_from_position(current_collumn + collumn, current_row + row, board_size);
    if (index != -1) {
      if ((mines[index].is_revealed == False) && (mines[index].is_marked == False)) {
	if (mines[index].is_mine == True) {return -1;}
	draw_empty_space(io, mines[index]);
	area_cleared += clear_area(io, mines, index, board_size);
      }
    }
  }
  return area_cleared;
}


int get_random(int lower, int upper) {
  // returns a random integer between the lower and upper bounds, inclusive.
  return (rand() % (upper - lower + 1)) + (lower - 1); 
}


int generate_mines(Channel io, Float64 origin_x, Float64 origin_y, struct mine_piece *mines, int mine_count, int board_size) {
  int mines_layed = 0;
  for (int i = 0; i < board_size; i++) {
    for (int j = 0; j < board_size; j++) {
      struct mine_piece mine;
      mine.x1 = origin_x + j * MINE_SIZE; // x is the left of the mine
      mine.y1 = origin_y + i * MINE_SIZE; // y is the top of the mine
      mine.x2 = mine.x1 + MINE_SIZE;
      mine.y2 = mine.y1 + MINE_SIZE;
      mine.is_marked = False;
      mine.is_revealed = False;
      mine.size = MINE_SIZE;
      mine.mines_surrounded = 0; // variable for number of mines surrounding this piece. 
      if (get_random(0, 100) >= MINE_FREQUENCY) {
	mine.is_mine = True;
	mines_layed += 1;
      }
      else {
	mine.is_mine = False;
      }
      mines[i * board_size + j] = mine;
    }
  }
  
  for (int i = 0; i < mine_count; i++) {
    struct mine_piece mine = mines[i];
    draw_space(io, mine);
    // assign number of surrounding mines.
    if (mine.is_mine == False) {
      mines[i].mines_surrounded = surrounding_mines(i, mines, board_size);
    }
  }
  return mines_layed;
}


void Main(Channel io) {
  // NOTE: coordinates 0, 0 is the bottom left
  // NOTE: size of screen is 320 x 240
  BeginDraw(io);
  FillFrame(io, Gray(128));
  EndDraw(io);
  
  int board_size = 23; // number of squares in width DEFAULT 23
  int mine_count = board_size * board_size; // number of total mines 
  
  srand(time(NULL)); // intialize random seed
  
  Int64 board_width = board_size * MINE_SIZE;
  Float64 origin_x = SCREEN_CENTRE_X - (board_width / 2); // get centralised origin on where to draw
  Float64 origin_y = SCREEN_CENTRE_Y - (board_width / 2); // same as x but for y
  
  struct mine_piece mines[mine_count]; // array to store the mines in

  // draw the entire grid once.
  generate_mines(io, origin_x, origin_y, mines, mine_count, board_size);
  score_text(io, mine_count);

  Bool first_move = True;
  int spaces_to_clear = 0;
  int mines_to_clear = 0;
  
  // main loop
  while (True) {
    // events
    while (True) {
      EventTag tag = WaitForEvent(io);
      if (tag == MousePress) {
	MouseButtonEvent event = TakeMouseButtonEvent(io);
	if (event.button == MouseLeft) {
	  int mine_index = within_mine(event.x, event.y, mines, mine_count);
	  if (mines[mine_index].is_marked == True) {break;}
	  if (mine_index == -1) {break;}
	  if (first_move == True) {
	    first_move = False;
	    // first move, we give the player enough information to keep playing.
	    int area_cleared = 0;
	    while (area_cleared < 3) {
	      mines_to_clear = generate_mines(io, origin_x, origin_y, mines, mine_count, board_size);
	      area_cleared = clear_area(io, mines, mine_index, board_size);
	    }
	    spaces_to_clear = mine_count - mines_to_clear - area_cleared;
	    score_text(io, spaces_to_clear);
	    mines_left_text(io, mines_to_clear);
	  }
	  else {
	    int spaces_cleared = 0;
	    if ((mines[mine_index].mines_surrounded > 0) && mines[mine_index].is_revealed == True) {
	      // clicked numbered square
	      spaces_cleared = clear_surroundings(io, mines, mine_index, board_size);
	    }
	    else {
	      // clicked non-revealed square
	      spaces_cleared = clear_area(io, mines, mine_index, board_size);
	    }
	    spaces_to_clear -= spaces_cleared;
	    score_text(io, spaces_to_clear);
	    if (spaces_cleared == -1) {
	      // lose state
	      // insert lose code here :P
	      reveal_board(io, mines, mine_count);
	      WaitForKeyPress(io);
	      Main(io);
	      return;
	    }
	    else if (spaces_to_clear <= 0) {
	      // win state
	      // insert win code here :P
	      win_text(io);
	      WaitForKeyPress(io);
	      Main(io);
	      return;
	    }
	  }
	}
	else if (event.button = MouseRight) {
	  int mine_index = within_mine(event.x, event.y, mines, mine_count);
	  if (mine_index == -1) {break;}
	  struct mine_piece mine = mines[mine_index];
	  if (mine.is_revealed == True) {break;}
	  if (mine.is_marked == False) {
	    if (mines_to_clear == 0) {break;}
	    mines_to_clear -= 1;
	    draw_flag(io, mine);
	  }
	  else {
	    mines_to_clear += 1;
	    draw_space(io, mine);
	  }
	  mines_left_text(io, mines_to_clear);
	  mines[mine_index].is_marked = !mine.is_marked;
	}
      }
      else {
	DiscardEvent(io);
      }
    }
    WaitForTick(io);
  }
}
