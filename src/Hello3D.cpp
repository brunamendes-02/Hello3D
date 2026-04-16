/* Hello Triangle - código adaptado de https://learnopengl.com/#!Getting-started/Hello-Triangle
 *
 * Adaptado por Rossana Baptista Queiroz
 * para as disciplinas de Processamento Gráfico/Computação Gráfica - Unisinos
 * Versão inicial: 7/4/2017
 * Última atualização em 07/03/2025
 */

#include <iostream>
#include <string>
#include <assert.h>
#include <vector>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Camera.h"

Camera camera;

float lastX = 800 / 2.0f;
float lastY = 600 / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::vec3 position(0.0f, 0.0f, 0.0f);
float scaleFactor = 0.5f;

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
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

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
int loadSimpleOBJ(std::string filePATH, int &nVertices);

// Protótipos das funções
int setupShader();
int setupGeometry();

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 1000;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar* vertexShaderSource = "#version 450\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 color;\n"
"layout (location = 2) in vec2 texCoord;\n"
"layout (location = 3) in vec3 normal;\n"
"\n"
"out vec2 TexCoord;\n"
"out vec3 FragPos;\n"
"out vec3 Normal;\n"
"\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"\n"
"void main()\n"
"{\n"
"FragPos = vec3(model * vec4(position, 1.0));\n"
"Normal = mat3(transpose(inverse(model))) * normal;\n"
"TexCoord = texCoord;\n"
"\n"
"gl_Position = projection * view * model * vec4(position, 1.0);\n"
"}\0";

//Código fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar* fragmentShaderSource = "#version 450\n"
"in vec2 TexCoord;\n"
"in vec3 FragPos;\n"
"in vec3 Normal;\n"
"\n"
"out vec4 color;\n"
"\n"
"uniform sampler2D tex_buffer;\n"
"\n"
"uniform vec3 lightPos;\n"
"uniform vec3 viewPos;\n"
"uniform vec3 lightColor;\n"
"\n"
"uniform vec3 ka;\n"
"uniform vec3 kd;\n"
"uniform vec3 ks;\n"
"uniform float shininess;\n"
"\n"
"void main()\n"
"{\n"
"vec3 texColor = texture(tex_buffer, TexCoord).rgb;\n"
"\n"
"vec3 ambient = ka * texColor;\n"
"\n"
"vec3 norm = normalize(Normal);\n"
"vec3 lightDir = normalize(lightPos - FragPos);\n"
"float diff = max(dot(norm, lightDir), 0.0);\n"
"vec3 diffuse = kd * diff * texColor;\n"
"\n"
"vec3 viewDir = normalize(viewPos - FragPos);\n"
"vec3 reflectDir = reflect(-lightDir, norm);\n"
"float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);\n"
"vec3 specular = ks * spec * lightColor;\n"
"\n"
"vec3 result = ambient + diffuse + specular;\n"
"color = vec4(result, 1.0);\n"
"}\n\0";

bool rotateX=false, rotateY=false, rotateZ=false;

GLuint loadTexture(const std::string& path)
{
	GLuint texID;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	// parâmetros
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

	if (data)
	{
		GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Erro ao carregar textura: " << path << std::endl;
	}

	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);

	return texID;
}

