const double PI = 3.14159265;
const int MaxRows = 24;
const int MaxCols = 50;

#include <ncurses.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include "random.cpp"
#include "map.cpp"
#include "actor.cpp"

#define min(x, y)  ((x) < (y) ? (x) : (y))
#define max(x, y)  ((x) > (y) ? (x) : (y))

void InitializeTerminal() {
	initscr(); //Initialize ncurses screen
	curs_set(0); //Turn off cursor
	keypad(stdscr, true); //Enable the keypad
	//Check that the terminal is big enough
	if(getmaxx(stdscr)<80 || getmaxy(stdscr)<24) {
		mvprintw(0,0,"Please use a larger terminal for this program.\nPress any key to continue.");
		cbreak();
		getch();
		endwin();
		exit(0);
	}
}

int main() {
	srand(time(nullptr)); //Seed random

	//Set core variables
	int InKey;
	bool Quit = false;

	//Terminal setup and initialization
	InitializeTerminal();

	//Set up UI "windows"
	WINDOW * MapWindow = newwin(25,50,0,0);
	WINDOW * MsgWindow = newwin(25,30,0,50);
	refresh();

	//Debug boundaries
	box(MapWindow,0,0);
	box(MsgWindow,0,0);
	wrefresh(MapWindow);
	wrefresh(MsgWindow);

	//Wait for keypress and clean screen for remainder of program
	getch();
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
	while(MadeWalls<6 || WallAttempts<100) {
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
	PlayerCharacter Player;
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
	NonPlayerCharacter Monsters[10];
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

	//Initialize lights
	LightSource Lights[10];
	for(int i=0; i<=9; i++) {
		do {
			Lights[i].Y = irandom(5,MaxRows-5);
			Lights[i].X = irandom(5,MaxCols-5);
		} while(Map.CanWalk(Lights[i].Y,Lights[i].X) == false);
		Lights[i].Symbol = '*';
		Lights[i].Intensity = irandom(5,10);
	}

	//--------------------------------
	//Main loop
	//--------------------------------
	while(!Quit) {
		//Map Updates
		//Lighting
		Map.SetAllLightLevel(0);
		for(int i=0; i<=9; i++) {
			Map.SetLightLevel(Lights[i].Y,Lights[i].X,Lights[i].Intensity);
		}
		Map.SetLightLevel(Player.Y,Player.X,
			max(Player.LightIntensity, Map.LightLevel(Player.Y,Player.X)));
		Map.UpdateLighting();

		//FOV
		Map.SetAllNotVisible();
		//Player FOV check
		for(double Angle=0;Angle<360;Angle+=0.1) {
			int LastLightLevel=Map.LightLevel(Player.Y,Player.X);
			for(int Dist=1;Dist<50;Dist++) {
				int CheckY = (double)0.5 + Player.Y + (Dist*sin(Angle*PI/180));
				int CheckX = (double)0.5 + Player.X + (Dist*cos(Angle*PI/180));
				if(CheckX<0 || CheckX>MaxCols || CheckY<0 or CheckY>MaxRows) { break; }
				if(Dist<=1 || Map.LightLevel(CheckY,CheckX)>0 || ((Map.IsWall(CheckY,CheckX)==true || Map.IsDoor(CheckY,CheckX)==true) && LastLightLevel>0)) {
					Map.SetVisibleState(CheckY,CheckX,true);
					if(Map.IsWall(CheckY,CheckX) == true || Map.IsDoor(CheckY,CheckX) == true) {
						Map.SetSeenState(CheckY,CheckX, true);
					}
				}
				LastLightLevel = Map.LightLevel(CheckY,CheckX);
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
		//Light sources
		for(int i=0; i<=9; i++) {
			if(Map.Visible(Lights[i].Y,Lights[i].X) == true) {
				mvwaddch(MapWindow,Lights[i].Y,Lights[i].X,Lights[i].Symbol);
			}
		}
		//Monsters
		for(int i=0; i<=9; i++) {
			if(Map.Visible(Monsters[i].Y,Monsters[i].X) == true) {
				mvwaddch(MapWindow,Monsters[i].Y,Monsters[i].X,Monsters[i].Symbol);
				Monsters[i].Target = &Player;
			}
		}
		//Player
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
			case('l'):
				Player.LightIntensity = 10-Player.LightIntensity;
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
