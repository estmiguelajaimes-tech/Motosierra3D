# Motosierra3D

Nombre del proyecto: Motosierra3D
Integrante: Miguel Angel Jaimes Bello
Código estudiantil:6014335

# Arquitectura del software
El proyecto Motosierra3D está desarrollado en C++ utilizando OpenGL y presenta una arquitectura modular, en la que cada archivo cumple una función específica para facilitar la organización, el mantenimiento y el funcionamiento del programa.

# Componentes principales
**main.cpp:** es el archivo principal del proyecto. Se encarga de inicializar OpenGL, crear la ventana, configurar la cámara, cargar el modelo 3D, administrar la iluminación y ejecutar el ciclo de renderizado.

**Camera.h:** controla la cámara virtual mediante su posición y orientación. Utiliza matrices de vista y proyección para permitir visualizar la escena tridimensional.

**Model.h:** se encarga de importar y procesar modelos 3D en formato glTF, organizando sus componentes para poder representarlos en la escena.
**Mesh.h:** administra las mallas que componen los modelos tridimensionales, incluyendo sus vértices, índices, texturas y operaciones de dibujo.
**shaderClass.h:** administra la creación, compilación y utilización de los programas de shaders de OpenGL.
**default.vert:** procesa la información de los vértices y aplica las transformaciones necesarias para representar el modelo en pantalla.
**default.frag:** calcula el color final de los fragmentos del modelo e implementa la iluminación basada en el modelo Phong.
**shadow.vert y shadow.frag:** participan en el proceso de generación de sombras mediante la técnica Shadow Mapping.
**CMakeLists.txt:** contiene la configuración necesaria para construir y compilar el proyecto.
**Resources:** almacena los recursos externos utilizados por el programa, como modelos 3D, archivos glTF y texturas.

# Flujo de funcionamiento
1. El programa inicia y configura el contexto gráfico mediante GLFW, GLAD y OpenGL.
2. Se inicializan los shaders y la cámara virtual.
3. Se importa el modelo tridimensional mediante el cargador glTF.
4. Se procesan las mallas y sus recursos asociados.
5. Se aplican las matrices de transformación para posicionar y representar los objetos.
6. Se calcula la iluminación mediante el modelo Phong y se generan las sombras.
7. Se calcula el movimiento del modelo mediante una curva Bézier.
8. Finalmente, OpenGL renderiza la escena y actualiza la imagen según las entradas del teclado.
