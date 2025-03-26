#include "p_main.h"

#include "gamedef.h"
#include "r_main.h"
#include "ui_main.h"
#include "mix_main.h"

#include <stdbool.h>
#include <math.h>
#include <SDL2/SDL.h>

#define PARALLAX_SPEED 0.5f

#define MAX_SPACESHIP_COUNT 16
#define MAX_BULLET_COUNT 32
#define MAX_EXPLOSION_COUNT 32
#define MAX_SCOREPOD_COUNT 16

//how long is each frame of explosion animation
#define EXPLOSION_FRAMETIME 0.04f

#define SIDE_PLAYER 1
#define SIDE_ENEMY 2

#define INITIAL_SPAWN_INTERVAL 1
#define SPAWN_INTERVAL 2

#define PLAYER_SPEED 8
#define PLAYER_COOLDOWN 0.3f
#define PLAYER_BULLET_SPEED 15

#define ENEMY_SPEED 8
#define ENEMY_COOLDOWN 1.0f
#define ENEMY_BULLET_SPEED 15
#define ENEMY_SCORE 3 //score gained for each enemy defeated

#define SCOREPOD_SPEED 8
#define SCOREPOD_SIZE 0.7f

#define HELL_TRIGGER 300
#define HELL_SPAWN_INTERVAL 1.2

typedef struct {
    bool dead;

    char side; //player or enemy?

    float posX;
    float posY;

    float width;
    float height;

    float velX;
    float velY;

    float cooldown; //cooldown until next shot

    SDL_Texture* texture;
} spaceship_t;

typedef struct {
    bool dead;

    char side;

    float posX;
    float posY;

    float width;
    float height;

    float velX;
    float velY;

    SDL_Texture* texture;
} bullet_t;

typedef struct {
    bool dead;

    float posX;
    float posY;

    float elapsed; //elapsed time since spawn
} explosion_t;

typedef struct {
    bool dead;

    float posX;
    float posY;

    int score;

    SDL_Texture* texture;
} scorepod_t;

SDL_Texture* background = NULL;
SDL_Texture* parallax = NULL;
SDL_Texture* explosionTexture = NULL;
SDL_Texture* enemyTexture = NULL;
SDL_Texture* playerBulletTexture = NULL;
SDL_Texture* enemyBulletTexture = NULL;
SDL_Texture* scorepodTexture1 = NULL;
SDL_Texture* scorepodTexture2 = NULL;

SDL_Texture* font = NULL;

Mix_Music* music = NULL;
Mix_Chunk* laserSfx = NULL;
Mix_Chunk* explosionSfx = NULL;

int explosionFrameWidth, explosionFrameHeight;

spaceship_t ships[MAX_SPACESHIP_COUNT];
spaceship_t* player = NULL;

bullet_t bullets[MAX_BULLET_COUNT];

explosion_t explosions[MAX_EXPLOSION_COUNT];

scorepod_t scorepods[MAX_SCOREPOD_COUNT];

bool isPaused = false; //is game paused
bool hellMode = false; //hard mode
float parallaxPos = 0.0f; //horizontal position of parallax
int currentState = 0; //is in 0.main menu - 1.game mode - 2.death screen
int score = 0; //current player score
int highScore = 0; //highest score ever gained
bool newRecord = false; //new high score has been reached
float timeScale = 1.0f; //controls the speed of time
float currentTime = 0.0f, deltaTime = 0.0f, fixedDt = ( 1.0f / 60.0f );
float tick = 0.0f; //time passed since last fixed update
float toSpawn = INITIAL_SPAWN_INTERVAL; //time remaining until next enemy spawn

