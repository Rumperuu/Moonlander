#ifndef MOONLANDER_H_
#define MOONLANDER_H_

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
 * Header file for `moonlander.c`.                                            
 */

#include <stdbool.h>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// I almost think I should start looking into enums, rather than the
// world's lengthiest macro lists all the time.
// Macros for ship jet directions.
#define NONE 0
#define UP 1
#define RIGHT 2
#define DOWN 3
#define LEFT 4

// Macros for the components of the landscape.
#define LEFT_INCLINE 0
#define STRAIGHT_UP 1
#define STRAIGHT_DOWN 2
#define RIGHT_DECLINE 3
#define PLATEAU 4

// Macros for the type of game end.
#define NONE 0
#define CRASH 1
#define LAND 2
#define QUIT 3

// Macros for setting difficulty.
#define STARTING_FUEL 900
#define CHANCE_OF_LANDING_PAD 3

// Macros for the ship.
#define TERMINAL_VELOCITY 0.9f

// Global variables aren't the best, but I feel like I can
// get away with a few here; it saves so very much fannying 
// around with pointers.
bool end = false;
unsigned int endType = NONE;
struct timespec_t s;
// Dirty cheat(s).
bool invincible = false;

// The bits and bobs that make up the landscape.
typedef struct _win_landscape_struct {
	chtype 	li, su, sd, rd, pl;
}WIN_LANDSCAPE;

// The landscape 'class'
typedef struct _WIN_landscape_struct {
	int startx, starty;
   int height, width;
   int landingPoints;
   WIN_LANDSCAPE graphics;
}LANDSCAPE;

// Same again, but with the ship.
typedef struct _win_ship_struct {
	chtype 	bod;
}WIN_SHIP;

// And the ship 'class'
typedef struct _WIN_ship_struct {
	int startx, starty;
   int lastx, lasty;
   int fuel;
   float xF, yF;
   int x, y;
	int height, width;
   float xMomentum, yMomentum;
	WIN_SHIP graphics;
}SHIP;

// I don't profess to know how `nanosleep()` works, but I nicked this
// off of SO and it seems to do the job, even if Geany gives me an
// 'implicit declaration' warning.
struct timespec_t {
    time_t tv_sec;        /* seconds */
    long   tv_nsec;       /* nanoseconds */
};

// Initialisation functions.
void initialisencurses();
void initialiseShip(SHIP* ship);
void initialiseLandscape(LANDSCAPE* landscape);

// Creation functions.
void createShip(SHIP* ship);
bool createLandscape();

// Physics application functions.
void applyJet(SHIP* ship, unsigned int dir);
void applyGravity(SHIP* ship);
void applyFriction(SHIP* ship);

// Ship movement function. Includes collision detection.
void moveShip(SHIP* ship, size_t lASize, unsigned int landscapeArray[],
                          size_t sASize, unsigned int safeArray[]);
                          
// Introduction display function.
void displayIntro();

// Arcane magic.
double getScore(SHIP* ship, unsigned int time);

#endif /* MOONLANDER_H_ */
