//--------------------------------
// Actor data and functions
//--------------------------------

// Simple object class, basically anything that can exist on the map
class Object {
	public:
		int X,Y;
		char Symbol;

};

class LightSource: public Object {
	public:
		int Intensity; //Brightness, obviously
		int Angle; //Direction the light faces, in degrees
		int Spread; //Spread, as degrees from the center angle (cone/FOV is spread*2)
};

class Actor: public Object {
	public:
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
