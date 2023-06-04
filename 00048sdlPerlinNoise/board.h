#pragma once
#include <vector>
#include <random>
#include <SDL.h>
#include "vector2D.h"
#include "game.h"
#include "AssetsManager.h"

class Board{
    public:
    Vector2D boardSize;
    Vector2D cellSize;
    Vector2D boardSizeInTiles;
    std::vector< std::vector<int> > board;

	//default constructor
	Board();

	//complete constructor
	Board(float boardWidth, float boardHeight, float cellWidth, float cellHeight);

    //fill board with points. 80% more ground than water
	void fillBoardWithPoints();

    //marching squares, draw points and show map with lines
	void showMarchingSquaresLines();

	void showMarchingSquaresTiles();

    //water and earth
	void showMarchingSquaresTerrain();

    //water and earth modified for better view
	void showMarchingSquaresTerrainRect();
};