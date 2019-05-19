 /*
CG HW1
Please fill the functions whose parameters are replaced by ... in the following function 
static int add_obj(unsigned int program, const char *filename)(line 140) 
static void render() (line 202)

For example : line 156 //glGenVertexArrays(...);

*/
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "tiny_obj_loader.h"
#include <queue>
#include "tree.h"

#define GLM_FORCE_RADIANS

using namespace std;

float aspect;
GLint modelLoc;
GLint projLoc;
GLint num;
glm::mat4 object_matrix[12];
glm::mat4  eye(1.0f);
float center[12][3];

static void error_callback(int error, const char* description);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
static unsigned int setup_shader(const char *vertex_shader, const char *fragment_shader);
static std::string readfile(const char *filename);
static int add_obj(unsigned int program, const char *filename);
static void releaseObjects();
static void render();
static void reshape(GLFWwindow* window, int width, int height);
void init_shader(unsigned int x);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void viewPointChange();
void init_matrix();
void moveForward();
void moveBackward();
void turnRight();
void turnLeft();
void calculate_center();
void total_matrix();
float walk_limit;
float walk_speed = 1.0f, turn_speed = 1.0f;
float walk_range = 0.0f, turn_range = 0.0f;
bool start_walk=false;
bool jump_or_not = false;
double jump_time = 0;
//settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;


glm::vec3 cameraPos = glm::vec3(19.0713f, 22.8659f, 8.49579f);
glm::vec3 cameraFront = glm::vec3(-0.63586f, -0.745475f, -0.199871f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float pitch = (float)(asin(cameraFront.y) * 180 / 3.1415926);
float yaw = (float)(acos(cameraFront.x / cameraFront.y) * 180 / 3.1415926 - 180);
bool firstMouse = true;
float lastX = 800.0f/2.0f, lastY = 600.0f/2.0f;  //   mouse position initialization
float fov = 45.0f;
int viewpoint = 1;

//timing
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame


struct object_struct{
	unsigned int program;
	unsigned int vao;
	unsigned int vbo[3];
	unsigned int texture;
	glm::mat4 model;

	//struct's constructor
	object_struct(): model(glm::mat4(1.0f)){}
} ;

std::vector<object_struct> objects;//vertex array object,vertex buffer object and texture(color) for objs
unsigned int program;

//a vector to store indices size(number) of a obj
//the size of this vector means how many objs do we want to draw
std::vector<int> indicesCount;//Number of indice of objs

static void error_callback(int error, const char* description)
{
	//to print the error message
	fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//if input "ESC" , then this function will close the window~~~ 
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_C && action == GLFW_PRESS)
		viewPointChange();

	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		jump_or_not = true;
		jump_time = glfwGetTime();
	}

}

static unsigned int setup_shader(const char *vertex_shader, const char *fragment_shader)
{
	GLuint vs=glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, (const GLchar**)&vertex_shader, nullptr);

	glCompileShader(vs);	//compile vertex shader

	int status, maxLength;
	char *infoLog=nullptr;
	glGetShaderiv(vs, GL_COMPILE_STATUS, &status);		//get compile status
	if(status==GL_FALSE)								//if compile error
	{
		glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &maxLength);	//get error message length

		/* The maxLength includes the NULL character */
		infoLog = new char[maxLength];
		
		glGetShaderInfoLog(vs, maxLength, &maxLength, infoLog);		//get error message

		fprintf(stderr, "Vertex Shader Error: %s\n", infoLog);

		/* Handle the error in an appropriate way such as displaying a message or writing to a log file. */
		/* In this simple program, we'll just leave */
		delete [] infoLog;
		return 0;
	}
	//	for fragment shader --> same as vertex shader
	GLuint fs=glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, (const GLchar**)&fragment_shader, nullptr);
	glCompileShader(fs);

	glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
	if(status==GL_FALSE)
	{
		glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &maxLength);

		/* The maxLength includes the NULL character */
		infoLog = new char[maxLength];

		glGetShaderInfoLog(fs, maxLength, &maxLength, infoLog);

		fprintf(stderr, "Fragment Shader Error: %s\n", infoLog);

		/* Handle the error in an appropriate way such as displaying a message or writing to a log file. */
		/* In this simple program, we'll just leave */
		delete [] infoLog;
		return 0;
	}

	unsigned int program=glCreateProgram();
	// Attach our shaders to our program
	glAttachShader(program, vs);
	glAttachShader(program, fs);


	
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &status);

	if(status==GL_FALSE)
	{
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);


		/* The maxLength includes the NULL character */
		infoLog = new char[maxLength];
		glGetProgramInfoLog(program, maxLength, NULL, infoLog);

		glGetProgramInfoLog(program, maxLength, &maxLength, infoLog);

		fprintf(stderr, "Link Error: %s\n", infoLog);

		/* Handle the error in an appropriate way such as displaying a message or writing to a log file. */
		/* In this simple program, we'll just leave */
		delete [] infoLog;
		return 0;
	}
	return program;
}

