#version 430 core

uniform vec3 objectColor;
uniform vec3 lightDir;

in vec3 interpNormal;
in vec3 verPos;

void main()
{
	vec3 normal = normalize(interpNormal);
	float diffuse = max(dot(normal, -lightDir), 0.0);
	float t = (1+sin(3.141 * verPos.z / 0.3)) / 2;
	gl_FragColor = vec4(((1 - t) * vec3(1.0,1.0,1.0) + t * objectColor) * diffuse, 1.0); 	
}
