// Windows includes (For Time, IO, etc.)
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <math.h>
#include <vector> // STL dynamic memory.

// OpenGL includes
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <C:\Users\Sarah\Documents\Colleen\Lab 04 - Sample Object Hierarchy\glm\glm\glm.hpp>

// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

// Project includes
#include "maths_funcs.h"



/*----------------------------------------------------------------------------
MESH TO LOAD
----------------------------------------------------------------------------*/
// this mesh is a dae file format but you should be able to use any other format too, obj is typically what is used
// put the mesh in your project directory, or provide a filepath for it here
#define MESH_NAME_1 "alienFinal.dae"
#define MESH_NAME_2 "ufoFinal.dae"
/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/

#pragma region SimpleTypes
typedef struct
{
	size_t mPointCount = 0;
	std::vector<vec3> mVertices;
	std::vector<vec3> mNormals;
	std::vector<vec2> mTextureCoords;
} ModelData;
#pragma endregion SimpleTypes

using namespace std;
GLuint shaderProgramID;
GLuint VAO[2];
GLuint VBO[2];
GLuint shaderProgramID2;
GLuint shaderProgramID1;

static const char* pVS1 = "                                                    \n\
#version 330                                                                  \n\
                                                                              \n\
in vec3 vPosition;															  \n\
in vec4 vColor;																  \n\
out vec4 color;																 \n\
                                                                              \n\
                                                                               \n\
void main()                                                                     \n\
{                                                                                \n\
    gl_Position = vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);  \n\
	color = vColor;							\n\
}";

// Fragment Shader
// Note: no input in this shader, it just outputs the colour of all fragments, in this case set to red (format: R, G, B, A).
static const char* pFS1 = "                                              \n\
#version 330                                                            \n\
                                                                        \n\
in vec4 color;                                                      \n\
out vec4 FragColor;                                                      \n\
                                                                          \n\
void main()                                                               \n\
{                                                                          \n\
FragColor = color;									 \n\
}";


static const char* pVS2 = "                                                    \n\
#version 330                                                                  \n\
                                                                              \n\
in vec3 vPosition;															  \n\
in vec4 vColor;																  \n\
out vec4 color;																 \n\
                                                                              \n\
                                                                               \n\
void main()                                                                     \n\
{                                                                                \n\
    gl_Position = vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);  \n\
	color = vColor;							\n\
}";

// Fragment Shader
// Note: no input in this shader, it just outputs the colour of all fragments, in this case set to red (format: R, G, B, A).
static const char* pFS2 = "                                              \n\
#version 330                                                            \n\
                                                                        \n\
in vec4 color;                                                      \n\
out vec3 FragColor;                                                      \n\
                                                                          \n\
void main()                                                               \n\
{                                                                          \n\
FragColor = vec3(1.0f,1.0f,0.0f);									 \n\
}";

ModelData mesh_data1;
ModelData mesh_data2;
unsigned int mesh_vao = 0;
int width = 800;
int height = 600;
mat4 model = identity_mat4();
char input;
float x, y = 0.0;
float z = -10.0;
float v = 0.0;
float b = 1.0;
float c = 1.0f;
vec3 cameraPos = vec3(0.0f, 0.0f, 3.0f);
vec3 cameraUp = vec3(0.0f, 1.0f, 0.0f);
vec3 cameraFront = vec3(0.0f, 0.0f, -1.0f);
//float cameraSpeed = 0.05f;
vec3 cameraSpeed =vec3(0.05f, 0.05f, 0.05f);



GLuint loc1, loc2, loc3;
GLfloat rotate_y = 0.0f;


#pragma region MESH LOADING
/*----------------------------------------------------------------------------
MESH LOADING FUNCTION
----------------------------------------------------------------------------*/

