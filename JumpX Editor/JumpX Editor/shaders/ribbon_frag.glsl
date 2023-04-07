#version 330

in vec2 vUv;

out vec4 fColor;

uniform sampler2D texRibbon;

uniform vec4 ribColor;

void main() {
	fColor = ribColor * texture(texRibbon, vUv);
}