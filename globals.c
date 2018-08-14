typedef struct Color {
	Uint8 r;
	Uint8 g;
	Uint8 b;
	Uint8 a;
} Color;
#define color_lit(c) (c).r, (c).g, (c).b, (c).a

typedef enum State {
	Menu,
	Playing,
	GameOver,
	Help,
	Paused,
} State;

typedef struct Tile {
	bool unavailable;
	bool occupied;
	int id;
} Tile;

typedef struct Piece {
	int id;
	int rotation;
	int posX;
	int posY;
} Piece;

SDL_Texture* pieceTexture[7];

// Piece, Rotation
int pieces[7][4][16] = {
	{
		{ // Box
			0, 0, 0, 0,
			0, 1, 1, 0,
			0, 1, 1, 0,
			0, 0, 0, 0,
		},
		{ // Box
			0, 0, 0, 0,
			0, 1, 1, 0,
			0, 1, 1, 0,
			0, 0, 0, 0,
		},
		{ // Box
			0, 0, 0, 0,
			0, 1, 1, 0,
			0, 1, 1, 0,
			0, 0, 0, 0,
		},
		{ // Box
			0, 0, 0, 0,
			0, 1, 1, 0,
			0, 1, 1, 0,
			0, 0, 0, 0,
		},
	},
	{
		{ // I
			0, 0, 0, 0,
			1, 1, 1, 1,
			0, 0, 0, 0,
			0, 0, 0, 0,
		},
		{ // I
			0, 1, 0, 0,
			0, 1, 0, 0,
			0, 1, 0, 0,
			0, 1, 0, 0,
		},
		{ // I
			0, 0, 0, 0,
			1, 1, 1, 1,
			0, 0, 0, 0,
			0, 0, 0, 0,
		},
		{ // I
			0, 1, 0, 0,
			0, 1, 0, 0,
			0, 1, 0, 0,
			0, 1, 0, 0,
		},
	},
	{
		{ // T
			0, 0, 0, 0,
			1, 1, 1, 0,
			0, 1, 0, 0,
			0, 0, 0, 0,
		},
		{
			0, 1, 0, 0,
			1, 1, 0, 0,
			0, 1, 0, 0,
			0, 0, 0, 0,
		},
		{
			0, 1, 0, 0,
			1, 1, 1, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
		},
		{
			0, 1, 0, 0,
			0, 1, 1, 0,
			0, 1, 0, 0,
			0, 0, 0, 0,
		},
	},
	{
		{ // J
			1, 0, 0, 0,
			1, 1, 1, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
		},
		{
			0, 1, 1, 0,
			0, 1, 0, 0,
			0, 1, 0, 0,
			0, 0, 0, 0,
		},
		{
			0, 0, 0, 0,
			1, 1, 1, 0,
			0, 0, 1, 0,
			0, 0, 0, 0,
		},
		{
			0, 1, 0, 0,
			0, 1, 0, 0,
			1, 1, 0, 0,
			0, 0, 0, 0,
		},
	},
	{
		{ // L
			0, 0, 1, 0,
			1, 1, 1, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
		},
		{
			0, 1, 0, 0,
			0, 1, 0, 0,
			0, 1, 1, 0,
			0, 0, 0, 0,
		},
		{
			0, 0, 0, 0,
			1, 1, 1, 0,
			1, 0, 0, 0,
			0, 0, 0, 0,
		},
		{
			1, 1, 0, 0,
			0, 1, 0, 0,
			0, 1, 0, 0,
			0, 0, 0, 0,
		},
	},
	{
		{ // S
			0, 1, 1, 0,
			1, 1, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
		},
		{
			0, 1, 0, 0,
			0, 1, 1, 0,
			0, 0, 1, 0,
			0, 0, 0, 0,
		},
		{
			0, 0, 0, 0,
			0, 1, 1, 0,
			1, 1, 0, 0,
			0, 0, 0, 0,
		},
		{
			1, 0, 0, 0,
			1, 1, 0, 0,
			0, 1, 0, 0,
			0, 0, 0, 0,
		},
	},
	{
		{ // Z
			1, 1, 0, 0,
			0, 1, 1, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
		},
		{
			0, 0, 1, 0,
			0, 1, 1, 0,
			0, 1, 0, 0,
			0, 0, 0, 0,
		},
		{
			0, 0, 0, 0,
			1, 1, 0, 0,
			0, 1, 1, 0,
			0, 0, 0, 0,
		},
		{
			0, 1, 0, 0,
			1, 1, 0, 0,
			1, 0, 0, 0,
			0, 0, 0, 0,
		},
	},
};

