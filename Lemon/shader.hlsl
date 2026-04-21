struct VSInput {
    float2 position : POSITION;
    float3 color : COLOR;
};

struct PSInput {
    float4 position : SV_POSITION;
    float3 color : COLOR;
};

cbuffer cb0 : register(b0) {
    uint angle;
};

PSInput VSMain(VSInput input) {
    PSInput output;

    float rotation_speed = -0.01f;

    float cosTheta = cos(angle * rotation_speed);
    float sinTheta = sin(angle * rotation_speed);

    float2 rotated_pos;
    rotated_pos.x = input.position.x * cosTheta - input.position.y * sinTheta;
    rotated_pos.y = input.position.x * sinTheta + input.position.y * cosTheta;

    output.position = float4(rotated_pos, 0.0f, 1.0f);
    output.color = input.color;

    return output;
}

// Pixel Shader
float4 PSMain(PSInput input) : SV_TARGET {
    return float4(input.color.r, input.color.g, input.color.b, 1.0);
}