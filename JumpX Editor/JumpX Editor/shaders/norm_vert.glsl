#version 330

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in int numBone;
layout(location = 3) in ivec4 boneId;
layout(location = 4) in vec4 boneWeight;

out vec4 vNormal;

uniform mat4 matCam;
uniform mat4 matView;
uniform mat3x4 boneMatrix[256];
uniform mat4 matBoneInv;
uniform mat4 matBone;
uniform int billboard;
uniform vec3 center;

void main() {
	vec3 totalPosition = vec3(0.0);
	vec3 totalNormal = vec3(0.0);
	for (int i = 0; i < numBone; i++) {
		vec3 localPosition = transpose(boneMatrix[boneId[i]]) * vec4(position, 1.0);
		totalPosition += localPosition * boneWeight[i];
		vec3 localNormal = inverse(mat3(boneMatrix[boneId[i]])) * normal;
		totalNormal += localNormal * boneWeight[i];
	}

	if (billboard > 0) {
		mat4 matBoard = mat4(1.0);
		matBoard[3] = vec4(center, 1.0);
		matBoard = matBone * matBoneInv * matBoard;
		mat4 matCamT = transpose(matCam);
		matBoard[0] = normalize(matBoard[0]);
		matBoard[1] = normalize(matBoard[1]);
		matBoard[2] = normalize(matBoard[2]);

		mat3 matRot;
		if (billboard == 1) {
			matRot = outerProduct(matCamT[2].xyz, normalize(totalNormal));
			matRot -= transpose(matRot);
			matRot = mat3(1.0) + matRot + matRot * matRot / (1 + dot(matCamT[2].xyz, normalize(totalNormal)));
		} else if (billboard == 2) {
			matRot = outerProduct(vec3(0, 0, 1), matBoard[2].xyz);
			matRot -= transpose(matRot);
			matRot = mat3(1.0) + matRot + matRot * matRot / (1 + matBoard[2].z);
		} else {
			mat3 matTarget = transpose(mat3(
				normalize(vec3(matCamT[0].xy, 0.0)),
				normalize(vec3(matCamT[2].xy, 0.0)),
				vec3(0, 0, 1)));
			matRot = matTarget * inverse(mat3(matBoard));
		}

		totalPosition = matBoard[3].xyz + matRot * (totalPosition - matBoard[3].xyz);
		totalNormal = matRot * totalNormal;
	}

	gl_Position = matView * matCam * vec4(totalPosition, 1.0);
	vNormal = matView * matCam * vec4(totalPosition + normalize(totalNormal), 1.0);
}