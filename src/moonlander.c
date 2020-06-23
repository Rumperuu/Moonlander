/**
 * @file															                         
 * @author  Ben Goldsworthy (rumps) <me+moonlander@bengoldworthy.net>              
 * @version 1.0	                                                          
 *                                                                          
 * @section LICENSE                                                         
 *                                                                          
 * This file is free software: you can redistribute it and/or modify        
 * it under the terms of the GNU General Public License as published by     
 * the Free Software Foundation, either version 3 of the License,           
 * or (at your option) any later version.                                   
 *                                                                          
 * This file is distributed in the hope that it will be useful,             
 * but WITHOUT ANY WARRANTY; without even the implied warranty of           
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            
 * GNU General Public License for more details.                             
 *                                                                          
 * You should have received a copy of the GNU General Public License        
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.    
 *                                                                          
 * @section DESCRIPTION                                                     
 *                                                                          
 * This program attempts to emulate the 1979 Atari arcade classic 'Lunar Lander'
 * in the *nix Terminal, using the ncurses library 
 * (<https://www.gnu.org/software/ncurses/ncurses.html>). This can only 
 * be because Joe Finney is a sadistic madman.
 * 
 * Implemented features so far are:
 * 
 * - momentum & gravity
 * - random landscape generation
 * - crashing & (rarely) landing
 * - cheats
 * - scoring
 * 
 * Features to implement by TOMORROW are:
 * 
 * - more cheats (or 'debug modes' as all the kids are calling them these days)
 * - leaderboards
 * - random gravity generation
 */

#include "moonlander.h"

/**
 * The main function of the program.
 * 
 * Sets everything up initially, and then contains the game loop.
 * 
 * @return 0 on success
 */