void p_init(void) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
            "play | initializing the system");

    background = r_loadTexture("gfx/background.jpg");
    parallax = r_loadTexture("gfx/parallax.png");
    explosionTexture = r_loadTexture("gfx/explosion.png");
    enemyTexture = r_loadTexture("gfx/Enemy.png");
    playerBulletTexture = r_loadTexture("gfx/VLaser.png");
    enemyBulletTexture = r_loadTexture("gfx/ECircle.png");
    font = r_loadTexture("gfx/font.png");
    scorepodTexture1 = r_loadTexture("gfx/scorepod-1.png");
    scorepodTexture2 = r_loadTexture("gfx/scorepod-2.png");

    music = mix_loadMusic("sfx/music.mp3");
    laserSfx = mix_loadChunk("sfx/laser.mp3");
    explosionSfx = mix_loadChunk("sfx/explosion.wav");

    int explosionTextureWidth, explosionTextureHeight;
    SDL_QueryTexture(explosionTexture, NULL, NULL,
            &explosionTextureWidth, &explosionTextureHeight);
    explosionFrameWidth = explosionTextureWidth / 4;
    explosionFrameHeight = explosionTextureHeight / 4;

    for (int i=1; i<MAX_SPACESHIP_COUNT; i++) ships[i].dead = true;
    for (int i=0; i<MAX_BULLET_COUNT; i++) bullets[i].dead = true;
    for (int i=0; i<MAX_EXPLOSION_COUNT; i++) explosions[i].dead = true;
    for (int i=0; i<MAX_SCOREPOD_COUNT; i++) scorepods[i].dead = true;

    /* setting up the player */
    //the first index of the array is reserved for the player
    player = &ships[0]; 
    player->dead = false;
    player->side = SIDE_PLAYER;
    player->posX = 1.0f;
    player->posY = HEIGHT_LENGTH / 2.0f - 0.5f;
    player->width = 1.0f;
    player->height = 1.0f;
    player->texture = r_loadTexture("gfx/Player.png");
}

void p_shutdown(void) {}

//restarts the game scene
void restart(void) {
    hellMode = false;
    score = 0;
    newRecord = false;
    tick = 0.0f;
    toSpawn = INITIAL_SPAWN_INTERVAL;

    for (int i=1; i<MAX_SPACESHIP_COUNT; i++) ships[i].dead = true;
    for (int i=0; i<MAX_BULLET_COUNT; i++) bullets[i].dead = true;
    for (int i=0; i<MAX_EXPLOSION_COUNT; i++) explosions[i].dead = true;
    for (int i=0; i<MAX_SCOREPOD_COUNT; i++) scorepods[i].dead = true;

    player->dead = false;
    player->side = SIDE_PLAYER;
    player->posX = 1.0f;
    player->posY = HEIGHT_LENGTH / 2.0f - 0.5f;
    player->velX = 0.0f;
    player->velY = 0.0f;
}

//searches the ships array and returns the first free one
spaceship_t* spawnShip() {
    //since the player ship is always at index 0,
    //we start the search from index 1
    for (int i = 1; i < MAX_SPACESHIP_COUNT; i++) {
        spaceship_t* ship = &ships[i];
        if (ship->dead) {
            ship->dead = false;
            return ship;
        }
    }

    return NULL;
}

//returns a free bullet to use from bullets array
bullet_t* spawnBullet() {
    for (int i = 0; i < MAX_BULLET_COUNT; i++) {
        bullet_t* bullet = &bullets[i];
        if (bullet->dead) {
            bullet->dead = false;
            return bullet;
        }
    }

    return NULL;
}

//returns a free explosion from the array
explosion_t* spawnExplosion() {
    for (int i = 0; i < MAX_EXPLOSION_COUNT; i++) {
        explosion_t* explosion = &explosions[i];
        if (explosion->dead) {
            explosion->dead = false;
            return explosion;
        }
    }

    return NULL;
}

//returns a free scorepod from the array
scorepod_t* spawnScorepod() {
    for (int i = 0; i < MAX_SCOREPOD_COUNT; i++) {
        scorepod_t* scorepod = &scorepods[i];
        if (scorepod->dead) {
            scorepod->dead = false;
            return scorepod;
        }
    }

    return NULL;
}

