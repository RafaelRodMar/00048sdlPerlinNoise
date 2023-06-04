#include<SDL.h>
#include<sdl_ttf.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <list>
#include <cmath>
#include <fstream>
#include <sstream>
#include <time.h>
#include "game.h"
#include "json.hpp"
#include <chrono>
#include <random>

class Rnd {
public:
	std::mt19937 rng;

	Rnd()
	{
		std::mt19937 prng(std::chrono::steady_clock::now().time_since_epoch().count());
		rng = prng;
	}

	int getRndInt(int min, int max)
	{
		std::uniform_int_distribution<int> distribution(min, max);
		return distribution(rng);
	}

	double getRndDouble(double min, double max)
	{
		std::uniform_real_distribution<double> distribution(min, max);
		return distribution(rng);
	}
} rnd;

//la clase juego:
Game* Game::s_pInstance = 0;

Game::Game()
{
	m_pRenderer = NULL;
	m_pWindow = NULL;
}

Game::~Game()
{

}

SDL_Window* g_pWindow = 0;
SDL_Renderer* g_pRenderer = 0;

bool Game::init(const char* title, int xpos, int ypos, int width,
	int height, bool fullscreen)
{
	// almacenar el alto y ancho del juego.
	m_gameWidth = width;
	m_gameHeight = height;

	// attempt to initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{
		int flags = 0;
		if (fullscreen)
		{
			flags = SDL_WINDOW_FULLSCREEN;
		}

		std::cout << "SDL init success\n";
		// init the window
		m_pWindow = SDL_CreateWindow(title, xpos, ypos,
			width, height, flags);
		if (m_pWindow != 0) // window init success
		{
			std::cout << "window creation success\n";
			m_pRenderer = SDL_CreateRenderer(m_pWindow, -1, 0);
			if (m_pRenderer != 0) // renderer init success
			{
				std::cout << "renderer creation success\n";
				SDL_SetRenderDrawColor(m_pRenderer,
					255, 255, 255, 255);
			}
			else
			{
				std::cout << "renderer init fail\n";
				return false; // renderer init fail
			}
		}
		else
		{
			std::cout << "window init fail\n";
			return false; // window init fail
		}
	}
	else
	{
		std::cout << "SDL init fail\n";
		return false; // SDL init fail
	}
	if (TTF_Init() == 0)
	{
		std::cout << "sdl font initialization success\n";
	}
	else
	{
		std::cout << "sdl font init fail\n";
		return false;
	}

	std::cout << "init success\n";
	m_bRunning = true; // everything inited successfully, start the main loop

	//Joysticks
	InputHandler::Instance()->initialiseJoysticks();

	//load images, sounds, music and fonts
	//AssetsManager::Instance()->loadAssets();
	AssetsManager::Instance()->loadAssetsJson(); //ahora con formato json
	Mix_Volume(-1, 16); //adjust sound/music volume for all channels

	//ReadHiScores();
	
	state = GAME;
	subState = LINES;

	//generate the map dots (more 1 than 0)
	/*for (int i = 0; i < (640 / cellSize) + 1; i++) {
		for (int j = 0; j < (480 / cellSize) + 1; j++) {
			if (rnd.getRndInt(0, 100) < 80)
				board[i][j] = 1;
			else
				board[i][j] = 0;
		}
	}*/
	
	mapa = new Board(640,480,20,20);
	mapa->fillBoardWithPoints();
	//cout << "tamaño del mapa: " << mapa->board.size() << "x" << mapa->board[0].size() << endl;

	//SDL_RenderSetScale(Game::Instance()->getRenderer(), 2, 2);

	return true;
}

