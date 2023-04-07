#version 330

layout(points) in;
layout(line_strip, max_vertices = 8) out;

in vec4 xDir[];
in vec4 yDir[];
in vec4 zDir[];
in vec4 parentPos[];
in float selected[];

out vec3 gColor;

void main() {
	gColor = vec3(1.0, 1.0, 0.0);
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();

	gl_Position = parentPos[0];
	EmitVertex();
	
	EndPrimitive();
	
	bool select = selected[0] > 0.5;
	float length = select ? 2.0 : 1.0;

	gColor = select ? vec3(1.0, 0.0, 0.0) : vec3(1.0, 1.0, 1.0);
	gl_Position = gl_in[0].gl_Position - xDir[0] * length;
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + xDir[0] * length;
	EmitVertex();
	
	EndPrimitive();

	gColor = select ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 1.0, 1.0);
	gl_Position = gl_in[0].gl_Position - yDir[0] * length;
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + yDir[0] * length;
	EmitVertex();
	
	EndPrimitive();

	gColor = select ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 1.0, 1.0);
	gl_Position = gl_in[0].gl_Position - zDir[0] * length;
	EmitVertex();

	gl_Position = gl_in[0].gl_Position + zDir[0] * length;
	EmitVertex();
	
	EndPrimitive();
}