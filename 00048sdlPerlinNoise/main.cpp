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

//nCount : number of elements in the output array "fOutput".
//fSeed : the initial values (random white noise).
//nOctaves : number of layers of noise to generate. Higher produces more detailed and complex noise.
//fBias : controls the amplitude decrease of each octave. Higher produces more contrasted noise.
//fOutput : the generated noise values.
void Game::PerlinNoise1D(int nCount, float * fSeed, int nOctaves, float fBias, float * fOutput)
{
	for (int x = 0; x < nCount; x++)
	{
		float fNoise = 0.0f;    //noise value.
		float fScaleAcc = 0.0f; //scaling factor.
		float fScale = 1.0f;    //current scale factor.

		//iterate through each octave.
		for (int o = 0; o < nOctaves; o++)
		{
			//the pitch is the distance between two sample points.
			int nPitch = nCount >> o;  //each octave has smaller pitch, producing finer details.
			//nSample1 and nSample2 represent the positions of the samples used to interpolate
			//the noise value for the current element.
			int nSample1 = (x / nPitch) * nPitch;
			int nSample2 = (nSample1 + nPitch) % nCount;

			//calculate the interpolation factor between nSample1 and nSample2.
			//It's a value between 0.0 and 1.0, and indicates how close x is to nSample2
			//relative to nSample1.
			float fBlend = (float)(x - nSample1) / (float)nPitch;

			//linear interpolation between fSeed[nSample1] and fSeed[nSample2] using fBlend.
			//This is the interpolated noise value for the current element and octave.
			float fSample = (1.0f - fBlend) * fSeed[nSample1] + fBlend * fSeed[nSample2];

			//fScaleAcc is accumulated by adding fScale at each octave. It keep track of
			//total scaling factor applied to the noise.
			fScaleAcc += fScale;
			//fNoise accumulates the final noise value by multiplying the interpolated sample
			//with the current scale.
			fNoise += fSample * fScale;
			//fScale is updated by dividing it by fBias at each octave. This reduces the amplitude
			//of each subsequent octave, allowing the smaller-scale details to have less impact
			//on the final noise.
			fScale = fScale / fBias;
		}

		// Scale to seed range
		// Ensures that the noise values are within a reasonable range (between 0 and 1).
		fOutput[x] = fNoise / fScaleAcc;
	}
}