ModelData load_mesh(const char* file_name) {
	ModelData modelData;

	/* Use assimp to read the model file, forcing it to be read as    */
	/* triangles. The second flag (aiProcess_PreTransformVertices) is */
	/* relevant if there are multiple meshes in the model file that   */
	/* are offset from the origin. This is pre-transform them so      */
	/* they're in the right position.                                 */
	const aiScene* scene = aiImportFile(
		file_name,
		aiProcess_Triangulate | aiProcess_PreTransformVertices
	);

	if (!scene) {
		fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
		return modelData;
	}

	printf("  %i materials\n in %s\n", scene->mNumMaterials, file_name);
	printf("  %i meshes\n in %s\n", scene->mNumMeshes, file_name);
	printf("  %i textures\n in %s\n", scene->mNumTextures, file_name);

	for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
		const aiMesh* mesh = scene->mMeshes[m_i];
		printf("    %i vertices in mesh\n", mesh->mNumVertices);
		modelData.mPointCount += mesh->mNumVertices;
		for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
			if (mesh->HasPositions()) {
				const aiVector3D* vp = &(mesh->mVertices[v_i]);
				modelData.mVertices.push_back(vec3(vp->x, vp->y, vp->z));
			}
			if (mesh->HasNormals()) {
				const aiVector3D* vn = &(mesh->mNormals[v_i]);
				modelData.mNormals.push_back(vec3(vn->x, vn->y, vn->z));
			}
			if (mesh->HasTextureCoords(0)) {
				const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
				modelData.mTextureCoords.push_back(vec2(vt->x, vt->y));
			}
			if (mesh->HasTangentsAndBitangents()) {
				/* You can extract tangents and bitangents here              */
				/* Note that you might need to make Assimp generate this     */
				/* data for you. Take a look at the flags that aiImportFile  */
				/* can take.                                                 */
			}
		}
	}

	aiReleaseImport(scene);
	return modelData;
}

#pragma endregion MESH LOADING

// Shader Functions- click on + to expand
#pragma region SHADER_FUNCTIONS
char* readShaderSource(const char* shaderFile) {
	FILE* fp;
	fopen_s(&fp, shaderFile, "rb");

	if (fp == NULL) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	buf[size] = '\0';

	fclose(fp);

	return buf;
}


static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		std::cerr << "Error creating shader..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	//const char* pShaderSource = readShaderSource(pShaderText);

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderText, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024] = { '\0' };
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		std::cerr << "Error compiling "
			<< (ShaderType == GL_VERTEX_SHADER ? "vertex" : "fragment")
			<< " shader program: " << InfoLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders(const char * pVS, const char * pFS)
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		std::cerr << "Error creating shader program..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(shaderProgramID, pVS, GL_VERTEX_SHADER);
	AddShader(shaderProgramID, pFS, GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { '\0' };
	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Error linking shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Invalid shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	//glUseProgram(shaderProgramID);
	return shaderProgramID;
}
#pragma endregion SHADER_FUNCTIONS

