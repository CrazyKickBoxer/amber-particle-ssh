#version 450 core

in vec4 vColor;
in vec2 vTexCoord;

out vec4 fragColor;

uniform float glowIntensity;
uniform float uBrightness;
uniform float uVibrance; // NEW
uniform float uScanlineIntensity;
uniform vec3 uColorTint;
uniform int uTheme;
uniform vec2 uResolution;

void main() {
    vec2 coord = vTexCoord - vec2(0.5);
    float dist = length(coord);
    
    if (dist > 0.5) discard;
    
    // === CRT DOT MATRIX ===
    // Sharp bright center dot + subtle phosphor glow
    
    // Sharp core (the actual dot)
    float core = smoothstep(0.25, 0.1, dist);
    
    // Soft phosphor glow around it
    float glow = exp(-dist * 8.0) * 0.5;
    
    float intensity = (core + glow) * glowIntensity;
    
    fragColor = vColor * intensity;
    
    // === THEMES ===
    // 1. Color Tint (Retro/Global)
    fragColor.rgb *= uColorTint;
    
    // Saturation / Vibrance
    float gray = dot(fragColor.rgb, vec3(0.299, 0.587, 0.114));
    fragColor.rgb = mix(vec3(gray), fragColor.rgb, uVibrance);
    
    fragColor.rgb *= uBrightness;
    
    // 2. Scanlines
    if (uScanlineIntensity > 0.001) {
        // High frequency sine wave (every 2-3 pixels)
        float scanline = 0.5 + 0.5 * sin(gl_FragCoord.y * 1.5);
        fragColor.rgb *= mix(1.0, scanline, uScanlineIntensity);
    }
    
    // 3. Synthwave Gradient (Theme 2)
    if (uTheme == 2) {
       // Purple top (t=1), Orange bottom (t=0)
       // Add slight curvature?
       float t = gl_FragCoord.y / uResolution.y;
       vec3 topColor = vec3(0.7, 0.0, 1.0); // Purple
       vec3 botColor = vec3(1.0, 0.2, 0.0); // Orange
       
       // Use lighter mix for text legibility
       vec3 grad = mix(botColor, topColor, t * 1.2 - 0.1);
       fragColor.rgb *= grad * 2.0; // Boost brightness standard
    }
}
