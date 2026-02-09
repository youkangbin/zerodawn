#version 430 core

layout(location = 0) in vec2 quadVertex;

layout(std430, binding = 0) readonly buffer TrailPoints {
    vec4 positions[];  // 所有轨道的点连续存储
};

layout(std430, binding = 1) readonly buffer TrailMetadata {
    vec4 data[];  // 交替存储：color, params, color, params...
};

out VS_OUT {
    vec2 uv;
    vec4 color;
    float fade;
    float glow;
} vs_out;

uniform mat4 viewProjection;
uniform vec3 cameraPos;
uniform float currentTime;
uniform float maxAge;
uniform int trailIndex;  // 当前绘制的轨道索引

void main() {
    // 获取当前轨道的元数据
    int metaOffset = trailIndex * 2;
    vec4 color = data[metaOffset];
    vec4 params = data[metaOffset + 1];
    
    int startIndex = int(params.z);
    int pointCount = int(params.w);
    float width = params.x;
    float glowIntensity = params.y;
    
    // gl_InstanceID 是线段索引
    int segmentID = gl_InstanceID;
    
    if (segmentID >= pointCount - 1) {
        gl_Position = vec4(0, 0, 0, 1);
        return;
    }
    
    // 计算全局点索引
    int idx0 = startIndex + segmentID;
    int idx1 = startIndex + segmentID + 1;
    
    vec3 p0 = positions[idx0].xyz;
    vec3 p1 = positions[idx1].xyz;
    float t0 = positions[idx0].w;
    
    // 计算面向相机的Quad
    vec3 segmentDir = normalize(p1 - p0);
    vec3 segmentCenter = (p0 + p1) * 0.5;
    vec3 toCamera = normalize(cameraPos - segmentCenter);
    vec3 right = normalize(cross(segmentDir, toCamera));
    
    vec3 offset = right * quadVertex.x * width;
    vec3 worldPos = mix(p0, p1, quadVertex.y) + offset;
    
    gl_Position = viewProjection * vec4(worldPos, 1.0);
    
    vs_out.uv = vec2(quadVertex.x + 0.5, quadVertex.y);
    vs_out.color = color;
    vs_out.fade = 1.0 - t0;
    vs_out.glow = glowIntensity;
}
