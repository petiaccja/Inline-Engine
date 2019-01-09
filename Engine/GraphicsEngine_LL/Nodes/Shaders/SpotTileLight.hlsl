/*
* TileSpotlightCulling.hlsl
* 2018/2/3 1:16 Xiaoyi.
*/

#ifndef MAX_NUM_LIGHT_PRE_TILE
#define MAX_NUM_LIGHT_PRE_TILE 64
#endif
#ifndef THREAD_NUM_X           
#define THREAD_NUM_X           32
#endif
#ifndef THREAD_NUM_Y      
#define THREAD_NUM_Y           32
#endif
#ifndef SPOT_LIGHT_ARRAY_SIZE   
#define SPOT_LIGHT_ARRAY_SIZE  1024
#endif
//< Macros   TILE_LIGHT_MAY_EXCEED>
#ifdef  USING_MSAA_RENDER
void GetPixelDepth(in Texture2D T2D,in uint2 ThreadID,out float emax,out float emin)
{
    uint3 T2DInfo;
    T2D.GetDimensions(T2DInfo.x,T2DInfo.y,T2DInfo.z);
    emax=0.0f;
    emin=3.402823466e+38F;
    for(uint i=0;i<T2DInfo.z;++i)
    {
        float Depth=T2D.Load(ThreadID,i).x;
        if(Depth!=0.f){
            emax=max(emax,Depth);
            emin=min(emin,Depth);
        }
    }
}
#endif
#ifndef USING_MSAA_RENDER
void GetPixelDepth(in Texture2D T2D,in uint2 ThreadID,out float eDepth)
{
    eDepth=T2D[ThreadID].x;
}
#endif
#define FLT_MAX    0x7f7fffff
struct Spotlight
{
    float4 LightPosAndLimit;
    float4 LightDirect;
    float2 CosAngle_TanAngle;
    float2 Padding;
};
cbuffer CPU_Update:register(b0,space0)
{
    uint   SpotLightNum;
    uint3  TexureSizeOfPretile;
    float4x4 ProjectionInv;
};
ConstantBuffer<Spotlight> mSpotLight[SPOT_LIGHT_ARRAY_SIZE]:register(b1,space0);
RWTexture2D<uint>  TileLightNum:register(u0,space0);
RWTexture2D<uint>  TileLightIndex:register(u1,space0);
#ifdef  USING_MSAA_RENDER
Texture2DMS DepthTexture:register(t0,space0);
#endif
#ifndef USING_MSAA_RENDER
Texture2D DepthTexture:register(t0,space0);
#endif
groupshared uint LightCount=0;
groupshared uint TileMax= FLT_MAX;
groupshared uint TileMin=0;
groupshared uint2 ThreadMax;//for finding view space postion
groupshared uint2 ThreadMin;
//Need Fast sine or cosine
bool IntersectShpereAndCone(in float3 eNormal,in float3 eCone,in float cosleAngle,in float tanAngle,in float Range,in float3 eSphere,in float eRadious)
{   
    bool hit=false;
    float3 eSpheretoCone=eSphere-eCone;
    float eSpheretoConeSq=dot(eSpheretoCone,eSpheretoCone);
    float eProjection= dot(eSpheretoCone, eNormal);
    if(eProjection + eRadious>Range || eProjection < - eRadious);
        return hit;
    float Temp1=sqrt(eSpheretoConeSq-eProjection*eProjection);
    float Temp2=eProjection * tanAngle;
    float eDist=cosleAngle*abs(Temp1-Temp2);
    hit = (eDist < eRadious)||((eDist > eRadious)&&(Temp1<Temp2));
    return hit;
}
void  GetViewPos(inout float4 Pos)
{
	Pos = mul(Pos, ProjectionInv);
	Pos /= Pos.w;
}
[numthreads(THREAD_NUM_X, THREAD_NUM_Y, 1)]
void main(uint3 DispatchID:SV_DISPATCHTHREADID,uint3 GThreadID:SV_GROUPTHREADID,uint3 TileIndex:SV_GROUPID)
{
    TileLightNum[GThreadID.xy]=0;
    //Calulate depth range
#ifdef USING_MSAA_RENDER //MSAA
    float emax;float emin;
    GetPixelDepth(DepthTexture,DispatchID.xy,emax,emin);
	InterlockedMax(TileMax, asuint(emax));
	InterlockedMin(TileMin, asuint(emin));
    GroupMemoryBarrierWithGroupSync();
    if(TileMax==asuint(emax))
        ThreadMax=DispatchID.xy;
    if(TileMax==asuint(emin))
        ThreadMin=DispatchID.xy;
groupshared uint ThreadY;
#endif
#ifndef USING_MSAA_RENDER //NOMSAA
	float eDepth;
	GetPixelDepth(DepthTexture, DispatchID.xy, eDepth);
	InterlockedMax(TileMax, asuint(eDepth));
	InterlockedMin(TileMin, asuint(eDepth));
    if(TileMax==asuint(eDepth))
        ThreadMax=DispatchID.xy;
    if(TileMax==asuint(eDepth))
        ThreadMin=DispatchID.xy;
#endif
    GroupMemoryBarrierWithGroupSync();
    //Get Bounding sphere
    float4 Spherecentre;
    uint2 Dimension;
    DepthTexture.GetDimensions(Dimension.x,Dimension.y);
    float4 MaxPos=float4(float(ThreadMax.x)/Dimension.x-0.5f,-float(ThreadMax.y)/Dimension.y+0.5f,TileMax,1);
	GetViewPos(MaxPos);
    float4 MinPos= float4(float(ThreadMin.x)/Dimension.x-0.5f,-float(ThreadMin.y)/Dimension.y+0.5f,TileMin,1);
    GetViewPos(MinPos);
    float4 Centre=(MaxPos+MinPos)/2;
    //Get Bounding sphere
    float Radious=(MaxPos.x-MaxPos.y)/2;
    //Intersect sphere and cone
    uint PerthreadProceeNum=ceil(float(SpotLightNum)/(THREAD_NUM_X*THREAD_NUM_X));
    uint eLightIndex=GThreadID.y*PerthreadProceeNum+GThreadID.x;
    TileLightNum[GThreadID.xy]=0;
    AllMemoryBarrierWithGroupSync();
    while(eLightIndex<SpotLightNum)
    {
        InterlockedAdd(LightCount,1);
        eLightIndex++;
        if(IntersectShpereAndCone(
            mSpotLight[eLightIndex].LightDirect.xyz,
            mSpotLight[eLightIndex].LightPosAndLimit.xyz,
            mSpotLight[eLightIndex].CosAngle_TanAngle.x,
            mSpotLight[eLightIndex].CosAngle_TanAngle.y,
            mSpotLight[eLightIndex].LightPosAndLimit.w,Centre.xyz,Radious))
            {
                uint TexureLightCount;
                //AddTileNum
                InterlockedAdd(TileLightNum[GThreadID.xy],1,TexureLightCount);
                //AddIndex,Transform to Texture Coordinate
                //Tcoord should't exceed  range of TexureSizeOfPretile.xy ,Or it will overwrite others tile 
                uint2 Tcoord=uint2(TexureLightCount%TexureSizeOfPretile.x,TexureLightCount/TexureSizeOfPretile.x);
#ifdef   TILE_LIGHT_MAY_EXCEED
                if(Tcoord.y>=TexureSizeOfPretile.y)
                    Tcoord.y=TexureSizeOfPretile.y-1;
#endif
                //Transform to global TextureCoord
                Tcoord =GThreadID.xy*TexureSizeOfPretile.xy+Tcoord;
                //Save Light Index
                TileLightIndex[Tcoord]=eLightIndex;

            }
    }
}