//increases player score
void addScore(int amount) {
    score += amount;
    if (score > highScore) {
        highScore = score;
        newRecord = true;
    }

    if (score >= HELL_TRIGGER) hellMode = true;
}

//checks keyboard events
void doKeyboard(SDL_KeyboardEvent *event, bool down) {
    if (event->repeat) return;

    switch (event->keysym.scancode) {
        case SDL_SCANCODE_P:
            if (down && currentState == 1 && !player->dead) {
                if (isPaused) {
                    isPaused = false;
                    timeScale = 1.0f;
                    Mix_ResumeMusic();
                }
                else {
                    isPaused = true;
                    timeScale = 0.0f;
                    Mix_PauseMusic();
                }
            }
            break;

        //player wants to shoot
        case SDL_SCANCODE_F:
            if (down && currentState != 1) {
                if (currentState != 0) currentState = 0;
                else currentState = 1;
                restart();
            }

            else if (down && player->cooldown <= 0.0f &&
                    !player->dead &&
                    !isPaused) {
                bullet_t* bullet = spawnBullet();
                if (bullet != NULL) {
                    bullet->side = SIDE_PLAYER;
                    bullet->posX = player->posX;
                    bullet->posY = player->posY + 0.45f; //middle of player height
                    bullet->width = 0.3f;
                    bullet->height = 0.1f;
                    bullet->velX = PLAYER_BULLET_SPEED;
                    bullet->velY = 0.0f;
                    bullet->texture = playerBulletTexture;
                    player->cooldown += PLAYER_COOLDOWN;

                    Mix_PlayChannel(-1, laserSfx, 0);
                }
            }
            break;

        case SDL_SCANCODE_UP:
            if (currentState == 1) player->velY = down ? -PLAYER_SPEED : 0.0f;
            break;

        case SDL_SCANCODE_DOWN:
            if (currentState == 1) player->velY = down ? PLAYER_SPEED : 0.0f;
            break;

        case SDL_SCANCODE_LEFT:
            if (currentState == 1) player->velX = down ? -PLAYER_SPEED : 0.0f;
            break;

        case SDL_SCANCODE_RIGHT:
            if (currentState == 1) player->velX = down ? PLAYER_SPEED : 0.0f;
            break;
    }
}