// Função MAIN
int main()
{
	glfwInit();

	// Criação da janela GLFW
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Ola 3D -- Bruna!", nullptr, nullptr);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	glfwMakeContextCurrent(window);

	// Fazendo o registro da função de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// GLAD: carrega todos os ponteiros d funções da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;

	}

	// Obtendo as informações de versão
	const GLubyte* renderer = glGetString(GL_RENDERER); /* get renderer string */
	const GLubyte* version = glGetString(GL_VERSION); /* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	// Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);


	// Compilando e buildando o programa de shader
	GLuint shaderID = setupShader();

	// Gerando um buffer simples, com a geometria de um triângulo
	// GLuint VAO = setupGeometry();
	int nVertices;
	GLuint VAO = loadSimpleOBJ("../assets/Modelos3D/Cube.obj", nVertices);
	std::vector<glm::vec3> cubePositions = {
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(1.5f, 0.0f, 0.0f),
		glm::vec3(-1.5f, 0.0f, 0.0f)
	};


	glUseProgram(shaderID);
	glUniform3f(glGetUniformLocation(shaderID, "lightPos"), 0.0f, 0.0f, 3.0f);
	glUniform3f(glGetUniformLocation(shaderID, "viewPos"), 0.0f, 0.0f, 3.0f);
	glUniform3f(glGetUniformLocation(shaderID, "lightColor"), 1.0f, 1.0f, 1.0f);
	glUniform3f(glGetUniformLocation(shaderID, "ka"), 0.2f, 0.2f, 0.2f);
	glUniform3f(glGetUniformLocation(shaderID, "kd"), 0.8f, 0.8f, 0.8f);
	glUniform3f(glGetUniformLocation(shaderID, "ks"), 1.0f, 1.0f, 1.0f);
	glUniform1f(glGetUniformLocation(shaderID, "shininess"), 32.0f);

	GLuint texID = loadTexture("../assets/tex/pixelWall.png");
	glUniform1i(glGetUniformLocation(shaderID, "tex_buffer"), 0);
	GLuint viewLoc = glGetUniformLocation(shaderID, "view");
	GLuint projLoc = glGetUniformLocation(shaderID, "projection");

	glm::mat4 model = glm::mat4(1); //matriz identidade;
	GLint modelLoc = glGetUniformLocation(shaderID, "model");

	glEnable(GL_DEPTH_TEST);

	glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
	// Loop da aplicação - "game loop"
	
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		glfwPollEvents();

		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		glm::mat4 projection = glm::perspective(
			glm::radians(camera.Zoom),
			(float)width / (float)height,
			0.1f,
			100.0f
		);
		glm::mat4 view = camera.GetViewMatrix();
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float angle = (GLfloat)glfwGetTime();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texID);

		glBindVertexArray(VAO);

		for (auto pos : cubePositions)
		{
			glm::mat4 model = glm::mat4(1);

			model = glm::translate(model, pos + position);

			// Rotação
			if (rotateX)
				model = glm::rotate(model, angle, glm::vec3(1,0,0));
			else if (rotateY)
				model = glm::rotate(model, angle, glm::vec3(0,1,0));
			else if (rotateZ)
				model = glm::rotate(model, angle, glm::vec3(0,0,1));

			// Escala
			model = glm::scale(model, glm::vec3(scaleFactor));

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

			glDrawArrays(GL_TRIANGLES, 0, nVertices);
		}

		glBindVertexArray(0);

		glfwSwapBuffers(window);
	}
	

	// Pede pra OpenGL desalocar os buffers
	glDeleteVertexArrays(1, &VAO);
	// Finaliza a execução da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
	return 0;
}

// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada
// ou solta via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_X && action == GLFW_PRESS)
	{
		rotateX = true;
		rotateY = false;
		rotateZ = false;
	}

	if (key == GLFW_KEY_Y && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = true;
		rotateZ = false;
	}

	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = false;
		rotateZ = true;
	}

	float step = 0.1f;

	if (key == GLFW_KEY_W && action == GLFW_PRESS) position.z -= step;
	if (key == GLFW_KEY_S && action == GLFW_PRESS) position.z += step;
	if (key == GLFW_KEY_A && action == GLFW_PRESS) position.x -= step;
	if (key == GLFW_KEY_D && action == GLFW_PRESS) position.x += step;

	if (key == GLFW_KEY_I && action == GLFW_PRESS) position.y += step;
	if (key == GLFW_KEY_J && action == GLFW_PRESS) position.y -= step;

	if (key == GLFW_KEY_LEFT_BRACKET && action == GLFW_PRESS) scaleFactor -= 0.1f;
	if (key == GLFW_KEY_RIGHT_BRACKET && action == GLFW_PRESS) scaleFactor += 0.1f;

	if (scaleFactor < 0.1f) scaleFactor = 0.1f;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

//Esta função está bastante hardcoded - objetivo é compilar e "buildar" um programa de
// shader simples e único neste exemplo de código
// O código fonte do vertex e fragment shader está nos arrays vertexShaderSource e
// fragmentShader source no inicio deste arquivo
// A função retorna o identificador do programa de shader
int setupShader()
{
	// Vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// Checando erros de compilação (exibição via log no terminal)
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// Fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// Checando erros de compilação (exibição via log no terminal)
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// Linkando os shaders e criando o identificador do programa de shader
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// Checando por erros de linkagem
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

// Esta função está bastante harcoded - objetivo é criar os buffers que armazenam a 
// geometria de um triângulo
// Apenas atributo coordenada nos vértices
// 1 VBO com as coordenadas, VAO com apenas 1 ponteiro para atributo
// A função retorna o identificador do VAO
int setupGeometry()
{
	// Aqui setamos as coordenadas x, y e z do triângulo e as armazenamos de forma
	// sequencial, já visando mandar para o VBO (Vertex Buffer Objects)
	// Cada atributo do vértice (coordenada, cores, coordenadas de textura, normal, etc)
	// Pode ser arazenado em um VBO único ou em VBOs separados
GLfloat vertices[] = {
	-0.5, -0.5, -0.5, 1, 0, 0,
	0.5, -0.5, -0.5, 1, 0, 0,
	0.5,  0.5, -0.5, 1, 0, 0,

	0.5,  0.5, -0.5, 1, 0, 0,
	-0.5,  0.5, -0.5, 1, 0, 0,
	-0.5, -0.5, -0.5, 1, 0, 0,

	-0.5, -0.5,  0.5, 0, 1, 0,
	0.5, -0.5,  0.5, 0, 1, 0,
	0.5,  0.5,  0.5, 0, 1, 0,

	0.5,  0.5,  0.5, 0, 1, 0,
	-0.5,  0.5,  0.5, 0, 1, 0,
	-0.5, -0.5,  0.5, 0, 1, 0,

	-0.5,  0.5,  0.5, 0, 0, 1,
	-0.5,  0.5, -0.5, 0, 0, 1,
	-0.5, -0.5, -0.5, 0, 0, 1,

	-0.5, -0.5, -0.5, 0, 0, 1,
	-0.5, -0.5,  0.5, 0, 0, 1,
	-0.5,  0.5,  0.5, 0, 0, 1,

	0.5,  0.5,  0.5, 1, 1, 0,
	0.5,  0.5, -0.5, 1, 1, 0,
	0.5, -0.5, -0.5, 1, 1, 0,

	0.5, -0.5, -0.5, 1, 1, 0,
	0.5, -0.5,  0.5, 1, 1, 0,
	0.5,  0.5,  0.5, 1, 1, 0,

	-0.5, -0.5, -0.5, 1, 0, 1,
	0.5, -0.5, -0.5, 1, 0, 1,
	0.5, -0.5,  0.5, 1, 0, 1,

	0.5, -0.5,  0.5, 1, 0, 1,
	-0.5, -0.5,  0.5, 1, 0, 1,
	-0.5, -0.5, -0.5, 1, 0, 1,

	-0.5,  0.5, -0.5, 1, 0.5, 0,
	0.5,  0.5, -0.5, 1, 0.5, 0,
	0.5,  0.5,  0.5, 1, 0.5, 0,

	0.5,  0.5,  0.5, 1, 0.5, 0,
	-0.5,  0.5,  0.5, 1, 0.5, 0,
	-0.5,  0.5, -0.5, 1, 0.5, 0,
	};
	GLuint VBO, VAO;

	//Geração do identificador do VBO
	glGenBuffers(1, &VBO);

	//Faz a conexão (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//Envia os dados do array de floats para o buffer da OpenGl
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//Geração do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);

	// Vincula (bind) o VAO primeiro, e em seguida conecta e seta o(s) buffer(s) de vértices
	// e os ponteiros para os atributos 
	glBindVertexArray(VAO);
	
	//Para cada atributo do vertice, criamos um "AttribPointer" (ponteiro para o atributo), indicando: 
	// Localização no shader * (a localização dos atributos devem ser correspondentes no layout especificado no vertex shader)
	// Numero de valores que o atributo tem (por ex, 3 coordenadas xyz) 
	// Tipo do dado
	// Se está normalizado (entre zero e um)
	// Tamanho em bytes 
	// Deslocamento a partir do byte zero 
	
	//Atributo posição (x, y, z)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	//Atributo cor (r, g, b)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
	glEnableVertexAttribArray(1);


	// Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice 
	// atualmente vinculado - para que depois possamos desvincular com segurança
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
	glBindVertexArray(0);

	return VAO;
}

