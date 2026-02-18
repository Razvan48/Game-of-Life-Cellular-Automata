#include <iostream>
#include <vector>
#include <unordered_set>
#include <utility>
#include <random>

#include <glew.h>
#include <glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

using namespace std;

const double WINDOW_WIDTH = 1024.0;
const double WINDOW_HEIGHT = 1024.0;

const double CONWAY_WAITING_TIME = 0.0125; // in seconds

double lastTimeConway = 0.0;

const int CELL_WIDTH = 10;
const int CELL_HEIGHT = 10;

const char* vertexShaderSource =
"#version 330 core \n"
"\n"
"layout (location = 0) in vec2 vertexPosition; \n"
"uniform mat4 ortho; \n"
"\n"
"void main() \n"
"{ \n"
"\n"
"   gl_Position = ortho * vec4(vertexPosition.x, vertexPosition.y, 0.0, 1.0); \n"
"\n"
"} \n"
"\0";

const char* fragmentShaderSource =
"#version 330 core \n"
"\n"
"out vec4 vertexColour; \n"
"uniform vec3 colour; \n"
"\n"
"void main() \n"
"{ \n"
"\n"
"   vertexColour = vec4(colour, 1.0); \n"
"\n"
"} \n"
"\0";

unsigned int VAO;
unsigned int VBO;

double currentTime;
double previousTime;
double deltaTime;

const int dx[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };
const int dy[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };

struct PairHash
{
    size_t operator()(const pair<long long, long long>& p) const
    {
        auto hash0 = hash<long long>()(p.first);
        auto hash1 = hash<long long>()(p.second);
        return hash0 ^ (hash1 << 1ll);
    }
};

unordered_set<pair<long long, long long>, PairHash> currentActiveCells;
unordered_set<pair<long long, long long>, PairHash> nextActiveCells;

unordered_set<pair<long long, long long>, PairHash> potentiallyActiveCells;
unordered_set<pair<long long, long long>, PairHash> nextPotentiallyActiveCells;

double cameraX = 0.0;
double cameraY = 0.0;

double cameraZoom = 1.0;

const double CAMERA_X_SPEED = 100.0;
const double CAMERA_Y_SPEED = 100.0;
const double CAMERA_ZOOM_SPEED = 0.5;

const double EPSILON_ZOOM = 0.0001;

const int RANDOM_RECTANGLE_WIDTH = 25;
const int RANDOM_RECTANGLE_HEIGHT = 25;
const int RANDOM_RECTANGLE_PROBABILITY = 25;

void initConway()
{
    currentActiveCells.clear();
	potentiallyActiveCells.clear();

    for (int i = 0; i < 100; ++i)
        for (int j = 0; j < 100; ++j)
			currentActiveCells.emplace(i, j);

    for (pair<long long, long long> activeCell : currentActiveCells)
    {
        long long cellX = activeCell.first;
        long long cellY = activeCell.second;

        for (int i = 0; i < 8; ++i)
        {
            long long neighbourX = cellX + dx[i];
            long long neighbourY = cellY + dy[i];

			potentiallyActiveCells.emplace(neighbourX, neighbourY);
        }
    }
}

void Conway()
{
	nextActiveCells.clear();
	nextPotentiallyActiveCells.clear();

	nextActiveCells.reserve(currentActiveCells.size());
	nextPotentiallyActiveCells.reserve(potentiallyActiveCells.size());

	for (pair<long long, long long> activeCell : currentActiveCells)
	{
		long long cellX = activeCell.first;
		long long cellY = activeCell.second;

        int numActiveNeighbours = 0;
		for (int i = 0; i < 8; ++i)
		{
			long long neighbourX = cellX + dx[i];
			long long neighbourY = cellY + dy[i];

			if (currentActiveCells.find({ neighbourX, neighbourY }) != currentActiveCells.end())
				++numActiveNeighbours;
		}
        if (numActiveNeighbours == 2 || numActiveNeighbours == 3)
        {
            nextActiveCells.insert(activeCell);

			for (int i = 0; i < 8; ++i)
			{
				long long neighbourX = cellX + dx[i];
				long long neighbourY = cellY + dy[i];

				nextPotentiallyActiveCells.emplace(neighbourX, neighbourY);
			}
        }
	}

	for (pair<long long, long long> activeCell : potentiallyActiveCells)
	{
		long long cellX = activeCell.first;
		long long cellY = activeCell.second;

		if (currentActiveCells.find({ cellX, cellY }) != currentActiveCells.end())
			continue;

		int numActiveNeighbours = 0;
		for (int i = 0; i < 8; ++i)
		{
			long long neighbourX = cellX + dx[i];
			long long neighbourY = cellY + dy[i];

			if (currentActiveCells.find({ neighbourX, neighbourY }) != currentActiveCells.end())
				++numActiveNeighbours;
		}
        if (numActiveNeighbours == 3)
        {
            nextActiveCells.insert(activeCell);

            for (int i = 0; i < 8; ++i)
            {
                long long neighbourX = cellX + dx[i];
                long long neighbourY = cellY + dy[i];

				nextPotentiallyActiveCells.emplace(neighbourX, neighbourY);
            }
        }
	}

    currentActiveCells = nextActiveCells;
	potentiallyActiveCells = nextPotentiallyActiveCells;
}

