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
#define MESH_NAME_1 "alienPoss1.dae"
#define MESH_NAME_2 "ufoWOBeams1.dae"
#define MESH_NAME_3 "grassPoss.dae"
#define MESH_NAME_4 "beams1.dae"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
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
GLuint VAO[5];
GLuint VBO[11];
GLuint shaderProgramID1;
GLuint shaderProgramID2;
GLuint shaderProgramID3;
GLuint shaderProgramID4;



ModelData mesh_data1;
ModelData mesh_data2;
ModelData mesh_data3;
ModelData mesh_data4;
unsigned int mesh_vao = 0;
int width = 800;
int height = 600;
mat4 model = identity_mat4();
char input;
float up, down = 0;
float x, y = 0.0;
float tx, ty, tz = 0.0;
float z = -10.0;
float v = 0.0;
float b = 1.0;
float c = 1.0f;
float ypos;
float rad = 0.017444444;
vec3 cameraPos = vec3(0.0f, 0.0f, 3.0f);
vec3 cameraUp = vec3(0.0f, 1.0f, 0.0f);
vec3 cameraFront = vec3(0.0f, 0.0f, -1.0f);
//float cameraSpeed = 0.05f;
vec3 cameraSpeed = vec3(0.1f, 0.1f, 0.1f);
float lastX = 400;
float lastY = 300;
bool firstMouse = true;
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
bool lbuttonDown = false;
unsigned int gtexture;
unsigned int utexture;
unsigned int btexture;




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
				if (file_name == MESH_NAME_3) {
					modelData.mTextureCoords.push_back(vec2(vt->x * 10.0f, vt->y*10.0f));
				}
				else {
					modelData.mTextureCoords.push_back(vec2(vt->x , vt->y));
				}
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
	const char* pShaderSource = readShaderSource(pShaderText);

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
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

