#version 330

in vec3 gColor;

out vec4 fColor;

void main() {
	fColor = vec4(gColor, 1.0);
}