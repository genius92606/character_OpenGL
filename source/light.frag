#version 330 core

layout (location = 0) out vec4 fragColor;

in VS_OUT
{
  vec3 worldPos;
  vec3 normal;
}fs_in;



uniform vec4 color_ambient = vec4(0.1f, 0.2f, 0.5f, 1.0f);
//uniform vec4 color_diffuse = vec4(0.2f, 0.3f, 0.8f, 1.0f);
uniform vec4 color_specular = vec4(0.0f);
uniform float shininess = 1.0f;
uniform int object_number;

uniform vec3 lightPos = vec3(200.0f, 1000.0f, 200.0f);

void main(void)
{
   
   vec4 color_diffuse;
   switch(object_number) {
    case 0  :  //body, gray
      color_diffuse=vec4(0.6f, 0.6f, 0.6f, 1.0f);
      break;
    case 1  :  //head, black
      color_diffuse=vec4(0.0f, 0.0f, 0.0f, 1.0f);
      break; 
    case 2  : //right shoulder, cyan
      color_diffuse=vec4(0.0f, 1.0f, 1.0f, 1.0f);
      break;
    case 3  : //left shoulder, cyan
      color_diffuse=vec4(0.0f, 1.0f, 1.0f, 1.0f);
      break;
    case 4  : //right thigh, red
      color_diffuse=vec4(1.0f, 0.0f, 0.0f, 1.0f);
      break;
	case 5  : //left thigh, red
      color_diffuse=vec4(1.0f, 0.0f, 0.0f, 1.0f);
      break;
	case 6  : //right elbow, green
      color_diffuse=vec4(0.0f, 1.0f, 0.0f, 1.0f);
      break;
	case 7  : //left elbow, green
      color_diffuse=vec4(0.0f, 1.0f, 0.0f, 1.0f);
      break;
	case 8  : //right shank, yellow
      color_diffuse=vec4(1.0f, 1.0f, 0.0f, 1.0f);
      break;
	case 9  : //left shank, yellow
      color_diffuse=vec4(1.0f, 1.0f, 0.0f, 1.0f);
      break;
	case 10  : //right ankle, white
      color_diffuse=vec4(1.0f, 1.0f, 1.0f, 1.0f);
      break;
	case 11  : //left ankle, white
      color_diffuse=vec4(1.0f, 1.0f, 1.0f, 1.0f);
      break;
}
  
   vec3 lightDir = normalize(lightPos - fs_in.worldPos);
   vec3 normal = normalize(fs_in.normal);
   vec3 r = reflect( -lightDir, normal );
   vec3 eye = vec3(0.0f, 0.0f, 1.0f);
   float diffuse = max(0.0f, dot(normal, lightDir) );
   float specular = pow(max(0.0f, dot(eye, r)), shininess);
   
   fragColor = diffuse * color_diffuse + specular * color_specular + color_ambient*0.1;
}
 