GLuint CompileShaders(const char* shaderFFile, const char* shaderVFile)
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
	AddShader(shaderProgramID, shaderVFile, GL_VERTEX_SHADER);
	AddShader(shaderProgramID, shaderFFile, GL_FRAGMENT_SHADER);

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
	//unsigned int texture;
	glGenTextures(1, &gtexture);
	glBindTexture(GL_TEXTURE_2D, gtexture);


	int wG, hG, nrChannelsG;
	unsigned char *grass = stbi_load("grassText1.jpg", &wG, &hG, &nrChannelsG, 0);
	if (grass)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, wG, hG, 0, GL_RGB, GL_UNSIGNED_BYTE, grass);
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(grass);
		glUniform1i(glGetUniformLocation(shaderProgramID3, "ourTextureG"), 0);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	mesh_data1 = load_mesh(MESH_NAME_1);//alien
	mesh_data2 = load_mesh(MESH_NAME_2);//UFO
	mesh_data3 = load_mesh(MESH_NAME_3);//Grass
	mesh_data4 = load_mesh(MESH_NAME_4);//Beams




	//Alien
	loc1 = glGetAttribLocation(shaderProgramID1, "vertex_position");
	loc2 = glGetAttribLocation(shaderProgramID1, "vertex_normal");
	loc3 = glGetAttribLocation(shaderProgramID1, "vertex_texture");

	glGenBuffers(1, &VBO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data1.mPointCount * sizeof(vec3), &mesh_data1.mVertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VBO[1]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data1.mPointCount * sizeof(vec3), &mesh_data1.mNormals[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VBO[2]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data1.mPointCount * sizeof(vec2), &mesh_data1.mTextureCoords[0], GL_STATIC_DRAW);


	glGenVertexArrays(1, &VAO[0]);
	glBindVertexArray(VAO[0]);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc3);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
	glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);


	//UFO
	glGenTextures(1, &utexture);
	glBindTexture(GL_TEXTURE_2D, utexture);


	int wU, hU, nrChannelsU;
	unsigned char *UFO = stbi_load("metal.jpg", &wU, &hU, &nrChannelsU, 0);
	if (UFO)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, wU, hU, 0, GL_RGB, GL_UNSIGNED_BYTE, UFO);
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(UFO);
		glUniform1i(glGetUniformLocation(shaderProgramID2, "ourTextureU"), 0);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	loc1 = glGetAttribLocation(shaderProgramID2, "vertex_position");
	loc2 = glGetAttribLocation(shaderProgramID2, "vertex_normal");
	loc3 = glGetAttribLocation(shaderProgramID2, "vertex_texture");

	glGenBuffers(1, &VBO[3]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data2.mPointCount * sizeof(vec3), &mesh_data2.mVertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VBO[4]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[4]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data2.mPointCount * sizeof(vec3), &mesh_data2.mNormals[0], GL_STATIC_DRAW);


	glGenBuffers(1, &VBO[5]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[5]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data2.mPointCount * sizeof(vec2), &mesh_data2.mTextureCoords[0], GL_STATIC_DRAW);


	glGenVertexArrays(1, &VAO[1]);
	glBindVertexArray(VAO[1]);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[4]);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc3);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[5]);
	glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	//UFO beams
	glGenTextures(1, &btexture);
	glBindTexture(GL_TEXTURE_2D, btexture);


	int wB, hB, nrChannelsB;
	unsigned char *beam = stbi_load("beams.jpg", &wB, &hB, &nrChannelsB, 0);
	if (beam)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, wB, hB, 0, GL_RGB, GL_UNSIGNED_BYTE, beam);
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(beam);
		glUniform1i(glGetUniformLocation(shaderProgramID3, "ourTextureB"), 0);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	loc1 = glGetAttribLocation(shaderProgramID4, "vertex_position");
	loc2 = glGetAttribLocation(shaderProgramID4, "vertex_normal");
	loc3 = glGetAttribLocation(shaderProgramID4, "vertex_texture");

	glGenBuffers(1, &VBO[5]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[5]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data4.mPointCount * sizeof(vec3), &mesh_data4.mVertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VBO[6]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[6]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data4.mPointCount * sizeof(vec3), &mesh_data4.mNormals[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VBO[7]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[7]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data4.mPointCount * sizeof(vec2), &mesh_data4.mTextureCoords[0], GL_STATIC_DRAW);


	glGenVertexArrays(1, &VAO[2]);
	glBindVertexArray(VAO[2]);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[5]);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[6]);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc3);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[7]);
	glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	//Grass

	loc1 = glGetAttribLocation(shaderProgramID3, "vertex_position");
	loc2 = glGetAttribLocation(shaderProgramID3, "vertex_normal");
	loc3 = glGetAttribLocation(shaderProgramID3, "vertex_texture");

	glGenBuffers(1, &VBO[8]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[8]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data3.mPointCount * sizeof(vec3), &mesh_data3.mVertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VBO[9]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[9]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data3.mPointCount * sizeof(vec3), &mesh_data3.mNormals[0], GL_STATIC_DRAW);

	glGenBuffers(1, &VBO[10]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[10]);
	glBufferData(GL_ARRAY_BUFFER, mesh_data3.mPointCount * sizeof(vec2), &mesh_data3.mTextureCoords[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO[3]);
	glBindVertexArray(VAO[3]);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[8]);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[9]);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(loc3);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[10]);
	glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);


	//glBindVertexArray(VAO[1]);
	//glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);

	//glBufferData(GL_ARRAY_BUFFER, mesh_data2.mPointCount * sizeof(vec3), &mesh_data2.mVertices[0], GL_STATIC_DRAW);
	//glBufferData(GL_ARRAY_BUFFER, mesh_data2.mPointCount * sizeof(vec3), &mesh_data2.mNormals[0], GL_STATIC_DRAW);

	//loc1 = glGetAttribLocation(shaderProgramID2, "vertex_position");
	//loc2 = glGetAttribLocation(shaderProgramID2, "vertex_normal");
	//loc3 = glGetAttribLocation(shaderProgramID2, "vertex_texture");

	//glEnableVertexAttribArray(loc1);
	//glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	//glEnableVertexAttribArray(loc2);
	//glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	////This is for texture coordinates which you don't currently need, so I have commented it out
	//

	////Grass
	//glBindVertexArray(VAO[2]);
	//glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);

	//glBufferData(GL_ARRAY_BUFFER, mesh_data3.mPointCount * sizeof(vec3), &mesh_data3.mVertices[0], GL_STATIC_DRAW);
	//glBufferData(GL_ARRAY_BUFFER, mesh_data3.mPointCount * sizeof(vec3), &mesh_data3.mNormals[0], GL_STATIC_DRAW);
	//glBufferData(GL_ARRAY_BUFFER, mesh_data3.mPointCount * sizeof(vec2), &mesh_data3.mTextureCoords[0], GL_STATIC_DRAW);

	//loc1 = glGetAttribLocation(shaderProgramID3, "vertex_position");
	//loc2 = glGetAttribLocation(shaderProgramID3, "vertex_normal");
	//loc3 = glGetAttribLocation(shaderProgramID3, "vertex_texture");

	//glEnableVertexAttribArray(loc1);
	//glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	//glEnableVertexAttribArray(loc2);
	//glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	//glEnableVertexAttribArray(loc3);
	//glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	//unsigned int vt_vbo = 0;
	//glGenBuffers(1, &vt_vbo);
	//glBindBuffer(GL_ARRAY_BUFFER, BVO);



	//This is for texture coordinates which you don't currently need, so I have commented it out
	//glEnableVertexAttribArray(loc3);
	//glBindBuffer(GL_ARRAY_BUFFER, vt_vbo);
	//glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);



}
#pragma endregion VBO_FUNCTIONS


