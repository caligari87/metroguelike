#include <ncurses.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include "random.cpp"

#define PI 3.14159265

class Tile {
	public:
		//Set everything to "floor" initially
		bool CanWalk=true;
		bool BlocksVision=false;
		char Symbol='.';

		bool Seen=false;
		bool Visible=false;
		bool VisibleChecked=false;

		void SetWall() {
			CanWalk = false;
			BlocksVision = true;
			Symbol = '#';
		}

		void SetFloor() {
			CanWalk = true;
			BlocksVision = false;
			Symbol = '.';
		}

		void SetDoor() {
			CanWalk = true;
			BlocksVision = true;
			Symbol = '+';
		}

		bool IsDoor() {
			return(CanWalk && BlocksVision);
		}
		
		bool IsWall() {
			return(!CanWalk && BlocksVision && Symbol=='#');
		}
};


class Actor {
	public:
		int X,Y;
		int OldX,OldY;
		char Symbol;

		int Health;
		int Energy;
};

void InitializeTerminal() {
	initscr(); //Initialize ncurses screen
	curs_set(0); //Turn off cursor
	keypad(stdscr, true); //Enable the keypad
	noecho(); //Don't echo getch characters
	//box(stdscr,0,0);
	//getmaxyx(stdscr,MaxRows,MaxCols); //Get maximum window size
}

