// Vertex shader:
// ================
#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tangent;
layout (location = 3) in vec2 texCoords;

struct PointLight {
    vec3 position;
    vec3 TangentPosition;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define NR_POINT_LIGHTS 2

out vec2 TexCoords;
out vec3 fragPosition;
out vec3 TangentViewPos;
out vec3 TangentFragPos;
out PointLight lights[NR_POINT_LIGHTS];

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform vec3 viewPos;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
    fragPosition = vec3(model * vec4(position, 1.0f));
    mat3 normalMat = mat3(transpose(inverse(model)));
    TexCoords = texCoords;

    vec3 T = normalize(normalMat * tangent);
    vec3 N = normalize(normalMat * normal);
    vec3 B = cross(T, N);
    mat3 TBN = transpose(mat3(T, B, N));

    lights = pointLights;
    for(int i = 0; i < NR_POINT_LIGHTS; i++){
      lights[i].TangentPosition = TBN * lights[i].position;
    }
    TangentViewPos  = TBN * viewPos;
    TangentFragPos  = TBN * fragPosition;
}
