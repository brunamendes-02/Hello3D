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

#include "../Code snippets/LoadSimpleOBJ.cpp"

glm::vec3 position(0.0f, 0.0f, 0.0f);
float scaleFactor = 1.0f;
struct Object3D
{
	GLuint VAO;
	int nVertices;

	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
};

std::vector<Object3D> objects;

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// Protótipos das funções
int setupShader();
int setupGeometry();

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 1000;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar* vertexShaderSource = "#version 450\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 color;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"out vec4 finalColor;\n"
"void main()\n"
"{\n"
"gl_Position = projection * view * model * vec4(position, 1.0);\n"
"finalColor = vec4(color, 1.0);\n"
"}\0";

//Código fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar* fragmentShaderSource = "#version 450\n"
"in vec4 finalColor;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"color = finalColor;\n"
"}\n\0";

bool rotateX=false, rotateY=false, rotateZ=false;
int selectedObject = 0;

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
	Object3D obj1;
	obj1.VAO = loadSimpleOBJ("../assets/Modelos3D/Cube.obj", obj1.nVertices);
	obj1.position = glm::vec3(-1.5f, 0.0f, 0.0f);
	obj1.rotation = glm::vec3(0.0f);
	obj1.scale = glm::vec3(0.3f);

	objects.push_back(obj1);

	Object3D obj2;
	obj2.VAO = loadSimpleOBJ("../assets/Modelos3D/Suzanne.obj", obj2.nVertices);
	obj2.position = glm::vec3(1.5f, 0.0f, 0.0f);
	obj2.rotation = glm::vec3(0.0f);
	obj2.scale = glm::vec3(0.3f);

	objects.push_back(obj2);


	glUseProgram(shaderID);
	GLuint viewLoc = glGetUniformLocation(shaderID, "view");
	GLuint projLoc = glGetUniformLocation(shaderID, "projection");

	glm::mat4 model = glm::mat4(1); //matriz identidade;
	GLint modelLoc = glGetUniformLocation(shaderID, "model");

	glEnable(GL_DEPTH_TEST);

	glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -8.0f));
	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

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

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		for (int i = 0; i < objects.size(); i++)
		{
			Object3D obj = objects[i];

			glm::mat4 model = glm::mat4(1.0f);

			model = glm::translate(model, obj.position);


			model = glm::rotate(
				model,
				glm::radians(obj.rotation.x),
				glm::vec3(1.0f, 0.0f, 0.0f)
			);

			model = glm::rotate(
				model,
				glm::radians(obj.rotation.y),
				glm::vec3(0.0f, 1.0f, 0.0f)
			);

			model = glm::rotate(
				model,
				glm::radians(obj.rotation.z),
				glm::vec3(0.0f, 0.0f, 1.0f)
			);


			model = glm::scale(model, obj.scale);

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

			glBindVertexArray(obj.VAO);

			glDrawArrays(GL_TRIANGLES, 0, obj.nVertices);
		}

		glBindVertexArray(0);

		glfwSwapBuffers(window);
	}
	// Finaliza a execução da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
	return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
	{
		selectedObject++;

		if (selectedObject >= objects.size())
			selectedObject = 0;
	}

	float step = 0.1f;


	if (key == GLFW_KEY_W && action == GLFW_PRESS)
		objects[selectedObject].position.z -= step;

	if (key == GLFW_KEY_S && action == GLFW_PRESS)
		objects[selectedObject].position.z += step;

	if (key == GLFW_KEY_A && action == GLFW_PRESS)
		objects[selectedObject].position.x -= step;

	if (key == GLFW_KEY_D && action == GLFW_PRESS)
		objects[selectedObject].position.x += step;

	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
		objects[selectedObject].position.y += step;

	if (key == GLFW_KEY_E && action == GLFW_PRESS)
		objects[selectedObject].position.y -= step;


	if (key == GLFW_KEY_R && action == GLFW_PRESS)
		objects[selectedObject].rotation.y += 10.0f;

	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
		objects[selectedObject].scale += glm::vec3(0.1f);
	if (key == GLFW_KEY_X && action == GLFW_PRESS)
		objects[selectedObject].scale -= glm::vec3(0.1f);
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

