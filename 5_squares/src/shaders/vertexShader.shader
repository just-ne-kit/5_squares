#version 330 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec4 vColor;
layout (location = 2) in vec2 vTexCoords;
layout (location = 3) in float vTexID;

out vec4 outColor;
out vec2 outTexCoords;
out float outTexID;

uniform mat4 u_MVP;

void main(){
   gl_Position = u_MVP * vec4(vPos.x, vPos.y, vPos.z, 1);
   
   outColor = vColor;
   outTexCoords = vTexCoords;
   outTexID = vTexID;
}