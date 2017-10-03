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