void display() {

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.5f, 0.8f, 0.9f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//UFO
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, utexture);
	glBindVertexArray(VAO[1]);
	glUseProgram(shaderProgramID2);


	int matrix_location2 = glGetUniformLocation(shaderProgramID2, "model");
	int view_mat_location2 = glGetUniformLocation(shaderProgramID2, "view");
	int proj_mat_location2 = glGetUniformLocation(shaderProgramID2, "proj");

	mat4 view2 = identity_mat4();
	mat4 persp_proj2 = perspective(120.0f, (float)width / (float)height, 0.1f, 1000.0f);
	mat4 model2 = identity_mat4();


	if (input == 'o') {
		persp_proj2 = orthographic((float)width, (float)height, 0.1f, 1000.0f);
	}
	else if (input == 'p') {
		persp_proj2 = perspective(120.0f, (float)width / (float)height, 0.1f, 1000.0f);
	}

	view2 = look_at(cameraPos, cameraPos + cameraFront, cameraUp);


	if (input == 'x') {
		model2 = rotate_x_deg(model2, rotate_y);

	}
	else if (input == 'y') {
		model2 = rotate_y_deg(model2, rotate_y);

	}
	else if (input == 'z') {
		model2 = rotate_z_deg(model2, rotate_y);
	}

	view2 = translate(view2, vec3(x, y, z));
	model2 = scale(model2, vec3(c, c, c));

	// update uniforms & draw
	glUniformMatrix4fv(proj_mat_location2, 1, GL_FALSE, persp_proj2.m);
	glUniformMatrix4fv(view_mat_location2, 1, GL_FALSE, view2.m);
	glUniformMatrix4fv(matrix_location2, 1, GL_FALSE, model2.m);
	glBindTexture(GL_TEXTURE_2D, utexture);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data2.mPointCount);

	//UFO beams
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, btexture);
	glBindVertexArray(VAO[2]);
	glUseProgram(shaderProgramID4);


	int matrix_location4 = glGetUniformLocation(shaderProgramID4, "model");
	int view_mat_location4 = glGetUniformLocation(shaderProgramID4, "view");
	int proj_mat_location4 = glGetUniformLocation(shaderProgramID4, "proj");

	mat4 view4 = identity_mat4();
	mat4 persp_proj4 = perspective(120.0f, (float)width / (float)height, 0.1f, 1000.0f);
	mat4 model4 = identity_mat4();
	model4 = model4 * model2;


	if (input == 'o') {
		persp_proj4 = orthographic((float)width, (float)height, 0.1f, 1000.0f);
	}
	else if (input == 'p') {
		persp_proj4 = perspective(120.0f, (float)width / (float)height, 0.1f, 1000.0f);
	}

	view4 = look_at(cameraPos, cameraPos + cameraFront, cameraUp);


	if (input == 'x') {
		model4 = rotate_x_deg(model4, rotate_y);

	}
	else if (input == 'y') {
		model4 = rotate_y_deg(model4, rotate_y);

	}
	else if (input == 'z') {
		model4 = rotate_z_deg(model4, rotate_y);
	}

	view4 = translate(view4, vec3(x, y, z));
	model4 = scale(model4, vec3(c, c, c));

	// update uniforms & draw
	glUniformMatrix4fv(proj_mat_location4, 1, GL_FALSE, persp_proj4.m);
	glUniformMatrix4fv(view_mat_location4, 1, GL_FALSE, view4.m);
	glUniformMatrix4fv(matrix_location4, 1, GL_FALSE, model4.m);
	glBindTexture(GL_TEXTURE_2D, btexture);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data4.mPointCount);

	//ALIEN
	glBindVertexArray(VAO[0]);
	glUseProgram(shaderProgramID1);



	//Declare your uniform variables that will be used in your shader
	int matrix_location1 = glGetUniformLocation(shaderProgramID1, "model");
	int view_mat_location1 = glGetUniformLocation(shaderProgramID1, "view");
	int proj_mat_location1 = glGetUniformLocation(shaderProgramID1, "proj");

	mat4 view1 = identity_mat4();
	mat4 persp_proj1 = perspective(120.0f, (float)width / (float)height, 0.1f, 1000.0f);
	mat4 model1 = identity_mat4();
	model1 = model1 * model4;

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


	if (input == 'a') {
		view1 = translate(view1, vec3(x, ypos, z));
	}
	else if (input == 'q') {
		view1 = translate(view1, vec3(x, y, z));
	}
	else {
		view1 = translate(view1, vec3(x, y, z));
	}

	model1 = scale(model1, vec3(c, c, c));

	// update uniforms & draw
	glUniformMatrix4fv(proj_mat_location1, 1, GL_FALSE, persp_proj1.m);
	glUniformMatrix4fv(view_mat_location1, 1, GL_FALSE, view1.m);
	glUniformMatrix4fv(matrix_location1, 1, GL_FALSE, model1.m);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data1.mPointCount);


	//GRASS
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, gtexture);
	glBindVertexArray(VAO[3]);
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glUseProgram(shaderProgramID3);

	int matrix_location3 = glGetUniformLocation(shaderProgramID3, "model");
	int view_mat_location3 = glGetUniformLocation(shaderProgramID3, "view");
	int proj_mat_location3 = glGetUniformLocation(shaderProgramID3, "proj");

	mat4 view3 = identity_mat4();
	mat4 persp_proj3 = perspective(120.0f, (float)width / (float)height, 0.1f, 1000.0f);
	mat4 model3 = identity_mat4();


	if (input == 'o') {
		persp_proj3 = orthographic((float)width, (float)height, 0.1f, 1000.0f);
	}
	else if (input == 'p') {
		persp_proj3 = perspective(120.0f, (float)width / (float)height, 0.1f, 1000.0f);
	}


	view3 = look_at(cameraPos, cameraPos + cameraFront, cameraUp);


	// update uniforms & draw
	glUniformMatrix4fv(proj_mat_location3, 1, GL_FALSE, persp_proj3.m);
	glUniformMatrix4fv(view_mat_location3, 1, GL_FALSE, view3.m);
	glUniformMatrix4fv(matrix_location3, 1, GL_FALSE, model3.m);
	glBindTexture(GL_TEXTURE_2D, gtexture);
	glDrawArrays(GL_TRIANGLES, 0, mesh_data3.mPointCount);

	glDisable(GL_TEXTURE_2D);
	glutSwapBuffers();
	glBindVertexArray(0);
}




