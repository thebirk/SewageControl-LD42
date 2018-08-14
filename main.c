#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#ifndef __EMSCRIPTEN__
	#include <SDL2/SDL_mixer.h>
#else
	#include <emscripten.h>
#endif
#include "common.c"
#include "globals.c"
#include "text.c"
#include "utils.c"

int getTile(int x, int y) {
	if(x < 0 || x >= field->width || y < 0 || y >= field->height) return -1;
	return field->tiles[x+y*field->width].occupied;
}

bool tryMove(int dx, int dy, int rotation) {
	Piece tPiece = field->fallingPiece;
	tPiece.rotation = rotation;
	if(tPiece.rotation >= 4) tPiece.rotation = 0;
	tPiece.posX += dx;
	tPiece.posY += dy;

	bool result = true;
	for(int y = 0; y < 4; y++) {
		for(int x = 0; x < 4; x++) {
			int pd = pieces[tPiece.id][tPiece.rotation][x+y*4];
			int td = getTile(tPiece.posX+x, tPiece.posY+y);

			if(pd && td) result = false;
		}
	}

	return result;
}

void newPiece() {
	field->fallingPiece = field->nextPieces[0];
	field->ghostPiece = field->fallingPiece;
	int tryY = 1;
	while(tryMove(0, tryY, field->fallingPiece.rotation)) {
		field->ghostPiece.posY += 1;
		tryY += 1;
	}


	for(int i = 1; i < 3; i++) {
		field->nextPieces[i-1] = field->nextPieces[i];
	}

	field->nextPieces[2].id = rand() % 7;
	field->nextPieces[2].rotation = 0;
	field->nextPieces[2].posX = 3;
	field->nextPieces[2].posY = 0;
	
	swappedHeldThisTurn = false;
}

void initField(int width, int height) {
	if(field) free(field);
	field = new(Field);
	field->width = width;
	field->height = height;
	field->tiles = newArray(width*height, Tile);

	newPiece();
	newPiece();
	newPiece();
	newPiece();
}

void updateField() {
	if(flushTimer > 0) {
		field->shadowHeight += flushSpeed * dt;
	} else {
		field->shadowHeight += shadowSpeed * dt;
	}
	if(field->shadowHeight >= (field->height-1)*TILE_SIZE) {
		currentState = GameOver;
		screenshake(0.75, 10, 40);
		// Screaming noise from streets?
		return;
	}

	for(int y = 0; y < field->height; y++) {
		int yHeight = field->height * TILE_SIZE - y * TILE_SIZE;
		for(int x = 0; x < field->width; x++) {
			Tile *t = &field->tiles[x+y*field->width];

			if(yHeight < field->shadowHeight) {
				t->unavailable = true;
			}
		}
	}
}

void drawPreviewPiece(int xPos, int yPos, int id, int rotation) {
	for(int y = 0; y < 4; y++) {
		int yy = (y * TILE_SIZE/2) + yPos;
		for(int x = 0; x < 4; x++) {
			int xx = (x * TILE_SIZE/2) + xPos;
			int i = pieces[id][rotation][x+y*4];

			if(i) {
				SDL_RenderCopy(renderer, pieceTexture[id], 0, &(SDL_Rect){xx, yy, TILE_SIZE/2, TILE_SIZE/2});
			}
		}
	}
}

