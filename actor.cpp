//--------------------------------
// Actor data and functions
//--------------------------------

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

};

class PlayerCharacter: public Actor {

};

class NonPlayerCharacter: public Actor {
	public:
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
