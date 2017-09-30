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
		char Symbol;

		int Health;
		int Energy;

		Actor *Target = nullptr;

		void ChaseTarget() {
			if(Target == nullptr) { return; }
			if(X < Target->X) {
				X++;
			}
			if(X > Target->X) {
				X--;
			}
			if(Y < Target->Y) {
				Y++;
			}
			if(Y > Target->Y) {
				Y--;
			}
		}
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
	srand(time(nullptr)); //Seed random

	//Set core variables
	int MaxRows = 24;
	int MaxCols = 50;
	int InKey;
	bool Quit = false;

	//Terminal setup and initialization
	InitializeTerminal();
	//Check that the terminal is big enough
	if(getmaxx(stdscr)<MaxCols || getmaxy(stdscr)<MaxRows) {
		mvprintw(0,0,"Please use a larger terminal for this program.\nPress any key to continue.");
		cbreak();
		getch();
		endwin();
	}

	//Set up UI "windows"
	WINDOW * MapWindow = newwin(25,50,0,0);
	WINDOW * MsgWindow = newwin(25,30,0,50);
	refresh();

	//Debug boundaries
	box(MapWindow,0,0);
	box(MsgWindow,0,0);
	wrefresh(MapWindow);
	wrefresh(MsgWindow);

	//Wait for keypress
	getch();
	//clean screen
	clear();
	refresh();

	//Initialize map
	Tile Map[MaxRows][MaxCols];

	//Make map borders
	for(int Y=0; Y<MaxRows; Y++) {
		Map[Y][0].SetWall();
		Map[Y][MaxCols-1].SetWall();
	}
	for(int X=0; X<MaxCols; X++) {
		Map[0][X].SetWall();
		Map[MaxRows-1][X].SetWall();
	}

	//Make DoomRL-style "rooms" map
	int MadeWalls = 0; //How many walls we've finished
	int WallAttempts = 0; //How many walls we've tried to make
	while(MadeWalls<10 || WallAttempts<100) {
		WallAttempts++;
		bool DoBuild = true;
		//Pick a random spot
		int StartY = irandom(2,MaxRows-2);
		int StartX = irandom(2,MaxCols-2);
		//Abort and increment if spot is a door/wall
		if(Map[StartY][StartX].IsWall() == true || Map[StartY][StartX].IsDoor() == true) {
			Map[StartY][StartX].SetDoor();
			DoBuild = false;
			MadeWalls++;
		}
		//Check sufficient space for "nice" wall placement
		for(int CheckAround=1; CheckAround<=3; CheckAround++) {
			if(Map[StartY-CheckAround][StartX].IsWall() == true || Map[StartY-CheckAround][StartX].IsDoor() == true ||
			   Map[StartY+CheckAround][StartX].IsWall() == true || Map[StartY+CheckAround][StartX].IsDoor() == true ||
			   Map[StartY][StartX-CheckAround].IsWall() == true || Map[StartY][StartX-CheckAround].IsDoor() == true ||
			   Map[StartY][StartX+CheckAround].IsWall() == true || Map[StartY][StartX+CheckAround].IsDoor() == true) {
				DoBuild=false;
				if(CheckAround>=2) {
					Map[StartY][StartX].SetWall();
				}
				break;
			}
		}
		//Actually build the walls outward from door location
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

	//Initialize player
	Actor Player;
	do {
		Player.Y = irandom(5,MaxRows-5);
		Player.X = irandom(5,MaxCols-5);
	}
	while(Map[Player.Y][Player.X].CanWalk == false);
	Player.Symbol = '@';
	Player.Health = 100;
	Player.Energy = 2000;

	//Initialize monsters
	Actor Monsters[10];
	for(int i=0; i<=9; i++) {
		do {
			Monsters[i].Y = irandom(5,MaxRows-5);
			Monsters[i].X = irandom(5,MaxCols-5);
		}
		while(Map[Monsters[i].Y][Monsters[i].X].CanWalk == false);
		Monsters[i].Symbol = 'G';
		Monsters[i].Health = 100;
		Monsters[i].Energy = 1000;
	}

	//--------------------------------
	//Main loop
	//--------------------------------
	while(!Quit) {
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
				if(Map[CheckY][CheckX].IsWall() == true || Map[CheckY][CheckX].IsDoor() == true) {
					Map[CheckY][CheckX].Seen = true;
				}
				Map[CheckY][CheckX].Visible = true;
				if(Map[CheckY][CheckX].BlocksVision == true) { break; }
			}
		}

		//Render map
		for(int Y=0; Y<MaxRows; Y++) {
			for(int X=0; X<MaxCols; X++) {
				if(Map[Y][X].Seen == true || Map[Y][X].Visible == true) {
					mvwaddch(MapWindow,Y,X,Map[Y][X].Symbol);
				}
				else {
					mvwaddch(MapWindow,Y,X,' ');
				}
			}
		}

		//Render actors
		for(int i=0; i<=9; i++) {
			if(Map[Monsters[i].Y][Monsters[i].X].Visible == true) {
				mvwaddch(MapWindow,Monsters[i].Y,Monsters[i].X,Monsters[i].Symbol);
				Monsters[i].Target = &Player;
			}
		}
		mvwaddch(MapWindow,Player.Y,Player.X,Player.Symbol);
		wrefresh(MapWindow);

		//Render Messages/Status
		wmove(MsgWindow,0,0);
		wvline(MsgWindow,0,MaxRows);
		mvwprintw(MsgWindow,0,1,"Health: %i",Player.Health);
		mvwprintw(MsgWindow,1,1,"Energy: %i",Player.Energy);
		wrefresh(MsgWindow);

		//Player input checks
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
				Quit=true;
				break;
			default:
				break;
		}

		//Monsters get their turn
		for(int i=0; i<=9; i++) {
			Monsters[i].ChaseTarget();
		}
	}
	endwin();

	return 0;
}