static std::string readfile(const char *filename)
{
	std::ifstream ifs(filename);
	if(!ifs)
		exit(EXIT_FAILURE);
	return std::string( std::istreambuf_iterator<char>(ifs),
			std::istreambuf_iterator<char>());
}

static int add_obj(unsigned int program, const char *filename)
{
	object_struct new_node;

	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err = tinyobj::LoadObj(shapes, materials, filename);

	if (!err.empty()||shapes.size()==0)
	{
		std::cerr<<err<<std::endl;
		exit(1);
	}

	//Generate memory for buffers.

	//to generate the VAO
	glGenVertexArrays(1, &new_node.vao);

	//to generate the VBOs
	glGenBuffers(3, new_node.vbo);


	glGenTextures(1,&new_node.texture);

	//Tell the program which VAO I am going to modify
	//to start VAO to store(point to) the information of VBO
	glBindVertexArray(new_node.vao);

	// Upload postion array

	//to bind the vbo[0] to GL_ARRAY_BUFFER
	glBindBuffer(GL_ARRAY_BUFFER, new_node.vbo[0]);

	//to send our obj information to vbo[0](GL_ARRAY_BUFFER)
	glBufferData(
		GL_ARRAY_BUFFER,										//the destination of our copying data
		sizeof(GLfloat)*shapes[0].mesh.positions.size(),		//the size of our copying data
		shapes[0].mesh.positions.data(),						//copying data
		GL_STATIC_DRAW											//management of this data
	);

	

	//if stride is set to 0 , it means that the stride will be determined by OpenGL
	//to give the data to position in vertex shader (location =0)
	glVertexAttribPointer(
		0,					//the vertex attribute we want to set (by location)
		3,					//depends on how much data we want to give to the vertex attribute
		GL_FLOAT,			//the type of our data,
		GL_FALSE,			//we want to normalize our data or not
		0,					//the stride
		0					//offset
	);					

	//to enable the vertex attribute buffer (location =0)
	glEnableVertexAttribArray(0);

	if(shapes[0].mesh.normals.size()>0)
	{
		//跟上面很像
		// Upload normal array
		glBindBuffer(GL_ARRAY_BUFFER, new_node.vbo[1]);
		glBufferData(
			GL_ARRAY_BUFFER,										//the destination of our copying data
			sizeof(GLfloat)*shapes[0].mesh.positions.size(),		//the size of our copying data
			shapes[0].mesh.positions.data(),						//copying data
			GL_STATIC_DRAW											//management of this data
		);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1,3, GL_FLOAT,GL_FALSE,0,0);
	}



	// Setup index buffer for glDrawElements
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, new_node.vbo[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
		sizeof(GLuint)*shapes[0].mesh.indices.size(), 
		shapes[0].mesh.indices.data(), 
		GL_STATIC_DRAW);
	indicesCount.push_back(shapes[0].mesh.indices.size());

	


	
	//glBindVertexArray(0);

	new_node.program = program;

	objects.push_back(new_node);
	return objects.size()-1;
}

static void releaseObjects()
{
	//to release all objs
	for(int i=0; (size_t)i<objects.size();i++){
		glDeleteVertexArrays(1, &objects[i].vao);
		glDeleteTextures(1, &objects[i].texture);
		glDeleteBuffers(4, objects[i].vbo);
	}
	//to release the shader
	glDeleteProgram(program);
}

