#include "board.h"

Board::Board() { Board(640, 480, 20, 20); }

Board::Board(float boardWidth, float boardHeight, float cellWidth, float cellHeight) {
	boardSize = { boardWidth, boardHeight };
	cellSize = { cellWidth, cellHeight };
	boardSizeInTiles = { boardSize.m_x / cellSize.m_x, boardSize.m_y / cellSize.m_y };
	//resize width and height of the board.
	board.resize(boardSizeInTiles.m_x + 1);
	for (int i = 0; i < board.size(); i++) {
		board[i].resize(boardSizeInTiles.m_y + 1);
	}
}

//fill board with points. 80% more ground than water
void Board::fillBoardWithPoints() {
	//generate the map dots (more 1 than 0)
	for (int i = 0; i < boardSizeInTiles.m_x + 1; i++) {
		for (int j = 0; j < boardSizeInTiles.m_y + 1; j++) {
			if ((rand() % 100) < 80)
				board[i][j] = 1;
			else
				board[i][j] = 0;
		}
	}
}

//marching squares, draw points and show map with lines
void Board::showMarchingSquaresLines() {
	//draw the dots
	for (int i = 0; i < boardSizeInTiles.m_x + 1; i++) {
		for (int j = 0; j < boardSizeInTiles.m_y + 1; j++) {
			if (board[i][j] == 0)
				SDL_SetRenderDrawColor(Game::Instance()->getRenderer(), 0, 0, 0, 255); //black
			else
				SDL_SetRenderDrawColor(Game::Instance()->getRenderer(), 255, 0, 0, 255); //red

			SDL_RenderDrawPoint(Game::Instance()->getRenderer(), i * cellSize.m_x, j * cellSize.m_y);
		}
	}

	//draw the lines
	SDL_SetRenderDrawColor(Game::Instance()->getRenderer(), 0, 0, 0, 255); //black
	for (int i = 0; i < boardSizeInTiles.m_x; i++) {
		for (int j = 0; j < boardSizeInTiles.m_y; j++){
			int x = i * cellSize.m_x;
			int y = j * cellSize.m_y;
			Vector2D a = Vector2D(x + cellSize.m_x * 0.5, y);
			Vector2D b = Vector2D(x + cellSize.m_x, y + cellSize.m_y * 0.5);
			Vector2D c = Vector2D(x + cellSize.m_x * 0.5, y + cellSize.m_y);
			Vector2D d = Vector2D(x, y + cellSize.m_y * 0.5);
			int state = board[i][j] * 8 + board[i + 1][j] * 4 + board[i + 1][j + 1] * 2 + board[i][j + 1];

			switch (state) {
			case 0:
				//do nothing
				break;
			case 1:
				SDL_RenderDrawLine(Game::Instance()->getRenderer(), c.m_x, c.m_y, d.m_x, d.m_y);
				break;
			case 2:
				SDL_RenderDrawLine(Game::Instance()->getRenderer(), b.m_x, b.m_y, c.m_x, c.m_y);
				break;
			case 3:
				SDL_RenderDrawLine(Game::Instance()->getRenderer(), b.m_x, b.m_y, d.m_x, d.m_y);
				break;
			case 4:
				SDL_RenderDrawLine(Game::Instance()->getRenderer(), a.m_x, a.m_y, b.m_x, b.m_y);
				break;
			case 5:
				SDL_RenderDrawLine(Game::Instance()->getRenderer(), a.m_x, a.m_y, d.m_x, d.m_y);
				SDL_RenderDrawLine(Game::Instance()->getRenderer(), b.m_x, b.m_y, c.m_x, c.m_y);
				break;
			case 6:
				SDL_RenderDrawLine(Game::Instance()->getRenderer(), a.m_x, a.m_y, c.m_x, c.m_y);
				break;
			case 7:
				SDL_RenderDrawLine(Game::Instance()->getRenderer(), a.m_x, a.m_y, d.m_x, d.m_y);
				break;
			case 8:
				SDL_RenderDrawLine(Game::Instance()->getRenderer(), a.m_x, a.m_y, d.m_x, d.m_y);
				break;
			case 9:
				SDL_RenderDrawLine(Game::Instance()->getRenderer(), a.m_x, a.m_y, c.m_x, c.m_y);
				break;
			case 10:
				SDL_RenderDrawLine(Game::Instance()->getRenderer(), a.m_x, a.m_y, b.m_x, b.m_y);
				SDL_RenderDrawLine(Game::Instance()->getRenderer(), c.m_x, c.m_y, d.m_x, d.m_y);
				break;
			case 11:
				SDL_RenderDrawLine(Game::Instance()->getRenderer(), a.m_x, a.m_y, b.m_x, b.m_y);
				break;
			case 12:
				SDL_RenderDrawLine(Game::Instance()->getRenderer(), b.m_x, b.m_y, d.m_x, d.m_y);
				break;
			case 13:
				SDL_RenderDrawLine(Game::Instance()->getRenderer(), b.m_x, b.m_y, c.m_x, c.m_y);
				break;
			case 14:
				SDL_RenderDrawLine(Game::Instance()->getRenderer(), c.m_x, c.m_y, d.m_x, d.m_y);
				break;
			case 15:
				//do nothing
				break;
			default:
				break;
			}
		}
	}
}

