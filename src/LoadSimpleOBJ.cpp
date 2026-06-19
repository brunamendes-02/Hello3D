#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// OpenGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Estrutura para armazenar uma malha
struct Mesh
{
    GLuint VAO;
};

// Carrega um arquivo OBJ e retorna o VAO criado
int loadSimpleOBJ(string filePATH, int &nVertices)
{
    // Armazena os dados lidos do arquivo OBJ
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;

    // Buffer final enviado para a GPU
    std::vector<GLfloat> vBuffer;

    // Cor padrão caso o modelo não utilize textura
    glm::vec3 color = glm::vec3(1.0, 0.0, 0.0);

    // Abre o arquivo OBJ
    std::ifstream arqEntrada(filePATH.c_str());

    if (!arqEntrada.is_open())
    {
        std::cerr << "Erro ao tentar ler o arquivo " << filePATH << std::endl;
        return -1;
    }

    std::string line;

    // Percorre todas as linhas do arquivo
    while (std::getline(arqEntrada, line))
    {
        std::istringstream ssline(line);
        std::string word;

        ssline >> word;

        // Lê vértices (v)
        if (word == "v")
        {
            glm::vec3 vertice;

            ssline >> vertice.x >> vertice.y >> vertice.z;

            vertices.push_back(vertice);
        }

        // Lê coordenadas de textura (vt)
        else if (word == "vt")
        {
            glm::vec2 vt;

            ssline >> vt.s >> vt.t;

            texCoords.push_back(vt);
        }

        // Lê vetores normais (vn)
        else if (word == "vn")
        {
            glm::vec3 normal;

            ssline >> normal.x >> normal.y >> normal.z;

            normals.push_back(normal);
        }

        // Lê faces (f)
        else if (word == "f")
        {
            while (ssline >> word)
            {
                int vi = 0;
                int ti = 0;
                int ni = 0;

                std::istringstream ss(word);
                std::string index;

                // Índice do vértice
                if (std::getline(ss, index, '/'))
                    vi = !index.empty() ? std::stoi(index) - 1 : 0;

                // Índice da textura
                if (std::getline(ss, index, '/'))
                    ti = !index.empty() ? std::stoi(index) - 1 : 0;

                // Índice da normal
                if (std::getline(ss, index))
                    ni = !index.empty() ? std::stoi(index) - 1 : 0;

                // -------------------------
                // POSIÇÃO
                // -------------------------
                vBuffer.push_back(vertices[vi].x);
                vBuffer.push_back(vertices[vi].y);
                vBuffer.push_back(vertices[vi].z);

                // -------------------------
                // COR
                // -------------------------
                vBuffer.push_back(color.r);
                vBuffer.push_back(color.g);
                vBuffer.push_back(color.b);

                // -------------------------
                // UV (TEXTURA)
                // -------------------------
                if (ti >= 0 && ti < texCoords.size())
                {
                    vBuffer.push_back(texCoords[ti].x);
                    vBuffer.push_back(texCoords[ti].y);
                }
                else
                {
                    vBuffer.push_back(0.0f);
                    vBuffer.push_back(0.0f);
                }

                // -------------------------
                // NORMAL
                // -------------------------
                if (ni >= 0 && ni < normals.size())
                {
                    vBuffer.push_back(normals[ni].x);
                    vBuffer.push_back(normals[ni].y);
                    vBuffer.push_back(normals[ni].z);
                }
                else
                {
                    vBuffer.push_back(0.0f);
                    vBuffer.push_back(0.0f);
                    vBuffer.push_back(0.0f);
                }
            }
        }
    }

    arqEntrada.close();

    std::cout << "Gerando o buffer de geometria..." << std::endl;

    GLuint VBO;
    GLuint VAO;

    // Cria o VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        vBuffer.size() * sizeof(GLfloat),
        vBuffer.data(),
        GL_STATIC_DRAW
    );

    // Cria o VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Layout 0 -> posição
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        11 * sizeof(GLfloat),
        (GLvoid*)0
    );
    glEnableVertexAttribArray(0);

    // Layout 1 -> cor
    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        11 * sizeof(GLfloat),
        (GLvoid*)(3 * sizeof(GLfloat))
    );
    glEnableVertexAttribArray(1);

    // Layout 2 -> coordenadas UV
    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        11 * sizeof(GLfloat),
        (GLvoid*)(6 * sizeof(GLfloat))
    );
    glEnableVertexAttribArray(2);

    // Layout 3 -> vetor normal
    glVertexAttribPointer(
        3,
        3,
        GL_FLOAT,
        GL_FALSE,
        11 * sizeof(GLfloat),
        (GLvoid*)(8 * sizeof(GLfloat))
    );
    glEnableVertexAttribArray(3);

    // Libera os binds
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Cada vértice possui 11 atributos:
    // 3 posição + 3 cor + 2 UV + 3 normal
    nVertices = vBuffer.size() / 11;

    // Retorna o VAO pronto para renderização
    return VAO;
}