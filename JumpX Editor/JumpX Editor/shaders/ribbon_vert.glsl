#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

out vec2 vUv;

uniform mat4 matCam;
uniform mat4 matView;

void main() {
	gl_Position = matView * matCam * vec4(position, 1.0);
	vUv = uv;
}