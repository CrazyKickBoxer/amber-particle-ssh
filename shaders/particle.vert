#version 450 core

layout(location = 0) in vec2 inPos;       // Quad vertex position (0..1)
layout(location = 1) in vec3 inInstancePos; // Per-instance position
layout(location = 2) in float inSize;       // Per-instance size
layout(location = 3) in vec4 inColor;       // Per-instance color
layout(location = 4) in vec4 inExtra;       // Pulse, Flicker, Radius, Unused

out vec4 vColor;
out vec2 vTexCoord;

uniform mat4 projection;
uniform float elapsedTime; // For animation
uniform float uShimmerSpeed;

// Pseudo-random noise
float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main() {
    vTexCoord = inPos;
    
    float pulse = inExtra.x;
    float flicker = inExtra.y; // Seed from C++
    
    // Time wrapper to preserve precision
    float time = mod(elapsedTime, 100.0);
    
    // NEW SHIMMER LOGIC
    // Base glow pulsing
    float speed = max(0.5, uShimmerSpeed); // Ensure non-zero
    float wave = sin(time * speed + pulse * 10.0); // Offset by particle ID
    
    // Organic Pulse: Normalize sin to 0.7 - 1.3
    float brightness = 1.0 + wave * 0.3; 
    
    // Occasional Sparkle (High frequency noise)
    if (uShimmerSpeed > 2.0) {
        float spark = rand(vec2(time * speed * 5.0, flicker));
        if (spark > 0.95) brightness += 0.5; // Sparkle
    }
    
    float finalBrightness = brightness;
    
    // Boost Color
    vColor = inColor * finalBrightness * 1.3; // 130% brightness boost
    
    // Tiny Jitter (Arcing Movement) - Sub-pixel only
    // "Staying within confines" -> very small amplitude
    vec2 jitter = vec2((rand(vec2(time * 5.0, flicker)) - 0.5), 
                       (rand(vec2(time * 5.0, pulse)) - 0.5)) * inSize * 0.1;

    vec2 pos = inPos * inSize + jitter; 
    vec3 finalPos = inInstancePos + vec3(pos, 0.0);
    
    gl_Position = projection * vec4(finalPos, 1.0);
}
