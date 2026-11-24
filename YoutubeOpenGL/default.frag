
#version 330 core

// Color final que se mostrara en pantalla.
out vec4 FragColor;

// Informacion recibida del Vertex Shader.
in vec3 crntPos;
in vec3 Normal;
in vec3 color;
in vec2 texCoord;
in vec4 fragPosLightSpace;

// Texturas del modelo 3D.
uniform sampler2D diffuse0;
uniform sampler2D specular0;
uniform sampler2D normal0;
uniform sampler2D shadowMap;

// Indica si el modelo tiene mapas normales y especulares.
uniform bool hasNormalMap;
uniform bool hasSpecularMap;
uniform bool hasDiffuseMap;
uniform bool useShadowMap;

// Informacion de la luz.
uniform vec4 lightColor;
uniform vec3 lightPos;

// Posicion de la camara.
uniform vec3 camPos;

// Control de intensidad de la iluminacion.
uniform float lightIntensity;
uniform vec3 directionalLightDirection;

vec4 getDiffuseColor()
{
    return hasDiffuseMap ? texture(diffuse0, texCoord) : vec4(0.72, 0.72, 0.72, 1.0);
}

float calculateShadow(vec3 normal)
{
    if (!useShadowMap)
        return 0.0;

    vec3 projectedCoordinates = fragPosLightSpace.xyz /
                                fragPosLightSpace.w;
    projectedCoordinates = projectedCoordinates * 0.5 + 0.5;

    if (projectedCoordinates.z > 1.0 ||
        projectedCoordinates.x < 0.0 || projectedCoordinates.x > 1.0 ||
        projectedCoordinates.y < 0.0 || projectedCoordinates.y > 1.0)
        return 0.0;

    vec3 lightVector = normalize(-directionalLightDirection);
    float bias = max(0.004 * (1.0 - dot(normal, lightVector)), 0.0005);
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    float shadow = 0.0;

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float closestDepth = texture(
                shadowMap,
                projectedCoordinates.xy + vec2(x, y) * texelSize
            ).r;
            shadow += projectedCoordinates.z - bias > closestDepth ? 1.0 : 0.0;
        }
    }

    return shadow / 9.0;
}


// ------------------------------------------
// CALCULO DE LAS NORMALES
// ------------------------------------------

vec3 getNormal()
{
    // Normaliza la normal del modelo.
    vec3 normal = normalize(Normal);

    if (hasNormalMap)
    {
        // Obtiene los cambios de posicion.
        vec3 positionDelta1 = dFdx(crntPos);
        vec3 positionDelta2 = dFdy(crntPos);

        // Obtiene los cambios de las coordenadas UV.
        vec2 uvDelta1 = dFdx(texCoord);
        vec2 uvDelta2 = dFdy(texCoord);

        // Calcula el determinante.
        float determinant = uvDelta1.x * uvDelta2.y -
                            uvDelta1.y * uvDelta2.x;

        if (abs(determinant) > 0.000001)
        {
            // Calcula la direccion tangente.
            vec3 tangent = normalize(
                (positionDelta1 * uvDelta2.y -
                 positionDelta2 * uvDelta1.y) / determinant
            );

            // Calcula la direccion bitangente.
            vec3 bitangent = normalize(
                (-positionDelta1 * uvDelta2.x +
                 positionDelta2 * uvDelta1.x) / determinant
            );

            // Construye la matriz TBN.
            mat3 tangentBasis = mat3(tangent, bitangent, normal);

            // Obtiene la normal desde la textura.
            vec3 mappedNormal = texture(normal0, texCoord).rgb * 2.0 - 1.0;

            // Aplica la nueva normal.
            normal = normalize(tangentBasis * mappedNormal);
        }
    }

    return normal;
}


// ------------------------------------------
// CALCULO DE LA TEXTURA ESPECULAR
// ------------------------------------------

float getSpecularTexture()
{
    // Obtiene la textura especular si existe.
    // Si no existe, utiliza un valor de 1.
    return hasSpecularMap ? texture(specular0, texCoord).r : 1.0;
}


