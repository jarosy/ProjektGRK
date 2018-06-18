#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <vector>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Camera.h"
#include "Texture.h"

class Fish {
public:
	bool isDead;
	glm::vec3 position;
	float fallingTime;
};

GLuint programColor;
GLuint programTexture;
GLuint programProcTexture;

Core::Shader_Loader shaderLoader;

obj::Model sharkModel;
obj::Model fishModel;
obj::Model fishboneModel;

float cameraHeight = 0;
glm::vec3 cameraPos = glm::vec3(0, cameraHeight, 5);
glm::vec3 cameraDir; // Wektor "do przodu" kamery
glm::vec3 cameraSide; // Wektor "w bok" kamery
float cameraAngle = glm::radians(-90.0f);

glm::mat4 cameraMatrix, perspectiveMatrix;

glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, -0.9f, -1.0f));

GLuint textureFish;

glm::vec3 fish1Points[10];

Fish fishes[10];

glm::mat4 fish1Matrix;
glm::mat4 fish2Matrix;
glm::mat4 fishboneMatrix;

bool fish1dead = false;

float appLoadingTime;

//glm::quat rotation = glm::quat(1, 0, 0, 0);
//glm::vec2 mousePosition;
//float key_yaw;
//float key_pitch;

void keyboard(unsigned char key, int x, int y)
{
	
	float angleSpeed = 0.1f;
	float moveSpeed = 0.1f;
	switch(key)
	{
	case 'z': cameraAngle -= angleSpeed; break;
	case 'x': cameraAngle += angleSpeed; break;
	case 'w': cameraPos += cameraDir * moveSpeed; break;
	case 's': cameraPos -= cameraDir * moveSpeed; break;
	case 'd': cameraPos += glm::cross(cameraDir, glm::vec3(0, 1, 0)) * moveSpeed; break;
	case 'a': cameraPos -= glm::cross(cameraDir, glm::vec3(0, 1, 0)) * moveSpeed; break;
	}
}

void mouse(int button, int state, int x, int y)
{

	switch (button) {
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN) {
			cameraHeight += 0.1f;
		}
		break;
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN) {
			cameraHeight -= 0.1f;
		}
		break;

	}
	
}

glm::mat4 createCameraMatrix()
{
	/// Obliczanie kierunku patrzenia kamery (w plaszczyznie x-z) przy uzyciu zmiennej cameraAngle kontrolowanej przez klawisze.
	cameraDir = glm::vec3(cosf(cameraAngle), 0.0f, sinf(cameraAngle));
	glm::vec3 up = glm::vec3(0,1,0);

	return Core::createViewMatrix(cameraPos, cameraDir, up);
}

void drawObjectColor(obj::Model * model, glm::mat4 modelMatrix, glm::vec3 color)
{
	GLuint program = programColor;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "objectColor"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}

void drawObjectTexture(obj::Model * model, glm::mat4 modelMatrix, GLuint textureId)
{
	GLuint program = programTexture;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
	Core::SetActiveTexture(textureId, "textureSampler", program, 0);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}

void drawObjectProceduralTexture(obj::Model * model, glm::mat4 modelMatrix, glm::vec3 color)
{
	GLuint program = programProcTexture;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "objectColor"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}

bool isColision(glm::mat4 matrix1, glm::mat4 matrix2) {
	if (matrix1[3][0] < matrix2[3][0] + 0.3 && matrix1[3][0] > matrix2[3][0] - 0.3 &&
		matrix1[3][1] < matrix2[3][1] + 0.3 && matrix1[3][1] > matrix2[3][1] - 0.3 &&
		matrix1[3][2] < matrix2[3][2] + 0.3 && matrix1[3][2] > matrix2[3][2] - 0.3) {
		return true;
	}
	else {
		return false;
	}
}