// VBO Functions - click on + to expand
#pragma region VBO_FUNCTIONS
void generateObjectBufferMesh() {
	/*----------------------------------------------------------------------------
	LOAD MESH HERE AND COPY INTO BUFFERS
	----------------------------------------------------------------------------*/

	//Note: you may get an error "vector subscript out of range" if you are using this code for a mesh that doesnt have positions and normals
	//Might be an idea to do a check for that before generating and binding the buffer.



	glGenVertexArrays(2, VAO);
	glBindVertexArray(VAO[0]);

	glGenBuffers(2, VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);


	mesh_data1 = load_mesh(MESH_NAME_1);
	mesh_data2 = load_mesh(MESH_NAME_2);
	unsigned int vp_vbo = 0;


	
	glBufferData(GL_ARRAY_BUFFER, mesh_data1.mPointCount * sizeof(vec3), &mesh_data1.mVertices[0], GL_STATIC_DRAW);
	//unsigned int vn_vbo = 0;
	//glGenBuffers(1, &vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data1.mPointCount * sizeof(vec3), &mesh_data1.mNormals[0], GL_STATIC_DRAW);
	//linkCurrentBuffertoShader(shaderProgramID1);

	loc1 = glGetAttribLocation(shaderProgramID1, "vertex_position");
	loc2 = glGetAttribLocation(shaderProgramID1, "vertex_normal");
	loc3 = glGetAttribLocation(shaderProgramID1, "vertex_texture");

	glEnableVertexAttribArray(loc1);
	//glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc2);
	//glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	//	This is for texture coordinates which you don't currently need, so I have commented it out
	//	unsigned int vt_vbo = 0;
	//	glGenBuffers (1, &vt_vbo);
	//	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	//	glBufferData (GL_ARRAY_BUFFER, monkey_head_data.mTextureCoords * sizeof (vec2), &monkey_head_data.mTextureCoords[0], GL_STATIC_DRAW);


	glBindVertexArray(VAO[1]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	// Put the vertices and colors into a vertex buffer object

	glBufferData(GL_ARRAY_BUFFER, mesh_data2.mPointCount * sizeof(vec3), &mesh_data2.mVertices[0], GL_STATIC_DRAW);
	//unsigned int vn_vbo = 0;
	//glGenBuffers(1, &vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data2.mPointCount * sizeof(vec3), &mesh_data2.mNormals[0], GL_STATIC_DRAW);
	// Link the current buffer to the shader
	//linkCurrentBuffertoShader(shaderProgramID2);

	loc1 = glGetAttribLocation(shaderProgramID2, "vertex_position");
	loc2 = glGetAttribLocation(shaderProgramID2, "vertex_normal");
	loc3 = glGetAttribLocation(shaderProgramID2, "vertex_texture");

	glEnableVertexAttribArray(loc1);
	//glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc2);
	//glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);


	//unsigned int vao = 0;
	//glBindVertexArray(vao);

	

	//	This is for texture coordinates which you don't currently need, so I have commented it out
	//	glEnableVertexAttribArray (loc3);
	//	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	//	glVertexAttribPointer (loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);
}

void linkCurrentBuffertoShader(GLuint shaderProgramID) {
	loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");

	glEnableVertexAttribArray(loc1);
	//glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc2);
	//glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
}
#pragma endregion VBO_FUNCTIONS


void display() {

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindVertexArray(VAO[0]);
	glUseProgram(shaderProgramID1);



	//Declare your uniform variables that will be used in your shader
	int matrix_location1 = glGetUniformLocation(shaderProgramID1, "model");
	int view_mat_location1 = glGetUniformLocation(shaderProgramID1, "view");
	int proj_mat_location1 = glGetUniformLocation(shaderProgramID1, "proj");

	mat4 view1 = identity_mat4();
	mat4 persp_proj1 = perspective(120.0f, (float)width / (float)height, 0.1f, 1000.0f);
	mat4 model1 = identity_mat4();


	if (input == 'o') {
		persp_proj1 = orthographic((float)width, (float)height, 0.1f, 1000.0f);
	}
	else if (input == 'p') {
		persp_proj1 = perspective(120.0f, (float)width / (float)height, 0.1f, 1000.0f);
	}


	view1 = look_at(cameraPos, cameraPos + cameraFront, cameraUp);



	// Root of the Hierarchy
	if (input == 'x') {
		model1 = rotate_x_deg(model1, rotate_y);

	}
	else if (input == 'y') {
		model1 = rotate_y_deg(model1, rotate_y);

	}
	else if (input == 'z') {
		model1 = rotate_z_deg(model1, rotate_y);
	}
	
	view1 = translate(view1, vec3(x, y, z));
	model1 = scale(model1, vec3(c, c, c));

	// update uniforms & draw
	glUniformMatrix4fv(proj_mat_location1, 1, GL_FALSE, persp_proj1.m);
	glUniformMatrix4fv(view_mat_location1, 1, GL_FALSE, view1.m);
	glUniformMatrix4fv(matrix_location1, 1, GL_FALSE, model1.m);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data1.mPointCount);




	glBindVertexArray(VAO[1]);
	glUseProgram(shaderProgramID2);


	int matrix_location2 = glGetUniformLocation(shaderProgramID2, "model");
	int view_mat_location2 = glGetUniformLocation(shaderProgramID2, "view");
	int proj_mat_location2 = glGetUniformLocation(shaderProgramID2, "proj");

	mat4 view2 = identity_mat4();
	mat4 persp_proj2 = perspective(120.0f, (float)width / (float)height, 0.1f, 1000.0f);
	mat4 model2 = identity_mat4();

	view2 = look_at(cameraPos, cameraPos + cameraFront, cameraUp);


	view2 = translate(view2, vec3(x, y, z));
	model2 = scale(model2, vec3(c, c, c));

	// update uniforms & draw
	glUniformMatrix4fv(proj_mat_location2, 1, GL_FALSE, persp_proj2.m);
	glUniformMatrix4fv(view_mat_location2, 1, GL_FALSE, view2.m);
	glUniformMatrix4fv(matrix_location2, 1, GL_FALSE, model2.m);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data2.mPointCount);

	// Set up the child matrix
	/*mat4 modelChild = identity_mat4();
	modelChild = rotate_z_deg(modelChild, 180);
	modelChild = rotate_y_deg(modelChild, rotate_y);
	modelChild = translate(modelChild, vec3(0.0f, 1.9f, 0.0f));*/

	// Apply the root matrix to the child matrix
	/*modelChild = model * modelChild;*/

	// Update the appropriate uniform and draw the mesh again
	//glUniformMatrix4fv(matrix_location, 1, GL_FALSE, modelChild.m);
	//glDrawArrays(GL_TRIANGLES, 0, mesh_data.mPointCount);

	glutSwapBuffers();
	glBindVertexArray(0);
}




