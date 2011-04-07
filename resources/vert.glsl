varying vec3 normal;
varying vec3 vertex_to_light_vector;

void main()
{
	// Transforming The Vertex
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

	// Transforming The Normal To ModelView-Space
	normal = gl_NormalMatrix * gl_Normal; 

	// Transforming The Vertex Position To ModelView-Space
	vec4 vertex_in_modelview_space = gl_ModelViewMatrix * gl_Vertex;

	// Calculating The Vector From The Vertex Position To The Light Position
 	vertex_to_light_vector = vec3(gl_LightSource[0].position Ð vertex_in_modelview_space);
}