int main() {
   // Declarations of variables used throughout `main()`
   // With all this talk of `SHIP`s and `LANDSCAPE`s, it all
   // feels a bit object oriented around here.
	SHIP ship;
	LANDSCAPE landscape;   
   // Values used for tracking the score.
   unsigned int time;
   // Used for animating the game.
   s.tv_sec = 0;
   s.tv_nsec = 180000000L;
   // Used for recording user input.
   unsigned int ch;
   // Used for moving the ship.
   unsigned int jetDir;
   // Used for dealing with the landscape arrays.
   size_t lASize, sASize;
   // Tracks the score.
   double score;
   
   // This is where the magic happens.
   initialisencurses();   
 
// I know, I know; 'Go To Statement Considered Harmful' and
// all that. I feel like even Dijkstra would let me off for this
// one though.
restart:  
   // (Re-)Initialises the relevant variables; the user isn't always coming
   // to this point fresh.
   end = false;
   endType = NONE;
   time = 0;
   invincible = false;
   ch = ' ';
   jetDir = NONE; 
   score = 0.0f;
   
   // `lASize` is initialised to 1 rather than 0 because the first coordinates
   // in `landscapeArray` will be the leftmost ones; that is, they will have a
   // x-coord of 0 and the size of the array will return as 0.
   lASize = 1;
   sASize = 0;
   
   // Displays the lovely nicked ASCII lunar lander splash screen.
   displayIntro();
   
   // Loops whilst on the intro screen until the button to start the
   // game is pressed. Meanwhile, the keys for the available cheats
   // can be used to toggle their effects.
   while ((ch = getch()) != 'a'){
      if (ch == 'i') {
         invincible = (invincible) ? false : true;
         mvaddch(LINES-1, COLS-1, (invincible) ? 'T' : 'F');
      }      
   }

   // Wipes the slate clean.
   clear();
   
   // ncurses has the weirdest system of dealing with colours.
	init_pair(1, COLOR_CYAN, COLOR_BLACK);
	init_pair(2, COLOR_RED, COLOR_BLACK);
	init_pair(3, COLOR_GREEN, COLOR_BLACK);
   
   // Initialises the parameters for the ship and landscape; did 
   // someone say object constructors?
	initialiseShip(&ship);
	initialiseLandscape(&landscape);
   
   // Seriously, who designed this thing?
	attron(COLOR_PAIR(1));
	mvprintw(0, 1, "Press F1 to exit");
	attroff(COLOR_PAIR(1));
   
   // There's almost certainly a more space-efficient way of doing this,
   // but making `landscapeArray` the size of the entire terminal is certainly
   // reliable. `safeArray` can safely be double the number of columns because
   // the landscape can never loop back on itself (I hope).
	unsigned int landscapeArray[COLS * LINES];
   unsigned int safeArray[COLS * 2];
   // Initialises the arrays to 0s; this is used to figure out how where
   // the coordinates end and useless data begins later on.
   // In MATLAB, these two lines and the lines before could be done as
   // `landscapeArray = zeros(2, COLS)`; who thought I'd ever miss MATLAB.
   for (size_t i = 0; i < COLS * LINES; i++) landscapeArray[i] = 0;
   for (size_t i = 0; i < COLS * LINES; i++) safeArray[i] = 0;
   // Used to ensure a game is never unwinnable (I'm nicer than 80s/90s
   // Sierra)
   bool validLandscape;
   // Fairly certain this is absolute arse, but it works. The game generates
   // random landscapes until one is made with at least one landing pad.
   do {
      validLandscape = createLandscape(&landscape, &landscapeArray, &safeArray);
   } while (!validLandscape);
   // Finds the ends of the two arrays. Again, this hurts a bit to look at,
   // but I'm lazy. So sue me.
   do { lASize++; } while (landscapeArray[lASize] != 0); --lASize;
   do { sASize++; } while (safeArray[sASize] != 0); --sASize;
   
   // Does what it says on the tin, really.
	createShip(&ship);
   
   // Sticks all of this onto the screen.
	refresh();
   
   // Despite what I said before, this is where the magic really happens.
   // The fabled game loop.
   do {  
      // If a direction key is entered, the jet direction is set to that key's
      // direction. If F1 is entered, the QUIT endstate is triggered. If no
      // key is entered, the jets are turned off.
      if ((ch = getch()) != ERR) {
         switch(ch) {
         case KEY_UP: jetDir = UP; break;
         case KEY_RIGHT: jetDir = RIGHT; break;
         case KEY_DOWN: jetDir = DOWN; break;
         case KEY_LEFT: jetDir = LEFT; break;
         case KEY_F(1): end = true; endType = QUIT; break;
         }
      } else jetDir = NONE;   
      // If the jets should be on, turns them on.
      if (jetDir != NONE) {
         applyJet(&ship, jetDir);
      }      
      // Like death and taxes, there's no getting away from gravity
      // and friction.
      applyGravity(&ship);
      applyFriction(&ship);
      // Figures out where the ship ought to be now.
      moveShip(&ship, lASize, landscapeArray, sASize, safeArray);
      // Updates the clock, ticking up mercilessly all the while.
      mvprintw(3, 1, "Time: %d", time++);
      // Slows the program down to human-comprehendable speed.
      nanosleep(&s, NULL);      
   } while (!end);
      
   // If a game end is triggers, tests what flavour of end it is.
   switch(endType) {
   case CRASH:      
      attron(COLOR_PAIR(2));
      mvprintw(6, COLS/3, "AW MAAAAN");
      attroff(COLOR_PAIR(2));      
      mvprintw(7, COLS/3, "Press r to restart");
      
      while ((ch = getch()) != 'r'){}
      // I'm so sorry Edsger.
      goto restart;
   case LAND:
      score = getScore(&ship, time);
      attron(COLOR_PAIR(3));
      mvprintw(6, COLS/3, "YOU LANDED");
      attroff(COLOR_PAIR(3));
      mvprintw(7, COLS/3, "Your score: %f", score);
      mvprintw(8, COLS/3, "Press r to restart");
      
      while ((ch = getch()) != 'r'){}
      goto restart;
   case QUIT:
      // Ends curses mode, else the terminal would play up afterwards.
      endwin();
      // Then calls it a night.
      return 0;
   }
}

