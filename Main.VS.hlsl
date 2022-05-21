struct VSInput
{
    float4 position : POSITION0;
    float4 color : COLOR0;
};

struct VSOutput
{
    float4 position : SV_Position;
    float4 color : COLOR0;
};

VSOutput main(VSInput input)
{
    VSOutput output;

    output.position = input.position;
    output.color = input.color;

    return output;
}