const double PI = 3.14159265;
const int MaxRows = 24;
const int MaxCols = 50;

#include <ncurses.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include "random.cpp"

#define PI 3.14159265
const int MaxRows = 24;
const int MaxCols = 50;

class TileData {
	public:
		//Set everything to "floor" initially
		bool CanWalk=true;
		bool BlocksVision=false;
		char Symbol='.';

		bool Seen=false;
		bool Visible=false;
};

class MapData {
	public:
		TileData Tiles[MaxRows*MaxCols];

		bool IsWall(int row, int col) {
			return(!Tiles[row+col*MaxRows].CanWalk && Tiles[row+col*MaxRows].BlocksVision && Tiles[row+col*MaxRows].Symbol=='#');
		}

		bool IsDoor(int row, int col) {
			return(Tiles[row+col*MaxRows].CanWalk && Tiles[row+col*MaxRows].BlocksVision);
		}

		bool CanWalk(int row, int col) {
			return(Tiles[row+col*MaxRows].CanWalk);
		}

		bool BlocksVision(int row, int col) {
			return(Tiles[row+col*MaxRows].BlocksVision);
		}

		bool Seen(int row, int col) {
			return(Tiles[row+col*MaxRows].Seen);
		}

		bool Visible(int row, int col) {
			return(Tiles[row+col*MaxRows].Visible);
		}

		char Symbol(int row, int col) {
			return(Tiles[row+col*MaxRows].Symbol);
		}

		void SetWall(int row, int col) {
			Tiles[row+col*MaxRows].CanWalk = false;
			Tiles[row+col*MaxRows].BlocksVision = true;
			Tiles[row+col*MaxRows].Symbol = '#';
		}

		void SetFloor(int row, int col) {
			Tiles[row+col*MaxRows].CanWalk = true;
			Tiles[row+col*MaxRows].BlocksVision = false;
			Tiles[row+col*MaxRows].Symbol = '.';
		}

		void SetDoor(int row, int col) {
			Tiles[row+col*MaxRows].CanWalk = true;
			Tiles[row+col*MaxRows].BlocksVision = true;
			Tiles[row+col*MaxRows].Symbol = '+';
		}

		void SetAllNotVisible() {
			for(int i=0; i<MaxRows*MaxCols; i++) {
				Tiles[i].Visible = false;
			}
		}

		void SetVisibleState(int row, int col, bool value) {
			Tiles[row+col*MaxRows].Visible = value;
		}

		void SetSeenState(int row, int col, bool value) {
			Tiles[row+col*MaxRows].Seen = value;
		}
};

class Actor {
	public:
		int X,Y;
		char Symbol;

		int Health;
		int Energy;

		MapData LocalMap;

		Actor *Target = nullptr;

		void TryMove(int NewY, int NewX) {
			if(LocalMap.IsWall(NewY,NewX) == false) {
				Y = NewY;
				X = NewX;
			}
		}

		void ChaseTarget() {
			if(Target == nullptr) { return; }
			if(X < Target->X) {
				TryMove(Y,X+1);
			}
			if(X > Target->X) {
				TryMove(Y,X-1);
			}
			if(Y < Target->Y) {
				TryMove(Y+1,X);
			}
			if(Y > Target->Y) {
				TryMove(Y-1,X);
			}
		}
};

void InitializeTerminal() {
	initscr(); //Initialize ncurses screen
	curs_set(0); //Turn off cursor
	keypad(stdscr, true); //Enable the keypad
	//noecho(); //Don't echo getch characters
	//box(stdscr,0,0);
	//getmaxyx(stdscr,MaxRows,MaxCols); //Get maximum window size
}