// ------------------------------------------
// ILUMINACION PUNTUAL
// ------------------------------------------

vec4 pointLight()
{
    // Calcula la direccion entre la luz y el modelo.
    vec3 lightVec = lightPos - crntPos;

    // Calcula la distancia de la luz.
    float dist = length(lightVec);

    float a = 3.0;
    float b = 0.7;

    // Calcula la perdida de intensidad por distancia.
    float inten = 1.0f / (a * dist * dist + b * dist + 1.0f);

    // Luz ambiental.
    float ambient = 0.20f;

    // Luz difusa.
    vec3 normal = getNormal();
    vec3 lightDirection = normalize(lightVec);

    float diffuse = max(dot(normal, lightDirection), 0.0f);

    // Luz especular.
    float specularLight = 0.50f;

    vec3 viewDirection = normalize(camPos - crntPos);
    vec3 reflectionDirection = reflect(-lightDirection, normal);

    float specAmount = pow(
        max(dot(viewDirection, reflectionDirection), 0.0f), 16
    );

    float specular = specAmount * specularLight;

    // Combina los componentes de iluminacion.
    vec4 finalColor =
        (getDiffuseColor() * (diffuse * inten + ambient) +
        getSpecularTexture() * specular * inten) * lightColor;

    // Aplica la intensidad general.
    return finalColor * lightIntensity;
}


// ------------------------------------------
// ILUMINACION DIRECCIONAL
// ------------------------------------------

vec4 direcLight()
{
    // Luz ambiental.
    float ambient = 0.20f;

    // Luz difusa.
    vec3 normal = getNormal();

    vec3 lightDirection = normalize(-directionalLightDirection);

    float diffuse = max(dot(normal, lightDirection), 0.0f);

    // Luz especular.
    float specularLight = 0.50f;

    vec3 viewDirection = normalize(camPos - crntPos);

    vec3 reflectionDirection = reflect(-lightDirection, normal);

    float specAmount = pow(
        max(dot(viewDirection, reflectionDirection), 0.0f), 16
    );

    float specular = specAmount * specularLight;

    // Combina las texturas y los componentes de iluminacion.
    float shadow = calculateShadow(normal);
    float directLight = diffuse * (1.0 - shadow);

    vec4 finalColor =
        (getDiffuseColor() * (directLight + ambient) +
        getSpecularTexture() * specular) * lightColor;

    // Multiplica por la intensidad que controla el usuario.
    return finalColor * lightIntensity;
}


// ------------------------------------------
// ILUMINACION TIPO REFLECTOR
// ------------------------------------------

vec4 spotLight()
{
    // Define el angulo de apertura del reflector.
    float outerCone = 0.90f;
    float innerCone = 0.95f;

    // Luz ambiental.
    float ambient = 0.20f;

    // Luz difusa.
    vec3 normal = getNormal();

    vec3 lightDirection = normalize(lightPos - crntPos);

    float diffuse = max(dot(normal, lightDirection), 0.0f);

    // Luz especular.
    float specularLight = 0.50f;

    vec3 viewDirection = normalize(camPos - crntPos);

    vec3 reflectionDirection = reflect(-lightDirection, normal);

    float specAmount = pow(
        max(dot(viewDirection, reflectionDirection), 0.0f), 16
    );

    float specular = specAmount * specularLight;

    // Calcula el angulo de iluminacion.
    float angle = dot(
        vec3(0.0f, -1.0f, 0.0f),
        -lightDirection
    );

    float inten = clamp(
        (angle - outerCone) / (innerCone - outerCone),
        0.0f,
        1.0f
    );

    // Combina los componentes de iluminacion.
    vec4 finalColor =
        (getDiffuseColor() * (diffuse * inten + ambient) +
        getSpecularTexture() * specular * inten) * lightColor;

    // Aplica la intensidad general.
    return finalColor * lightIntensity;
}


// ------------------------------------------
// FUNCION PRINCIPAL
// ------------------------------------------

void main()
{
    // Utiliza la iluminacion direccional.
    FragColor = direcLight();
}