void updateScene() {

	static DWORD last_time = 0;
	DWORD curr_time = timeGetTime();
	if (last_time == 0)
		last_time = curr_time;
	float delta = (curr_time - last_time) * 0.001f;
	last_time = curr_time;

	// Rotate the model slowly around the y axis at 20 degrees per second
	rotate_y += 20.0f * delta;
	rotate_y = fmodf(rotate_y, 360.0f);

	// Draw the next frame
	glutPostRedisplay();
}


void init()
{
	// Set up the shaders
	//GLuint shaderProgramID = CompileShaders();
	// load mesh into a vertex buffer array

	shaderProgramID1 = CompileShaders(pVS1, pFS1);
	shaderProgramID2 = CompileShaders(pVS2, pFS2);


	generateObjectBufferMesh();

}

// Placeholder code for the keypress
void keypress(unsigned char key, int xx, int yy) {
	if (key == 'x') {
		input = 'x';
	}
	else if (key == 'y') {
		input = 'y';
	}
	else if (key == 'z') {
		input = 'z';
	}
	else if (key == 'e') {
		x += 5.0;
	}
	else if (key == 'n') {
		y += 5.0;
	}
	else if (key == 's') {
		y += -5.0;
	}
	else if (key == 'w') {
		x += -5.0;
	}
	else if (key == 'c') {
		c += 0.5;
	}
	else if (key == 'v') {
		c -= 0.5;
	}
	else if (key == 'o') {
		input = 'o';
	}
	else if (key == 'p') {
		input = 'p';
	}
	else if (key == 't') {
		
		cameraPos += cross(cameraFront,(cameraSpeed));
	}
	else if (key == 'f') {
		cameraPos -= cross(normalise(cross(cameraFront, cameraUp)), cameraSpeed);
	}
	else if (key == 'g') {
		cameraPos -= cross(cameraFront, (cameraSpeed)); 
	}
	else if (key == 'h') {
		cameraPos += cross(normalise(cross(cameraFront, cameraUp)),cameraSpeed);
	}

}

int main(int argc, char** argv) {

	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("Hello Triangle");

	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutIdleFunc(updateScene);
	glutKeyboardFunc(keypress);

	// A call to glewInit() must be done after glut is initialized!
	GLenum res = glewInit();
	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}
	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();
	return 0;
}