#include <filesystem>                 // Permite buscar archivos y carpetas.
#include <iostream>                   // Permite mostrar mensajes en consola.
#include <memory>                     // Permite utilizar punteros inteligentes.
#include <exception>                  // Permite controlar errores al cargar el modelo.

#include <glad/glad.h>                // Permite utilizar las funciones de OpenGL.
#include <GLFW/glfw3.h>               // Permite crear la ventana y controlar el teclado.

#include <glm/glm.hpp>                // Permite trabajar con vectores y matrices.
#include <glm/gtc/matrix_transform.hpp> // Permite crear matrices de transformacion.
#include <glm/gtc/type_ptr.hpp>       // Permite enviar matrices a los shaders.

#include "Model.h"                    // Incluye las clases necesarias para cargar el modelo 3D.

namespace fs = std::filesystem;        // Permite escribir fs en lugar de std::filesystem.


// -----------------------------------------
// DIMENSIONES DE LA VENTANA
// -----------------------------------------

const unsigned int width = 800;       // Define el ancho de la ventana.
const unsigned int height = 800;      // Define el alto de la ventana.

glm::vec3 cubicBezier(
    const glm::vec3& p0,
    const glm::vec3& p1,
    const glm::vec3& p2,
    const glm::vec3& p3,
    float t)
{
    const float inverseT = 1.0f - t;

    return inverseT * inverseT * inverseT * p0 +
        3.0f * inverseT * inverseT * t * p1 +
        3.0f * inverseT * t * t * p2 +
        t * t * t * p3;
}


// -----------------------------------------
// FUNCION PRINCIPAL
// -----------------------------------------

