#version 330 core

uniform sampler2D uScene;          // framebuffer texture
uniform vec2  uPulseCenter;        // UV space (0–1)
uniform float uPulseRadius;        // radius of the pulse
uniform float uPulseStrength;      // distortion strength
uniform float uPulseWidth;         // optional: pulse thickness
uniform int uPulseActive;       // is pulse active

uniform vec2 uResolution;        // screen resolution

in vec2 TexCoord;
out vec4 FragColor;

void main()
{
    // Convert TexCoord (0–1) to screen space if needed
    vec2 fragPos = gl_FragCoord.xy;

    fragPos.y = uResolution.y - fragPos.y; // Flip Y for UV space

    // Distance from pulse center (in UV space)
    float d = distance(fragPos, uPulseCenter);

    // Smooth pulse: 0 outside ring, 1 at center of ring
    // Use smoothstep to interpolate from 0 to 1 inside the ring
    float halfWidth = uPulseWidth * 0.5;
    float pulse = smoothstep(uPulseRadius - halfWidth, uPulseRadius, d) - 
                  smoothstep(uPulseRadius, uPulseRadius + halfWidth, d);

    // Distortion intensity
    float distort = pulse * uPulseStrength;

    // Direction away from center
    vec2 dir = normalize(fragPos - uPulseCenter);

    // Apply smooth displacement
    vec2 displacedUV = TexCoord;
    if(uPulseActive == 1) 
    {
    displacedUV = displacedUV + dir * distort;
    } 

    // Sample final color
    FragColor = texture(uScene, displacedUV);
}