static void render()
{
	float scale = 15.0;
	float time = (float)(glfwGetTime() / 100.0f);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	


	glUseProgram(program);
	glm::mat4 proj_matrix, view_matrix;
	
	//set camera matrix

	view_matrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

	proj_matrix = glm::perspective(glm::radians(fov), aspect, 0.1f, 100.0f);

	//proj_matrix = glm::frustum(-aspect / scale, aspect / scale, -1.0f / scale, 1.0f / scale, 0.10f, 1000.0f);
	//view_matrix = glm::lookAt(glm::vec3(2.0f), glm::vec3(), glm::vec3(0, 1, 0)) * glm::mat4(1.0f);

	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj_matrix*view_matrix));

	total_matrix();

	if (jump_or_not == true)
	{
		double x = glfwGetTime() - jump_time;
		if (x > 1.0)
		{
			jump_or_not = false;
			walk_limit = 0;
		}
		else
		{
			
			for (int i = 0; (size_t)i<objects.size(); i++) {

				if (x > 0.5) {
					object_matrix[i] = glm::translate(eye, glm::vec3(0.0f,(1.0f-(float)x)*3.0f, 0.0f))*object_matrix[i];
				}
				else {
					object_matrix[i] = glm::translate(eye, glm::vec3(0.0f, ((float)x)*3.0f, 0.0f))*object_matrix[i];
				}
				

			}
		}

	}

	for(int i=0; (size_t)i<objects.size();i++){
		
		//Bind VAO
		glBindVertexArray(objects[i].vao);
		
		
		glUniform1i(num, i);  //sent uniform to object_number in fragment shader
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(object_matrix[i]));
		
		//Draw object
		glDrawElements(GL_TRIANGLES, indicesCount[i], GL_UNSIGNED_INT, nullptr);
	}
	
	//Unbind VAO
	glBindVertexArray(0);
}

static void reshape(GLFWwindow* window, int width, int height)
{
	aspect = (float) width / height;
	glViewport(0, 0, width, height);
}	

void init_shader(unsigned int x)
{
	modelLoc = glGetUniformLocation(x, "model");
	projLoc	 = glGetUniformLocation(x, "proj");
	num = glGetUniformLocation(x, "object_number");
}

void init_matrix()
{
	
	for (int i = 0; (size_t)i<objects.size(); i++) {

		object_matrix[i] = glm::translate(eye, glm::vec3(0.0f, -0.0f, 0.0f));
		//right shoulder
		if (i == 2) {
			object_matrix[i] = glm::translate(eye, glm::vec3(0.0f, 0.0f, 3.7f));
		}
		//right elbow
		if (i == 6) {
			object_matrix[i] = glm::translate(eye, glm::vec3(0.0f, 0.0f, 7.25f));
		}
		//right leg
		if (i == 4 || i == 8 || i == 10) {
			object_matrix[i] = glm::translate(eye, glm::vec3(0.0f, 0.0f, 1.05f));
		}
	}
	

	
}

void processInput(GLFWwindow *window)
{
	float cameraSpeed = 2.5f * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		moveForward();
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		moveBackward();
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		turnRight();
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		turnLeft();
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = false;
	}

	float xoffset = (float)xpos - lastX;
	float yoffset = lastY - (float)ypos;
	lastX = (float)xpos;
	lastY = (float)ypos;

	float sensitivity = 0.05f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);

	
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (fov >= 1.0f && fov <= 45.0f)
		fov -= (float)yoffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;
}

