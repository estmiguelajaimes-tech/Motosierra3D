
#include "shaderClass.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <string>

// Lee un archivo y devuelve todo su contenido como texto
std::string get_file_contents(const char* filename)
{
    std::ifstream in(filename, std::ios::binary);

    // Comprueba si el archivo se pudo abrir
    if (!in.is_open())
    {
        std::cerr << "ERROR: No se pudo abrir el archivo:\n"
            << filename << std::endl;

        throw std::runtime_error(
            std::string("No se pudo abrir el archivo: ") + filename
        );
    }

    // Lee todo el contenido del archivo
    std::stringstream contents;
    contents << in.rdbuf();

    // Devuelve el contenido como una cadena de texto
    return contents.str();
}

// Constructor que crea el programa Shader usando dos shaders
Shader::Shader(const char* vertexFile, const char* fragmentFile)
{
    // Lee los archivos del Vertex Shader y Fragment Shader
    std::string vertexCode = get_file_contents(vertexFile);
    std::string fragmentCode = get_file_contents(fragmentFile);

    // Convierte el codigo de los shaders en arreglos de caracteres
    const char* vertexSource = vertexCode.c_str();
    const char* fragmentSource = fragmentCode.c_str();

    // Crea el objeto Vertex Shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

    // Asigna el codigo fuente al Vertex Shader
    glShaderSource(vertexShader, 1, &vertexSource, NULL);

    // Compila el Vertex Shader
    glCompileShader(vertexShader);

    // Comprueba si el Vertex Shader se compilo correctamente
    compileErrors(vertexShader, "VERTEX");

    // Crea el objeto Fragment Shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    // Asigna el codigo fuente al Fragment Shader
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);

    // Compila el Fragment Shader
    glCompileShader(fragmentShader);

    // Comprueba si el Fragment Shader se compilo correctamente
    compileErrors(fragmentShader, "FRAGMENT");

    // Crea el programa Shader
    ID = glCreateProgram();

    // Une los dos shaders al programa
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragmentShader);

    // Enlaza los shaders
    glLinkProgram(ID);

    // Comprueba si el programa se enlazo correctamente
    compileErrors(ID, "PROGRAM");

    // Elimina los objetos de shaders que ya no se necesitan
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

// Activa el programa Shader
void Shader::Activate()
{
    glUseProgram(ID);
}

// Elimina el programa Shader
void Shader::Delete()
{
    glDeleteProgram(ID);
}

// Comprueba si los shaders se compilaron y enlazaron correctamente
void Shader::compileErrors(unsigned int shader, const char* type)
{
    // Guarda el resultado de la compilacion
    GLint hasCompiled = GL_FALSE;

    // Espacio para almacenar el mensaje de error
    char infoLog[1024] = {};

    // Comprueba si se trata del programa completo
    if (std::string(type) != "PROGRAM")
    {
        // Consulta el estado de compilacion del shader
        glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);

        // Si fallo, muestra el mensaje de error
        if (hasCompiled == GL_FALSE)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);

            std::cerr << "SHADER_COMPILATION_ERROR para: "
                << type << "\n"
                << infoLog << std::endl;
        }
    }
    else
    {
        // Consulta el estado de enlace del programa
        glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);

        // Si fallo, muestra el mensaje de error
        if (hasCompiled == GL_FALSE)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);

            std::cerr << "SHADER_LINKING_ERROR para: "
                << type << "\n"
                << infoLog << std::endl;
        }
    }
}
