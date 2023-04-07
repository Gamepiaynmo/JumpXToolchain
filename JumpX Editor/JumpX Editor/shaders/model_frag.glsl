#version 330

in vec4 vColor;
in vec3 vPos;
in vec3 vNormal;
in vec2 vUv;
in vec2 vUv1;

out vec4 fColor;

uniform sampler2D texDiffuse;
uniform sampler2D texBump;
uniform sampler2D texSpecular;
uniform sampler2D texLight;
uniform sampler2D texCartSpec;
uniform sampler2D texCartShadow;
uniform sampler2D texDissolve;

uniform mat4 matCam;
uniform vec3 lightDir;
uniform bool alphaBlend;
uniform bool alphaTest;
uniform bool uvClamp;
uniform bool selected;
uniform bool uv2;
uniform bool cartoon;
uniform vec3 ambientColor;
uniform vec3 shadowColor;
uniform float ambientInt;
uniform float shadowTh;
uniform float specularSmth;
uniform bool specular;
 
void main() {
	if (!selected && !alphaBlend && vColor.a < 0.01)
		discard;

	vec4 color = vColor * texture(texDiffuse, uvClamp ? clamp(vUv, 0.0, 1.0) : vUv);
	if (alphaTest && color.a < 0.01)
		discard;

	vec2 vUv2 = uv2 ? vUv1 : vUv;
	vec3 camPos = matCam[3].xyz;
	vec3 camDir = vec3(matCam[0][2], matCam[1][2], matCam[2][2]);
	vec3 halfVec = normalize(camDir + lightDir);
	float ndotl = max(dot(lightDir, vNormal), 0.0);
	float ndoth = max(dot(halfVec, vNormal), 0.0);

	if (cartoon) {
		vec3 ilmTex = texture(texCartSpec, vUv2).rgb;
		vec3 shadowTex = texture(texCartShadow, vUv2).rgb;
		float lambert = ndotl + (ilmTex.g * 2 - 1);
		float shadowStep = smoothstep(shadowTh, shadowTh + 0.05, lambert);
		vec3 light = mix(shadowTex * shadowColor, ambientColor * ambientInt, shadowStep);
		float specInt = pow(ndoth, 8) * ilmTex.r + pow(ndoth, 4) * ilmTex.b;
		float specStep = smoothstep(0, specularSmth, specInt);
		float edge = smoothstep(0.8, 0.85, max(1 - dot(camDir, vNormal), 0)) * (shadowStep - 0.5);
		color.rgb *= light * (1 + specStep * 0.8 + edge);
	} else {
		if (!alphaBlend) {
			float diffuse = 0.6 + 0.6 * ndotl;
			vec3 light = vec3(diffuse);
			if (specular) {
				vec3 specTex = texture(texSpecular, vUv2).rgb;
				light += pow(ndoth, 8) * specTex;
			}
			color.rgb *= light;
		}
	}
	if (selected) {
		color = mix(color, vec4(1.0, 0.0, 0.0, color.a), 0.7);
		color.a = max(0.3, color.a);
	}

	fColor = color;
}