void p_mainloop(void) {
    //play the music
    Mix_PlayMusic(music, -1);

    bool quit = false;
    currentTime = SDL_GetTicks() / 1000.0f;
    float previousTime = currentTime;
    while (!quit) {
        currentTime = SDL_GetTicks() / 1000.0f;
        deltaTime = (currentTime - previousTime) * timeScale;

        //decrease player cooldown
        if (player->cooldown > 0.0f) player->cooldown -= deltaTime;

        /* event polling */
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = true;
                    break;

                case SDL_WINDOWEVENT:
                    switch (event.window.event) {
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                            r_refresh(event.window.data1, event.window.data2);
                            break;
                    }
                    break;

                case SDL_KEYDOWN:
                    doKeyboard(&event.key, true);
                    break;

                case SDL_KEYUP:
                    doKeyboard(&event.key, false);
                    break;
            }
        }

        /* game update */
        //fixed update
        tick += deltaTime;
        while (tick >= fixedDt) {
            tick -= fixedDt;

            //spawn enemies at specified time intervals
            if (toSpawn <= 0.0f) {
                spaceship_t* enemy = spawnShip();
                if (enemy != NULL) {
                    enemy->side = SIDE_ENEMY;
                    enemy->posX = ASPECT_RATIO * HEIGHT_LENGTH;
                    enemy->posY = rand() % (HEIGHT_LENGTH - 1);
                    enemy->width = 1.0f;
                    enemy->height = 1.0f;
                    enemy->velX = -ENEMY_SPEED;
                    enemy->velY = 0.0f;
                    enemy->texture = enemyTexture;
                }

                if (hellMode) toSpawn = HELL_SPAWN_INTERVAL;
                else toSpawn = SPAWN_INTERVAL;
            }

            else if (currentState != 0) toSpawn -= fixedDt;

            /* moving spaceships - starting with player */
            if (!player->dead) {
                player->posX += player->velX * fixedDt;
                player->posY += player->velY * fixedDt;
            }

            //do not let player get out of boundaries
            if (player->posY < 0.0f)
                player->posY = 0.0f;
            else if (player->posY + player->height > HEIGHT_LENGTH)
                player->posY = (float)HEIGHT_LENGTH - player->height;

            if (player->posX < 0.0f)
                player->posX = 0.0f;
            else if (player->posX + player->width > HEIGHT_LENGTH*ASPECT_RATIO)
                player->posX = HEIGHT_LENGTH*ASPECT_RATIO - player->width;

            //moving non player ships
            for (int i = 1; i < MAX_SPACESHIP_COUNT; i++) {
                spaceship_t* ship = &ships[i];
                if (ship->dead) continue;

                ship->posX += ship->velX * fixedDt;
                ship->posY += ship->velY * fixedDt;

                //kill the spaceship if it gets out of the screen from left side
                if (ship->posX + ship->width < 0.0f) {
                    ship->dead = true;
                    continue;
                }

                //shoot bullet
                if (ship->cooldown > 0.0f) ship->cooldown -= fixedDt;
                else {
                    //calculate velocity vector (homing missle)
                    float steps = 0.0f;
                    float diffX = ship->posX - player->posX;
                    float diffY = ship->posY - player->posY;

                    //tan(20 degrees) == 0.36397023426f
                    if (fabsf(diffY / diffX) > 0.36397023426f ||
                            diffX <= 0.0f) continue;

                    if (diffX > diffY) steps = diffX;
                    else steps = diffY;

                    bullet_t* bullet = spawnBullet();
                    if (bullet == NULL) continue;

                    bullet->side = SIDE_ENEMY;
                    bullet->posX = ship->posX;
                    bullet->posY = ship->posY + 0.5f;
                    bullet->width = 0.2f;
                    bullet->height = 0.2f;
                    bullet->velX = -diffX / steps * ENEMY_BULLET_SPEED;
                    bullet->velY = -diffY / steps * ENEMY_BULLET_SPEED;
                    bullet->texture = enemyBulletTexture;
                    ship->cooldown += ENEMY_COOLDOWN;
                }
            }

            /* moving bullets */
            for (int i = 0; i < MAX_BULLET_COUNT; i++) {
                bullet_t* bullet = &bullets[i];
                if (bullet->dead) continue;

                bullet->posX += bullet->velX * fixedDt;
                bullet->posY += bullet->velY * fixedDt;

                //kill the bullet if it gets out of the screen
                if (bullet->posX + bullet->width < 0.0f ||
                        bullet->posX > HEIGHT_LENGTH*ASPECT_RATIO ||
                        bullet->posY > HEIGHT_LENGTH ||
                        bullet->posY + bullet->height < 0.0f) {
                    bullet->dead = true;
                    continue;
                }

                //check if the bullet contacts with any spaceship
                for (int j = 0; j < MAX_SPACESHIP_COUNT; j++) {
                    spaceship_t* ship = &ships[j];
                    if (ship->dead) continue;

                    if (ship->side != bullet->side &&
                            bullet->posX + bullet->width >= ship->posX &&
                            bullet->posX <= ship->posX + ship->width &&
                            bullet->posY + bullet->height >= ship->posY &&
                            bullet->posY <= ship->posY + ship->height) {
                        bullet->dead = true;
                        ship->dead = true;

                        explosion_t* explosion = spawnExplosion();
                        if (explosion == NULL) break;

                        explosion->posX = ship->posX;
                        explosion->posY = ship->posY;
                        explosion->elapsed = 0.0f;

                        Mix_PlayChannel(-1, explosionSfx, 0);

                        if (ship->side == SIDE_PLAYER) {
                            currentState = 2;
                            break;
                        }

                        if (!player->dead) addScore(ENEMY_SCORE);
                        scorepod_t* scorepod = spawnScorepod();
                        int chance = rand() % 100;
                        if (chance >= 75) {
                            scorepod->score = 2;
                            scorepod->texture = scorepodTexture2;
                        }

                        else {
                            scorepod->score = 1;
                            scorepod->texture = scorepodTexture1;
                        }
                        scorepod->posX = ship->posX;
                        scorepod->posY = ship->posY;
                        break;
                    }
                }
            }

            /* moving scorepods */
            for (int i = 0; i < MAX_SCOREPOD_COUNT; i++) {
                scorepod_t* scorepod = &scorepods[i];
                if (scorepod->dead) continue;

                scorepod->posX -= SCOREPOD_SPEED * fixedDt;
                if (scorepod->posX + SCOREPOD_SIZE <= 0.0f) {
                    scorepod->dead = true;
                    continue;
                }

                //check if scorepod collides with player
                if (player->dead) continue;
                if (scorepod->posX + SCOREPOD_SIZE >= player->posX &&
                        scorepod->posX <= player->posX + 1.0f &&
                        scorepod->posY + SCOREPOD_SIZE >= player->posY &&
                        scorepod->posY <= player->posY + 1.0f) {
                    scorepod->dead = true;
                    addScore(scorepod->score);
                }
            }
        }

        // updating explosions
        for (int i = 0; i < MAX_EXPLOSION_COUNT; i++) {
            explosion_t* explosion = &explosions[i];
            if (explosion->dead) continue;

            explosion->elapsed += deltaTime;
            if (explosion->elapsed >= 16 * EXPLOSION_FRAMETIME) {
                explosion->dead = true;
                continue;
            }
        }

        //updating the parallax
        parallaxPos -= PARALLAX_SPEED * deltaTime;
        if (parallaxPos + HEIGHT_LENGTH * ASPECT_RATIO < 0.0f) {
            parallaxPos = HEIGHT_LENGTH * ASPECT_RATIO;
        }

        /* rendering */
        SDL_FRect coords;
        r_setColor(0, 0, 0, 255);
        r_clearScreen();

        //rendering the background
        coords.x = 0.0f;
        coords.y = 0.0f;
        coords.w = HEIGHT_LENGTH*ASPECT_RATIO;
        coords.h = HEIGHT_LENGTH;
        r_renderTexture(background, &coords);

        //rendering the parallax
        coords.x = parallaxPos;
        coords.w = HEIGHT_LENGTH * ASPECT_RATIO;
        coords.h = HEIGHT_LENGTH;
        r_renderTexture(parallax, &coords);

        if (parallaxPos < 0.0f) {
            coords.x += coords.w;
            r_renderTexture(parallax, &coords);
        }

        else if (parallaxPos > 0.0f) {
            coords.x -= coords.w;
            r_renderTexture(parallax, &coords);
        }

        //rendering bullets
        for (int i = 0; i < MAX_BULLET_COUNT; i++) {
            bullet_t* bullet = &bullets[i];
            if (bullet->dead) continue;

            coords.x = bullet->posX;
            coords.y = bullet->posY;
            coords.w = bullet->width;
            coords.h = bullet->height;
            r_renderTexture(bullet->texture, &coords);
        }

        //rendering spaceships
        for (int i = 0; i < MAX_SPACESHIP_COUNT; i++) {
            spaceship_t* ship = &ships[i];
            if (ship->dead) continue;

            coords.x = ship->posX;
            coords.y = ship->posY;
            coords.w = ship->width;
            coords.h = ship->height;
            r_renderTexture(ship->texture, &coords);
        }

        //rendering explosions
        for (int i = 0; i < MAX_EXPLOSION_COUNT; i++) {
            explosion_t* explosion = &explosions[i];
            if (explosion->dead) continue;

            int frame = (int)(explosion->elapsed / EXPLOSION_FRAMETIME);
            int frameY = (int)(frame / 4);
            int frameX = frame - frameY * 4;

            SDL_Rect crop;
            crop.x = frameX * explosionFrameWidth;
            crop.y = frameY * explosionFrameHeight;
            crop.w = explosionFrameWidth;
            crop.h = explosionFrameHeight;

            coords.x = explosion->posX - 0.2f;
            coords.y = explosion->posY - 0.2f;
            coords.w = 1.4f;
            coords.h = 1.4f;
            r_renderCrop(explosionTexture, &crop, &coords);
        }

        //rendering scorepods
        for (int i = 0; i < MAX_SCOREPOD_COUNT; i++) {
            scorepod_t* scorepod = &scorepods[i];
            if (scorepod->dead) continue;

            coords.x = scorepod->posX;
            coords.y = scorepod->posY;
            coords.w = SCOREPOD_SIZE;
            coords.h = SCOREPOD_SIZE;
            r_renderTexture(scorepod->texture, &coords);
        }

        /* GUI */
        switch (currentState) {
            //main menu
            case 0:
                coords.x = UI_HEIGHT * ASPECT_RATIO / 2.0f;
                coords.y = UI_HEIGHT / 2.0f - 0.5f;
                coords.w = 1.0f;
                coords.h = 1.0f;
                ui_drawText(font, &coords, 1, "STARMarine Jack");
                coords.y += 1.5f;
                coords.w = 0.5f;
                coords.h = 0.5f;
                ui_drawText(font, &coords, 1, "Press F to fire/start game");
                coords.y += 0.7f;
                ui_drawText(font, &coords, 1, "Press P to pause");
                coords.y += 0.7f;
                ui_drawText(font, &coords, 1, "Use Arrow keys to move");
                break;

            //HUD
            case 1:
                coords.x = 0.0f;
                coords.y = UI_HEIGHT - 0.6f;
                coords.w = 0.6f;
                coords.h = 0.6f;
                ui_drawText(font, &coords, 0, "SCORE-%03d", score);

                if (hellMode) {
                    r_setTextureColor(font, 255, 0, 0);
                    coords.x = UI_HEIGHT * ASPECT_RATIO / 2.0f;
                    ui_drawText(font, &coords, 1, "WELCOME TO HELL");
                    r_setTextureColor(font, 255, 255, 255);
                }

                coords.x = UI_HEIGHT * ASPECT_RATIO;
                ui_drawText(font, &coords, 2, "HIGH SCORE-%03d", highScore);

                if (isPaused) {
                    coords.x = UI_HEIGHT * ASPECT_RATIO / 2.0f;
                    coords.y = UI_HEIGHT / 2.0f - 0.3f;
                    ui_drawText(font, &coords, 1, "PAUSED");
                    coords.y = coords.y + 1.0f;
                    coords.w = 0.4f;
                    coords.h = 0.4f;
                    ui_drawText(font, &coords, 1, "Press P to resume");
                }
                break;

            //death screen
            case 2:
                coords.x = UI_HEIGHT * ASPECT_RATIO / 2.0f;
                coords.y = 2;
                coords.w = 0.6f;
                coords.h = 0.6f;
                ui_drawText(font, &coords, 1, "GAME OVER");

                coords.w = 0.4f;
                coords.h = 0.4f;
                if (newRecord) {
                    coords.y += 1.0f;
                    ui_drawText(font, &coords, 1, "New high score achieved!");
                }

                coords.y += 0.8f;
                ui_drawText(font, &coords, 1, "SCORE: %03d", score);
                coords.y += 1.0f;
                coords.w = 0.4f;
                coords.h = 0.4f;
                ui_drawText(font, &coords, 1, "Press F to continue");
                break;
        }

        r_present(); //present rendered content to screen

        previousTime = currentTime;

        /* Framerate cap */
        SDL_Delay(16); //try to remain close to 60fps
    }
}
