SDL_Texture *fontTexture = 0;

const char *layout = 
"ABCDEFGHIJKLMNOP"
"QRSTUVWXYZ!?.:<>"
"1234567890-/"
;

void initText() {
	fontTexture = IMG_LoadTexture(renderer, "./res/font.png");
}

int indexOfChar(int c) {
	for(int i = 0; i < strlen(layout); i++) {
		if(layout[i] == c) return i;
	}
	return -1;
}

int getTextWidth(int scale, const char *fmt, ...) {
	char buffer[4096];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, 4096, fmt, args);
	va_end(args);

	int len = strlen(buffer);
	int x = 0;

	for(int i = 0; i < len; i++) {
		char c = buffer[i];
		if(c == ' ') {
			x += 4*scale;
			continue;
		}

		int index = indexOfChar(c);
		if(index < 0) {
			printf("Invalid char! Skipping!\n");
			continue;
		}

		x += 8*scale;
	}

	return x;
}

void drawText(int x, int y, Color color, int scale, const char *fmt, ...) {
	char buffer[4096];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, 4096, fmt, args);
	va_end(args);

	int len = strlen(buffer);

	for(int i = 0; i < len; i++) {
		char c = buffer[i];
		if(c == ' ') {
			x += 4*scale;
			continue;
		}

		int index = indexOfChar(c);
		if(index < 0) {
			printf("Invalid char! Skipping!\n");
			continue;
		}

		int tx = (index % 16) * 8;
		int ty = (index / 16) * 8;

		SDL_Rect src = {tx, ty, 8, 8};
		SDL_Rect dst = {x, y, 8*scale, 8*scale};
		SDL_SetTextureColorMod(fontTexture, color.r, color.g, color.b);
		SDL_RenderCopy(renderer, fontTexture, &src, &dst);

		x += 8*scale;
	}
}