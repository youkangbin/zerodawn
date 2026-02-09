#version 430 core

in VS_OUT {
    vec2 uv;
    vec4 color;
    float fade;
    float glow;
} fs_in;

out vec4 fragColor;

uniform sampler1D gradientTex;

void main() {
    // 横向渐变（边缘柔化）
    float radialFade = texture(gradientTex, fs_in.uv.x).r;
    
    // 纵向渐变（轨道淡出）
    float lengthFade = fs_in.fade;
    
    // 发光效果
    vec3 baseColor = fs_in.color.rgb;
    vec3 glowColor = baseColor * (1.0 + fs_in.glow * lengthFade);
    
    // 最终颜色
    float alpha = radialFade * lengthFade * 0.8;
    fragColor = vec4(glowColor, alpha);
}
