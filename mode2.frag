#version 400
precision highp float;
in  vec3 vNormal;
out vec4 FragColor;

void main(void) {
    vec3 uAmbient = vec3(0.1,0.1,0.0);
    vec3 uSurfaceColour = vec3(0.0,1.0,0.0);
    vec3 uLightColour = vec3(1.0);
    vec3 uLightDirection = vec3(0.,0.,1.);
    vec3 LightDirection = normalize(uLightDirection);
    float CosTheta = dot(LightDirection, vNormal);
    float theta = clamp(CosTheta, 0., 1.);
    vec3 diffuse = uSurfaceColour * theta * uLightColour;
    FragColor = vec4(uAmbient + diffuse,1.0);
}