/**
 * Initialises ncurses.
 * 
 * I couldn't decide whether it should be `initialisencurses()`, 
 * `initialiseNCURSES()` or `initialiseNcurses()`. In the end I plumped for
 * `initialisencurses()` because, whilst it does break the camelCase rule I've
 * used throughout for variables and functions, ncurses is one of those trendy
 * uncapitalised names.
 */
void initialisencurses() {
   // Starts ncurses mode.
	initscr();
   // Starts the colour functionality.
	start_color();
   // Disables line buffering.
	cbreak();
   // Keeps the program from hanging around waiting for character entry.
   nodelay(stdscr, TRUE);
   // Enables the keyboard.
	keypad(stdscr, TRUE);
   // Disables echoing user input.
	noecho();
   // Hides the cursor.
   curs_set(0);
}

/**
 * Initialises the ship's parameters.
 * 
 * @param ship the ship in question
 */
void initialiseShip(SHIP* ship) {
   // The ship displays thusly: *
	ship->height = 1;
	ship->width = 1;
	ship->graphics.bod = '*';
   
   // The ship starts at the centre top of the terminal.
	ship->starty = 2;	
	ship->startx = (COLS - ship->width)/2;
	ship->xF = ship->startx;	
	ship->yF = ship->starty;
	ship->lastx = ship->startx;	
	ship->lasty = ship->starty;
   
   // The ship starts fueled, and with a little bit of upward momentum
   // to keep the player from getting caught too unawares and careening
   // straight into a cliff (still happens).
   ship->fuel = STARTING_FUEL;
   ship->yMomentum = -0.5f;
}

/**
 * Initialises the landscape's parameters.
 * 
 * @param landscape the landscape in question
 */
void initialiseLandscape(LANDSCAPE* landscape) {
   // The landscape spans the width of the console window and starts halfway
   // down it.
	landscape->height = LINES - 7;
	landscape->width = COLS;
	landscape->starty = LINES / 2;
	landscape->startx = 0;
	
   // Landscape graphics
	landscape->graphics.li = '/';
	landscape->graphics.su = '|';
	landscape->graphics.sd = '|';
	landscape->graphics.rd = '\\';
	landscape->graphics.pl = '_';
}

/**
 * Creates the ship in the game world.
 * 
 * @param ship the ship in question
 */
void createShip(SHIP* ship) {	
	int x, y;
	
	x = ship->startx;
	y = ship->starty;
   
   mvaddch(y, x, ship->graphics.bod | A_BOLD);	
   
   ship->xMomentum = 0;
   ship->yMomentum = 0;
}

/**
 * Creates the landscape in the game world.
 * 
 * @param landscape the landscape in question
 * @param landscapeArray[] the array of all the coordinates of the landscape
 * components
 * @param safeArray[] the array of all the coordinates of the landing pad
 * components
 * @return true if the landscape is valid (has at least one landing pad), false
 * if not
 */
