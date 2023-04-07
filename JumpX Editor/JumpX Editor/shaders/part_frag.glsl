#version 330

in vec2 vUv;
in vec4 vColor;

out vec4 fColor;

uniform sampler2D texPart;

void main() {
	fColor = vColor * texture(texPart, vUv);
}