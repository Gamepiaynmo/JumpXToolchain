#version 330

out vec4 fColor;

uniform vec4 lineColor;

void main() {
	fColor = lineColor;
}