void drawField() {
	SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
	SDL_RenderFillRect(renderer, &(SDL_Rect){0, HEIGHT - 80 +(int)menuOffset, 100, 40});

	// draw the tiles
	int xPos = WIDTH/4 - (field->width*TILE_SIZE) / 2;
	int yPos = 16-TILE_SIZE + (int)menuOffset;//TILE_SIZE*2;
	for(int y = 0; y < field->height; y++) {
		int yy = (y * TILE_SIZE) + yPos;
		for(int x = 0; x < field->width; x++) {
			int xx = (x * TILE_SIZE) + xPos;
			Tile *t = &field->tiles[x+y*field->width];

			if(t->unavailable, false) {
				SDL_SetRenderDrawColor(renderer, 122, 122, 122, 255);
				SDL_RenderFillRect(renderer, &(SDL_Rect){xx, yy, TILE_SIZE-1, TILE_SIZE-1});
			} else {
				SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
				SDL_RenderFillRect(renderer, &(SDL_Rect){xx, yy, TILE_SIZE, TILE_SIZE});
				if(t->occupied) {

					SDL_RenderCopy(renderer, pieceTexture[t->id], 0, &(SDL_Rect){xx, yy, TILE_SIZE, TILE_SIZE});
					//SDL_SetRenderDrawColor(renderer, color_lit(t->color));
				} else {
					
				}
			}
			
		}
	}

	// draw falling piece
	for(int y = 0; y < 4; y++) {
		int yy = ((y + field->fallingPiece.posY) * TILE_SIZE) + yPos;
		for(int x = 0; x < 4; x++) {
			int xx = ((x + field->fallingPiece.posX) * TILE_SIZE) + xPos;
			int i = pieces[field->fallingPiece.id][field->fallingPiece.rotation][x+y*4];

			if(i) {
				SDL_RenderCopy(renderer, pieceTexture[field->fallingPiece.id], 0, &(SDL_Rect){xx, yy, TILE_SIZE, TILE_SIZE});
			}
		}
	}

	// draw ghost piece
	SDL_SetTextureAlphaMod(pieceTexture[field->fallingPiece.id], 122);
	for(int y = 0; y < 4; y++) {
		int yy = ((y + field->ghostPiece.posY) * TILE_SIZE) + yPos;
		for(int x = 0; x < 4; x++) {
			int xx = ((x + field->ghostPiece.posX) * TILE_SIZE) + xPos;
			int i = pieces[field->ghostPiece.id][field->ghostPiece.rotation][x+y*4];

			if(i) {
				SDL_RenderCopy(renderer, pieceTexture[field->fallingPiece.id], 0, &(SDL_Rect){xx, yy, TILE_SIZE, TILE_SIZE});
			}
		}
	}
	SDL_SetTextureAlphaMod(pieceTexture[field->fallingPiece.id], 255);

	// draw sewer
	yPos += field->height*TILE_SIZE - field->shadowHeight;
	SDL_SetRenderDrawColor(renderer, 0, 145, 23, 122);
	SDL_RenderFillRect(renderer, &(SDL_Rect){xPos, yPos, field->width*TILE_SIZE, field->shadowHeight+1});

	// render menu offseted
	SDL_RenderCopy(renderer, menuTexture, 0, &(SDL_Rect){0, ((int)menuOffset) - (600-32), WIDTH, HEIGHT});

	int x = WIDTH/2 + 128;
	int y = 48+16 + (int)menuOffset;
	SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
	drawText(WIDTH/2, y - 16-4-2, (Color){255, 255, 255, 255}, 2, "HOLD");
	SDL_RenderFillRect(renderer, &(SDL_Rect){WIDTH/2, y, 16*4+4, 16*4+4});
	drawPreviewPiece(WIDTH/2+2, y+2, field->heldPiece.id, field->heldPiece.rotation);
	
	drawText(x, y - 16-4-2, (Color){255, 255, 255, 255}, 2, "NEXT");
	SDL_RenderFillRect(renderer, &(SDL_Rect){x, y, 16*4+4, 16*4+4});
	drawPreviewPiece(x+2, y+2, field->nextPieces[0].id, field->nextPieces[0].rotation);
	x += 16*4+4 + 16;
	SDL_RenderFillRect(renderer, &(SDL_Rect){x, y, 16*4+4, 16*4+4});
	drawPreviewPiece(x+2, y+2, field->nextPieces[1].id, field->nextPieces[1].rotation);
	x += 16*4+4 + 16;
	SDL_RenderFillRect(renderer, &(SDL_Rect){x, y, 16*4+4, 16*4+4});
	drawPreviewPiece(x+2, y+2, field->nextPieces[2].id, field->nextPieces[2].rotation);
}

void drawParticles() {
	int draws = 0;
	for(int i = 0; i < PARTICLE_COUNT; i++) {
		Particle *p = &particles[i];
		if(p->used) {
			Uint8 alpha = (Uint8) ((p->age/p->maxAge) * 255.0);
			SDL_SetTextureBlendMode(p->texture, SDL_BLENDMODE_BLEND);
			int w, h;
			SDL_QueryTexture(p->texture, 0, 0, &w, &h);
			SDL_SetTextureAlphaMod(p->texture, alpha);
			SDL_RenderCopy(renderer, p->texture, 0, &(SDL_Rect){p->x, p->y, w, h});
			SDL_SetTextureBlendMode(p->texture, 0);
			draws++;
		}
	}
}

