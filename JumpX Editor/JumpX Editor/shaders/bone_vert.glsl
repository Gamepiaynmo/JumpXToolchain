#version 330

layout(location = 0) in int parent;

out vec4 xDir;
out vec4 yDir;
out vec4 zDir;
out vec4 parentPos;
out float selected;

uniform mat4 matCam;
uniform mat4 matView;
uniform mat3x4 boneMatrix[256];
uniform int selectId;

void main() {
	mat4x3 mat = transpose(boneMatrix[gl_VertexID]);
	vec4 position = matView * matCam * vec4(mat[3], 1.0);
	if (parent >= 0) {
		mat4x3 parentMat = transpose(boneMatrix[parent]);
		parentPos = matView * matCam * vec4(parentMat[3], 1.0);
	} else parentPos = position;

	gl_Position = position;
	selected = (selectId == gl_VertexID ? 1.0 : 0.0);
	xDir = matView * matCam * vec4(mat[0], 0.0);
	yDir = matView * matCam * vec4(mat[1], 0.0);
	zDir = matView * matCam * vec4(mat[2], 0.0);
}