void viewPointChange()
{
	if (viewpoint == 1)
	{
		cameraPos = glm::vec3(0.179363f, 26.3753f, 23.7401f);
		cameraFront = glm::vec3(-0.0157691f, -0.782065f, -0.622997f);
		cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
		pitch = (float)(asin(cameraFront.y) * 180 / 3.1415926);
		yaw = (float)(-acos(cameraFront.x / cameraFront.y) * 180 / 3.1415926);
		viewpoint = 0;
		
	}
	else
	{
		cameraPos = glm::vec3(19.0713f, 22.8659f, 8.49579f);
		cameraFront = glm::vec3(-0.63586f, -0.745475f, -0.199871f);
		cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
		viewpoint = 1;
		pitch = (float)(asin(cameraFront.y) * 180 / 3.1415926);
		yaw = (float)(acos(cameraFront.x / cameraFront.y) * 180 / 3.1415926 - 180);
		
	}
	
	/*cout << "pitch: " << pitch << ", yaw: " << yaw << endl;
	cout << "camera position: " << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << endl;
	cout << "camera front: " << cameraFront.x << ", " << cameraFront.y << ", " << cameraFront.z << endl;*/
}

void moveForward()
{
	walk_limit += walk_speed*0.8;
	walk_range += abs(walk_speed)*0.05f;
	
	if (walk_limit > 30.0)
	{
		walk_limit = 30.0f;
		walk_speed = -walk_speed;
	}
	if (walk_limit< -30.0)
	{
		walk_limit = -30.0f;
		walk_speed = -walk_speed;
	}
	std::cout << walk_range << endl;

	
}

void moveBackward()
{
	walk_limit -= walk_speed*0.8;
	walk_range -= abs(walk_speed)*0.05f;
	if (walk_limit > 30.0)
	{
		walk_limit = 30.0f;
		walk_speed = -walk_speed;
	}
	if (walk_limit< -30.0)
	{
		walk_limit = -30.0f;
		walk_speed = -walk_speed;
	}
}

void turnRight()
{
	if (turn_range >= 360)
		turn_range = 0;
	if (turn_range <= -360)
		turn_range = 0;
	if (walk_range < 3)
		turn_speed = 1.0f;
	else if(walk_range<6)
		turn_speed = 0.5f;
	else if (walk_range<9)
		turn_speed = 0.25f;
	else if (walk_range<12)
		turn_speed = 0.125f;
	else
		turn_speed = 0.1f;
	turn_range -= turn_speed;
}

void turnLeft()
{
	if (turn_range >= 360)
		turn_range = 0;
	if (turn_range <= -360)
		turn_range = 0;
	if (walk_range < 3)
		turn_speed = 1.0f;
	else if (walk_range<6)
		turn_speed = 0.5f;
	else if (walk_range<9)
		turn_speed = 0.25f;
	else if (walk_range<12)
		turn_speed = 0.125f;
	else
		turn_speed = 0.1f;
	turn_range += turn_speed;
}

void total_matrix()
{

	for (int i = 0; (size_t)i<objects.size(); i++) {

		if (i == 4 || i == 8 || i == 10) {


			object_matrix[i] = object_matrix[i] * glm::translate(eye, glm::vec3(center[4][0], center[4][1], center[4][2]))*
				glm::rotate(eye, glm::radians(walk_limit), glm::vec3(0.0f, 0.0f, 1.0f))*
				glm::translate(eye, glm::vec3(-center[4][0], -center[4][1], -center[4][2]));

		}
		else if (i == 5 || i == 9 || i == 11) {

			object_matrix[i] = object_matrix[i] * glm::translate(eye, glm::vec3(center[4][0], center[4][1], center[4][2]))*
				glm::rotate(eye, glm::radians(-walk_limit), glm::vec3(0.0f, 0.0f, 1.0f))*
				glm::translate(eye, glm::vec3(-center[4][0], -center[4][1], -center[4][2]));

		}
		else {
			object_matrix[i] = object_matrix[i] * glm::translate(eye, glm::vec3(0.0f, 0.0f, 0.0f));
		}


	}
	


	for (int i = 0; (size_t)i<objects.size(); i++) {
		
		object_matrix[i] = glm::translate(eye, glm::vec3(center[0][0], center[0][1], center[0][2]))*glm::rotate(eye, glm::radians(turn_range), glm::vec3(0.0f, 1.0f, 0.0f))
			* glm::translate(eye, glm::vec3(-center[0][0], -center[0][1], -center[0][2]))*object_matrix[i];
		//object_matrix[i] = glm::translate(eye, glm::vec3(walk_range, 0.0f, 0.0f))*object_matrix[i] ;
		object_matrix[i] =glm::translate(eye, glm::vec3(walk_range*cos(glm::radians(-turn_range)),0.0f,walk_range*sin(glm::radians(-turn_range))))* object_matrix[i];
		//cout << "angle: " << turn_range << endl << "walk_distance: " << walk_range << endl << walk_range*cos(glm::radians(-turn_range)) << "," << walk_range*sin(glm::radians(-turn_range)) << endl;
	}

}