bool createLandscape(LANDSCAPE* landscape, unsigned int landscapeArray[], 
                                           unsigned int safeArray[]) {
   int x = landscape->startx;
	int y = landscape->starty;
   int dir;
   bool landingPad = false;
   
	int w = landscape->width;
	int h = landscape->height;
   
   // Wipes the previous landscape attempts, if there were any.
   for(int j = y; j <= y + h; ++j)
      for(int i = x; i <= x + w; ++i)
         mvaddch(j, i, ' ');
   
   // Seeds C's psuedorandom number generator.
   srand(time(NULL));
   
   // Starts the landscape off with a fruity left incline.
   landscapeArray[0] = x; landscapeArray[1] = y;
   mvaddch(y--, x++, landscape->graphics.li);
   int lastPiece = LEFT_INCLINE;
   // Indexes for adding coordinates to the two arrays.
   size_t lA = 2; size_t sA = 0;
   
   for (; x <= landscape->width; x++) {
      // Gives a value between 0-4, or the four valid landscape
      // component identifiers.
      dir = (rand() % 5);
      
      // Places the appropriate piece at the correct point, altering the
      // y-index if necessary; if placing a piece would result in the
      // landscape going beyond the top and bottom boundaries set for it,
      // `x` is rewound and a new piece selected.
      switch(dir) {
      case LEFT_INCLINE:
         if (y - 1 > 10) {
            switch(lastPiece) {
            case STRAIGHT_DOWN:
               x++; y--; break;
            case RIGHT_DECLINE:
               y--; break;                
            }
            landscapeArray[lA++] = x; landscapeArray[lA++] = y;
            mvaddch(y--, x, landscape->graphics.li);
         } else --x;  
         break;
      case STRAIGHT_UP:
         if ((y - 1 > 10) && (lastPiece != STRAIGHT_DOWN)) {
            switch(lastPiece) {
            case RIGHT_DECLINE:
               y--; break;            
            }
            landscapeArray[lA++] = x; landscapeArray[lA++] = y;
            mvaddch(y--, x--, landscape->graphics.su);
         } else --x;  
         break;      
      case STRAIGHT_DOWN:
         if ((y + 1 < (LINES - 2)) && (lastPiece != STRAIGHT_UP)) {
            switch(lastPiece) {
            case LEFT_INCLINE:
               y++; break;
            case RIGHT_DECLINE:
               x--; break;
            case PLATEAU:
               y++; break;               
            }
            landscapeArray[lA++] = x; landscapeArray[lA++] = y;
            mvaddch(y++, x--, landscape->graphics.sd);
         } else --x;       
         break;   
      case RIGHT_DECLINE:
         if (y + 1 < (LINES - 2)) {
            switch(lastPiece) {
            case LEFT_INCLINE:
               y++; break;
            case STRAIGHT_UP:
               x++; break;
            case STRAIGHT_DOWN:
               x++; break;
            case PLATEAU:
               y++; break;               
            }
            landscapeArray[lA++] = x; landscapeArray[lA++] = y;
            mvaddch(y++, x, landscape->graphics.rd);
         } else --x;
         break;
      case PLATEAU:
         switch(lastPiece) {
         case STRAIGHT_DOWN:
            x++; y--; 
            break;
         case RIGHT_DECLINE:
            y--; break;               
         }
         
         // For every plateau, there is a chance it will form a landing pad.
         if ((rand() % CHANCE_OF_LANDING_PAD) == 0) {
            attron(COLOR_PAIR(1));
            for (int i = 0; i <= 3; i++) {
               safeArray[sA++] = x; safeArray[sA++] = y;
               landscapeArray[lA++] = x; landscapeArray[lA++] = y;
               mvaddch(y, x++, landscape->graphics.pl | A_BOLD);
            }
            x--;
            attroff(COLOR_PAIR(1));
            landingPad = true;
         } else mvaddch(y, x, landscape->graphics.pl);
         break; 
      }
      lastPiece = dir;
   }
   
   // Returns whether the generated landscape has a landing pad or not.
   return landingPad;
}

/**
 * Applies the thrust of the jet to the ship.
 * 
 * If the ship is out of fuel, the jets won't fire.
 * 
 * @param ship the ship in question
 * @param dir the thrust direction
 */
void applyJet(SHIP* ship, unsigned int dir) {
   if (ship->fuel > 0) {
      ship->fuel--;
      
      // Shows the player their remaining fuel balance.
      if (ship->fuel == 0)
         attron(COLOR_PAIR(2));
      mvprintw(2,1,"Fuel: %d", ship->fuel);
      if (ship->fuel == 0)
         attroff(COLOR_PAIR(2));

      // Adds a bit of momentum in the chosen direction.
      switch(dir) {
      case UP:
         if(ship->yMomentum >= -1) ship->yMomentum -= 0.2f; break;
      case RIGHT:
         if(ship->xMomentum <= 1) ship->xMomentum += 0.2f; break;
      case DOWN:
         if(ship->yMomentum <= 1) ship->yMomentum += 0.2f; break;
      case LEFT:
         if(ship->xMomentum >= -1) ship->xMomentum -= 0.2f; break;
      }
   }
}

