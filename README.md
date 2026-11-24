# Motosierra3D

**Nombre del proyecto:** Motosierra3D

**Integrante:** Miguel Angel Jaimes Bello

**Código estudiantil:** 6014335

# Arquitectura del software

El proyecto Motosierra3D está desarrollado en C++ utilizando OpenGL 3.3 y presenta una arquitectura modular, en la que cada archivo cumple una función específica para facilitar la organización, el mantenimiento y el funcionamiento del programa.

# Componentes principales

**main.cpp:** es el archivo principal del proyecto. Se encarga de inicializar OpenGL, crear la ventana, configurar la cámara, cargar el modelo 3D, administrar la iluminación, controlar la animación mediante una curva de Bézier y ejecutar el ciclo de renderizado.

**Camera.h y Camera.cpp:** controlan la cámara virtual mediante su posición y orientación. Utilizan matrices de vista y proyección para permitir visualizar la escena tridimensional y desplazarse por ella mediante el teclado y el mouse.

**Model.h y Model.cpp:** se encargan de importar y procesar modelos 3D en formato glTF, organizando sus componentes, nodos, mallas y recursos asociados para poder representarlos en la escena.

**Mesh.h y Mesh.cpp:** administran las mallas que componen los modelos tridimensionales, incluyendo sus vértices, índices, texturas, buffers gráficos y operaciones de dibujo.

**shaderClass.h:** administra la creación, compilación y utilización de los programas de shaders de OpenGL.

**default.vert:** procesa la información de los vértices y aplica las transformaciones necesarias para representar el modelo en pantalla.

**default.frag:** calcula el color final de los fragmentos del modelo e implementa la iluminación basada en el modelo Phong, utilizando componentes ambiental, difusa y especular. También participa en el cálculo de las sombras y permite controlar la intensidad de la iluminación.

**shadow.vert y shadow.frag:** participan en el proceso de generación de sombras mediante la técnica Shadow Mapping, utilizando la información de profundidad calculada desde la perspectiva de la luz.

**Texture.h:** administra la carga y configuración de las texturas utilizadas en los modelos tridimensionales.

**CMakeLists.txt:** contiene la configuración necesaria para construir y compilar el proyecto y organizar sus dependencias.

**Resources:** almacena los recursos externos utilizados por el programa, como modelos 3D, archivos glTF, texturas y otros archivos necesarios para representar los objetos.

# Flujo de funcionamiento

1. El programa inicia y configura el contexto gráfico mediante GLFW, GLAD y OpenGL.
2. Se inicializan los shaders principales, los shaders de sombras y la cámara virtual.
3. Se busca e importa el modelo tridimensional mediante el cargador glTF.
4. Se procesan las mallas, vértices, índices y texturas del modelo.
5. Se aplican las matrices de transformación para posicionar y representar los objetos.
6. Se calcula la iluminación mediante el modelo Phong, utilizando componentes ambiental, difusa y especular.
7. Se genera el mapa de profundidad mediante Shadow Mapping para calcular las sombras de la escena.
8. Se calcula el movimiento de la motosierra mediante una curva de Bézier cúbica, utilizando cuatro puntos para definir su trayectoria.
9. OpenGL renderiza la escena y actualiza la imagen continuamente según las entradas del teclado y el mouse.

# Iluminación y sombras

El proyecto implementa un sistema de iluminación basado en el modelo Phong, compuesto por iluminación ambiental, difusa y especular, que permite representar diferentes efectos de luz sobre las superficies del modelo.

También incorpora la técnica Shadow Mapping, que utiliza un framebuffer y una textura de profundidad para almacenar la información de los objetos desde la perspectiva de la luz. El proceso se realiza mediante dos pasadas de renderizado, permitiendo calcular qué partes de la escena reciben luz y cuáles se encuentran en sombra.

Adicionalmente, se implementa un control de intensidad de iluminación que permite modificar el brillo de la escena durante la ejecución.

# Animación mediante curva de Bézier

Se implementa una curva de Bézier cúbica para controlar el desplazamiento de la motosierra dentro de la escena. Esta curva utiliza cuatro puntos: un punto inicial, dos puntos de control y un punto final.

Mediante una función matemática se calculan posiciones intermedias que permiten generar una trayectoria curva y suave. La animación se activa presionando la tecla T y tiene una duración aproximada de seis segundos.

# Controles del proyecto

- **W:** desplazar la cámara hacia adelante.
- **S:** desplazar la cámara hacia atrás.
- **A:** desplazar la cámara hacia la izquierda.
- **D:** desplazar la cámara hacia la derecha.
- **Espacio:** subir la cámara.
- **Ctrl izquierdo:** bajar la cámara.
- **Mouse:** orientar la cámara.
- **T:** activar la animación mediante la curva de Bézier.
- **+ o =:** aumentar la intensidad de la iluminación.
- **- o _:** disminuir la intensidad de la iluminación.
- **0:** restablecer la intensidad de la iluminación.