void addActiveCellsInRectangle()
{
    static std::random_device random_device;
	static std::mt19937 generator(random_device());
    std::uniform_int_distribution<> uniformDistribution(0, RANDOM_RECTANGLE_PROBABILITY);

	for (int i = -RANDOM_RECTANGLE_WIDTH / 2; i < RANDOM_RECTANGLE_WIDTH / 2; ++i)
    {
		for (int j = -RANDOM_RECTANGLE_HEIGHT / 2; j < RANDOM_RECTANGLE_HEIGHT / 2; ++j)
        {
			if (uniformDistribution(generator) == RANDOM_RECTANGLE_PROBABILITY)
			{
				long long cellX = (long long)cameraX + i;
				long long cellY = (long long)cameraY + j;

				currentActiveCells.emplace(cellX, cellY);

				for (int k = 0; k < 8; ++k)
				{
					long long neighbourX = cellX + dx[k];
					long long neighbourY = cellY + dy[k];

					potentiallyActiveCells.emplace(neighbourX, neighbourY);
				}
			}
        }
    }
}

void updateDeltaTime()
{
    currentTime = glfwGetTime();
    deltaTime = currentTime - previousTime;
    previousTime = currentTime;
}

void handleInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        if (currentTime - lastTimeConway >= CONWAY_WAITING_TIME)
        {
            lastTimeConway = currentTime;
            Conway();
        }
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraY += CAMERA_Y_SPEED * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraY -= CAMERA_Y_SPEED * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraX -= CAMERA_X_SPEED * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraX += CAMERA_X_SPEED * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        cameraZoom -= CAMERA_ZOOM_SPEED * deltaTime;
		cameraZoom = max(cameraZoom, EPSILON_ZOOM);
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        cameraZoom += CAMERA_ZOOM_SPEED * deltaTime;
    }

    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
    {
        addActiveCellsInRectangle();
    }
}

vector<double> vertices;

/*
2----3/4
|  /   |
| /    |
1/5----6
(x1, y1) down-left
(x2, y2) up-right
*/

void addRectangleToDrawings(double x1, double y1, double x2, double y2)
{
    vertices.emplace_back(x1);
    vertices.emplace_back(y1);

    vertices.emplace_back(x1);
    vertices.emplace_back(y2);

    vertices.emplace_back(x2);
    vertices.emplace_back(y2);

    vertices.emplace_back(x2);
    vertices.emplace_back(y2);

    vertices.emplace_back(x1);
    vertices.emplace_back(y1);

    vertices.emplace_back(x2);
    vertices.emplace_back(y1);
}

int colourPath;

void draw()
{
    vertices.clear();

	for (pair<long long, long long> activeCell : currentActiveCells)
	{
		long long cellX = activeCell.first;
		long long cellY = activeCell.second;

		double relativeCellX = cellX - cameraX;
		double relativeCellY = cellY - cameraY;

        double finalCellWidth = CELL_WIDTH * cameraZoom;
        double finalCellHeight = CELL_HEIGHT * cameraZoom;

        double finalCellX = relativeCellX * finalCellWidth;
		double finalCellY = relativeCellY * finalCellHeight;

		addRectangleToDrawings(finalCellX - finalCellWidth / 2.0, finalCellY - finalCellHeight / 2.0, finalCellX + finalCellWidth / 2.0, finalCellY + finalCellHeight / 2.0);
	}

    if (vertices.size() > 0)
    {
        glBufferData(GL_ARRAY_BUFFER, sizeof(double) * vertices.size(), &(vertices.front()), GL_DYNAMIC_DRAW);

        glUniform3f(colourPath, 1.0, 1.0, 1.0);

        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 2);
    }
}

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "John Conway's Game of Life", 0, 0);
    // glfwGetPrimaryMonitor();

    glfwMakeContextCurrent(window);

    glewInit();

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glUseProgram(shaderProgram);

    colourPath = glGetUniformLocation(shaderProgram, "colour");
    int orthoPath = glGetUniformLocation(shaderProgram, "ortho");

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 2, GL_DOUBLE, GL_FALSE, 2 * sizeof(double), (void*)0);
    glEnableVertexAttribArray(0);

    glm::mat4 ortho = glm::ortho(-0.5 * WINDOW_WIDTH, 0.5 * WINDOW_WIDTH, -0.5 * WINDOW_HEIGHT, 0.5 * WINDOW_HEIGHT);
    glUniformMatrix4fv(orthoPath, 1, GL_FALSE, glm::value_ptr(ortho));

	initConway();

    while (!glfwWindowShouldClose(window))
    {
        updateDeltaTime();

        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        handleInput(window);

        draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}