void Game::render()
{
	SDL_SetRenderDrawColor(Game::Instance()->getRenderer(), 255, 255, 255, 0);
	SDL_RenderClear(m_pRenderer); // clear the renderer to the draw color

	//title and menu
	AssetsManager::Instance()->Text("Marching Squares", "font", 640, 0, SDL_Color({ 0,0,0,255 }), Game::Instance()->getRenderer());
	AssetsManager::Instance()->Text("1-With lines", "font", 650, 30, SDL_Color({ 0,0,0,255 }), Game::Instance()->getRenderer());
	AssetsManager::Instance()->Text("2-With 2 tiles", "font", 650, 60, SDL_Color({ 0,0,0,255 }), Game::Instance()->getRenderer());
	AssetsManager::Instance()->Text("3-With 2 terrains", "font", 650, 90, SDL_Color({ 0,0,0,255 }), Game::Instance()->getRenderer());
	AssetsManager::Instance()->Text("4-With 2 terrains Rect", "font", 650, 120, SDL_Color({ 0,0,0,255 }), Game::Instance()->getRenderer());

	//draw a 640x480 rectangle
	SDL_SetRenderDrawColor(Game::Instance()->getRenderer(), 0, 0, 0, 255); //black
	SDL_RenderDrawLine(Game::Instance()->getRenderer(), 0, 480, 640, 480);
	SDL_RenderDrawLine(Game::Instance()->getRenderer(), 640, 0, 640, 480);

		if (state == MENU)
		{
		}

		if (state == GAME)
		{
			if(subState == LINES) mapa->showMarchingSquaresLines();
			if(subState == TILES) mapa->showMarchingSquaresTiles();
			if(subState == TERRAIN) mapa->showMarchingSquaresTerrain();
			if(subState == TERRAINRECT) mapa->showMarchingSquaresTerrainRect();
		}

		//if (state == MENU)
		//{
		//	if (subState == LINES)
		//	{
		//		//draw the dots
		//		for (int i = 0; i < (640 / cellSize) + 1; i++) {
		//			for (int j = 0; j < (480 / cellSize) + 1; j++) {
		//				if (board[i][j] == 0)
		//					SDL_SetRenderDrawColor(Game::Instance()->getRenderer(), 0, 0, 0, 255); //black
		//				else
		//					SDL_SetRenderDrawColor(Game::Instance()->getRenderer(), 255, 0, 0, 255); //red

		//				SDL_RenderDrawPoint(Game::Instance()->getRenderer(), i * 20, j * 20);
		//			}
		//		}

		//		//draw the lines
		//		SDL_SetRenderDrawColor(Game::Instance()->getRenderer(), 0, 0, 0, 255); //black
		//		for (int i = 0; i < (640 / cellSize); i++) {
		//			for (int j = 0; j < (480 / cellSize); j++) {
		//				int x = i * cellSize;
		//				int y = j * cellSize;
		//				Vector2D a = Vector2D(x + cellSize * 0.5, y);
		//				Vector2D b = Vector2D(x + cellSize, y + cellSize * 0.5);
		//				Vector2D c = Vector2D(x + cellSize * 0.5, y + cellSize);
		//				Vector2D d = Vector2D(x, y + cellSize * 0.5);
		//				int state = board[i][j] * 8 + board[i + 1][j] * 4 + board[i + 1][j + 1] * 2 + board[i][j + 1];

		//				switch (state) {
		//				case 0:
		//					//do nothing
		//					break;
		//				case 1:
		//					SDL_RenderDrawLine(Game::Instance()->getRenderer(), c.m_x, c.m_y, d.m_x, d.m_y);
		//					break;
		//				case 2:
		//					SDL_RenderDrawLine(Game::Instance()->getRenderer(), b.m_x, b.m_y, c.m_x, c.m_y);
		//					break;
		//				case 3:
		//					SDL_RenderDrawLine(Game::Instance()->getRenderer(), b.m_x, b.m_y, d.m_x, d.m_y);
		//					break;
		//				case 4:
		//					SDL_RenderDrawLine(Game::Instance()->getRenderer(), a.m_x, a.m_y, b.m_x, b.m_y);
		//					break;
		//				case 5:
		//					SDL_RenderDrawLine(Game::Instance()->getRenderer(), a.m_x, a.m_y, d.m_x, d.m_y);
		//					SDL_RenderDrawLine(Game::Instance()->getRenderer(), b.m_x, b.m_y, c.m_x, c.m_y);
		//					break;
		//				case 6:
		//					SDL_RenderDrawLine(Game::Instance()->getRenderer(), a.m_x, a.m_y, c.m_x, c.m_y);
		//					break;
		//				case 7:
		//					SDL_RenderDrawLine(Game::Instance()->getRenderer(), a.m_x, a.m_y, d.m_x, d.m_y);
		//					break;
		//				case 8:
		//					SDL_RenderDrawLine(Game::Instance()->getRenderer(), a.m_x, a.m_y, d.m_x, d.m_y);
		//					break;
		//				case 9:
		//					SDL_RenderDrawLine(Game::Instance()->getRenderer(), a.m_x, a.m_y, c.m_x, c.m_y);
		//					break;
		//				case 10:
		//					SDL_RenderDrawLine(Game::Instance()->getRenderer(), a.m_x, a.m_y, b.m_x, b.m_y);
		//					SDL_RenderDrawLine(Game::Instance()->getRenderer(), c.m_x, c.m_y, d.m_x, d.m_y);
		//					break;
		//				case 11:
		//					SDL_RenderDrawLine(Game::Instance()->getRenderer(), a.m_x, a.m_y, b.m_x, b.m_y);
		//					break;
		//				case 12:
		//					SDL_RenderDrawLine(Game::Instance()->getRenderer(), b.m_x, b.m_y, d.m_x, d.m_y);
		//					break;
		//				case 13:
		//					SDL_RenderDrawLine(Game::Instance()->getRenderer(), b.m_x, b.m_y, c.m_x, c.m_y);
		//					break;
		//				case 14:
		//					SDL_RenderDrawLine(Game::Instance()->getRenderer(), c.m_x, c.m_y, d.m_x, d.m_y);
		//					break;
		//				case 15:
		//					//do nothing
		//					break;
		//				default:
		//					break;
		//				}
		//			}
		//		}
		//	}

		//	if (subState == TILES)
		//	{
		//		/*AssetsManager::Instance()->draw("MarchingSquares20x20", 0, 0, 80, 80, Game::Instance()->getRenderer());
		//		AssetsManager::Instance()->drawTile("MarchingSquares20x20", 0, 0, 90, 0, 20, 20, 0, 0, Game::Instance()->getRenderer());
		//		AssetsManager::Instance()->drawTile("MarchingSquares20x20", 0, 0, 110, 0, 20, 20, 0, 1, Game::Instance()->getRenderer());
		//		AssetsManager::Instance()->drawTile("MarchingSquares20x20", 0, 0, 130, 0, 20, 20, 0, 2, Game::Instance()->getRenderer());
		//		AssetsManager::Instance()->drawTile("MarchingSquares20x20", 0, 0, 150, 0, 20, 20, 0, 3, Game::Instance()->getRenderer());*/

		//		//draw the tiles
		//		for (int i = 0; i < (640 / cellSize); i++) {
		//			for (int j = 0; j < (480 / cellSize); j++) {
		//				int x = i * cellSize;
		//				int y = j * cellSize;
		//				int state = board[i][j] * 8 + board[i + 1][j] * 4 + board[i + 1][j + 1] * 2 + board[i][j + 1];
		//				//string str = to_string(state);
		//				//AssetsManager::Instance()->Text(str, "font", x+5, y+5, SDL_Color({ 0,0,0,255 }), Game::Instance()->getRenderer());


		//				switch (state) {
		//				case 0:
		//					AssetsManager::Instance()->drawTile("MarchingSquares20x20", 0, 0, x, y, 20, 20, 0, 0, Game::Instance()->getRenderer());
		//					break;
		//				case 1:
		//					AssetsManager::Instance()->drawTile("MarchingSquares20x20", 0, 0, x, y, 20, 20, 0, 1, Game::Instance()->getRenderer());
		//					break;
		//				case 2:
		//					AssetsManager::Instance()->drawTile("MarchingSquares20x20", 0, 0, x, y, 20, 20, 0, 2, Game::Instance()->getRenderer());
		//					break;
		//				case 3:
		//					AssetsManager::Instance()->drawTile("MarchingSquares20x20", 0, 0, x, y, 20, 20, 0, 3, Game::Instance()->getRenderer());
		//					break;
		//				case 4:
		//					AssetsManager::Instance()->drawTile("MarchingSquares20x20", 0, 0, x, y, 20, 20, 1, 0, Game::Instance()->getRenderer());
		//					break;
		//				case 5:
		//					AssetsManager::Instance()->drawTile("MarchingSquares20x20", 0, 0, x, y, 20, 20, 1, 1, Game::Instance()->getRenderer());
		//					break;
		//				case 6:
		//					AssetsManager::Instance()->drawTile("MarchingSquares20x20", 0, 0, x, y, 20, 20, 1, 2, Game::Instance()->getRenderer());
		//					break;
		//				case 7:
		//					AssetsManager::Instance()->drawTile("MarchingSquares20x20", 0, 0, x, y, 20, 20, 1, 3, Game::Instance()->getRenderer());
		//					break;
		//				case 8:
		//					AssetsManager::Instance()->drawTile("MarchingSquares20x20", 0, 0, x, y, 20, 20, 2, 0, Game::Instance()->getRenderer());
		//					break;
		//				case 9:
		//					AssetsManager::Instance()->drawTile("MarchingSquares20x20", 0, 0, x, y, 20, 20, 2, 1, Game::Instance()->getRenderer());
		//					break;
		//				case 10:
		//					AssetsManager::Instance()->drawTile("MarchingSquares20x20", 0, 0, x, y, 20, 20, 2, 2, Game::Instance()->getRenderer());
		//					break;
		//				case 11:
		//					AssetsManager::Instance()->drawTile("MarchingSquares20x20", 0, 0, x, y, 20, 20, 2, 3, Game::Instance()->getRenderer());
		//					break;
		//				case 12:
		//					AssetsManager::Instance()->drawTile("MarchingSquares20x20", 0, 0, x, y, 20, 20, 3, 0, Game::Instance()->getRenderer());
		//					break;
		//				case 13:
		//					AssetsManager::Instance()->drawTile("MarchingSquares20x20", 0, 0, x, y, 20, 20, 3, 1, Game::Instance()->getRenderer());
		//					break;
		//				case 14:
		//					AssetsManager::Instance()->drawTile("MarchingSquares20x20", 0, 0, x, y, 20, 20, 3, 2, Game::Instance()->getRenderer());
		//					break;
		//				case 15:
		//					AssetsManager::Instance()->drawTile("MarchingSquares20x20", 0, 0, x, y, 20, 20, 3, 3, Game::Instance()->getRenderer());
		//					break;
		//				default:
		//					break;
		//				}
		//			}
		//		}
		//	}

		//	if (subState == TERRAIN)
		//	{
		//		/*AssetsManager::Instance()->draw("MarchingSquares20x20", 0, 0, 80, 80, Game::Instance()->getRenderer());
		//		AssetsManager::Instance()->drawTile("MarchingSquares20x20", 0, 0, 90, 0, 20, 20, 0, 0, Game::Instance()->getRenderer());
		//		AssetsManager::Instance()->drawTile("MarchingSquares20x20", 0, 0, 110, 0, 20, 20, 0, 1, Game::Instance()->getRenderer());
		//		AssetsManager::Instance()->drawTile("MarchingSquares20x20", 0, 0, 130, 0, 20, 20, 0, 2, Game::Instance()->getRenderer());
		//		AssetsManager::Instance()->drawTile("MarchingSquares20x20", 0, 0, 150, 0, 20, 20, 0, 3, Game::Instance()->getRenderer());*/

		//		//draw the tiles
		//		for (int i = 0; i < (640 / cellSize); i++) {
		//			for (int j = 0; j < (480 / cellSize); j++) {
		//				int x = i * cellSize;
		//				int y = j * cellSize;
		//				int state = board[i][j] * 8 + board[i + 1][j] * 4 + board[i + 1][j + 1] * 2 + board[i][j + 1];
		//				//string str = to_string(state);
		//				//AssetsManager::Instance()->Text(str, "font", x+5, y+5, SDL_Color({ 0,0,0,255 }), Game::Instance()->getRenderer());


		//				switch (state) {
		//				case 0:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2Terrain20x20", 0, 0, x, y, 20, 20, 0, 0, Game::Instance()->getRenderer());
		//					break;
		//				case 1:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2Terrain20x20", 0, 0, x, y, 20, 20, 0, 1, Game::Instance()->getRenderer());
		//					break;
		//				case 2:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2Terrain20x20", 0, 0, x, y, 20, 20, 0, 2, Game::Instance()->getRenderer());
		//					break;
		//				case 3:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2Terrain20x20", 0, 0, x, y, 20, 20, 0, 3, Game::Instance()->getRenderer());
		//					break;
		//				case 4:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2Terrain20x20", 0, 0, x, y, 20, 20, 1, 0, Game::Instance()->getRenderer());
		//					break;
		//				case 5:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2Terrain20x20", 0, 0, x, y, 20, 20, 1, 1, Game::Instance()->getRenderer());
		//					break;
		//				case 6:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2Terrain20x20", 0, 0, x, y, 20, 20, 1, 2, Game::Instance()->getRenderer());
		//					break;
		//				case 7:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2Terrain20x20", 0, 0, x, y, 20, 20, 1, 3, Game::Instance()->getRenderer());
		//					break;
		//				case 8:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2Terrain20x20", 0, 0, x, y, 20, 20, 2, 0, Game::Instance()->getRenderer());
		//					break;
		//				case 9:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2Terrain20x20", 0, 0, x, y, 20, 20, 2, 1, Game::Instance()->getRenderer());
		//					break;
		//				case 10:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2Terrain20x20", 0, 0, x, y, 20, 20, 2, 2, Game::Instance()->getRenderer());
		//					break;
		//				case 11:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2Terrain20x20", 0, 0, x, y, 20, 20, 2, 3, Game::Instance()->getRenderer());
		//					break;
		//				case 12:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2Terrain20x20", 0, 0, x, y, 20, 20, 3, 0, Game::Instance()->getRenderer());
		//					break;
		//				case 13:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2Terrain20x20", 0, 0, x, y, 20, 20, 3, 1, Game::Instance()->getRenderer());
		//					break;
		//				case 14:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2Terrain20x20", 0, 0, x, y, 20, 20, 3, 2, Game::Instance()->getRenderer());
		//					break;
		//				case 15:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2Terrain20x20", 0, 0, x, y, 20, 20, 3, 3, Game::Instance()->getRenderer());
		//					break;
		//				default:
		//					break;
		//				}
		//			}
		//		}
		//	}

		//	if (subState == TERRAINRECT)
		//	{
		//		/*AssetsManager::Instance()->draw("MarchingSquares20x20", 0, 0, 80, 80, Game::Instance()->getRenderer());
		//		AssetsManager::Instance()->drawTile("MarchingSquares20x20", 0, 0, 90, 0, 20, 20, 0, 0, Game::Instance()->getRenderer());
		//		AssetsManager::Instance()->drawTile("MarchingSquares20x20", 0, 0, 110, 0, 20, 20, 0, 1, Game::Instance()->getRenderer());
		//		AssetsManager::Instance()->drawTile("MarchingSquares20x20", 0, 0, 130, 0, 20, 20, 0, 2, Game::Instance()->getRenderer());
		//		AssetsManager::Instance()->drawTile("MarchingSquares20x20", 0, 0, 150, 0, 20, 20, 0, 3, Game::Instance()->getRenderer());*/

		//		//draw the tiles
		//		for (int i = 0; i < (640 / cellSize); i++) {
		//			for (int j = 0; j < (480 / cellSize); j++) {
		//				int x = i * cellSize;
		//				int y = j * cellSize;
		//				int state = board[i][j] * 8 + board[i + 1][j] * 4 + board[i + 1][j + 1] * 2 + board[i][j + 1];
		//				//string str = to_string(state);
		//				//AssetsManager::Instance()->Text(str, "font", x+5, y+5, SDL_Color({ 0,0,0,255 }), Game::Instance()->getRenderer());


		//				switch (state) {
		//				case 0:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2TerrainRect20x20", 0, 0, x, y, 20, 20, 0, 0, Game::Instance()->getRenderer());
		//					break;
		//				case 1:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2TerrainRect20x20", 0, 0, x, y, 20, 20, 0, 1, Game::Instance()->getRenderer());
		//					break;
		//				case 2:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2TerrainRect20x20", 0, 0, x, y, 20, 20, 0, 2, Game::Instance()->getRenderer());
		//					break;
		//				case 3:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2TerrainRect20x20", 0, 0, x, y, 20, 20, 0, 3, Game::Instance()->getRenderer());
		//					break;
		//				case 4:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2TerrainRect20x20", 0, 0, x, y, 20, 20, 1, 0, Game::Instance()->getRenderer());
		//					break;
		//				case 5:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2TerrainRect20x20", 0, 0, x, y, 20, 20, 1, 1, Game::Instance()->getRenderer());
		//					break;
		//				case 6:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2TerrainRect20x20", 0, 0, x, y, 20, 20, 1, 2, Game::Instance()->getRenderer());
		//					break;
		//				case 7:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2TerrainRect20x20", 0, 0, x, y, 20, 20, 1, 3, Game::Instance()->getRenderer());
		//					break;
		//				case 8:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2TerrainRect20x20", 0, 0, x, y, 20, 20, 2, 0, Game::Instance()->getRenderer());
		//					break;
		//				case 9:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2TerrainRect20x20", 0, 0, x, y, 20, 20, 2, 1, Game::Instance()->getRenderer());
		//					break;
		//				case 10:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2TerrainRect20x20", 0, 0, x, y, 20, 20, 2, 2, Game::Instance()->getRenderer());
		//					break;
		//				case 11:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2TerrainRect20x20", 0, 0, x, y, 20, 20, 2, 3, Game::Instance()->getRenderer());
		//					break;
		//				case 12:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2TerrainRect20x20", 0, 0, x, y, 20, 20, 3, 0, Game::Instance()->getRenderer());
		//					break;
		//				case 13:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2TerrainRect20x20", 0, 0, x, y, 20, 20, 3, 1, Game::Instance()->getRenderer());
		//					break;
		//				case 14:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2TerrainRect20x20", 0, 0, x, y, 20, 20, 3, 2, Game::Instance()->getRenderer());
		//					break;
		//				case 15:
		//					AssetsManager::Instance()->drawTile("MarchingSquares2TerrainRect20x20", 0, 0, x, y, 20, 20, 3, 3, Game::Instance()->getRenderer());
		//					break;
		//				default:
		//					break;
		//				}
		//			}
		//		}
		//	}

		//	for (auto i : entities)
		//		i->draw();
		//}

		if (state == END_GAME)
		{
		}

	SDL_RenderPresent(m_pRenderer); // draw to the screen
}