int main()
{
    glfwInit();                       // Inicializa GLFW para poder crear la ventana.

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Establece OpenGL version 3.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Establece OpenGL version 3.3.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Activa el perfil CORE.

    // Crea la ventana con las dimensiones y el titulo indicados.
    GLFWwindow* window = glfwCreateWindow(
        width, height, "YoutubeOpenGL", NULL, NULL);

    // Comprueba si la ventana se creo correctamente.
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;

        glfwTerminate();              // Cierra GLFW.
        return -1;                    // Finaliza el programa por error.
    }

    glfwMakeContextCurrent(window);   // Activa la ventana para trabajar con OpenGL.

    // Carga las funciones de OpenGL mediante GLAD.
    if (!gladLoadGL())
    {
        std::cerr << "Error al inicializar GLAD." << std::endl;

        glfwDestroyWindow(window);    // Destruye la ventana.
        glfwTerminate();              // Cierra GLFW.
        return -1;                    // Finaliza el programa.
    }

    glViewport(0, 0, width, height);  // Define el area donde se dibujara la escena.


    // -----------------------------------------
    // BUSQUEDA DE ARCHIVOS
    // -----------------------------------------

    // Esta funcion busca un archivo dentro de la carpeta del proyecto
    // y tambien revisa las carpetas superiores.
    auto findResource = [](const fs::path& relativePath) -> fs::path
        {
            fs::path directory = fs::current_path(); // Obtiene la carpeta actual.

            while (true)
            {
                // Construye la ruta donde podria encontrarse el archivo.
                fs::path candidate = directory / relativePath;

                // Comprueba si el archivo existe.
                if (fs::exists(candidate) && fs::is_regular_file(candidate))
                {
                    return fs::absolute(candidate); // Devuelve la ruta completa.
                }

                // Obtiene la carpeta superior.
                fs::path parent = directory.parent_path();

                // Termina si ya no existen mas carpetas superiores.
                if (parent == directory)
                    break;

                directory = parent;          // Continua buscando desde la carpeta superior.
            }

            return {};                        // Devuelve una ruta vacia si no encuentra el archivo.
        };


    // -----------------------------------------
    // CARGA DE LOS SHADERS PRINCIPALES
    // -----------------------------------------

    // Busca el Vertex Shader y el Fragment Shader.
    fs::path vertexPath = findResource("default.vert");
    fs::path fragmentPath = findResource("default.frag");

    // Comprueba que los dos shaders existan.
    if (vertexPath.empty() || fragmentPath.empty())
    {
        std::cerr << "No se encontraron los shaders principales."
            << std::endl;

        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // Crea el programa de shaders que controla la iluminacion del modelo.
    Shader shaderProgram(
        vertexPath.string().c_str(),
        fragmentPath.string().c_str());


    // -----------------------------------------
    // CARGA DE LOS SHADERS DE SOMBRAS
    // -----------------------------------------

    // Busca los archivos encargados de calcular las sombras.
    fs::path shadowVertexPath = findResource("shadow.vert");
    fs::path shadowFragmentPath = findResource("shadow.frag");

    // Comprueba que los archivos existan.
    if (shadowVertexPath.empty() || shadowFragmentPath.empty())
    {
        std::cerr << "No se encontraron los shaders de sombras."
            << std::endl;

        shaderProgram.Delete();

        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // Crea el programa de shaders que calcula las sombras.
    Shader shadowShader(
        shadowVertexPath.string().c_str(),
        shadowFragmentPath.string().c_str());


    // -----------------------------------------
    // CONFIGURACION DE LA ILUMINACION
    // -----------------------------------------

    glm::vec4 lightColor(1.0f, 1.0f, 1.0f, 1.0f); // Define el color blanco de la luz.

    glm::vec3 lightPos(0.5f, 0.5f, 0.5f); // Define la posicion de la luz.

    shaderProgram.Activate();           // Activa el shader principal.

    // Envia el color de la luz al Fragment Shader.
    glUniform4f(
        glGetUniformLocation(shaderProgram.ID, "lightColor"),
        lightColor.x,
        lightColor.y,
        lightColor.z,
        lightColor.w
    );

    // Envia la posicion de la luz al Fragment Shader.
    glUniform3f(
        glGetUniformLocation(shaderProgram.ID, "lightPos"),
        lightPos.x,
        lightPos.y,
        lightPos.z
    );

    glEnable(GL_DEPTH_TEST);            // Activa la prueba de profundidad para mostrar correctamente los objetos 3D.


    // -----------------------------------------
    // CONFIGURACION DE LA CAMARA
    // -----------------------------------------

    // Define la posicion inicial de la camara.
    Camera camera(width, height, glm::vec3(0.0f, 4.0f, 12.0f));

    // Define laDireccion hacia donde mira la camara.
    camera.Orientation = glm::normalize(
        glm::vec3(0.0f, -0.20f, -1.0f));


    // -----------------------------------------
    // BUSQUEDA DEL MODELO 3D
    // -----------------------------------------

    // Busca el modelo de la motosierra en la carpeta GLTM.
    fs::path modelPath = findResource(
        fs::path("Resources") / "MiModelo" / "GLTM" /
        "Animated Chainsaw  Low-Poly Version_2.8x.gltf");

    // Si no encuentra el modelo, intenta buscarlo en la carpeta GLTF.
    if (modelPath.empty())
    {
        modelPath = findResource(
            fs::path("Resources") / "MiModelo" / "GLTF" /
            "Animated Chainsaw  Low-Poly Version_2.8x.gltf");
    }


    // -----------------------------------------
    // CONFIGURACION DEL MAPA DE SOMBRAS
    // -----------------------------------------

    const unsigned int shadowWidth = 2048;  // Define el ancho del mapa de sombras.
    const unsigned int shadowHeight = 2048; // Define el alto del mapa de sombras.

    GLuint shadowFramebuffer = 0;           // Almacena el identificador del framebuffer.
    GLuint shadowTexture = 0;               // Almacena la textura de profundidad.

    glGenFramebuffers(1, &shadowFramebuffer); // Crea el framebuffer para las sombras.
    glGenTextures(1, &shadowTexture);         // Crea la textura donde se guardan las profundidades.

    glBindTexture(GL_TEXTURE_2D, shadowTexture); // Activa la textura de sombras.

    // Reserva espacio para guardar la profundidad de los objetos.
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_DEPTH_COMPONENT,
        shadowWidth,
        shadowHeight,
        0,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        nullptr
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // Define el filtro al reducir la textura.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // Define el filtro al ampliar la textura.

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); // Evita repetir la textura horizontalmente.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER); // Evita repetir la textura verticalmente.

    // Define el color que se utilizara en los bordes de la textura.
    const float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glTexParameterfv(
        GL_TEXTURE_2D,
        GL_TEXTURE_BORDER_COLOR,
        borderColor
    );

    glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer); // Activa el framebuffer de sombras.

    // Conecta la textura de profundidad al framebuffer.
    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D,
        shadowTexture,
        0
    );

    glDrawBuffer(GL_NONE);                // Evita dibujar colores en este framebuffer.
    glReadBuffer(GL_NONE);                // Evita leer colores de este framebuffer.

    // Comprueba que el framebuffer se haya creado correctamente.
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Error: el framebuffer de sombras no esta completo."
            << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Regresa al framebuffer principal.


    // -----------------------------------------
    // POSICION Y DIRECCION DE LA LUZ PARA SOMBRAS
    // -----------------------------------------

    const glm::vec3 lightPosition(0.0f, 10.0f, 4.0f); // Posicion desde donde se calculan las sombras.

    const glm::vec3 lightTarget(0.0f, 1.5f, 0.0f); // Punto hacia donde apunta la luz.

    // Calcula la direccion en la que apunta la luz.
    const glm::vec3 directionalLightDirection =
        glm::normalize(lightTarget - lightPosition);

    // Crea la matriz de vista desde la posicion de la luz.
    const glm::mat4 lightView = glm::lookAt(
        lightPosition,
        lightTarget,
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    // Crea la matriz de proyeccion de la luz.
    const glm::mat4 lightProjection =
        glm::ortho(-14.0f, 14.0f, -10.0f, 14.0f, 1.0f, 35.0f);

    // Combina la vista y la proyeccion para calcular las sombras.
    const glm::mat4 lightSpaceMatrix = lightProjection * lightView;


    // -----------------------------------------
    // COMPROBAR Y CARGAR EL MODELO
    // -----------------------------------------

    // Comprueba que el modelo se haya encontrado.
    if (modelPath.empty())
    {
        std::cerr << "No se encontro el modelo GLTF."
            << "\nDirectorio actual: " << fs::current_path()
            << std::endl;

        shaderProgram.Delete();
        shadowShader.Delete();

        glDeleteFramebuffers(1, &shadowFramebuffer);
        glDeleteTextures(1, &shadowTexture);

        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // Muestra la ruta del modelo en la consola.
    std::cout << "Cargando modelo: " << modelPath << std::endl;

    // Crea un puntero inteligente para guardar el modelo.
    std::unique_ptr<Model> model;

    // Intenta cargar el modelo 3D.
    try
    {
        model = std::make_unique<Model>(modelPath.string().c_str());
    }
    catch (const std::exception& exception)
    {
        // Muestra el error si el modelo no se puede cargar.
        std::cerr << "Error al cargar el modelo: "
            << exception.what() << std::endl;

        shaderProgram.Delete();
        shadowShader.Delete();

        glDeleteFramebuffers(1, &shadowFramebuffer);
        glDeleteTextures(1, &shadowTexture);

        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }


    // -----------------------------------------
    // POSICION DE LA MOTOSIERRA
    // -----------------------------------------

    const float chainsawHeight = 1.0f; // Altura de la motosierra.
    const float chainsawDepth = 0.75f; // Desplazamiento de la motosierra en profundidad.

    // Crea una matriz que mueve la motosierra a la posicion indicada.
    model->SetTransform(glm::translate(
        glm::mat4(1.0f),
        glm::vec3(0.0f, chainsawHeight, chainsawDepth)
    ));

    const glm::vec3 bezierStart(0.0f, chainsawHeight, chainsawDepth);
    const glm::vec3 bezierControl1(-7.0f, chainsawHeight + 4.0f, chainsawDepth + 1.0f);
    const glm::vec3 bezierControl2(7.0f, chainsawHeight + 4.0f, chainsawDepth - 5.0f);
    const glm::vec3 bezierEnd(0.0f, chainsawHeight, chainsawDepth - 8.0f);
    const float bezierDuration = 6.0f;

    float bezierElapsed = 0.0f;
    bool bezierActive = false;
    bool bezierKeyWasPressed = false;


    // -----------------------------------------
    // CONTROL DE INTENSIDAD DE LA LUZ
    // -----------------------------------------

    float lightIntensity = 1.0f;       // Intensidad inicial de la luz.

    const float minIntensity = 0.0f;   // Intensidad minima permitida.
    const float maxIntensity = 3.0f;   // Intensidad maxima permitida.

    float lastFrame = 0.0f;            // Guarda el tiempo del fotograma anterior.

    const float lightSpeed = 1.0f;     // Velocidad con la que cambia la intensidad.


    // -----------------------------------------
    // BUCLE PRINCIPAL DEL PROGRAMA
    // -----------------------------------------

    while (!glfwWindowShouldClose(window))
    {
        // Obtiene el tiempo actual del programa.
        float currentFrame = (float)glfwGetTime();

        // Calcula el tiempo transcurrido desde el fotograma anterior.
        float deltaTime = currentFrame - lastFrame;

        // Guarda el tiempo actual para el siguiente fotograma.
        lastFrame = currentFrame;

        const bool bezierKeyPressed =
            glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS;

        if (bezierKeyPressed && !bezierKeyWasPressed)
        {
            bezierElapsed = 0.0f;
            bezierActive = true;
        }

        bezierKeyWasPressed = bezierKeyPressed;

        if (bezierActive)
        {
            bezierElapsed = glm::min(
                bezierElapsed + deltaTime,
                bezierDuration
            );

            const float t = bezierElapsed / bezierDuration;
            const glm::vec3 position = cubicBezier(
                bezierStart,
                bezierControl1,
                bezierControl2,
                bezierEnd,
                t
            );

            model->SetTransform(glm::translate(glm::mat4(1.0f), position));

            if (bezierElapsed >= bezierDuration)
                bezierActive = false;
        }


        // -----------------------------------------
        // CONTROL DE INTENSIDAD DE LA LUZ
        // -----------------------------------------

        // Aumenta la intensidad al presionar +.
        if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
        {
            lightIntensity += lightSpeed * deltaTime;
        }

        // Disminuye la intensidad al presionar -.
        if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
        {
            lightIntensity -= lightSpeed * deltaTime;
        }

        // Recupera la intensidad inicial al presionar 0.
        if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
        {
            lightIntensity = 1.0f;
        }

        // Evita que la intensidad sea menor que 0 o mayor que 3.
        lightIntensity = glm::clamp(
            lightIntensity,
            minIntensity,
            maxIntensity
        );


        // -----------------------------------------
        // PRIMERA PASADA: CALCULAR LAS SOMBRAS
        // -----------------------------------------

        shadowShader.Activate(); // Activa el shader de sombras.

        // Envia la matriz de la luz al shader de sombras.
        glUniformMatrix4fv(
            glGetUniformLocation(shadowShader.ID, "lightSpaceMatrix"),
            1,
            GL_FALSE,
            glm::value_ptr(lightSpaceMatrix)
        );

        // Cambia el area de dibujo al tamaño del mapa de sombras.
        glViewport(0, 0, shadowWidth, shadowHeight);

        // Activa el framebuffer que guarda las profundidades.
        glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer);

        // Limpia las profundidades del fotograma anterior.
        glClear(GL_DEPTH_BUFFER_BIT);

        // Dibuja el modelo desde el punto de vista de la luz.
        model->Draw(shadowShader, camera);

        // Regresa al framebuffer principal.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        // -----------------------------------------
        // SEGUNDA PASADA: DIBUJAR LA ESCENA
        // -----------------------------------------

        // Recupera las dimensiones originales de la ventana.
        glViewport(0, 0, width, height);

        // Define el color de fondo de la ventana.
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);

        // Limpia los colores y las profundidades del fotograma anterior.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Activa el shader principal que dibuja el modelo con iluminacion.
        shaderProgram.Activate();

        // Envia la intensidad actual de la luz al Fragment Shader.
        glUniform1f(
            glGetUniformLocation(shaderProgram.ID, "lightIntensity"),
            lightIntensity
        );

        // Envia la matriz utilizada para calcular las sombras.
        glUniformMatrix4fv(
            glGetUniformLocation(shaderProgram.ID, "lightSpaceMatrix"),
            1,
            GL_FALSE,
            glm::value_ptr(lightSpaceMatrix)
        );

        // Envia la direccion de la luz al shader principal.
        glUniform3f(
            glGetUniformLocation(shaderProgram.ID, "directionalLightDirection"),
            directionalLightDirection.x,
            directionalLightDirection.y,
            directionalLightDirection.z
        );

        // Indica al shader que debe utilizar el mapa de sombras.
        glUniform1i(
            glGetUniformLocation(shaderProgram.ID, "useShadowMap"),
            GL_TRUE
        );

        // Selecciona la unidad de textura numero 5.
        glActiveTexture(GL_TEXTURE5);

        // Conecta la textura que contiene el mapa de sombras.
        glBindTexture(GL_TEXTURE_2D, shadowTexture);

        // Indica al shader que el mapa de sombras esta en la unidad 5.
        glUniform1i(
            glGetUniformLocation(shaderProgram.ID, "shadowMap"),
            5
        );


        // -----------------------------------------
        // ACTUALIZACION DE LA CAMARA
        // -----------------------------------------

        camera.Inputs(window); // Lee las teclas utilizadas para mover la camara.

        // Actualiza las matrices de vista y proyeccion de la camara.
        camera.updateMatrix(45.0f, 0.1f, 100.0f);


        // -----------------------------------------
        // DIBUJAR EL MODELO 3D
        // -----------------------------------------

        model->Draw(shaderProgram, camera); // Dibuja la motosierra con iluminacion y sombras.


        // -----------------------------------------
        // FINALIZAR EL FOTOGRAMA
        // -----------------------------------------

        glfwSwapBuffers(window); // Muestra el fotograma terminado en la ventana.

        glfwPollEvents();        // Procesa los eventos del teclado y la ventana.
    }


    // -----------------------------------------
    // LIBERAR RECURSOS
    // -----------------------------------------

    shaderProgram.Delete(); // Libera el shader principal.
    shadowShader.Delete();  // Libera el shader de sombras.

    glDeleteFramebuffers(1, &shadowFramebuffer); // Elimina el framebuffer de sombras.
    glDeleteTextures(1, &shadowTexture);         // Elimina la textura de sombras.

    glfwDestroyWindow(window); // Destruye la ventana.
    glfwTerminate();           // Cierra GLFW y libera sus recursos.

    return 0;                   // Finaliza el programa correctamente.
}