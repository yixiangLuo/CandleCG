// Fragment shader:
// ================
#version 330 core
struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_height1;
    float shininess;
};
/* Note: because we now use a material struct again you want to change your
mesh class to bind all the textures using material.texture_diffuseN instead of
texture_diffuseN. */

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

struct Pos_Norm {
    float darken;
    vec2 texCoords;
    vec3 normal;
};


#define NR_POINT_LIGHTS 2

in vec3 fragPosition;
in vec2 TexCoords;
in vec3 TangentViewPos;
in vec3 TangentFragPos;
in PointLight lights[NR_POINT_LIGHTS];

out vec4 color;

uniform vec3 viewPos;
uniform Material material;
uniform float far_plane;
uniform samplerCube shadowMap1;
uniform samplerCube shadowMap2;

// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1),
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

// Function prototypes
vec3 CalcPointLight(PointLight light, Material mat, int shadowInd, vec3 normal, vec3 TangentFragPos, vec3 viewDir, vec2 TexCoord, float darken);

Pos_Norm ParallaxMapping(Material mat, vec2 texCoords, vec3 viewDir, float height_scale);

void main()
{
    float height_scale = 0.001;
    vec3 result = vec3(0, 0, 0);
    vec3 viewDir = normalize(TangentViewPos - TangentFragPos);

    for(int i = 0; i < NR_POINT_LIGHTS; i++){
        Pos_Norm pos_norm = ParallaxMapping(material, TexCoords, viewDir, height_scale);
        // if(TexCoord.x > 1.0 || TexCoord.y > 1.0 || TexCoord.x < 0.0 || TexCoord.y < 0.0) continue;
        result += CalcPointLight(lights[i], material, i, pos_norm.normal, TangentFragPos, viewDir, pos_norm.texCoords, pos_norm.darken);
    }

    color = vec4(result, 1.0f);
}

vec3 estNorm(Material mat, vec2 texCoords, float scale){
  // vec3 normal = vec3(0, 0, 0);
  // int half_len = 10;
  // float step = 1.0/half_len;
  // for(int i = -half_len; i <= half_len; i++){
  //   for(int j = -half_len; j <= half_len; j++){
  //     if(!(abs(i)>=10 && abs(j)>=10)){
  //       if(texture(mat.texture_height1, texCoords + vec2(step*i*scale, step*j*scale)).r < 0.5){
  //         normal += normalize(vec3(step*i*scale, step*j*scale, 0));
  //       }
  //     }
  //   }
  // }
  // if(distance(normal, vec3(0,0,0))<1e-2) return vec3(0,0,1);
  // return normalize(normal);
  float theta=0;
  float delta_theta=0;
  int total = 50;
  float step = 2*3.14159/total;
  vec3 normal = vec3(0, 0, 0);
  for(int i = 0; i < total; i++){
    float cur_height = texture(mat.texture_height1, texCoords + scale*vec2(cos(theta), sin(theta)) ).r;
    // float forward_height = texture(mat.texture_height1, texCoords + scale*vec2(cos(theta+step), sin(theta+step)) ).r;
    // float delta_height = forward_height - cur_height;
    // if(abs(delta_height)>0.1){
    //   normal += normalize(delta_height*cross(vec3(cos(theta), sin(theta), 0), vec3(0, 0, 1)));
    // }
    if(cur_height < 0.5){
      normal += normalize(vec3(cos(theta), sin(theta), 0));
    }
    theta += step;
  }
  if(distance(normal, vec3(0,0,0))<1e-2) return vec3(0,0,1);
  return normalize(normal);
}

vec2 bisearch(Material mat, vec2 a, vec2 b, float tol){
  while(true) {
    vec2 c = (a+b)/2;
    if(distance(a, b)<tol) return c;
    if(texture(mat.texture_height1, c).r > 0.5) b=c;
    else a=c;
  }
}

Pos_Norm ParallaxMapping(Material mat, vec2 texCoords, vec3 viewDir, float height_scale)
{
    Pos_Norm pos_norm;
    pos_norm.texCoords = texCoords;
    pos_norm.normal = vec3(0, 0, 1);
    pos_norm.darken = 1;

    int total = 10;
    float step = 1.0/total;
    float tol = 1e-3;

    vec2 dir = - viewDir.xy / viewDir.z * height_scale;
    float cur_height =  texture(mat.texture_height1, texCoords).r;
    float hit_height =  texture(mat.texture_height1, texCoords + dir).r;
    if(cur_height<0.5){
      if(hit_height<0.5){
        pos_norm.texCoords = texCoords + dir;
        pos_norm.darken = 0.9;
      }
      else{
        for(int i = 1; i <= total; i++){
          if(texture(mat.texture_height1, texCoords + step*i*dir).r > 0.5)
            pos_norm.texCoords = bisearch(mat, texCoords + step*(i-1)*dir, texCoords + step*i*dir, tol);
            pos_norm.normal = estNorm(mat, pos_norm.texCoords, tol*0.5);
        }
      }
    }
    return pos_norm;
    // float height =  texture(mat.texture_height1, texCoords).r;
    // vec2 p = viewDir.xy / viewDir.z * (height * height_scale);
    // return texCoords - p;
}

float ShadowCalculation(vec3 lightPos, samplerCube shadowMap, float bias)
{
    // get vector between fragment position and light position
    vec3 fragToLight = fragPosition - lightPos;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    float shadow = 0.0;
    int samples = 20;
    float viewDistance = length(viewPos - fragPosition);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(shadowMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= far_plane;   // undo mapping [0;1]
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);

    // display closestDepth as debug (to visualize depth cubemap)
    // FragColor = vec4(vec3(closestDepth / far_plane), 1.0);

    return shadow;
}

// Calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, Material mat, int shadowInd, vec3 normal, vec3 TangentFragPos, vec3 viewDir, vec2 TexCoord, float darken)
{
    vec3 lightDir = normalize(light.TangentPosition - TangentFragPos);
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), mat.shininess);
    // Attenuation
    float distance = length(light.TangentPosition - TangentFragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // Combine results
    vec3 ambient = light.ambient * vec3(texture(mat.texture_diffuse1, TexCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(mat.texture_diffuse1, TexCoord));
    vec3 specular = light.specular * spec * vec3(texture(mat.texture_specular1, TexCoord));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    float shadow;
    if(shadowInd==0) shadow = ShadowCalculation(light.position, shadowMap1, 0.01);
    else if (shadowInd==1) shadow = ShadowCalculation(light.position, shadowMap2, 0.07);
    vec3 lighting = ambient + (1-shadow) * (darken * diffuse + specular);
    return lighting;
}