void Board::showMarchingSquaresTiles() {
	//draw the tiles
	for (int i = 0; i < boardSizeInTiles.m_x; i++) {
		for (int j = 0; j < boardSizeInTiles.m_y; j++) {
			int x = i * cellSize.m_x;
			int y = j * cellSize.m_y;
			int state = board[i][j] * 8 + board[i + 1][j] * 4 + board[i + 1][j + 1] * 2 + board[i][j + 1];

			int tilemapRow = 0;
			int tilemapCol = 0;

			tilemapRow = state / 4;
			tilemapCol = state - tilemapRow * 4;
			AssetsManager::Instance()->drawTile("MarchingSquares20x20", 0, 0, x, y, cellSize.m_x, cellSize.m_y, tilemapRow, tilemapCol, Game::Instance()->getRenderer());
		}
	}
}

//water and earth
void Board::showMarchingSquaresTerrain() {
	//draw the tiles
	for (int i = 0; i < boardSizeInTiles.m_x; i++) {
		for (int j = 0; j < boardSizeInTiles.m_y; j++) {
			int x = i * cellSize.m_x;
			int y = j * cellSize.m_y;
			int state = board[i][j] * 8 + board[i + 1][j] * 4 + board[i + 1][j + 1] * 2 + board[i][j + 1];

			int tilemapRow = 0;
			int tilemapCol = 0;

			tilemapRow = state / 4;
			tilemapCol = state - tilemapRow * 4;
			AssetsManager::Instance()->drawTile("MarchingSquares2Terrain20x20", 0, 0, x, y, cellSize.m_x, cellSize.m_y, tilemapRow, tilemapCol, Game::Instance()->getRenderer());
		}
	}
}

//water and earth modified for better view
void Board::showMarchingSquaresTerrainRect() {
	//draw the tiles
	for (int i = 0; i < boardSizeInTiles.m_x; i++) {
		for (int j = 0; j < boardSizeInTiles.m_y; j++) {
			int x = i * cellSize.m_x;
			int y = j * cellSize.m_y;
			int state = board[i][j] * 8 + board[i + 1][j] * 4 + board[i + 1][j + 1] * 2 + board[i][j + 1];

			int tilemapRow = 0;
			int tilemapCol = 0;

			tilemapRow = state / 4;
			tilemapCol = state - tilemapRow * 4;
			AssetsManager::Instance()->drawTile("MarchingSquares2TerrainRect20x20", 0, 0, x, y, cellSize.m_x, cellSize.m_y, tilemapRow, tilemapCol, Game::Instance()->getRenderer());
		}
	}
}