//nWidth * nHeight : number of elements in the output array "fOutput".
//fSeed : the initial values (random white noise).
//nOctaves : number of layers of noise to generate. Higher produces more detailed and complex noise.
//fBias : controls the amplitude decrease of each octave. Higher produces more contrasted noise.
//fOutput : the generated noise values.
void Game::PerlinNoise2D(int nWidth, int nHeight, float * fSeed, int nOctaves, float fBias, float * fOutput)
{
	for (int x = 0; x < nWidth; x++)
		for (int y = 0; y < nHeight; y++)
		{
			float fNoise = 0.0f;     //noise value.
			float fScaleAcc = 0.0f;  //accumulated scale factor.
			float fScale = 1.0f;     //current scale factor.

			for (int o = 0; o < nOctaves; o++)
			{
				//the pitch is the distance between two sample points.
				int nPitch = nWidth >> o;  //256, 128, 64...
				//samples represent the positions of the samples used to interpolate
				//the noise value for the current element. It gets two points.
				int nSampleX1 = (x / nPitch) * nPitch;
				int nSampleY1 = (y / nPitch) * nPitch;

				int nSampleX2 = (nSampleX1 + nPitch) % nWidth;
				int nSampleY2 = (nSampleY1 + nPitch) % nWidth;

				//blend is the interpolation factor. 
				float fBlendX = (float)(x - nSampleX1) / (float)nPitch;
				float fBlendY = (float)(y - nSampleY1) / (float)nPitch;

				//bilinear interpolation between the two values
				float fSampleT = (1.0f - fBlendX) * fSeed[nSampleY1 * nWidth + nSampleX1] + fBlendX * fSeed[nSampleY1 * nWidth + nSampleX2];
				float fSampleB = (1.0f - fBlendX) * fSeed[nSampleY2 * nWidth + nSampleX1] + fBlendX * fSeed[nSampleY2 * nWidth + nSampleX2];

				//accumulate the scale.
				fScaleAcc += fScale;
				fNoise += (fBlendY * (fSampleB - fSampleT) + fSampleT) * fScale;
				fScale = fScale / fBias;
			}

			// Scale to seed range (beween 0 and 1)
			fOutput[y * nWidth + x] = fNoise / fScaleAcc;
		}
}

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
	AssetsManager::Instance()->loadAssetsJson(); //ahora con formato json
	Mix_Volume(-1, 16); //adjust sound/music volume for all channels

	state = GAME;
	
	/*mapa = new Board(640,480,20,20);
	mapa->fillBoardWithPoints();*/

	//initialize perlin noise variables
	nOutputWidth = Game::Instance()->getGameWidth();
	nOutputHeight = Game::Instance()->getGameHeight();

	//1D
	nOutputSize = Game::Instance()->getGameWidth();
	fNoiseSeed1D = new float[nOutputSize];    //random values between 0 and 1, it creates white noise.
	fPerlinNoise1D = new float[nOutputSize];  //the data from fNoiseSeed1D after the perlin noise algorithm.
	//init with random values
	for (int i = 0; i < nOutputSize; i++) fNoiseSeed1D[i] = (float)rand() / (float)RAND_MAX; //numbers between 0 and 1

	//2D
	fNoiseSeed2D = new float[nOutputWidth * nOutputHeight];
	fPerlinNoise2D = new float[nOutputWidth * nOutputHeight];
	//init with random values
	for (int i = 0; i < nOutputWidth * nOutputHeight; i++) fNoiseSeed2D[i] = (float)rand() / (float)RAND_MAX;

	std::cout << "initial random values created" << std::endl;

	//run them once with initial values for presentation while not key is pressed.
	PerlinNoise1D(nOutputSize, fNoiseSeed1D, nOctaveCount, fScalingBias, fPerlinNoise1D);
	PerlinNoise2D(nOutputWidth, nOutputHeight, fNoiseSeed2D, nOctaveCount, fScalingBias, fPerlinNoise2D);

	return true;
}