void updateParticles() {
	for(int i = 0; i < PARTICLE_COUNT; i++) {
		Particle *p = &particles[i];
		if(p->used) {
			p->age -= dt;
			if(p->age <= 0.0) {
				p->used = false;
				p->age = 0;
				continue;
			}

			if(!SDL_PointInRect(&(SDL_Point){p->x, p->y}, p->bbox)) {
				p->vx = -p->vx;
				p->vy = -p->vy;	
			}

			p->x += p->vx * dt;
			p->y += p->vy * dt;
		}
	}
}

void spawnParticle(double x, double y, double vx, double vy, SDL_Texture *texture, SDL_Rect *bbox, double maxAge) {
	Particle *p = 0;
	for(int i = 0; i < PARTICLE_COUNT; i++) {
		if(!particles[i].used) p = &particles[i];
	}
	if(!p) return; // We just drop the particle, who even cares

	p->used = true;
	p->x = x;
	p->y = y;
	p->vx = vx;
	p->vy = vy;
	p->texture = texture;
	p->bbox = bbox;
	p->maxAge = maxAge;
	p->age = maxAge;
}

void renderMenu() {
	SDL_RenderCopy(renderer, titleTexture, 0, &(SDL_Rect){WIDTH/2-800/2, 25+menuOffset-600-32, 400*2, 200*2});
	
	int x = WIDTH/2;
	int y = 425 + (int)menuOffset - 600 - 32;
	Color c;
	for(int i = 0; i < menuOptions; i++) {
		if(menuSelectedOption == i) {
			c = (Color){255, 255, 0, 255};
			int width = getTextWidth(4, menuOptionSelected[i]);
			drawText(x - width/2, y, c, 4, menuOptionSelected[i]);
		} else {
			c = (Color){255, 255, 255, 255};
			int width = getTextWidth(4, menuOption[i]);
			drawText(x - width/2, y, c, 4, menuOption[i]);
		}

		y += 4*8 + 8;
	}
	
}

void renderPlaying() {
	drawField();
	// Draw up next over the SCORE
	drawText(WIDTH/2 - 16, 128+32 + (int)menuOffset, (Color){255, 255, 255, 255}, 4, "SCORE:%d", score);
	double tuf = timeUntilFlush;
	if(tuf < 0) tuf = -tuf;
	drawText(WIDTH/2 - 16, 128+32+32+8 + 48  + (int)menuOffset, timeUntilFlushColor, 4, "NEXT FLUSH:%.0f", tuf);

	drawParticles();
}

void renderGameOver() {
	drawField();
	drawText(WIDTH/2 - 16, 128+32 + (int)menuOffset, (Color){255, 255, 255, 255}, 4, "SCORE:%d", score);
	double tuf = timeUntilFlush;
	if(tuf < 0) tuf = -tuf;
	drawText(WIDTH/2 - 16, 128+32+32+8  + (int)menuOffset, timeUntilFlushColor, 4, "NEXT FLUSH:%.0f", tuf);
	drawParticles();
	renderMenu();
	
	if(!fadeToMenuInitiated) {
		SDL_SetRenderDrawColor(renderer, 10, 10, 10, 200);
		SDL_RenderFillRect(renderer, &(SDL_Rect){0, 0, WIDTH, HEIGHT});
		
		int tw = getTextWidth(6, "GAME OVER!");
		drawText(WIDTH/2-tw/2, 150, (Color){255, 0, 0, 255}, 6, "GAME OVER!");
		
		tw = getTextWidth(4, "SCORE: %d", score);
		drawText(WIDTH/2-tw/2, 300, (Color){255, 255, 0, 255}, 4, "SCORE: %d", score);
		
		if(((int)t) % 2 == 0) {
			tw = getTextWidth(2, "PRESS ENTER TO CONTINUE!");
			drawText(WIDTH/2-tw/2, 450, (Color){255, 255, 255, 255}, 2, "PRESS ENTER TO CONTINUE!");
		}
	}
}