typedef struct Field {
	int width;
	int height;
	Tile *tiles;
	double shadowHeight;
	Piece fallingPiece;
	Piece nextPieces[3];
	Piece ghostPiece;
	Piece heldPiece;
} Field;

typedef struct Particle {
	bool used;
	double x, y;
	double vx, vy;
	double age;
	double maxAge;
	SDL_Rect *bbox;
	SDL_Texture *texture;
} Particle;
#define PARTICLE_COUNT (1024)
Particle particles[PARTICLE_COUNT];


#define WIDTH (800)
#define HEIGHT (600)

SDL_Window *window = 0;
SDL_Renderer *renderer = 0;
SDL_Texture *backbuffer = 0;

SDL_Texture *topTexture = 0;
SDL_Texture *poopTexture = 0;
SDL_Texture *menuTexture = 0;
SDL_Texture *titleTexture = 0;

SDL_Texture *brownTexture = 0;
SDL_Texture *yellowTexture = 0;
SDL_Texture *cyanTexture = 0;
SDL_Texture *redTexture = 0;
SDL_Texture *magentaTexture = 0;
SDL_Texture *greyTexture = 0;
SDL_Texture *greenTexture = 0;

#ifndef __EMSCRIPTEN__
	Mix_Chunk *lineClearSound = 0;
	Mix_Chunk *flushSound = 0;
	Mix_Chunk *flushFastSound = 0;
	Mix_Chunk *menuSound = 0;
	Mix_Chunk *plopSound = 0;
#endif

State currentState;
bool running = true;

Field *field;

int score;
int lastClearedLines;

#define TILE_SIZE 32

enum {
	KEY_UP = 0,
	KEY_DOWN,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_SPACE,
	KEY_ENTER,
	KEY_ESC,
	KEY_C,
	KEY_ANY,

	NUM_KEYS,
};
// hey dummy, if you are adding a key remember to add it into the loop and add the enum
bool keys[NUM_KEYS];
bool pkeys[NUM_KEYS];

double t = 0.0;
const double dt = 1/60.0;

double menuOffset = 0;
double menuOffsetTarget = 0.0;
bool fadeToPlayInitiated = false;
bool fadeToMenuInitiated = false;
bool fadeToPausedInitiated = false;
State fadeToPauseTargetState = Paused;

bool swappedHeldThisTurn = false;

double shadowSpeed = 2.0;

double pieceMoveDelay = 0.0;
double downForceDelay = 0.1;

//double timeUntilFlush = 20.0;
double timeUntilFlush = 10.0;
double flushTimer = 0.0;
double flushSpeed = 10.0;

double screenshakeX = 0.0;
double screenshakeY = 0.0;
double screenshakeTimer = 0.0;
double screenshakeWeightX = 0.0;
double screenshakeWeightY = 0.0;

Color timeUntilFlushColor = {255, 255, 255, 255};

char *menuOption[] = {
	"PLAY",
	"HELP",
	"EXIT",
};
char *menuOptionSelected[] = {
	"> PLAY <",
	"> HELP <" ,
	"> EXIT <",	
};
char* pauseOption[] = {
	"RESUME",
	"BACK TO MENU",
	"EXIT",
};
char* pauseOptionSelected[] = {
	"> RESUME <",
	"> BACK TO MENU <",
	"> EXIT <",
};

int menuOptions = 3;
int pauseOptions = 3;

 // We share this one, I hope this works out
int menuSelectedOption = 0;