void Game::render()
{
	SDL_SetRenderDrawColor(Game::Instance()->getRenderer(), 255, 255, 255, 0);
	SDL_RenderClear(m_pRenderer); // clear the renderer to the draw color

	//title and menu
	AssetsManager::Instance()->Text("Perlin Noise", "font", 640, 0, SDL_Color({ 0,0,0,255 }), Game::Instance()->getRenderer());
	AssetsManager::Instance()->Text("Mode (1,2,3): " + std::to_string(nMode), "font", 650, 30, SDL_Color({ 0,0,0,255 }), Game::Instance()->getRenderer());
	AssetsManager::Instance()->Text("Octaves (space) : " + std::to_string(nOctaveCount), "font", 650, 60, SDL_Color({ 0,0,0,255 }), Game::Instance()->getRenderer());
	AssetsManager::Instance()->Text("Scaling (q,a): " + std::to_string(fScalingBias), "font", 650, 90, SDL_Color({ 0,0,0,255 }), Game::Instance()->getRenderer());
	AssetsManager::Instance()->Text("Noise between 0 and 1 or -1 and 1 (z)", "font", 650, 120, SDL_Color({ 0,0,0,255 }), Game::Instance()->getRenderer());

	//draw a 640x480 rectangle
	//SDL_SetRenderDrawColor(Game::Instance()->getRenderer(), 0, 0, 0, 255); //black
	//SDL_RenderDrawLine(Game::Instance()->getRenderer(), 0, 480, 640, 480);
	//SDL_RenderDrawLine(Game::Instance()->getRenderer(), 640, 0, 640, 480);

		if (state == MENU)
		{
		}

		if (state == GAME)
		{
			if (nMode == 1)
			{
				for (int x = 0; x < nOutputSize; x++)
				{
					int y = -(fPerlinNoise1D[x] * (float)Game::Instance()->getGameHeight() / 2.0f) + (float)Game::Instance()->getGameHeight() / 2.0f;
					if (y < Game::Instance()->getGameHeight() / 2)
					{
						SDL_SetRenderDrawColor(m_pRenderer, 0, 255, 0, 255); //green
						for (int f = y; f < Game::Instance()->getGameHeight() / 2; f++)
							SDL_RenderDrawPoint(m_pRenderer, x, f);
					}
					else
					{
						SDL_SetRenderDrawColor(m_pRenderer, 255, 0, 0, 255); //red
						for (int f = Game::Instance()->getGameHeight() / 2; f <= y; f++)
							SDL_RenderDrawPoint(m_pRenderer, x, f);
					}
				}
			}

			if (nMode == 2)
			{

			}

			if (nMode == 3)
			{

			}
		}

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
	delete[] fNoiseSeed2D;
	delete[] fPerlinNoise2D;
	delete[] fNoiseSeed1D;
	delete[] fPerlinNoise1D;

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
		bool recalculate = false;
		if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_SPACE))
		{
			nOctaveCount++;
			recalculate = true;
		}
		if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_1)) nMode = 1;
		if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_2)) nMode = 2;
		if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_3)) nMode = 3;
		if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_Q))
		{
			fScalingBias += 0.2f;
			recalculate = true;
		}
		if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_A))
		{
			fScalingBias -= 0.2f;
			recalculate = true;
		}

		if (fScalingBias < 0.2f) fScalingBias = 0.2f;
		if (nOctaveCount == 9) nOctaveCount = 1;

		if(recalculate) PerlinNoise1D(nOutputSize, fNoiseSeed1D, nOctaveCount, fScalingBias, fPerlinNoise1D);

		//1D
		if (nMode == 1)
		{
			bool calculate = false;
			//noise between 0 and +1
			if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_Z))
			{
				for (int i = 0; i < nOutputSize; i++) fNoiseSeed1D[i] = (float)rand() / (float)RAND_MAX;
				calculate = true;
			}

			//noise between -1 and +1
			if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_X))
			{
				for (int i = 0; i < nOutputSize; i++) fNoiseSeed1D[i] = 2.0f * ((float)rand() / (float)RAND_MAX) - 1.0f;
				calculate = true;
			}

			if(calculate == true) PerlinNoise1D(nOutputSize, fNoiseSeed1D, nOctaveCount, fScalingBias, fPerlinNoise1D);
		}

		//2D
		if (nMode == 2)
		{
			//noise between 0 and +1
			if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_Z))
			{
				for (int i = 0; i < nOutputWidth * nOutputHeight; i++) fNoiseSeed2D[i] = (float)rand() / (float)RAND_MAX;
				PerlinNoise2D(nOutputWidth, nOutputHeight, fNoiseSeed2D, nOctaveCount, fScalingBias, fPerlinNoise2D);
			}
		}

		//2D with colors
		if (nMode == 3)
		{
			//noise between 0 and +1
			if (InputHandler::Instance()->isKeyDown(SDL_SCANCODE_Z))
			{
				for (int i = 0; i < nOutputWidth * nOutputHeight; i++) fNoiseSeed2D[i] = (float)rand() / (float)RAND_MAX;
				PerlinNoise2D(nOutputWidth, nOutputHeight, fNoiseSeed2D, nOctaveCount, fScalingBias, fPerlinNoise2D);
			}
		}
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
	/*if (state == GAME)
	{
		for (auto i = entities.begin(); i != entities.end(); i++)
		{
			Entity *e = *i;

			e->update();
		}
	}*/

}

const int FPS = 60;
const int DELAY_TIME = 1000.0f / FPS;

int main(int argc, char* args[])
{
	srand(time(NULL));

	Uint32 frameStart, frameTime;

	std::cout << "game init attempt...\n";
	if (Game::Instance()->init("Perlin Noise", 100, 100, 900, 600,
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