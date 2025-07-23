#version 330 core

in vec4 outColor;
in vec2 outTexCoords;
in float outTexID;

uniform sampler2D u_Textures[2];

void main(){
    int index = int(outTexID);
    switch(index) 
    {
        case 0: gl_FragColor = outColor; break;
        case 1: gl_FragColor = texture(u_Textures[0], outTexCoords); break;
        case 2: gl_FragColor = texture(u_Textures[1], outTexCoords);
   }
}