int main() {
	srand(time(nullptr)); //Seed random

	//Set core variables
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
	MapData Map;

	//Make map borders
	for(int Y=0; Y<MaxRows; Y++) {
		Map.SetWall(Y,0);
		Map.SetWall(Y,MaxCols-1);
	}
	for(int X=0; X<MaxCols; X++) {
		Map.SetWall(0,X);
		Map.SetWall(MaxRows-1,X);
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
		if(Map.IsWall(StartY,StartX) == true || Map.IsDoor(StartY,StartX) == true) {
			Map.SetDoor(StartY,StartX);
			DoBuild = false;
			MadeWalls++;
		}
		//Check sufficient space for "nice" wall placement
		for(int CheckAround=1; CheckAround<=3; CheckAround++) {
			if(Map.IsWall(StartY-CheckAround,StartX) == true || Map.IsDoor(StartY-CheckAround,StartX) == true ||
			   Map.IsWall(StartY+CheckAround,StartX) == true || Map.IsDoor(StartY+CheckAround,StartX) == true ||
			   Map.IsWall(StartY,StartX-CheckAround) == true || Map.IsDoor(StartY,StartX-CheckAround) == true ||
			   Map.IsWall(StartY,StartX+CheckAround) == true || Map.IsDoor(StartY,StartX+CheckAround) == true) {
				DoBuild=false;
				if(CheckAround>=2) {
					Map.SetWall(StartY,StartX);
				}
				break;
			}
		}
		//Actually build the walls outward from door location
		if(DoBuild==true) {
			Map.SetDoor(StartY,StartX);
			if(irandom(0,1)==0) {
				for(int X=StartX-1; X>0; X--) {
					if(Map.IsWall(StartY,X)==true || Map.IsDoor(StartY,X) == true) { break; }
					Map.SetWall(StartY,X);
				}
				for(int X=StartX+1; X<MaxCols; X++) {
					if(Map.IsWall(StartY,X)==true || Map.IsDoor(StartY,X) == true) { break; }
					Map.SetWall(StartY,X);
				}
				MadeWalls++;
			}
			else {
				for(int Y=StartY-1; Y>0; Y--) {
					if(Map.IsWall(Y,StartX)==true || Map.IsDoor(Y,StartX) == true) { break; }
					Map.SetWall(Y,StartX);
				}
				for(int Y=StartY+1; Y<MaxRows; Y++) {
					if(Map.IsWall(Y,StartX)==true || Map.IsDoor(Y,StartX) == true) { break; }
					Map.SetWall(Y,StartX);
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
	while(Map.CanWalk(Player.Y,Player.X) == false);
	Player.Symbol = '@';
	Player.Health = 100;
	Player.Energy = 2000;
	Player.LocalMap = Map;

	//Initialize monsters
	Actor Monsters[10];
	for(int i=0; i<=9; i++) {
		do {
			Monsters[i].Y = irandom(5,MaxRows-5);
			Monsters[i].X = irandom(5,MaxCols-5);
		}
		while(Map.CanWalk(Monsters[i].Y,Monsters[i].X) == false);
		Monsters[i].Symbol = 'G';
		Monsters[i].Health = 100;
		Monsters[i].Energy = 1000;
		Monsters[i].LocalMap = Map;
	}

	//--------------------------------
	//Main loop
	//--------------------------------
	while(!Quit) {
		Map.SetAllNotVisible();
		//Player FOV check
		for(double Angle=0;Angle<360;Angle+=0.1) {
			for(int Dist=1;Dist<25;Dist++) {
				int CheckY = (double)0.5 + Player.Y + (Dist*sin(Angle*PI/180));
				int CheckX = (double)0.5 + Player.X + (Dist*cos(Angle*PI/180));
				if(CheckX<0 || CheckX>MaxCols || CheckY<0 or CheckY>MaxRows) { break; }
				if(Map.IsWall(CheckY,CheckX) == true || Map.IsDoor(CheckY,CheckX) == true) {
					Map.SetSeenState(CheckY,CheckX, true);
				}
				Map.SetVisibleState(CheckY,CheckX,true);
				if(Map.BlocksVision(CheckY,CheckX) == true) { break; }
			}
		}

		//Render map
		wclear(MapWindow);
		for(int Y=0; Y<MaxRows; Y++) {
			for(int X=0; X<MaxCols; X++) {
				if(Map.Seen(Y,X) == true || Map.Visible(Y,X) == true) {
					mvwaddch(MapWindow,Y,X,Map.Symbol(Y,X));
				}
			}
		}

		//Render actors
		for(int i=0; i<=9; i++) {
			if(Map.Visible(Monsters[i].Y,Monsters[i].X) == true) {
				mvwaddch(MapWindow,Monsters[i].Y,Monsters[i].X,Monsters[i].Symbol);
				Monsters[i].Target = &Player;
			}
		}
		mvwaddch(MapWindow,Player.Y,Player.X,Player.Symbol);
		wrefresh(MapWindow);

		//Render Messages/Status
		wclear(MsgWindow);
		wmove(MsgWindow,0,0);
		wvline(MsgWindow,0,MaxRows);
		mvwprintw(MsgWindow,0,1,"Health: %i",Player.Health);
		mvwprintw(MsgWindow,1,1,"Energy: %i",Player.Energy);
		wrefresh(MsgWindow);

		//Player input checks
		InKey=getch();
		switch(InKey) {
			case(KEY_UP):
				Player.Energy--;
				Player.TryMove(Player.Y-1, Player.X);
				break;
			case(KEY_DOWN):
				Player.Energy--;
				Player.TryMove(Player.Y+1, Player.X);
				break;
			case(KEY_LEFT):
				Player.Energy--;
				Player.TryMove(Player.Y, Player.X-1);
				break;
			case(KEY_RIGHT):
				Player.Energy--;
				Player.TryMove(Player.Y, Player.X+1);
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