void calculate_center()
{
	/*body*/center[0][0] =  (float)(0.410195 + 1.535768) / 2; center[0][1] = (float)(4.960414 + 9.017490) / 2; center[0][2] = (float)(0.857712 - 1.142288) / 2;
	/*head*/center[1][0] = (float)(0.229764 + 1.716199) / 2; center[1][1] = (float)(9.079612 + 11.079612) / 2; center[1][2] = (float)(0.857712 - 1.142288) / 2;
	/*rsho*/center[2][0] = (float)(0.604978 + 1.340985) / 2; center[2][1] = (float)(7.586390 + 8.307195) / 2; center[2][2] = (float)(-2.899722 - 1.101599) / 2;
	/*lsho*/center[3][0] = (float)(0.604978 + 1.340985) / 2; center[3][1] = (float)(7.586390 + 8.307195) / 2; center[3][2] = (float)(-2.899722 - 1.101599) / 2;
	/*rthi*/center[4][0] = (float)(0.731918 + 1.214045) / 2; center[4][1] = (float)(2.959748 + 4.977076) / 2; center[4][2] = (float)(-0.258849 - 1.115528) / 2;
	/*lthi*/center[5][0] = (float)(0.731918 + 1.214045) / 2; center[5][1] = (float)(2.959748 + 4.977076) / 2; center[5][2] = (float)(-0.258849 - 1.115528) / 2;
	/*relb*/center[6][0] = (float)(0.604978 + 1.340985) / 2; center[6][1] = (float)(7.586390 + 8.307195) / 2; center[6][2] = (float)(-4.666141 - 2.868017) / 2;
	/*lelb*/center[7][0] = (float)(0.604978 + 1.340985) / 2; center[7][1] = (float)(7.586390 + 8.307195) / 2; center[7][2] = (float)(-4.666141 - 2.868017) / 2;
	/*rsha*/center[8][0] = (float)(0.731918 + 1.21404) / 2; center[8][1] = (float)(0.920968 + 2.938296) / 2; center[8][2] = (float)(-0.258849 - 1.115528) / 2;
	/*lsha*/center[9][0] = (float)(0.731918 + 1.21404) / 2; center[9][1] = (float)(0.920968 + 2.938296) / 2; center[9][2] = (float)(-0.258849 - 1.115528) / 2;
	/*rank*/center[10][0] = (float)(0.720726 + 1.734643) / 2; center[10][1] = (float)(0.480430 + 0.905175) / 2; center[10][2] = (float)(-0.258849 - 1.115528) / 2;
	/*lank*/center[11][0] = (float)(0.720726 + 1.734643) / 2; center[11][1] = (float)(0.480430 + 0.905175) / 2; center[11][2] = (float)(-0.258849 - 1.115528) / 2;

}