void renderScene()
{
	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f - appLoadingTime;
	// Aktualizacja macierzy widoku i rzutowania
	cameraPos = glm::vec3(cameraPos.x, cameraHeight, cameraPos.z);
	cameraMatrix = createCameraMatrix();
	perspectiveMatrix = Core::createPerspectiveMatrix();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.1f, 0.3f, 1.0f);

	glm::mat4 sharkInitialTransformation = glm::translate(glm::vec3(0,-0.4f,0)) * glm::scale(glm::vec3(0.08f));
	glm::mat4 sharkModelMatrix = glm::translate(cameraPos + cameraDir * 0.5f) * glm::rotate(-cameraAngle + glm::radians(90.0f), glm::vec3(0, 1, 0)) * sharkInitialTransformation;
	drawObjectProceduralTexture(&sharkModel, sharkModelMatrix, glm::vec3(0.4f, 0.2f, 0.0f));

	int point = glm::floor(time);
	glm::vec3 up = glm::vec3(0, 1, 0);

	glm::vec3 fish1Pos = glm::catmullRom(fish1Points[(point - 1) % 10], fish1Points[point % 10], fish1Points[(point + 1) % 10], fish1Points[(point + 2) % 10], glm::fract(time+1));
	glm::vec3 fish1Dir = glm::normalize(glm::catmullRom(fish1Points[(point - 1) % 10], fish1Points[point % 10], fish1Points[(point + 1) % 10], fish1Points[(point + 2) % 10], glm::fract(time) + 0.001) - glm::catmullRom(fish1Points[(point - 1) % 10], fish1Points[point % 10], fish1Points[(point + 1) % 10], fish1Points[(point + 2) % 10], glm::fract(time + 1) - 0.001));
	fish1Matrix = Core::createViewMatrix(fish1Pos, fish1Dir, up) * glm::scale(glm::vec3(0.05f));

	if (!fish1dead) {
		drawObjectProceduralTexture(&sharkModel, fish1Matrix, glm::vec3(0.8f, 0.1f, 0.1f));
	}

	for (int i = 0; i < 10; i++) {
		fish2Matrix = glm::translate(fishes[i].position) * glm::rotate(glm::radians(10.0f)*time * 3, glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.1f));
		fishboneMatrix = glm::translate(fishes[i].position) * glm::translate(glm::vec3(0.0f, -0.3f * (time - fishes[i].fallingTime), 0.0f))  * 
			glm::rotate(glm::radians(10.0f)*time * 3, glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.1f));
		if (!fishes[i].isDead) {
			drawObjectTexture(&fishModel, fish2Matrix, textureFish);
		}
		else {
			drawObjectColor(&fishboneModel, fishboneMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
		}

		if (isColision(sharkModelMatrix, fish2Matrix)) {
			 fishes[i].isDead= true;
			 fishes[i].fallingTime = time;
		}
	}

	glutSwapBuffers();
}


void init()
{
	srand(time(0));
	glEnable(GL_DEPTH_TEST);
	programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	programProcTexture = shaderLoader.CreateProgram("shaders/shader_proc_tex.vert", "shaders/shader_proc_tex.frag");
	fishModel = obj::loadModelFromFile("models/fish2.obj");
	sharkModel = obj::loadModelFromFile("models/shark.obj");
	fishboneModel = obj::loadModelFromFile("models/fishbone2.obj");
	textureFish = Core::LoadTexture("textures/fish.png");

	static const float fish1Radius = 2.5;
	static const float fish1Offset = 1.35;
	for (int i = 0; i < 10; i++)
	{
		float angle = (float(i))*(2 * glm::pi<float>() / 10);
		float radius = fish1Radius * (0.95 + glm::linearRand(0.0f, 0.1f));
		fish1Points[i] = glm::vec3(cosf(angle) + fish1Offset, 0.0f, sinf(angle)) * fish1Radius;
		fishes[i].position = glm::ballRand(5.0f);
		fishes[i].isDead = false;
	}

	appLoadingTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
}

void shutdown()
{
	shaderLoader.DeleteProgram(programColor);
	shaderLoader.DeleteProgram(programTexture);
}

void idle()
{
	glutPostRedisplay();
}

int main(int argc, char ** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(150, 80);
	glutInitWindowSize(600, 600);
	glutCreateWindow("OpenGL Pierwszy Program");
	glewInit();

	init();
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutDisplayFunc(renderScene);
	glutIdleFunc(idle);

	glutMainLoop();

	shutdown();

	return 0;
}
