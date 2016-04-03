float4x4 world_view_proj_matrix;
float4 Light_Ambient;
float4 Light1_Position;
float4x4 inv_world_matrix;


struct VS_OUTPUT 
{
   float4 position: POSITION;
   float3 color: COLOR;
};

struct VS_INPUT
{
	float4 position: POSITION;
	float3 normal: NORMAL;
};

VS_OUTPUT vs_main(VS_INPUT input)
{
   VS_OUTPUT Out;

   // Compute the projected position and send out the texture coordinates
   Out.position = mul(input.position,world_view_proj_matrix );

   // the normals might not be normalised (paraniod)
   float4 normal=normalize(float4(input.normal, 1.0));

   // Start with the ambient color
   float4 Color = Light_Ambient;


   // Determine the light vector
   // first get the light vector in object space
   float4 obj_light=mul(Light1_Position,inv_world_matrix);
   float4 LightDir = normalize(obj_light - input.position);


   // Diffuse using Lambert
   float DiffuseAttn = max(0, dot(normal, LightDir) );

   // Compute final lighting
   // assume white light
   float4 light = float4(0.8,0.8,0.8,1);
   Color += light*DiffuseAttn;

   // Output Final Color
   Out.color=Color;

   return Out;
}
 
float4 ps_main(VS_OUTPUT output) : COLOR 
{
   return float4(output.color, 1.0);
}
 
technique Diffuse
{
    pass P0
    {
        vertexShader = compile vs_2_0 vs_main();
        pixelShader = compile ps_2_0 ps_main();
    }
}
