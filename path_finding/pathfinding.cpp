#include <stdint.h>
#include <stdio.h>
#include <cstdlib>
#include <assert.h>
#include <glut/glut.h>
#include <memory.h>
#include <utility>
#include <vector>
#include <algorithm>
#include <Windows.h>
#include <iostream>
#include <list>

namespace
{
	// Global variables
	
	// OpenGL texture ids for rendering.
	GLuint  inputTexture = 0;
	GLuint  outputTexture = 0;
	// Input and output data in pixels. outputData is updated to outputTexture each frame
	uint8_t* inputData = 0;
	uint8_t* outputData = 0;
	// width and height of the input and output datas
	int width = 0;
	int height = 0;
	// start and end position for path finding. These are found automatically from input file.
	int startX = -1;
	int startY = -1;
	int endX = -1;
	int endY = -1;
	bool pressStart = false;
}

	typedef std::pair<int, int> Position;
	std::list<Position> path;

	class Waypoint
	{
	public:

		float value;
		Position position;

		Waypoint* nextWaypoint = NULL;


	};
	std::vector<Waypoint> waypoints;
	
	bool sortByValue(const Waypoint &lhs, const Waypoint &rhs)
	{
		return lhs.value > rhs.value;
	}

	class Car
	{
	public:

		int X = -1;
		int Y = -1;

		bool foundGoal = false;
		Waypoint* currentWaypoint = NULL;

		bool FindPath =  true;
		bool Drive = false;
	};

	Car carAI;

	class SearchNode
	{
	public:

		float G;
		float H;
		float F;

		SearchNode(const Position& currentPosition, float _H, float deltaG, SearchNode* prev) : prevNode(prev), pos(currentPosition), G(0), H(_H)
		{
			/*resetPrev(prev, deltaG);*/
		}

		void resetPrev(SearchNode* prev, float deltaG)
		{
			if (prev == 0)
			{
				G = 0.0f;
			}
			else
			{
				G = deltaG + prev->G;
			}
			F = H + G;

		};
		SearchNode* prevNode;
		Position pos;

		float Distance() const { return F; }
		static bool lessThan(SearchNode* n1, SearchNode* n2) { return n1->Distance() > n2->Distance(); }
	};


		namespace
		{
			bool IsWalkable(int x, int y, int width, int height, bool* Map)
			{
				if (x >= 0 && x < width && y >= 0 && y < height) {
					int i = x + y * width;
					return Map[i];
				}
				else
					return false;
			};
			bool HasNodeAt(int x, int y, std::vector<SearchNode*> list)
			{
				for (SearchNode* s : list)
				{
					if (s->pos.first == x && s->pos.second == y)
					{
						return true;
					}
				}
				return false;
			}

		void setPixel(uint8_t* data, int w, int h, int x, int y, uint8_t r, uint8_t g, uint8_t b)
		{
			uint8_t* pixel = &data[y*w * 3 + x * 3];
			pixel[0] = b;
			pixel[1] = g;
			pixel[2] = r;
		}
		bool comp(SearchNode* s1, SearchNode* s2)
		{
			return s1->Distance() < s2->Distance();
		};
		// STUDENT_TODO: Make implementation for doPathFinding function, which writes found path to outputData
		void doPathFinding(const uint8_t* inputData, int width, int height, uint8_t* outputData, int startX, int startY, int endX, int endY)
		{

		
			bool* IsWalkableTile = new bool[width*height];
			for (int i = 0; i < width*height; i++)
			{
				int r = inputData[i * 3 + 2];
				int g = inputData[i * 3 + 1];
				int b = inputData[i * 3 + 0];

				IsWalkableTile[i] = true;
				if (g == 255 && r == 0 && b == 0)
				{
					IsWalkableTile[i] = false;
				}
			}

			std::vector<SearchNode*> openList;
			std::vector<SearchNode*> closedList;

			openList.push_back(new SearchNode(std::pair<int, int>(startX, startY), 0.0f, 0.0f, NULL));

			while (openList.size() > 0)
			{
				SearchNode* currentNode = *openList.begin();

				float x, y, dx, dy;
				x = currentNode->pos.first;
				y = currentNode->pos.second;
				dx = sqrt((endX - x)*(endX - x));
				dy = sqrt((endY - y)*(endY - y));
				
				float H = abs(dx) + abs(dy);
				float G = sqrt((x - startX)*(x - startX) + (y - startY) - (y - startY));
				float F = H + G;
				
				
				//setPixel(outputData, width, height, currentNode->pos.first, currentNode->pos.second,-F,-F,-F);
				
				openList.erase(openList.begin());

				closedList.push_back(currentNode);

				if (x == endX && y == endY)
				{
					path.push_front(currentNode->pos);

					while (true) {
						currentNode = currentNode->prevNode;
						if (currentNode == NULL)
						{
							break;
						}
						else
						{
							carAI.FindPath = false;
							path.push_front(currentNode->pos);
						}
					}
					break;
				}

				if (IsWalkable(x, y - 1, width, height, IsWalkableTile) && !HasNodeAt(x, y - 1, closedList) && !HasNodeAt(x, y - 1, openList))
				{
					openList.push_back(new SearchNode(std::pair<int, int>(x, y - 1), H - 1.f, G + 1.f, currentNode));
				}
				if (IsWalkable(x - 1, y, width, height, IsWalkableTile) && !HasNodeAt(x - 1, y, closedList) && !HasNodeAt(x - 1, y, openList))
				{
					openList.push_back(new SearchNode(std::pair<int, int>(x - 1, y), H - 1.f, G + 1.f, currentNode));
				}
				if (IsWalkable(x, y + 1, width, height, IsWalkableTile) && !HasNodeAt(x, y + 1, closedList) && !HasNodeAt(x, y + 1, openList))
				{
					openList.push_back(new SearchNode(std::pair<int, int>(x, y + 1), H - 1.f, G + 1.f, currentNode));
				}
				if (IsWalkable(x + 1, y, width, height, IsWalkableTile) && !HasNodeAt(x + 1, y, closedList) && !HasNodeAt(x + 1, y, openList))
				{
					openList.push_back(new SearchNode(std::pair<int, int>(x + 1, y), H - 1.f, G + 1.f, currentNode));
				}
				std::sort(openList.begin(), openList.end(), comp);

			};

			for (SearchNode* sn : closedList)
			{
				int x, y;
				x = sn->pos.first;
				y = sn->pos.second;
			
			}

			auto it = openList.begin();
			while (it != openList.end())
			{
				delete *it;
				it = openList.erase(it);
			}
			auto itc = closedList.begin();
			while (itc != closedList.end())
			{
				delete *itc;
				itc = closedList.erase(itc);
			}
			delete[] IsWalkableTile;
		}
	}

