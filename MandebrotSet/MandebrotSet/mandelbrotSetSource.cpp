#include<fstream>
#include<iomanip>
#include<iostream>
#include<conio.h>
#include<sstream>
#include<string>
#include<vector>
#include<limits>
#include<GL/glew.h>
#include<glfw3.h>
#include<sys/stat.h>

using namespace std;

static void ErrorCallback(int error, const char* description)
{
	cerr << "GLFW ERROR : " << description << endl;
}

double cx = 0.0;
double cy = 0.0;
double zoom = 1.0;
int iterations = 100;
int fps = 0;

GLFWwindow *window = nullptr;

int width = 2000;
int height = 1000;

GLuint program;
GLuint shader;

double last_time = 0, current_time = 0;
unsigned int ticks = 0;
bool keys[1024] = {0};

static void CursorCallback(GLFWwindow* window, double xposition, double yposition)
{

}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	double xposition, yposition;
	glfwGetCursorPos(window, &xposition, &yposition);

	double xr = 2.0 * (xposition / (double)width - 0.5);
	double yr = 2.0 * (yposition / (double)height - 0.5);
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		cx += (xr - cx) / zoom / 2.0;
		cy -= (yr - cy) / zoom / 2.0;
	}
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	zoom += yoffset * 0.1 * zoom;
	if (zoom < 0.1)
	{
		zoom = 0.1;
	}
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	const double d = 0.1 / zoom;
	if (action == GLFW_PRESS)
	{
		keys[key] = true;
	}
	else if (action == GLFW_RELEASE)
	{
		keys[key] = false;
	}

	if (keys[GLFW_KEY_ESCAPE])
	{
		glfwSetWindowShouldClose(window, 1);
	}
	else if (keys[GLFW_KEY_A])
	{
		cx -= d;
	}
	else if (keys[GLFW_KEY_D])
	{
		cx += d;
	}
	else if (keys[GLFW_KEY_W])
	{
		cy += d;
	}
	else if (keys[GLFW_KEY_S])
	{
		cy -= d;
	}
	else if (keys[GLFW_KEY_MINUS] && iterations < numeric_limits<int>::max() - 10)
	{
		iterations += 10;
	}
	else if (keys[GLFW_KEY_EQUAL])
	{
		iterations -= 10;
		if (iterations <= 0)
		{
			iterations = 0;
		}
	}
}

const char* vertex_shader = 
	"#version 410\n"
	"in vec3 v;"
	"void main()"
	"{"
	"gl_Position = vec4(v, 1.0);"
	"}" 
	;

static void UpdateWindowTitle()
{
	ostringstream ss;
	ss << "MANDELBROT 2D VIEWER";
	ss << " FPS:" << fps;
	ss << " ITERATIONS:" << iterations;
	ss << " ZOOM:" << zoom;
	ss << " AT:(" << setprecision(8) << cx << "+" << cy << "i)";
	glfwSetWindowTitle(window, ss.str().c_str());
}

static void CompileShader(GLuint &program)
{
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertex_shader, NULL);
	glCompileShader(vertexShader);

	ifstream t("fragmentShader.glsl");
	if (!t.is_open())
	{
		cerr << "CANNOT OPEN fragmentShader.glsl" << endl;
		return;
	}
	string str((istreambuf_iterator<char>(t)), istreambuf_iterator<char>());
	const char* source = str.c_str();
	
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &source, NULL);
	glCompileShader(fragmentShader);

	int success;
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		int s;
		glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &s);
		char *buffer = new char[s];
		glGetShaderInfoLog(fragmentShader, s, &s, buffer);
		cerr << buffer << endl;
		delete[] buffer;
		return;
	}

	program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		int s;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &s);
		char *buffer = new char[s];
		glGetProgramInfoLog(program, s, &s, buffer);

		cerr << buffer << endl;
		delete[] buffer;
		return;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

static time_t last_mtime;

static time_t get_mtime(const char *path)
{
	struct stat statbuffer;
	if (stat(path, &statbuffer) == -1)
	{
		perror(path);
		exit(1);
	}
	return statbuffer.st_mtime;
}

int main(int argc, char* argv[])
{
	if (!glfwInit())
	{
		cerr << "FAILED TO INITIALISE GLFW" << endl;
		return 1;
	}
	atexit(glfwTerminate);
	glfwSetErrorCallback(ErrorCallback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(width, height, "MANDELBROT 2D SET", NULL, NULL);
	if (!window)
	{
		cerr << "FAILED TO CREATE WINDOW" << endl;
		return 1;
	}

	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, CursorCallback);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);
	glfwSetScrollCallback(window, ScrollCallback);
	glfwSetInputMode(window, GLFW_CURSOR_NORMAL, GLFW_STICKY_KEYS);

	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	glewInit();

	cout << "RENDERER : " << glGetString(GL_RENDERER) << endl;
	cout << "OPNEGL VERSION : " << glGetString(GL_VERSION) << endl;

	GLuint program;
	CompileShader(program);

	last_mtime = get_mtime("fragmentShader.glsl");

	float points[] =
	{
		-1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,

		-1.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
	};

	GLuint vbo = 0;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 2 * 9 * sizeof(float), points, GL_STATIC_DRAW);

	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glUseProgram(program);
	last_time = glfwGetTime();
	glBindVertexArray(vao);

	while (!glfwWindowShouldClose(window))
	{
		time_t new_time = get_mtime("fragmentShader.glsl");
		if (new_time != last_mtime)
		{
			glDeleteProgram(program);
			CompileShader(program);
			glUseProgram(program);
			last_mtime = new_time;

			cout << "RELOADER SHADER : " << last_mtime << endl;
		}

		glfwGetWindowSize(window, &width, &height);
		glUniform2d(glGetUniformLocation(program, "screen_size"), (double)width, (double)height);
		glUniform1d(glGetUniformLocation(program, "screen_ratio"), (double)width / (double)height);
		glUniform2d(glGetUniformLocation(program, "center"), cx, cy);
		glUniform1d(glGetUniformLocation(program, "zoom"), zoom);
		glUniform1i(glGetUniformLocation(program, "iterations"), iterations);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0, 0, 0, 0);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);
		glfwPollEvents();

		ticks++;
		current_time = glfwGetTime();
		if (current_time - last_time > 1.0)
		{
			fps = ticks;
			UpdateWindowTitle();
			last_time = glfwGetTime();
			ticks = 0;
		}
	}

	glfwDestroyWindow(window);
	
	return 0;
}