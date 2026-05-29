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


glm::vec3 position(0.0f, 0.0f, 0.0f);
float scaleFactor = 0.5f;

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

const GLchar* fragmentShaderSource = "#version 450\n"
"in vec2 TexCoord;\n"
"in vec3 FragPos;\n"
"in vec3 Normal;\n"
"\n"
"out vec4 color;\n"
"\n"
"uniform sampler2D tex_buffer;\n"
"\n"
"uniform vec3 keyLightPos;\n"
"uniform vec3 fillLightPos;\n"
"uniform vec3 backLightPos;\n"
"\n"
"uniform vec3 keyIntensity;\n"
"uniform vec3 fillIntensity;\n"
"uniform vec3 backIntensity;\n"
"\n"
"uniform bool keyLightEnabled;\n"
"uniform bool fillLightEnabled;\n"
"uniform bool backLightEnabled;\n"
"\n"
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
"\n"
"vec3 diffuse = vec3(0.0);\n"
"\n"
"if(keyLightEnabled)\n"
"{\n"
"vec3 lightDir = normalize(keyLightPos - FragPos);\n"
"float distance = length(keyLightPos - FragPos);\n"
"float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);\n"
"float diff = max(dot(norm, lightDir), 0.0);\n"
"diffuse += kd * diff * texColor * attenuation * keyIntensity;\n"
"}\n"
"\n"
"if(fillLightEnabled)\n"
"{\n"
"vec3 lightDir = normalize(fillLightPos - FragPos);\n"
"float distance = length(fillLightPos - FragPos);\n"
"float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);\n"
"float diff = max(dot(norm, lightDir), 0.0);\n"
"diffuse += kd * diff * texColor * attenuation * fillIntensity;\n"
"}\n"
"\n"
"if(backLightEnabled)\n"
"{\n"
"vec3 lightDir = normalize(backLightPos - FragPos);\n"
"float distance = length(backLightPos - FragPos);\n"
"float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);\n"
"float diff = max(dot(norm, lightDir), 0.0);\n"
"diffuse += kd * diff * texColor * attenuation * backIntensity;\n"
"}\n"
"\n"
"vec3 viewDir = normalize(viewPos - FragPos);\n"
"vec3 reflectDir = reflect(-normalize(keyLightPos - FragPos), norm);\n"
"float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);\n"
"vec3 specular = ks * spec * lightColor;\n"
"\n"
"vec3 result = ambient + diffuse + specular;\n"
"color = vec4(result, 1.0);\n"
"}\n\0";

bool rotateX=false, rotateY=false, rotateZ=false;
bool keyLightEnabled = true;
bool fillLightEnabled = true;
bool backLightEnabled = true;

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
	// Inicialização da GLFW
	glfwInit();

	//Muita atenção aqui: alguns ambientes não aceitam essas configurações
	//Você deve adaptar para a versão do OpenGL suportada por sua placa
	//Sugestão: comente essas linhas de código para desobrir a versão e
	//depois atualize (por exemplo: 4.5 com 4 e 5)
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Essencial para computadores da Apple
//#ifdef __APPLE__
//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//#endif

	// Criação da janela GLFW
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Ola 3D -- Bruna!", nullptr, nullptr);
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

	// Luz principal
	glUniform3f(glGetUniformLocation(shaderID, "keyLightPos"), 3.0f, 2.0f, 3.0f);
	glUniform3f(glGetUniformLocation(shaderID, "keyIntensity"), 1.0f, 1.0f, 1.0f);

	// Luz de preenchimento
	glUniform3f(glGetUniformLocation(shaderID, "fillLightPos"), -3.0f, 1.0f, 2.0f);
	glUniform3f(glGetUniformLocation(shaderID, "fillIntensity"), 0.4f, 0.4f, 0.4f);

	// Luz de fundo
	glUniform3f(glGetUniformLocation(shaderID, "backLightPos"), 0.0f, 2.0f, -3.0f);
	glUniform3f(glGetUniformLocation(shaderID, "backIntensity"), 5.0f, 5.0f, 5.0f);

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
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		glm::mat4 projection = glm::perspective(
			glm::radians(60.0f),
			(float)width / (float)height,
			0.1f,
			100.0f
		);
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glfwPollEvents();
		glUniform1i(
			glGetUniformLocation(shaderID, "keyLightEnabled"),
			keyLightEnabled
		);

		glUniform1i(
			glGetUniformLocation(shaderID, "fillLightEnabled"),
			fillLightEnabled
		);

		glUniform1i(
			glGetUniformLocation(shaderID, "backLightEnabled"),
			backLightEnabled
		);

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

	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
		fillLightEnabled = !fillLightEnabled;

	if (key == GLFW_KEY_3 && action == GLFW_PRESS)
		backLightEnabled = !backLightEnabled;

	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
	{
		keyLightEnabled = !keyLightEnabled;
		std::cout << "Key Light: "
				<< (keyLightEnabled ? "ON" : "OFF")
				<< std::endl;
	}
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

