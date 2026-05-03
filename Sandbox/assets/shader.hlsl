struct VSInput {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

struct PSInput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

cbuffer cb0 : register(b0) {
    float time;
}

cbuffer matrices : register(b0, space1) {
    column_major float4x4 model;
    column_major float4x4 view;
    column_major float4x4 proj;
}

cbuffer cb1 : register(b1) {
    uint angle;
}

Texture2D tex : register(t0);

sampler linearSmp : register(s0);


PSInput VSMain(VSInput input) {
    PSInput output;

    float rotation_speed = -0.01f;

    // float cosTheta = cos(angle * rotation_speed);
    // float sinTheta = sin(angle * rotation_speed);

    // float2 rotated_pos;
    // rotated_pos.x = input.position.x * cosTheta - input.position.y * sinTheta;
    // rotated_pos.y = input.position.x * sinTheta + input.position.y * cosTheta;

    // rotated_pos.y += sin(time*2) / 2;
    // rotated_pos.x += cos(time*2) / 2;

    float4 pos = float4(input.position, 1.0f);

    output.position = mul(proj, mul(view, mul(model, pos)));
    output.normal = input.normal;
    output.uv = input.uv;

    return output;
}

// Pixel Shader
float4 PSMain(PSInput input) : SV_TARGET {
    float3 color = input.normal;//float3(input.normal, 0.0);
    
    return float4(input.normal, 1.0);
    // return tex.Sample(linearSmp, input.uv);
}