int main() {
	srand(time(NULL));

	int MaxRows = 25;
	int MaxCols = 50;
	int InKey;
	bool quit = false;

	Actor Player;
	Player.Y = MaxRows/2;
	Player.X = MaxCols/2;
	Player.Symbol = '@';
	Player.Health = 100;
	Player.Energy = 2000;

	Tile Map[MaxRows][MaxCols];
	//Map[][].SetFloor();

	InitializeTerminal();
	//Check that the terminal is big enough
	if(getmaxx(stdscr)<MaxCols || getmaxy(stdscr)<MaxRows) {
		mvprintw(0,0,"Please use a larger terminal for this program.\nPress any key to continue.");
		cbreak();
		getch();
		endwin();
	}

	WINDOW * MapWindow = newwin(25,50,0,0);
	WINDOW * MsgWindow = newwin(25,30,0,50);
	refresh();

	box(MapWindow,0,0);
	box(MsgWindow,0,0);
	wrefresh(MapWindow);
	wrefresh(MsgWindow);

	getch();
	//clean screen
	clear();
	refresh();

	//Make map borders
	for(int Y=0; Y<MaxRows; Y++) {
		Map[Y][0].SetWall();
		Map[Y][MaxCols-1].SetWall();
	}
	for(int X=0; X<MaxCols; X++) {
		Map[0][X].SetWall();
		Map[MaxRows-1][X].SetWall();
	}

	/*Make random map
	for(int i=0; i<(MaxRows*MaxCols*0.25); i++) {
	int Y=irandom(0,MaxRows-1);
	int X=irandom(0,MaxCols-1);
	Map[Y][X].SetWall();
	}*/
	
	//Make DoomRL-style "rooms" map
	int MadeWalls = 0; //How many walls we've finished
	int WallAttempts = 0; //How many walls we've tried to make
	while(MadeWalls<10 || WallAttempts<100) {
		WallAttempts++;
		bool DoBuild = true; //Actually build the walls
		int StartY = irandom(2,MaxRows-2);
		int StartX = irandom(2,MaxCols-2);
		if(Map[StartY][StartX].IsWall() == true || Map[StartY][StartX].IsDoor() == true) {
			Map[StartY][StartX].SetDoor();
			DoBuild = false;
			MadeWalls++;
		}
		for(int CheckAround=1; CheckAround<=3; CheckAround++) {
			if(Map[StartY-CheckAround][StartX].IsWall() == true || Map[StartY-CheckAround][StartX].IsDoor() == true ||
			   Map[StartY+CheckAround][StartX].IsWall() == true || Map[StartY+CheckAround][StartX].IsDoor() == true ||
			   Map[StartY][StartX-CheckAround].IsWall() == true || Map[StartY][StartX-CheckAround].IsDoor() == true ||
			   Map[StartY][StartX+CheckAround].IsWall() == true || Map[StartY][StartX+CheckAround].IsDoor() == true) {
			   	DoBuild=false;
			   	break;
			}
		}		
		if(DoBuild==true) {
			Map[StartY][StartX].SetDoor();
			if(irandom(0,1)==0) {
				for(int X=StartX-1; X>0; X--) {
					if(Map[StartY][X].IsWall()==true || Map[StartY][X].IsDoor() == true) { break; }
					Map[StartY][X].SetWall();
				}
				for(int X=StartX+1; X<MaxCols; X++) {
					if(Map[StartY][X].IsWall()==true || Map[StartY][X].IsDoor() == true) { break; }
					Map[StartY][X].SetWall();
				}
				MadeWalls++;
			}
			else {
				for(int Y=StartY-1; Y>0; Y--) {
					if(Map[Y][StartX].IsWall()==true || Map[Y][StartX].IsDoor() == true) { break; }
					Map[Y][StartX].SetWall();
				}
				for(int Y=StartY+1; Y<MaxRows; Y++) {
					if(Map[Y][StartX].IsWall()==true || Map[Y][StartX].IsDoor() == true) { break; }
					Map[Y][StartX].SetWall();
				}
				MadeWalls++;
			}
		}
	}

	while(!quit) {
		//Set map to all nonvisible;
		for(int Y=0; Y<MaxRows; Y++) {
			for(int X=0; X<MaxCols; X++) {
				Map[Y][X].Visible = false;
				Map[Y][X].VisibleChecked = false;
			}
		}

		//Player FOV check
		for(double Angle=0;Angle<360;Angle+=0.1) {
			for(int Dist=1;Dist<25;Dist++) {
				int CheckY = (double)0.5 + Player.Y + (Dist*sin(Angle*PI/180));
				int CheckX = (double)0.5 + Player.X + (Dist*cos(Angle*PI/180));
				if(CheckX<0 || CheckX>MaxCols || CheckY<0 or CheckY>MaxRows) { break; }
				//if(Map[CheckY][CheckX].VisibleChecked == true) { break; }
				Map[CheckY][CheckX].Visible = true;
				//Map[CheckY][CheckX].VisibleChecked = true;
				if(Map[CheckY][CheckX].BlocksVision == true) { break; }
			}
		}

		//Render map
		for(int Y=0; Y<MaxRows; Y++) {
			for(int X=0; X<MaxCols; X++) {
				if(Map[Y][X].Visible == true) {
					mvwaddch(MapWindow,Y,X,Map[Y][X].Symbol);
				}
				else { mvwaddch(MapWindow,Y,X,' '); }
			}
		}
		mvwaddch(MapWindow,Player.Y,Player.X,Player.Symbol);
		wrefresh(MapWindow);

		//Render Messages/Status
		wmove(MsgWindow,0,0);
		wvline(MsgWindow,0,25);
		mvwprintw(MsgWindow,0,1,"Health: %i",Player.Health);
		mvwprintw(MsgWindow,1,1,"Energy: %i",Player.Energy);
		wrefresh(MsgWindow);
		
		
		//Player.OldY = Player.Y;
		//Player.OldX = Player.X;
		InKey=getch();
		switch(InKey) {
			case(KEY_UP):
				if(Map[Player.Y-1][Player.X].CanWalk) {
					Player.Y--;
					Player.Energy--;
				}
				break;
			case(KEY_DOWN):
				if(Map[Player.Y+1][Player.X].CanWalk) {
					Player.Y++;
					Player.Energy--;
				}
				break;
			case(KEY_LEFT):
				if(Map[Player.Y][Player.X-1].CanWalk) {
					Player.X--;
					Player.Energy--;
				}
				break;
			case(KEY_RIGHT):
				if(Map[Player.Y][Player.X+1].CanWalk) {
					Player.X++;
					Player.Energy--;
				}
				break;
			case('Q'):
				quit=true;
				break;
			default:
				break;
		}
	}
	endwin();

	return 0;
}
