struct VSInput {
    float2 position : POSITION;
    float3 color : COLOR;
};

struct PSInput {
    float4 position : SV_POSITION;
    float3 color : COLOR;
};

cbuffer cb0 : register(b0) {
    float time;
}

cbuffer cb1 : register(b1) {
    uint angle;
}
float3 HSVtoRGB(float3 hsv)
{
    float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    float3 p = abs(frac(hsv.xxx + K.xyz) * 6.0 - K.www);
    return hsv.z * lerp(K.xxx, saturate(p - K.xxx), hsv.y);
}

PSInput VSMain(VSInput input) {
    PSInput output;

    float rotation_speed = -0.01f;

    float cosTheta = cos(angle * rotation_speed);
    float sinTheta = sin(angle * rotation_speed);

    float2 rotated_pos;
    rotated_pos.x = input.position.x * cosTheta - input.position.y * sinTheta;
    rotated_pos.y = input.position.x * sinTheta + input.position.y * cosTheta;

    rotated_pos.y += sin(time*2) / 2;
    rotated_pos.x += cos(time*2) / 2;

    output.position = float4(rotated_pos, 0.0f, 1.0f);
    output.color = input.color;

    return output;
}

// Pixel Shader
float4 PSMain(PSInput input) : SV_TARGET {
    float3 color = input.color;
    color *= HSVtoRGB(float3(fmod(time/10, 1), 1.0, 1));
    return float4(color, 1.0);
}
