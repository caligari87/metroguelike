//--------------------------------
// Map data structures
//--------------------------------

class TileData {
	public:
		//Set everything to "floor" initially
		bool CanWalk=true;
		bool BlocksVision=false;
		char Symbol='.';

		bool Seen=false;
		bool Visible=false;
		int LightLevel=0;
};

class MapData {
	public:
		TileData Tiles[MaxRows*MaxCols];

		int XYToFlat(int row, int col) {
			return (row+col*MaxRows);
		}

		//Information functions
		bool IsWall(int row, int col) {
			return(!Tiles[XYToFlat(row,col)].CanWalk && Tiles[XYToFlat(row,col)].BlocksVision && Tiles[XYToFlat(row,col)].Symbol=='#');
		}

		bool IsDoor(int row, int col) {
			return(Tiles[XYToFlat(row,col)].CanWalk && Tiles[XYToFlat(row,col)].BlocksVision);
		}

		bool CanWalk(int row, int col) {
			return(Tiles[XYToFlat(row,col)].CanWalk);
		}

		bool BlocksVision(int row, int col) {
			return(Tiles[XYToFlat(row,col)].BlocksVision);
		}

		bool Seen(int row, int col) {
			return(Tiles[XYToFlat(row,col)].Seen);
		}

		bool Visible(int row, int col) {
			return(Tiles[XYToFlat(row,col)].Visible);
		}

		char Symbol(int row, int col) {
			return(Tiles[XYToFlat(row,col)].Symbol);
		}

		int LightLevel(int row, int col) {
			return(Tiles[XYToFlat(row,col)].LightLevel);
		}

		//Setting functions
		void SetWall(int row, int col) {
			Tiles[XYToFlat(row,col)].CanWalk = false;
			Tiles[XYToFlat(row,col)].BlocksVision = true;
			Tiles[XYToFlat(row,col)].Symbol = '#';
		}

		void SetFloor(int row, int col) {
			Tiles[XYToFlat(row,col)].CanWalk = true;
			Tiles[XYToFlat(row,col)].BlocksVision = false;
			Tiles[XYToFlat(row,col)].Symbol = '.';
		}

		void SetDoor(int row, int col) {
			Tiles[XYToFlat(row,col)].CanWalk = true;
			Tiles[XYToFlat(row,col)].BlocksVision = true;
			Tiles[XYToFlat(row,col)].Symbol = '+';
		}

		void SetAllNotVisible() {
			for(int i=0; i<MaxRows*MaxCols; i++) {
				Tiles[i].Visible = false;
			}
		}

		void SetVisibleState(int row, int col, bool value) {
			Tiles[XYToFlat(row,col)].Visible = value;
		}

		void SetSeenState(int row, int col, bool value) {
			Tiles[XYToFlat(row,col)].Seen = value;
		}

		void SetLightLevel(int row, int col, int value) {
			Tiles[XYToFlat(row,col)].LightLevel = value;
		}

		void UpdateLighting() {
			bool LoopUpdated;
			do {
				LoopUpdated = false;
				for(int row=0; row<MaxRows; row++) {
					for(int col=0; col<MaxCols; col++) {
						if(IsWall(row,col) == false && IsDoor(row,col) == false) {
							if(LightLevel(row,col) <= LightLevel(row-1,col)-2) { SetLightLevel(row, col, LightLevel(row-1,col)-1); LoopUpdated=true; }
							if(LightLevel(row,col) <= LightLevel(row+1,col)-2) { SetLightLevel(row, col, LightLevel(row+1,col)-1); LoopUpdated=true; }
							if(LightLevel(row,col) <= LightLevel(row,col-1)-2) { SetLightLevel(row, col, LightLevel(row,col-1)-1); LoopUpdated=true; }
							if(LightLevel(row,col) <= LightLevel(row,col+1)-2) { SetLightLevel(row, col, LightLevel(row,col+1)-1); LoopUpdated=true; }
						}
					}
				}
			} while(LoopUpdated == true);
		}
};