namespace
{
	// Quick and dirty function for reading bmp-files to opengl textures.
	GLuint loadBMPTexture(const char *fileName, int* w, int* h, uint8_t** data)
	{
		assert(w != 0);
		assert(h != 0);
		assert(data != 0);
		FILE *file;
		if ((file = fopen(fileName, "rb")) == NULL)
			return 0;
		fseek(file, 18, SEEK_SET);

		int width = 0;
		fread(&width, 2, 1, file);
		fseek(file, 2, SEEK_CUR);
		int height = 0;
		fread(&height, 2, 1, file);
		printf("Image \"%s\" (%dx%d)\n", fileName, width, height);

		*data = new uint8_t[3 * width * height];
		assert(data != 0);
		fseek(file, 30, SEEK_CUR);
		fread(*data, 3, width * height, file);
		fclose(file);

		GLuint  texId;
		glGenTextures(1, &texId);
		glBindTexture(GL_TEXTURE_2D, texId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_BGR_EXT, GL_UNSIGNED_BYTE, *data);
		glBindTexture(GL_TEXTURE_2D, 0);
		if (w) *w = width;
		if (h) *h = height;
		return texId;
	}

	// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


	// Initialization
	bool init()
	{
		glMatrixMode(GL_PROJECTION);
		glOrtho(0, 512 + 4, 256 + 2, 0, -1, 1);

		// Load input file
		inputTexture = loadBMPTexture("input3.bmp", &width, &height, &inputData);
		if (0 == inputTexture)
		{
			printf("Error! Cannot open file: \"input.bmp\"\n");
			return false;	
		}

		// Make outputTexture
		glGenTextures(1, &outputTexture);
		glBindTexture(GL_TEXTURE_2D, outputTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);

		// Copy inputData also to outputData
		outputData = new uint8_t[3 * width*height];
		memcpy(outputData, inputData, 3 * width*height);

		// find start and end
		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				uint8_t* pix = &inputData[3 * (y*width + x)]; // get pixel
				uint8_t b = pix[0];
				uint8_t g = pix[1];
				uint8_t r = pix[2];


				if (r == 255 && g == 0 && b == 255) 
				{
					// Start
					startX = x;
					startY = y;
					//printf("Start position: <%d,%d>\n", x, y);
				}
				if (r == 255 && g == 0 && b == 0) 
				{
					// End
					endX = x;
					endY = y;
					//printf("End position: <%d,%d>\n", x, y);
				}

				if (r == 0 && g == 0 && b <= 255 && b >= 10)
					{
						Waypoint newWaypoint;

						newWaypoint.value = b;
						newWaypoint.position.first = x;
						newWaypoint.position.second = y;

						waypoints.push_back(newWaypoint);
					}
			}
		}
		
		carAI.X = startX;
		carAI.Y = startY;
		std::sort(waypoints.begin(), waypoints.end(), sortByValue);

		printf( "Press S to start pathfinding \n"
				"Press P to pause pathfinding \n"
				"Press R to restart pathfinding \n\n");

		for (int i = 0; i < waypoints.size() - 1; i++)
		{
			waypoints[i].nextWaypoint = &waypoints[i + 1];
		}

		carAI.currentWaypoint = &waypoints[0];

		if (startX < 0 || startY < 0)
		{
			printf("Error! Start position not found\n");
			return false;
		}

		if (endX < 0 || endY < 0)
		{
			printf("Error! End position not found\n");
			return false;
		}
		return true;
	}


} // end - anonymous namespace

