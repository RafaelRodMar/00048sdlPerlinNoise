#pragma once

#include <vector>
#include <list>
#include <SDL.h>
#include <SDL_image.h>
#include "Vector2D.h"
#include "AssetsManager.h"
#include "InputHandler.h"
#include "Entity.h"
#include "board.h"

////Game States
enum GAMESTATES {SPLASH, MENU, LOAD_LEVEL, GAME, END_GAME};
enum SUBSTATES {LINES, TILES, TERRAIN, TERRAINRECT};

class Game {
public:
	static Game* Instance()
	{
		if (s_pInstance == 0)
		{
			s_pInstance = new Game();
			return s_pInstance;
		}
		return s_pInstance;
	}
	
	SDL_Renderer* getRenderer() const { return m_pRenderer; }
	
	~Game();

	bool init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen);
	void render();
	void update();
	void handleEvents();
	void clean();
	void quit();

	bool running() { return m_bRunning; }

	int getGameWidth() const { return m_gameWidth; }
	int getGameHeight() const { return m_gameHeight; }
	void PerlinNoise1D(int nCount, float *fSeed, int nOctaves, float fBias, float *fOutput);
	void PerlinNoise2D(int nWidth, int nHeight, float *fSeed, int nOctaves, float fBias, float *fOutput);

private:
	Game();
	static Game* s_pInstance;
	SDL_Window* m_pWindow;
	SDL_Renderer* m_pRenderer;

	player *p;
	int state = -1;
	int subState = -1;
	int cellSize = 20;

	class Board *mapa;

	//1D noise
	float *fNoiseSeed1D = nullptr;
	float *fPerlinNoise1D = nullptr;
	int nOutputSize = 256;

	//2D noise
	int nOutputWidth = 256;
	int nOutputHeight = 256;
	float *fNoiseSeed2D = nullptr;
	float *fPerlinNoise2D = nullptr;

	//
	int nOctaveCount = 1;
	float fScalingBias = 2.0f;
	int nMode = 1;

	std::list<Entity*> entities;
	bool isCollide(Entity *a, Entity *b);
	bool isCollideRect(Entity *a, Entity *b);
	//std::vector<GameObject*> m_gameObjects;

	bool m_bRunning;
	int m_gameWidth;
	int m_gameHeight;
};