void updateScene() {

	if (down = 1) {
		if (ypos > y - 8) {
			ypos -= 0.03;
		}
	}
	else if (up = 1) {
		if (ypos < y + 8) {
			ypos += 0.001;
		}
	}

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

	
	shaderProgramID1 = CompileShaders("alienFragmentShader.txt","alienVertexShader.txt");
	shaderProgramID2 = CompileShaders("simpleFragmentShader.txt", "simpleVertexShader.txt"); //ufo
	shaderProgramID3 = CompileShaders("grassFragmentShader.txt","grassVertexShader.txt"); //grass
	shaderProgramID4 = CompileShaders("beamFragmentShader.txt", "beamVertexShader.txt");

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
	else if (key == 'd') {
		x += 5.0;
	}
	else if (key == 'w') {
		y += 5.0;
	}
	else if (key == 's') {
		y += -5.0;
	}
	else if (key == 'a') {
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
		cameraPos += cross(cameraFront, (cameraSpeed));
	}
	else if (key == 'f') {
		cameraPos -= cross(normalise(cross(cameraFront, cameraUp)), cameraSpeed);
	}
	else if (key == 'g') {
		cameraPos -= cross(cameraFront, (cameraSpeed));
	}
	else if (key == 'h') {
		cameraPos += cross(normalise(cross(cameraFront, cameraUp)), cameraSpeed);
	}
	else if (key == 'e') {
		input = 'a';
		up = 0;
		down = 1;
		ypos = y;
	}
	else if (key == 'q') {
		input = 'q';
		down = 0;
		up = 1;
		ypos = y;
	}
}

void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN)
			lbuttonDown = true;
		else
			lbuttonDown = false;
	}
}

void motion(int xpos, int ypos)
{
	if (lbuttonDown) {
		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos;
		lastX = xpos;
		lastY = ypos;

		float sensitivity = 0.05;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw += xoffset;
		pitch += yoffset;

		if (pitch > 150.0f)
			pitch = 150.0f;
		if (pitch < -150.0f)
			pitch = -150.0f;


		vec3 front = vec3(cos((rad *(yaw)) * cos((rad *(pitch)))), sin(rad * (pitch)), sin(rad * (yaw)) * cos(rad * (pitch)));
		cameraFront = normalise(front);
	}
	else {
		firstMouse = true;
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
	glutMouseFunc(mouse);
	glutMotionFunc(motion);



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