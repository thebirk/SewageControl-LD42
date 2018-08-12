uint64_t getTime() {
	return SDL_GetPerformanceCounter();
}

double getTimeS(uint64_t start, uint64_t end) {
	static Uint64 freq = 0;
	if(freq == 0) {
		freq = SDL_GetPerformanceFrequency();
	}
	return (double)(end - start) / freq;
}

void keysUpdate() {
	for(int i = 0; i < NUM_KEYS; i++) {
		pkeys[i] = keys[i];
	}
}

bool keyDown(int key) {
	return keys[key];
}

bool keyPressed(int key) {
	return keys[key] && !pkeys[key];
}

void screenshake(double time, double wx, double wy) {
	screenshakeTimer = time;
	screenshakeWeightX = wx;
	screenshakeWeightY = wy;
}

void updateScreenshake() {
	if(screenshakeTimer > 0.0) {
		screenshakeTimer -= dt;
		double randAngle = ((float)rand() / (float)RAND_MAX*M_PI);
		double randStrengthX = (float)(rand() % (6 + (int)screenshakeWeightX) + 4);
		double randStrengthY = (float)(rand() % (6 + (int)screenshakeWeightY) + 4);
		screenshakeX = cos(randAngle) * randStrengthX;
		screenshakeY = sin(randAngle) * randStrengthY;
	} else {
		screenshakeX = 0;
		screenshakeY = 0;
	}
}

float lerp(float a, float b, float t) {
	return a + t * (b - a);
}