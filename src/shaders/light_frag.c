// Fragment shader:
// ================
#version 330 core
struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_height1;
};
/* Note: because we now use a material struct again you want to change your
mesh class to bind all the textures using material.texture_diffuseN instead of
texture_diffuseN. */

in vec2 TexCoords;

out vec4 color;

uniform Material material;

void main()
{
    color = vec4(1.1*texture(material.texture_diffuse1, TexCoords).rgb, texture(material.texture_height1, TexCoords).r);
}