void renderHelp() {
	renderPlaying();
	
	char *text = "- SEWAGE CONTROL -";
	int scale = 5;
	int tw = getTextWidth(scale, text);
	drawText(WIDTH/2-tw/2, 75, (Color){255, 255, 255, 255}, scale, text);
	
	scale = 2;
	text = "HELP KEEP THE CITY CLEAN BY USING YOU ABILITY TO";
	tw = getTextWidth(scale, text);
	drawText(WIDTH/2-tw/2, 150, (Color){255, 255, 255, 255}, scale, text);
	
	scale = 2;
	text = "CLEAR THE SEWER BY PLAYING WITH WHAT YOU  FIND INSIDE!";
	tw = getTextWidth(scale, text);
	drawText(WIDTH/2-tw/2, 150+16+8, (Color){255, 255, 255, 255}, scale, text);
	
	scale = 4;
	text = "CONTROLS";
	tw = getTextWidth(scale, text);
	drawText(WIDTH/2-tw/2, 150+16+8+50, (Color){255, 255, 255, 255}, scale, text);
	
	scale = 3;
	text = "ESC - PAUSE THE GAME";
	tw = getTextWidth(scale, text);
	drawText(WIDTH/2-tw/2, 150+16+8+50+3*8+42, (Color){255, 255, 255, 255}, scale, text);
	
	scale = 3;
	text = "LEFT/RIGHT - MOVE PIECE LEFT/RIGHT";
	tw = getTextWidth(scale, text);
	drawText(WIDTH/2-tw/2, 150+16+8+50+3*8+42+8+42, (Color){255, 255, 255, 255}, scale, text);
	
	scale = 3;
	text = "UP - ROTATE PIECE CLOCKWISE";
	tw = getTextWidth(scale, text);
	drawText(WIDTH/2-tw/2, 150+16+8+50+3*8+42+8+42+42+8, (Color){255, 255, 255, 255}, scale, text);
	
	scale = 3;
	text = "DOWN - SOFT LANDING";
	tw = getTextWidth(scale, text);
	drawText(WIDTH/2-tw/2, 150+16+8+50+3*8+42+8+42+42+8+42+8, (Color){255, 255, 255, 255}, scale, text);
	
	scale = 3;
	text = "SPACE - HARD LANDING";
	tw = getTextWidth(scale, text);
	drawText(WIDTH/2-tw/2, 150+16+8+50+3*8+42+8+42+42+8+42+8+42+8, (Color){255, 255, 255, 255}, scale, text);
	
	scale = 3;
	text = "C - HOLD PIECE/SWAP HELD PIECE";
	tw = getTextWidth(scale, text);
	drawText(WIDTH/2-tw/2, 150+16+8+50+3*8+42+8+42+42+8+42+8+42+8+42+8, (Color){255, 255, 255, 255}, scale, text);
	
	scale = 2;
	text = "MADE BY - THEBIRKISREAL";
	tw = getTextWidth(scale, text);
	drawText(WIDTH/2-tw/2, 20, (Color){0, 0, 0, 255}, scale, text);
	
	scale = 2;
	text = "FOR LUDUMDARE 42";
	tw = getTextWidth(scale, text);
	drawText(WIDTH/2-tw/2, 20+16+8, (Color){0, 0, 0, 255}, scale, text);
}

void renderPaused() {
	renderPlaying();
	
	if(!fadeToPausedInitiated, true) {
		char *text = "PAUSED";
		int scale = 5;
		int tw = getTextWidth(scale, text);
		drawText(WIDTH/2-tw/2, 200 + (int)menuOffset - 600 - 32, (Color){255, 0, 0, 255}, scale, text);
		
		int x = WIDTH/2;
		int y = 425 + (int)menuOffset - 600 - 32;
		Color c;
		for(int i = 0; i < menuOptions; i++) {
			if(menuSelectedOption == i) {
				c = (Color){255, 255, 0, 255};
				int width = getTextWidth(4, pauseOptionSelected[i]);
				drawText(x - width/2, y, c, 4, pauseOptionSelected[i]);
			} else {
				c = (Color){255, 255, 255, 255};
				int width = getTextWidth(4, pauseOption[i]);
				drawText(x - width/2, y, c, 4, pauseOption[i]);
			}

			y += 4*8 + 8;
		}
	}
}

