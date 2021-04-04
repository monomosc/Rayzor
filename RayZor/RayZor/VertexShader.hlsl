
struct VertexInputType
{
	float4 position : POSITION;
	float4 normal : NORMAL;
};
struct PixelInputType
{
	float4 position: SV_POSITION;
	float4 color: COLOR;
	float4 normal: NORMAL;
};

cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
	float4 extra_params[48];
};


PixelInputType main(VertexInputType input)
{
		PixelInputType output;
		// Change the position vector to be 4 units for proper matrix calculations.
		input.position.w = 1.0f;

		// Calculate the position of the vertex against the world, view, and projection matrices.
		output.position = mul(input.position, worldMatrix);
		output.position = mul(output.position, viewMatrix);
		output.position = mul(output.position, projectionMatrix);


		output.normal = mul(input.normal, worldMatrix);
		output.color = extra_params[0];
		return output;
}