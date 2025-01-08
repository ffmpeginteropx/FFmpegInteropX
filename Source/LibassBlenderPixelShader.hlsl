struct PSInput
{
    float4 pos : SV_POSITION; 
    float2 tex : TEXCOORD; 
};

Texture2D inputTexture : register(t0); 
SamplerState samplerState : register(s0); 

float4 main(PSInput input) : SV_Target
{
    float4 color = inputTexture.Sample(samplerState, input.tex);

    return color;
}