void render() {
	switch(currentState) {
		case Menu: renderPlaying(); renderMenu(); break;
		case GameOver: renderGameOver(); break;
		case Playing: renderPlaying();break;
		case Help: renderHelp(); break;
		case Paused: renderPaused(); break;
	}
}

void resetGame() {
	initField(10, 18);
	field->heldPiece.id = -1;
	timeUntilFlush = 20.0;
	newPiece(); // redundant?
	score = 0;
}

void updateMenu() {
	if(!fadeToPlayInitiated) {
		if(keyPressed(KEY_ENTER)) {
			switch(menuSelectedOption) {
				case 0: {
					fadeToPlayInitiated = true;
					resetGame();
					menuOffsetTarget = 0;
					#ifndef __EMSCRIPTEN__
					Mix_PlayChannel(-1, flushFastSound, 0);
					#endif
				} break;
				case 1: {
					currentState = Help;
				} break;
				case 2: {
					running = false;
				} break;
			}
		}
		if(keyPressed(KEY_UP)) {
			menuSelectedOption -= 1;
			if(menuSelectedOption < 0) menuSelectedOption = menuOptions - 1;
			#ifndef __EMSCRIPTEN__
			Mix_PlayChannel(-1, menuSound, 0);
			#endif
		}

		if(keyPressed(KEY_DOWN)) {
			menuSelectedOption += 1;
			if(menuSelectedOption >= menuOptions) menuSelectedOption = 0;
			#ifndef __EMSCRIPTEN__
			Mix_PlayChannel(-1, menuSound, 0);
			#endif
		}
	}

	if(fadeToPlayInitiated && ((int)menuOffset == (int)menuOffsetTarget)) {
		fadeToPlayInitiated = false;
		currentState = Playing;
	} else if(fadeToPlayInitiated) {
		menuOffset = lerp(menuOffset, menuOffsetTarget, dt*3);
	}
}

void placePiece() {
	for(int y = 0; y < 4; y++) {
		int yy = y + field->fallingPiece.posY;
		for(int x = 0; x < 4; x++) {
			int xx = x + field->fallingPiece.posX;
			if(pieces[field->fallingPiece.id][field->fallingPiece.rotation][x+y*4]) {
				field->tiles[xx+yy*field->width].occupied = true;
				field->tiles[xx+yy*field->width].id = field->fallingPiece.id;
			}
		}
	}

	// check line 17
	bool piece = false;
	for(int x = 0; x < field->width; x++) {
		if(field->tiles[x+1*field->width].occupied) piece = true;
	}

	if(piece) {
		for(int i = 0; i < field->width*field->height; i++) {
			field->tiles[i].occupied = false;
		}
		field->shadowHeight += 120.0;
		#ifndef __EMSCRIPTEN__
		Mix_PlayChannel(-1, flushFastSound, 0);
		#endif
	}

	newPiece();
	#ifndef __EMSCRIPTEN__
	Mix_PlayChannel(-1, plopSound, 0);
	#endif
}

