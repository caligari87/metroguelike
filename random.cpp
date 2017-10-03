//--------------------------------
// Friendly random functions, courtesy of dpJudas
//--------------------------------

// Returns the linear interpolation between a and b, with t being a value between 0 and 1
double mix(double a, double b, double t) {
	return a * (1.0 - t) + b * t;
}

// Random value between 0 and 1, then use that to mix between min and max
double frandom (double min, double max) {
	return mix(min, max, (double)rand() / RAND_MAX);
}

// Random value between min and max, then round it to the nearest integer
double irandom(int min, int max) {
	return (int)round(frandom(min, max));
}
