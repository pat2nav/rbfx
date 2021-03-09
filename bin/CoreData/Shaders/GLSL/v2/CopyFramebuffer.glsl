#define URHO3D_GEOMETRY_STATIC

#include "_Config.glsl"
#include "_Uniforms.glsl"
#include "_GammaCorrection.glsl"
#include "_VertexLayout.glsl"
#include "_VertexTransform.glsl"
#include "_VertexScreenPos.glsl"
#include "_PixelOutput.glsl"

uniform sampler2D sDiffMap;

VERTEX_OUTPUT(vec2 vScreenPos)

#ifdef URHO3D_VERTEX_SHADER
void main()
{
    VertexTransform vertexTransform = GetVertexTransform();
    gl_Position = GetClipPos(vertexTransform.position.xyz);
    vScreenPos = GetScreenPosPreDiv(gl_Position);
}
#endif

#ifdef URHO3D_PIXEL_SHADER
void main()
{
    vec4 color = texture2D(sDiffMap, vScreenPos);
    #if defined(URHO3D_GAMMA_TO_LINEAR)
        gl_FragColor = GammaToLinearSpaceAlpha(color);
    #elif defined(URHO3D_LINEAR_TO_GAMMA)
        gl_FragColor = LinearToGammaSpaceAlpha(color);
    #else
        gl_FragColor = color;
    #endif
}
#endif