void updatePlay() {
	if(keyPressed(KEY_ESC)) {
		currentState = Paused;
		fadeToPausedInitiated = true;
		menuOffsetTarget = 600-32;
		fadeToPauseTargetState = Paused;
		return;
	}
	
	//////////////// update all timers here
	if(pieceMoveDelay > 0) pieceMoveDelay -= dt;
	if(downForceDelay > 0) downForceDelay -= dt;
	if(timeUntilFlush > 0) timeUntilFlush -= dt;
	if(flushTimer > 0) flushTimer -= dt;
	////////////////

	if(timeUntilFlush < 5.0) {
		int tufi = (int)timeUntilFlush;
		if(tufi % 2 == 0) {
			timeUntilFlushColor = (Color){255, 0, 0, 255};
		} else {
			timeUntilFlushColor = (Color){255, 255, 255, 255};
		}
	}

	if(timeUntilFlush < 0)  {
		timeUntilFlush = rand()%30+15;
		flushTimer = 4.0;
		flushSpeed = 20.0;
		timeUntilFlushColor = (Color){255, 255, 255, 255};
		#ifndef __EMSCRIPTEN__
		Mix_PlayChannel(-1, flushSound, 0);
		#endif
	}

	updateField();
	
	if(keyPressed(KEY_C) && !swappedHeldThisTurn) {
		if(field->heldPiece.id == -1) {
			field->heldPiece = field->fallingPiece;
			newPiece();
		} else {
			Piece temp = field->fallingPiece;
			field->fallingPiece = field->heldPiece;
			field->fallingPiece.posX = 3;
			field->fallingPiece.posY = 0;
			field->heldPiece = temp;
		}
		swappedHeldThisTurn = true;
	}

	if(keyPressed(KEY_UP))  {
		// rotate, check collision before the new rotation is actually applied
		if(tryMove(0, 0, field->fallingPiece.rotation+1)) {
			field->fallingPiece.rotation++;
			if(field->fallingPiece.rotation >= 4) field->fallingPiece.rotation = 0;
		}
	}

	/*if(keyPressed(KEY_DOWN)) {
		pieceMoveDelay = 0;
	}*/
	if(keyDown(KEY_DOWN) && downForceDelay <= 0) {
		pieceMoveDelay = 0.04;
		downForceDelay = 0.05;
	}

	if(keyPressed(KEY_SPACE)) {
		while(tryMove(0, 1, field->fallingPiece.rotation)) {
			field->fallingPiece.posY += 1;
		}
		placePiece();
		screenshake(0.05, 0.02, 0);
	}

	if(keyPressed(KEY_LEFT)) {
		if(tryMove(-1, 0, field->fallingPiece.rotation)) {
			field->fallingPiece.posX -= 1;
		}
	} else if(keyPressed(KEY_RIGHT)) {
		if(tryMove(1, 0, field->fallingPiece.rotation)) {
			field->fallingPiece.posX += 1;
		}
	}

	if(pieceMoveDelay <= 0) {
		// check collision for posY+1 before doing this
		if(tryMove(0, 1, field->fallingPiece.rotation)) {
			field->fallingPiece.posY += 1;
			pieceMoveDelay = 1.0;
		} else {
			placePiece();
		}
	}

	field->ghostPiece = field->fallingPiece;
	int tryY = 1;
	while(tryMove(0, tryY, field->fallingPiece.rotation)) {
		field->ghostPiece.posY += 1;
		tryY += 1;
	}

	int clearedLines = 0;
	for(int y = field->height-1; y >= 0; y--) {
		bool filled = true;
		for(int x = 0; x < field->width; x++) {
			if(!field->tiles[x+y*field->width].occupied) filled = false;
		}

		if(filled) {
			for(int x = 0; x < field->width; x++) {
				field->tiles[x+y*field->width].occupied = false;
			}

			// move all lines above this down one
			for(int yy = y; yy >= 0; yy--) {
				int ya = yy - 1;
				for(int x = 0; x < field->width; x++) {
					if(ya == -1) {
						field->tiles[x+yy*field->width].occupied = false;
					} else {
						field->tiles[x+yy*field->width].occupied = field->tiles[x+ya*field->width].occupied;
					}
				}
			}

			// redo *this* line
			y++;
			clearedLines++;
		}
	}
	if(clearedLines > 0) {
		if(clearedLines >= 4) {
			field->shadowHeight = 0;
		} else {
			field->shadowHeight -= clearedLines * 16;
			if(field->shadowHeight < 0) field->shadowHeight = 0.0;
		}

		#ifndef __EMSCRIPTEN__
		Mix_PlayChannel(-1, lineClearSound, 0);
		#endif
		screenshake(0.1, pow(2.5, clearedLines > 4 ? 4 : clearedLines), 0);

		if(clearedLines == 4 && lastClearedLines == 4) {
			score += 2000;
		} else {
			score += 100 * pow(2, clearedLines-1);
		}
		lastClearedLines = clearedLines;
	}
}

void updateGameOver() {
	if(!fadeToMenuInitiated) {
		if(keyPressed(KEY_ENTER)) {
			resetGame();
			
			menuOffsetTarget = 600-32;
			fadeToMenuInitiated = true;
		}
	}
	
	if(fadeToMenuInitiated && (ceil(menuOffset) == ceil(menuOffsetTarget-0.5))) {
		fadeToMenuInitiated = false;
		menuOffset = menuOffsetTarget;
		currentState = Menu;
		menuSelectedOption = 0;
	} else if(fadeToMenuInitiated) {
		menuOffset = lerp(menuOffset, menuOffsetTarget, dt*10);
	}
}