void Game::quit()
{
	m_bRunning = false;
}

void Game::clean()
{
	//WriteHiScores();
	std::cout << "cleaning game\n";
	InputHandler::Instance()->clean();
	AssetsManager::Instance()->clearFonts();
	TTF_Quit();
	SDL_DestroyWindow(m_pWindow);
	SDL_DestroyRenderer(m_pRenderer);
	Game::Instance()->m_bRunning = false;
	SDL_Quit();
}

void Game::handleEvents()
{
	InputHandler::Instance()->update();

	//HandleKeys
	if (state == MENU)
	{
	}

	if (state == GAME)
	{
		if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_1)) subState = LINES;
		if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_2)) subState = TILES;
		if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_3)) subState = TERRAIN;
		if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_4)) subState = TERRAINRECT;
	}

	if (state == END_GAME)
	{
	}

}

bool Game::isCollide(Entity *a, Entity *b)
{
	return (b->m_position.m_x - a->m_position.m_x)*(b->m_position.m_x - a->m_position.m_x) +
		(b->m_position.m_y - a->m_position.m_y)*(b->m_position.m_y - a->m_position.m_y) <
		(a->m_radius + b->m_radius)*(a->m_radius + b->m_radius);
}

bool Game::isCollideRect(Entity *a, Entity * b) {
	if (a->m_position.m_x < b->m_position.m_x + b->m_width &&
		a->m_position.m_x + a->m_width > b->m_position.m_x &&
		a->m_position.m_y < b->m_position.m_y + b->m_height &&
		a->m_height + a->m_position.m_y > b->m_position.m_y) {
		return true;
	}
	return false;
}