int main(int argc, char *argv[])
{
	
	TreeNode *itr = NULL;

	TreeNode *node = new TreeNode(0, "body");												//level 0

	node->appendChild(new TreeNode(1, "head"));												//level 1
	node->appendChild(new TreeNode(2, "right_shoulder"));				
	node->appendChild(new TreeNode(3, "left_shoulder"));	
	node->appendChild(new TreeNode(4, "right_thigh"));		
	node->appendChild(new TreeNode(5, "left_thigh"));		


	node->DFS(node, "right_shoulder")->appendChild(new TreeNode(6, "right_elbow"));			//levle 2
	node->DFS(node, "left_shoulder")->appendChild(new TreeNode(7, "left_elbow"));
	node->DFS(node, "right_thigh")->appendChild(new TreeNode(8, "right_shank"));
	node->DFS(node, "left_thigh")->appendChild(new TreeNode(9, "left_shank"));

	node->DFS(node, "right_shank")->appendChild(new TreeNode(10, "right_ankle"));			//level 3
	node->DFS(node, "left_shank")->appendChild(new TreeNode(11, "left_ankle"));
	
	
	std::cout << "example~~~" << endl;
	std::cout <<"child of body: "<< node->grandChildrenNum()<<endl;

	std::cout <<"child of right thigh: "<< node->getChild(3)->grandChildrenNum() << endl;
	

	char windowName[20] = "Character moving";

	GLFWwindow* window;

	//set error function
	glfwSetErrorCallback(error_callback);

	//initialization
	if (!glfwInit())
		exit(EXIT_FAILURE);

	// OpenGL 3.3, Mac OS X is reported to have some problem. However I don't have Mac to test
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);		//set hint to glfwCreateWindow, (target, hintValue)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//hint--> window not resizable,  explicit use core-profile,  opengl version 3.3
	// For Mac OS X
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, windowName, NULL, NULL);
	if (!window)
	{
		glfwTerminate();   //end of glfw
		return EXIT_FAILURE;
	}
	glfwSetFramebufferSizeCallback(window, reshape);
	reshape(window, 800, 600);

	glfwMakeContextCurrent(window);	//set current window as main window to focus

	//tell glew to use more modern technique for managing OpenGL funtionality
	// This line MUST put below glfwMakeContextCurrent
	glewExperimental = GL_TRUE;		//tell glew to use more modern technique for managing OpenGL functionality
	glewInit();

	// Enable vsync(vertical synchronization)
	glfwSwapInterval(1);    //set the number of screen updates to wait from the time

	// Setup input callback
	glfwSetKeyCallback(window, key_callback);	//set key event handler

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);   //disable the cursor

	glfwSetCursorPosCallback(window, mouse_callback);				//for mouse input

	glfwSetScrollCallback(window, scroll_callback);


	// load shader program
	program = setup_shader(readfile("../../source/light.vert").c_str(), readfile("../../source/light.frag").c_str());
	//program = setup_shader(readfile("vs.txt").c_str(), readfile("fs.txt").c_str());
	init_shader(program);

	if (argc == 1)
	{
		add_obj(program, "../../source/obj/body.obj");							// 0, body
		add_obj(program, "../../source/obj/head.obj");							// 1, head
		add_obj(program, "../../source/obj/shoulder.obj");						// 2, right shoulder
		add_obj(program, "../../source/obj/shoulder.obj");						// 3, left shoulder
		add_obj(program, "../../source/obj/thigh.obj");							// 4, right thigh
		add_obj(program, "../../source/obj/thigh.obj");							// 5, left thigh
		add_obj(program, "../../source/obj/elbow.obj");							// 6, right elbow
		add_obj(program, "../../source/obj/elbow.obj");							// 7, left elbow
		add_obj(program, "../../source/obj/shank.obj");							// 8, right shank
		add_obj(program, "../../source/obj/shank.obj");							// 9, left shank
		add_obj(program, "../../source/obj/ankle.obj");							// 10, right ankle
		add_obj(program, "../../source/obj/ankle.obj");							// 11, left ankle
		//add_obj(program, "../../source/obj/floor.obj");							// 12, floor
	}
	else
		add_obj(program, argv[1]);
	
	calculate_center();

	//to enable z buffer
	glEnable(GL_DEPTH_TEST);
	// prevent faces rendering to the front while they're behind other faces. 

	//cull those faces which are not in our sight
	glCullFace(GL_BACK);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	glUseProgram(program);
	//glUniform1i(glGetUniformLocation(program, "texture"), 0);

	init_matrix();
	
	while (!glfwWindowShouldClose(window))
	{

		init_matrix();
		processInput(window);

		
		//program will keep draw here until you close the window
		render();

		//for same moving speed in different computer~~
		float currentFrame = (float)(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;


		glfwSwapBuffers(window);	//swap the color buffer and show it as output to the screen.
		glfwPollEvents();			//check if there is any event being triggered
	}

	releaseObjects();
	glfwDestroyWindow(window);
	
	//end of glfw
	glfwTerminate();
	
	
	return EXIT_SUCCESS;
}