void updateHelp() {
	if(keyPressed(KEY_ANY)) {
		currentState = Menu;
		// menuSelectedOption = 0; // We dont really need this here ;)
	}
}

void updatePaused() {
	if(!fadeToPausedInitiated) {
		if(keyPressed(KEY_ENTER)) {
			switch(menuSelectedOption) {
				case 0: {
					fadeToPausedInitiated = true;
					fadeToPauseTargetState = Playing;
					menuOffsetTarget = 0.5;
				} break;
				case 1: {
					currentState = Menu;
					menuSelectedOption = 0;
				} break;
				case 2: {
					running = false;
				} break;
			}
		}
		if(keyPressed(KEY_UP)) {
			menuSelectedOption -= 1;
			if(menuSelectedOption < 0) menuSelectedOption = menuOptions - 1;
			#ifndef __EMSCRIPTEN__
			Mix_PlayChannel(-1, menuSound, 0);
			#endif
		}

		if(keyPressed(KEY_DOWN)) {
			menuSelectedOption += 1;
			if(menuSelectedOption >= menuOptions) menuSelectedOption = 0;
			#ifndef __EMSCRIPTEN__
			Mix_PlayChannel(-1, menuSound, 0);
			#endif
		}
	}
	
	if(fadeToPausedInitiated && (ceil(menuOffset) == ceil(menuOffsetTarget))) {
		fadeToPausedInitiated = false;
		menuOffset = menuOffsetTarget;
		currentState = fadeToPauseTargetState;
		menuSelectedOption = 0;
		printf("fadeToPauseTargetState %d\n", fadeToPauseTargetState);
	} else if(fadeToPausedInitiated) {
		menuOffset = lerp(menuOffset, menuOffsetTarget, dt*10);
	}
}

void update() {
	updateScreenshake();
	updateParticles();

	switch(currentState) {
		case Menu: updateMenu(); break;
		case GameOver: updateGameOver(); break;
		case Playing: updatePlay(); break;
		case Help: updateHelp(); break;
		case Paused: updatePaused(); break;
	}
}

void loadTextures() {
	topTexture = IMG_LoadTexture(renderer, "res/top.png");
	poopTexture = IMG_LoadTexture(renderer, "res/poop.png");
	menuTexture = IMG_LoadTexture(renderer, "res/back.png");
	titleTexture = IMG_LoadTexture(renderer, "res/title.png");	

	brownTexture = IMG_LoadTexture(renderer, "res/brown2.png");
	yellowTexture = IMG_LoadTexture(renderer, "res/yellow.png");
	cyanTexture = IMG_LoadTexture(renderer, "res/cyan.png");
	redTexture = IMG_LoadTexture(renderer, "res/red.png");
	magentaTexture = IMG_LoadTexture(renderer, "res/magenta.png");
	greyTexture = IMG_LoadTexture(renderer, "res/grey.png");
	greenTexture = IMG_LoadTexture(renderer, "res/green.png");

	pieceTexture[0] = brownTexture;
	pieceTexture[1] = yellowTexture;
	pieceTexture[2] = cyanTexture;
	pieceTexture[3] = redTexture;
	pieceTexture[4] = magentaTexture;
	pieceTexture[5] = greyTexture;
	pieceTexture[6] = greenTexture;
}

void loadSounds() {
#ifndef __EMSCRIPTEN__
	lineClearSound = Mix_LoadWAV("res/sounds/eplode.wav");
	flushSound = Mix_LoadWAV("res/sounds/flush.wav");
	flushFastSound = Mix_LoadWAV("res/sounds/flushFast.wav");
	menuSound = Mix_LoadWAV("res/sounds/move.wav");
	plopSound = Mix_LoadWAV("res/sounds/plopPiece.wav");
#endif
}

double accumulator = 0.0;
	
uint64_t lastTime = 0;
uint64_t lastTimeFps = 0;
int frames = 0;
int updates = 0;

