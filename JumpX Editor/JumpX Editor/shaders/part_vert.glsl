#version 330

layout(location = 0) in vec2 vertPos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 position;
layout(location = 3) in vec3 velocity;
layout(location = 4) in float life;
layout(location = 5) in float lifeSpan;
layout(location = 6) in int uvAnim;
layout(location = 7) in vec4 rotation;
layout(location = 8) in float size;

out vec2 vUv;
out vec4 vColor;

uniform mat4 matCam;
uniform mat4 matView;
uniform mat4x3 matBone;
uniform bool modelSpace;
uniform bool alignVel;
uniform bool xyQuad;
uniform float midTime;
uniform vec3 colors[3];
uniform vec3 alphas;
uniform vec3 sizes;
uniform ivec2 grid;
uniform bool tailMode;
uniform vec2 tailLength;
uniform float tailSpeed;

vec4 quat_mult(vec4 q1, vec4 q2) {
	vec4 qr;
	qr.x = (q1.w * q2.x) + (q1.x * q2.w) + (q1.y * q2.z) - (q1.z * q2.y);
	qr.y = (q1.w * q2.y) - (q1.x * q2.z) + (q1.y * q2.w) + (q1.z * q2.x);
	qr.z = (q1.w * q2.z) + (q1.x * q2.y) - (q1.y * q2.x) + (q1.z * q2.w);
	qr.w = (q1.w * q2.w) - (q1.x * q2.x) - (q1.y * q2.y) - (q1.z * q2.z);
	return qr;
}

vec3 rotate(vec3 pos, vec4 quat) {
	return quat_mult(quat_mult(quat, vec4(pos.xyz, 0.0)), vec4(-quat.xyz, quat.w)).xyz;
}

void main() {
	vec4 color; float partSize = size;
	if (life < midTime * lifeSpan) {
		float prog = life / (midTime * lifeSpan);
		color = mix(vec4(colors[0], alphas.x), vec4(colors[1], alphas.y), prog);
		partSize *= mix(sizes.x, sizes.y, prog);
	} else {
		float prog = (life - midTime * lifeSpan) / (lifeSpan - midTime * lifeSpan);
		color = mix(vec4(colors[1], alphas.y), vec4(colors[2], alphas.z), prog);
		partSize *= mix(sizes.y, sizes.z, prog);
	}
	vColor = color;

	vec3 vertexPos = position, speedDir = velocity;
	if (modelSpace) {
		vertexPos = matBone * vec4(vertexPos, 1.0);
		speedDir = matBone * vec4(speedDir, 0.0);
	}
	vec2 partVertPos = vertPos;
	if (alignVel) {
		partVertPos.y += 1;
		partVertPos *= 0.5;
	}
	vec2 partScale = vec2(partSize);
	if (tailMode) {
		partScale.y = mix(tailLength.x, tailLength.y, life / lifeSpan);
	}
	vec3 rotatedVert = rotate(vec3(partVertPos * partScale, 0.0), vec4(rotation.yzw, rotation.x));
	if (alignVel) {
		vec3 alignY = normalize(mat3(matCam) * speedDir);
		vec3 alignX = normalize(vec3(-alignY.y, alignY.x, 0.0));
		vec3 alignZ = normalize(cross(alignX, alignY));
		mat3 alignCam = transpose(mat3(alignX, alignY, alignZ));
		vertexPos += rotatedVert * alignCam * mat3(matCam);
	}
	else if (xyQuad)
		vertexPos += rotatedVert;
	else vertexPos += rotatedVert * mat3(matCam);

	gl_Position = matView * matCam * vec4(vertexPos, 1.0);

	int texU = uvAnim % grid.x, texV = uvAnim / grid.x;
	vUv = vec2((texU + uv.x) / grid.x, (texV + uv.y) / grid.y);
}