void drawLevel()
{

	if (pressStart == true)
	{
		

	if (carAI.foundGoal == false)
	{
		if (path.size() <= 0)
		{
			if (carAI.FindPath == true)
			{
				//printf("car finding path \n");
				doPathFinding(inputData, width, height, outputData, carAI.X, carAI.Y, carAI.currentWaypoint->position.first, carAI.currentWaypoint->position.second);

				for (Position pos : path)
				{
					setPixel(outputData, width, height, pos.first, pos.second, 30, 30, 30);
				}
			}
		}
		else
		{
			if (carAI.FindPath == false)
			{
				carAI.Drive = true;

				if (carAI.Drive == true)
				{
					//printf("car driving path \n);
					setPixel(outputData, width, height, carAI.X, carAI.Y, 255, 255, 255);
					Position pos = path.front();
					carAI.X = pos.first;
					carAI.Y = pos.second;
					path.pop_front();

					setPixel(outputData, width, height, carAI.X, carAI.Y, 75, 75, 75);

					if (path.size() == 0)
					{
						carAI.Drive = false;
						carAI.FindPath = true;

						carAI.currentWaypoint = carAI.currentWaypoint->nextWaypoint;

						if (carAI.currentWaypoint == NULL)
						{
							printf("Car in goal \n");
							carAI.foundGoal = true;
						}

					}
				}

			}

		}
	}
	}


	// Copy outputData to outputTexture
	glBindTexture(GL_TEXTURE_2D, outputTexture);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_BGR_EXT, GL_UNSIGNED_BYTE, outputData);
	glBindTexture(GL_TEXTURE_2D, 0);

	glClear(GL_COLOR_BUFFER_BIT);

	// Draw input texture to left half of the screen
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, inputTexture);
	glBegin(GL_QUADS);
	glTexCoord2d(0, 1); glVertex2d(1, 1);
	glTexCoord2d(0, 0); glVertex2d(1, 1 + 256);
	glTexCoord2d(1, 0); glVertex2d(1 + 256, 1 + 256);
	glTexCoord2d(1, 1); glVertex2d(1 + 256, 1);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	// Draw output texture to right half of the screen
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, outputTexture);
	glBegin(GL_QUADS);
	glTexCoord2d(0, 1); glVertex2d(2 + 256, 1);
	glTexCoord2d(0, 0); glVertex2d(2 + 256, 1 + 256);
	glTexCoord2d(1, 0); glVertex2d(2 + 512, 1 + 256);
	glTexCoord2d(1, 1); glVertex2d(2 + 512, 1);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	glutSwapBuffers();
	glutPostRedisplay();

	Sleep(15);
}

void Keyboard(unsigned char key, int x, int y)
{

	if (key == 'r')
	{
		printf("pathfinding restarted \n");

		memcpy(outputData, inputData, 3 * width*height);

		carAI.X = startX;
		carAI.Y = startY;
		carAI.currentWaypoint = &waypoints[0];
		carAI.foundGoal = false;
		carAI.Drive = false;
		carAI.FindPath = true;
		path.clear();
	}
	if (key == 's')
	{
		if (pressStart == false)
		{
		printf("pathfinding started \n");
		pressStart = true;
		}

		if (pressStart == true)
		{
			printf("pathfinding is running \n");
		}
	}

	if (key == 'p' )
	{
		if (pressStart == false)
		{
		printf("pathfinding already paused \n");
		}
		if (pressStart == true)
		{
		printf("pathfinding paused \n");
		}

		if (carAI.currentWaypoint != nullptr && carAI.currentWaypoint != nullptr )
		{
			
			setPixel(outputData, width, height, carAI.currentWaypoint->position.first, carAI.currentWaypoint->position.second, 0, 0, 255);
			pressStart = false;

		}
	}


};

// Main
int main(int argc, char ** argv) {


	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(2*(512+4), 2*(256+2));
	glutCreateWindow("Pathfinding Demo");
	glutDisplayFunc(drawLevel);
	glutKeyboardFunc(Keyboard);
	if (!init()) return -1;
	glutMainLoop();
	delete inputData;
	delete outputData;

	return 0;
}