/**
 * Applies the relentless march of gravity to the ship.
 * 
 * @param ship the ship in question
 */
void applyGravity(SHIP* ship) {
   if (ship->yMomentum <= TERMINAL_VELOCITY) ship->yMomentum += 0.05f;
}

/**
 * Applies the equally relentless force of friction to the ship.
 * 
 * @param ship the ship in question
 */
void applyFriction(SHIP* ship) {
   // Takes off a bit of the up/down speed.
   if (ship->yMomentum > 0.0f)
      ship->yMomentum -= 0.025f;
   else 
      ship->yMomentum += 0.025f;
   
   // Then does the same for the left/right speed.
   if (ship->xMomentum > 0.0f)
      ship->xMomentum -= 0.025f;
   else 
      ship->xMomentum += 0.025f;
}

/**
 * Moves the ship within the game world.
 * 
 * @param ship the ship in question
 * @param lASize the length of useful data in `landscapeArray`
 * @param landscapeArray the array of coordinates for the components of the
 * landscape
 * @param sASize the length of useful data in `safeArray`
 * @param safeArray the array of coordinates for the components of the
 * landing pad(s)
 */
void moveShip(SHIP* ship, size_t lASize, unsigned int landscapeArray[], 
                          size_t sASize, unsigned int safeArray[]) {
   // Adds the ships momentum to its current location.
   ship->xF += ship->xMomentum;
   ship->yF += ship->yMomentum;
   
   // Displays the momentum for the player.
	mvprintw(1,1,"Momentum: %f,%f", ship->xMomentum, ship->yMomentum);
   	
   // Rounds the floating point coordinates to the nearest integer coords
	signed int x = round(ship->xF);
	signed int y = round(ship->yF);
   // Gets the coordinates the ship used to be at.
   signed int prevX = ship->lastx;
   signed int prevY = ship->lasty;
   
   // Deletes the ship from its old coordinates.
   mvaddch(prevY, prevX, ' ');
   
   mvprintw(11, 1, "%d, %d", safeArray[0], safeArray[1]);
   mvprintw(12, 1, "%d, %d", safeArray[8], safeArray[9]);
   mvprintw(13, 1, "%d, %d", safeArray[16], safeArray[17]);
   mvprintw(15, 1, "%d, %d", x, y);
   
   // Runs though the `landscapeArray` to see the the ship's new location
   // means a collision with any landscape features.
   for (size_t i = 0; i <= lASize; i++) {
      if ((landscapeArray[i] == x) && (landscapeArray[++i] == y)) {
         // If there is a collision, runs through the `safeArray` too see if
         // the feature is a landing pad.
         for (int j = 0; j <= sASize; j++) {
            if ((safeArray[j] == x) && (safeArray[++j] == y)) {
               // If it is a landing pad, and the user has invincibility turned
               // on, lands the ship.
               if (invincible) {
                  end = true;
                  endType = LAND;      
               } else {
                  // Else, if the player comes in sufficiently slowly, lands the
                  // ship.
                  if (ship->yMomentum <= 0.8f) {
                     end = true;
                     endType = LAND;
                  }
               }
            }
         }
         // Otherwise, the player crashes and burns.
         if (!end) {
            end = true;
            endType = CRASH;
         }
      }
   }
   
   // Colours the ship in red if it's crashed.
   if (endType == CRASH)
      attron(COLOR_PAIR(2));
   else if (endType == LAND)
      attron(COLOR_PAIR(3));
   // Draws the ship at its new coordinates.
   mvaddch(y, x, ship->graphics.bod | A_BOLD);	
   if (endType == CRASH)
      attroff(COLOR_PAIR(2));
   else if (endType == LAND)
      attroff(COLOR_PAIR(3));
   
   // If the ship has exceeded the top of the screen, adds a small arrow 
   // to show the column the ship is in.
   if (y < 0) {
      for (int i = 0; i <= COLS; i++)
         mvaddch(0, i, ' ');
      mvaddch(0, x, '^' | A_BOLD);
      attron(COLOR_PAIR(1));
      mvprintw(0, 1, "Press F1 to exit");
      attroff(COLOR_PAIR(1));
   } else {
      for (int i = 0; i <= COLS; i++)
         mvaddch(0, i, ' ');
      attron(COLOR_PAIR(1));
      mvprintw(0, 1, "Press F1 to exit");
      attroff(COLOR_PAIR(1));
   }
   
   // Stores the new coordinates for comparison next time the function is run.
   ship->lastx = x;
   ship->lasty = y;
   
   // Pushes the new ship to the terminal.
	refresh();
} 