void main_loop() {
	uint64_t now = getTime();
	double frameTime = getTimeS(lastTime, now);
	lastTime = now;
	
	accumulator += frameTime;
	
	while(accumulator >= dt) {
		update();
		keysUpdate();
		updates++;
		accumulator -= dt;
		t += dt;
	}

	SDL_SetRenderTarget(renderer, backbuffer);
	SDL_SetRenderDrawColor(renderer, 144, 86, 30, 255);
	SDL_RenderClear(renderer);
	
	render();

	// Render backbuffer
	SDL_SetRenderTarget(renderer, 0);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, backbuffer, 0, &(SDL_Rect){screenshakeX, screenshakeY, WIDTH, HEIGHT});
	SDL_RenderPresent(renderer);

	frames++;
	if(getTimeS(lastTimeFps, now) >= 1.0) {
		printf("%d fps, %d ups\n", frames, updates);
		//char buffer[4096];
		//snprintf(buffer, 4096, "%d fps, %d ups\n", frames, updates);
		//SDL_SetWindowTitle(window, buffer);
		frames = 0;
		updates = 0;
		lastTimeFps = now;
	}
	
	SDL_Event e;
	while(SDL_PollEvent(&e)) {
		switch(e.type) {
			case SDL_QUIT: {
				running = false;
			} break;

			case SDL_KEYDOWN: {
				keys[KEY_ANY] = true;
				switch(e.key.keysym.sym) {
					case SDLK_UP: keys[KEY_UP] = true; break;
					case SDLK_DOWN: keys[KEY_DOWN] = true; break;
					case SDLK_LEFT: keys[KEY_LEFT] = true; break;
					case SDLK_RIGHT: keys[KEY_RIGHT] = true; break;
					case SDLK_SPACE: keys[KEY_SPACE] = true; break;
					case SDLK_RETURN: keys[KEY_ENTER] = true; break;
					case SDLK_ESCAPE: keys[KEY_ESC] = true; break;
					case SDLK_c: keys[KEY_C] = true; break;
				}
			} break;
			case SDL_KEYUP: {
				keys[KEY_ANY] = false;
				switch(e.key.keysym.sym) {
					case SDLK_UP: keys[KEY_UP] = false; break;
					case SDLK_DOWN: keys[KEY_DOWN] = false; break;
					case SDLK_LEFT: keys[KEY_LEFT] = false; break;
					case SDLK_RIGHT: keys[KEY_RIGHT] = false; break;
					case SDLK_SPACE: keys[KEY_SPACE] = false; break;
					case SDLK_RETURN: keys[KEY_ENTER] = false; break;
					case SDLK_ESCAPE: keys[KEY_ESC] = false; break;
					case SDLK_c: keys[KEY_C] = false; break;
				}
			} break;
		}
	}
}

/*
emcc build:
emcc main.c -O2 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS=["png"] --preload-file res -o emscripten-build-dir\index.html --shell-file emscripten-build-dir\shell_minimal.html
*/

int main(int argc, char **argv) {
	srand(time(0));

	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_PNG);
	#ifndef __EMSCRIPTEN__
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	#endif
	
	window = SDL_CreateWindow(
		"Sewage Control", 
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
		WIDTH, HEIGHT, 
		0
	);
	SDL_SetWindowIcon(window, IMG_Load("res/icon.png"));
	
	bool vsync = true;
	renderer = SDL_CreateRenderer(window, -1, vsync ? SDL_RENDERER_PRESENTVSYNC : 0);
	backbuffer = SDL_CreateTexture(renderer, SDL_GetWindowPixelFormat(window), SDL_TEXTUREACCESS_TARGET, WIDTH, HEIGHT);
	
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	initText();
	loadTextures();
	loadSounds();

#if 1
	// Use these when you ship numpty
	currentState = Menu;
	menuSelectedOption = 0;
	menuOffset = 600-32;	
#else
	currentState = GameOver;
#endif
	
	initField(10, 18);
	field->heldPiece.id = -1;
	newPiece();
	
	lastTime = getTime();
	lastTimeFps = getTime();
	
	#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(main_loop, 0, 1);
	#else
	while(running) {	
		main_loop();
	}
	#endif
	
	return 0;
}