void Game::update()
{
	if (state == GAME)
	{
		for (auto i = entities.begin(); i != entities.end(); i++)
		{
			Entity *e = *i;

			e->update();
		}
	}

}

void Game::UpdateHiScores(int newscore)
{
	//new score to the end
	vhiscores.push_back(newscore);
	//sort
	sort(vhiscores.rbegin(), vhiscores.rend());
	//remove the last
	vhiscores.pop_back();
}

void Game::ReadHiScores()
{
	std::ifstream in("hiscores.dat");
	if (in.good())
	{
		std::string str;
		getline(in, str);
		std::stringstream ss(str);
		int n;
		for (int i = 0; i < 5; i++)
		{
			ss >> n;
			vhiscores.push_back(n);
		}
		in.close();
	}
	else
	{
		//if file does not exist fill with 5 scores
		for (int i = 0; i < 5; i++)
		{
			vhiscores.push_back(0);
		}
	}
}

void Game::WriteHiScores()
{
	std::ofstream out("hiscores.dat");
	for (int i = 0; i < 5; i++)
	{
		out << vhiscores[i] << " ";
	}
	out.close();
}

const int FPS = 60;
const int DELAY_TIME = 1000.0f / FPS;

int main(int argc, char* args[])
{
	srand(time(NULL));

	Uint32 frameStart, frameTime;

	std::cout << "game init attempt...\n";
	if (Game::Instance()->init("Marching Squares", 100, 100, 900, 500,
		false))
	{
		std::cout << "game init success!\n";
		while (Game::Instance()->running())
		{
			frameStart = SDL_GetTicks(); //tiempo inicial

			Game::Instance()->handleEvents();
			Game::Instance()->update();
			Game::Instance()->render();

			frameTime = SDL_GetTicks() - frameStart; //tiempo final - tiempo inicial

			if (frameTime < DELAY_TIME)
			{
				//con tiempo fijo el retraso es 1000 / 60 = 16,66
				//procesar handleEvents, update y render tarda 1, y hay que esperar 15
				//cout << "frameTime : " << frameTime << "  delay : " << (int)(DELAY_TIME - frameTime) << endl;
				SDL_Delay((int)(DELAY_TIME - frameTime)); //esperamos hasta completar los 60 fps
			}
		}
	}
	else
	{
		std::cout << "game init failure - " << SDL_GetError() << "\n";
		return -1;
	}
	std::cout << "game closing...\n";
	Game::Instance()->clean();
	return 0;
}