/**
 * Displays the lovely ASCII lunar lander I nicked.
 */
void displayIntro() {
   clear();
   // I assumed I would be able to fit all this in one line-broken `printw()`
   // statement, but it didn't seem to want to play ball.
   printw("                     _  _     ____________.--.      _____  ___  ___  ___\n");
   printw("                  |\\|_|//_.-\"\" .'    \\   /|  |     |     ||   ||   ||   |\n");
   printw("                  |.-\"\"\"-.|   /       \\_/ |  |     | | | || | || | || | |\n");
   printw("                  \\  ||  /| __\\_____________ |     |_|_|_||___||___||_|_|\n");
   printw("                  _\\_||_/_| .-\"\"            \"\"-.  __      by rumps\n");
   printw("                .' '.    \\//                    \".\\/\n");
   printw("                ||   '. >()_                     |()<     press 'a' to start\n");
   printw("                ||__.-' |/\\ \\                    |/\\\n");
   printw("                   |   / \"|  \\__________________/.\"\"\n");
   printw("                  /   //  | / \\ \"-.__________/  /\\\n");
   printw("               ___|__/_|__|/___\\___\".______//__/__\\\n");
   printw("              /|\\     [____________] \\__/         |\n");
   printw("             //\\ \\     |  |=====| |   /\\\\         |\\\\\n");
   printw("            // |\\ \\    |  |=====| |   | \\\\        | \\\\        ____...____..\n");
   printw("          .//__| \\ \\   |  |=====| |   | |\\\\       |--\\\\---\"\"\"\"     .   \n");
   printw("_____....-//___|  \\_\\  |  |=====| |   |_|_\\\\      |___\\\\    .              \n");
   printw(" .      .//-.__|_______|__|_____|_|_____[__\\\\_____|__.-\\\\      .     .    ...\n");
   printw("        //        //        /          \\ `-_\\\\/         \\\\          .....:::\n");
   printw("  -... //     .  / /       /____________\\    \\\\       .  \\ \\     .          \n");
   printw("      //   .. .-/_/-.                 .       \\\\        .-\\_\\-.              \n");
   printw("     / /      '-----'           .             \\ \\      '._____.'         .\n");
   printw("  .-/_/-.         .                          .-\\_\\-.                          .\n");
   printw(" '._____.'                            .     '._____.'                       .....\n");
   printw(" JRO      ......           . ..   (ASCII nicked from https://www.ascii.co.uk)\n");
}

/**
 * Works out the player's score upon successful landing.
 * 
 * The specification states "score is calculated using some heuristic based on 
 * time used and fuel remaining." Heuristic sounds fancy, and so is the method
 * of working out the score.
 * 
 * @param ship the ship in question
 * @return the score
 */
double getScore(SHIP* ship, unsigned int time) {
   unsigned int fuel = ship->fuel;
   
   switch(rand() % 3) {
   case 0:
      return (double)(cos(fuel) * (time / 2));
   case 1:
      return (double)(sin(time * 2) - (time / 3) + fuel);
   case 2:
      return (double)(tan(tan(fuel + time)) + 2);
   }
   return rand() % 1000;
}
