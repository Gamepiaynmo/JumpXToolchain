#version 330

layout(points) in;
layout(line_strip, max_vertices = 2) out;

in vec4 vNormal[];

void main() {
	gl_Position = gl_in[0].gl_Position; 
	EmitVertex();

	gl_Position = vNormal[0];
	EmitVertex();
	
	EndPrimitive();
}