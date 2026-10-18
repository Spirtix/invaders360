#include "stdafx.h"
#include <stdio.h>
#include <i8080.h>
//-------------------------------------------------------------------------------------
// Vertex shader
//-------------------------------------------------------------------------------------
const char* g_strVertexShaderProgram = 
" float4x4 matWVP : register(c0);              "  
"                                              "  
" struct VS_IN                                 "  
" {                                            " 
"     float4 ObjPos   : POSITION;              "
"     float2 Tex      : TEXCOORD0;             "
" };                                           "
"                                              "
" struct VS_OUT                                " 
" {                                            " 
"     float4 ProjPos  : POSITION;              "
"     float2 Tex      : TEXCOORD0;             "
" };                                           "  
"                                              "  
" VS_OUT main( VS_IN In )                      "  
" {                                            "  
"     VS_OUT Out;                              "  
"     Out.ProjPos = mul( matWVP, In.ObjPos );  "
"     Out.Tex = In.Tex;                        "
"     return Out;                              "
" }                                            ";

//-------------------------------------------------------------------------------------
// Pixel shader
//-------------------------------------------------------------------------------------
const char* g_strPixelShaderProgram = 
" sampler2D s0 : register(s0);                 "
" struct PS_IN                                 "
" {                                            "
"     float2 Tex : TEXCOORD0;                  "                  
" };                                           "
"                                              "  
" float4 main( PS_IN In ) : COLOR              "  
" {                                            "  
"     return tex2D( s0, In.Tex );              "
" }                                            "; 

//-------------------------------------------------------------------------------------
// Structure to hold vertex data.
//-------------------------------------------------------------------------------------
struct TEXVERTEX
{
    float Position[3];
    float Tex[2];
};

//-------------------------------------------------------------------------------------
// Time             Since fAppTime is a float, we need to keep the quadword app time 
//                  as a LARGE_INTEGER so that we don't lose precision after running
//                  for a long time.
//-------------------------------------------------------------------------------------
struct TimeInfo
{    
    LARGE_INTEGER qwTime;    
    LARGE_INTEGER qwAppTime;   

    float fAppTime;    
    float fElapsedTime;    

    float fSecsPerTick;    
};

//-------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------
D3DDevice*             g_pd3dDevice;    // Rendering device
D3DVertexBuffer*       g_pVB;           // Buffer to hold vertices
D3DVertexDeclaration*  g_pVertexDecl;   // Vertex format decl
D3DVertexShader*       g_pVertexShader; // Vertex Shader
D3DPixelShader*        g_pPixelShader;  // Pixel Shader
D3DTexture* g_pTexture;

const UINT SI_WIDTH  = 224;
const UINT SI_HEIGHT = 256;

XMMATRIX g_matWorld;
XMMATRIX g_matProj;
XMMATRIX g_matView;

TimeInfo g_Time;

BOOL g_bWidescreen = TRUE;
UCHAR memory[16384];

void InitTime()
{    
    // Get the frequency of the timer
    LARGE_INTEGER qwTicksPerSec;
    QueryPerformanceFrequency( &qwTicksPerSec );
    g_Time.fSecsPerTick = 1.0f / (float)qwTicksPerSec.QuadPart;

    // Save the start time
    QueryPerformanceCounter( &g_Time.qwTime );
    
    // Zero out the elapsed and total time
    g_Time.qwAppTime.QuadPart = 0;
    g_Time.fAppTime = 0.0f; 
    g_Time.fElapsedTime = 0.0f;    
}

HRESULT InitD3D()
{
    // Create the D3D object.
    Direct3D* pD3D = Direct3DCreate9( D3D_SDK_VERSION );
    if( !pD3D )
        return E_FAIL;

    // Set up the structure used to create the D3DDevice.
    D3DPRESENT_PARAMETERS d3dpp; 
    ZeroMemory( &d3dpp, sizeof(d3dpp) );
    XVIDEO_MODE VideoMode;
    XGetVideoMode( &VideoMode );
    g_bWidescreen = VideoMode.fIsWideScreen;
    d3dpp.BackBufferWidth        = min( VideoMode.dwDisplayWidth, 1280 );
    d3dpp.BackBufferHeight       = min( VideoMode.dwDisplayHeight, 720 );
    d3dpp.BackBufferFormat       = D3DFMT_X8R8G8B8;
    d3dpp.BackBufferCount        = 1;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
    d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_ONE;

    // Create the Direct3D device.
    if( FAILED( pD3D->CreateDevice( 0, D3DDEVTYPE_HAL, NULL,
                                    D3DCREATE_HARDWARE_VERTEXPROCESSING,
                                    &d3dpp, &g_pd3dDevice ) ) )
        return E_FAIL;

    return S_OK;
}

HRESULT InitScene()
{
    // Compile vertex shader.
    ID3DXBuffer* pVertexShaderCode;
    ID3DXBuffer* pVertexErrorMsg;
    HRESULT hr = D3DXCompileShader( g_strVertexShaderProgram, 
                                    (UINT)strlen( g_strVertexShaderProgram ),
                                    NULL, 
                                    NULL, 
                                    "main", 
                                    "vs_2_0", 
                                    0, 
                                    &pVertexShaderCode, 
                                    &pVertexErrorMsg, 
                                    NULL );
    if( FAILED(hr) )
    {
        if( pVertexErrorMsg )
            OutputDebugString( (char*)pVertexErrorMsg->GetBufferPointer() );
        return E_FAIL;
    }    

    // Create vertex shader.
    g_pd3dDevice->CreateVertexShader( (DWORD*)pVertexShaderCode->GetBufferPointer(), 
                                      &g_pVertexShader );

    // Compile pixel shader.
    ID3DXBuffer* pPixelShaderCode;
    ID3DXBuffer* pPixelErrorMsg;
    hr = D3DXCompileShader( g_strPixelShaderProgram, 
                            (UINT)strlen( g_strPixelShaderProgram ),
                            NULL, 
                            NULL, 
                            "main", 
                            "ps_2_0", 
                            0, 
                            &pPixelShaderCode, 
                            &pPixelErrorMsg,
                            NULL );
    if( FAILED(hr) )
    {
        if( pPixelErrorMsg )
            OutputDebugString( (char*)pPixelErrorMsg->GetBufferPointer() );
        return E_FAIL;
    }

    // Create pixel shader.
    g_pd3dDevice->CreatePixelShader( (DWORD*)pPixelShaderCode->GetBufferPointer(), 
                                     &g_pPixelShader );
    
    // Define the vertex elements and
    // Create a vertex declaration from the element descriptions.
    D3DVERTEXELEMENT9 VertexElements[3] =
    {
		{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
    };
    g_pd3dDevice->CreateVertexDeclaration( VertexElements, &g_pVertexDecl );

    // Create the vertex buffer. Here we are allocating enough memory
    // (from the default pool) to hold all our 3 custom vertices. 
    if( FAILED( g_pd3dDevice->CreateVertexBuffer( 6*sizeof(TEXVERTEX),
                                                  D3DUSAGE_WRITEONLY, 
                                                  NULL,
                                                  D3DPOOL_MANAGED, 
                                                  &g_pVB, 
                                                  NULL ) ) )
        return E_FAIL;

    // Now we fill the vertex buffer. To do this, we need to Lock() the VB to
    // gain access to the vertices. This mechanism is required because the
    // vertex buffer may still be in use by the GPU. This can happen if the
    // CPU gets ahead of the GPU. The GPU could still be rendering the previous
    // frame.
    TEXVERTEX g_Vertices[] =
    {
		{ -1.0f, -1.0f, 0.0f,  0.0f, 1.0f },
		{ -1.0f,  1.0f, 0.0f,  0.0f, 0.0f },
		{  1.0f,  1.0f, 0.0f,  1.0f, 0.0f },
		{  1.0f,  1.0f, 0.0f,  1.0f, 0.0f },
		{  1.0f, -1.0f, 0.0f,  1.0f, 1.0f },
		{ -1.0f, -1.0f, 0.0f,  0.0f, 1.0f },
    };

    TEXVERTEX* pVertices;
    if( FAILED( g_pVB->Lock( 0, 0, (void**)&pVertices, 0 ) ) )
        return E_FAIL;
    memcpy( pVertices, g_Vertices, 6*sizeof(TEXVERTEX) );
    g_pVB->Unlock();

    // Initialize the projection matrix
    FLOAT fAspect = ( g_bWidescreen ) ? (16.0f / 9.0f) : (4.0f / 3.0f); 
    g_matProj = XMMatrixPerspectiveFovLH( XM_PIDIV4, fAspect, 1.0f, 200.0f );

    // Initialize the view matrix
    XMVECTOR vEyePt    = { 0.0f, 0.0f,-7.0f, 0.0f };
    XMVECTOR vLookatPt = { 0.0f, 0.0f, 0.0f, 0.0f };
    XMVECTOR vUp       = { 0.0f, 1.0f, 0.0f, 0.0f };
    g_matView = XMMatrixLookAtLH( vEyePt, vLookatPt, vUp );

	if (FAILED(g_pd3dDevice->CreateTexture( SI_WIDTH, SI_HEIGHT, 1, 0,
                                         D3DFMT_LIN_A8R8G8B8, 0,
                                         &g_pTexture, NULL)))
		return E_FAIL;


	g_matWorld = XMMatrixScaling((float)SI_WIDTH / SI_HEIGHT, 1.0f, 1.0f);

    return S_OK;
}

void UpdateTime()
{
    LARGE_INTEGER qwNewTime;
    LARGE_INTEGER qwDeltaTime;
    
    QueryPerformanceCounter( &qwNewTime );    
    qwDeltaTime.QuadPart = qwNewTime.QuadPart - g_Time.qwTime.QuadPart;

    g_Time.qwAppTime.QuadPart += qwDeltaTime.QuadPart;    
    g_Time.qwTime.QuadPart     = qwNewTime.QuadPart;
    
    g_Time.fElapsedTime      = g_Time.fSecsPerTick * ((FLOAT)(qwDeltaTime.QuadPart));
    g_Time.fAppTime          = g_Time.fSecsPerTick * ((FLOAT)(g_Time.qwAppTime.QuadPart));    
}

void Update()
{
}

void Render()
{

	D3DLOCKED_RECT lr;
    if (FAILED(g_pTexture->LockRect(0, &lr, NULL, 0 )))
        return;

    for (int x = 0; x < 224; x++)
    {
        for (int yb = 0; yb < 32; yb++)
        {
            BYTE b = memory[0x2400 + x * 32 + yb];
            for( int bit = 0; bit < 8; bit++ )
            {
                int y = 255 - (yb * 8 + bit);
                DWORD* row = (DWORD*)((BYTE*)lr.pBits + y * lr.Pitch);
                row[x] = ((b >> bit) & 1) ? 0xFFFFFFFF : 0xFF000000;
            }
        }
    }

    g_pTexture->UnlockRect(0);
    // Clear the backbuffer to a blue color
    g_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, 
                         D3DCOLOR_XRGB(0,0,0), 1.0f, 0L );

    // Draw the triangles in the vertex buffer. This is broken into a few steps:
    
    // We are passing the vertices down a "stream", so first we need
    // to specify the source of that stream, which is our vertex buffer. 
    // Then we need to let D3D know what vertex and pixel shaders to use. 
    g_pd3dDevice->SetVertexDeclaration( g_pVertexDecl );
    g_pd3dDevice->SetStreamSource( 0, g_pVB, 0, sizeof(TEXVERTEX) );
    g_pd3dDevice->SetVertexShader( g_pVertexShader );
    g_pd3dDevice->SetPixelShader( g_pPixelShader );
   
    // Build the world-view-projection matrix and pass it into the vertex shader
    XMMATRIX matWVP = g_matWorld * g_matView * g_matProj;
    g_pd3dDevice->SetVertexShaderConstantF( 0, (FLOAT*)&matWVP, 4);

	g_pd3dDevice->SetTexture(0, g_pTexture);
	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP);
	g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP);

    // Draw the vertices in the vertex buffer
    g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);

    // Present the backbuffer contents to the display
    g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}

static UCHAR readByte(void* userdata, USHORT addr) {
  if (addr >= 0x6000)
    return 0;
  if (addr >= 0x4000)
    addr -= 0x2000;

  return memory[addr];
}

static void writeByte(void* userdata, USHORT addr, UCHAR val) {
  if (addr >= 0x2000 && addr < 0x4000) {
    memory[addr] = val;
  }
}

static UCHAR portIn(void* userdata, UCHAR port) {
	return 0;
}

static void portOut(void* userdata, UCHAR port, UCHAR value) {
	return;
}

void __cdecl main()
{
    if( FAILED(InitD3D()))
        return;

    if (FAILED(InitScene()))
        return;

    InitTime();

	FILE* f = fopen("game:\\invaders.rom", "rb");
	if (!f) return;

	memset(memory, 0, 16384);
	size_t n = fread(memory, 1, sizeof(memory), f);
	fclose(f);

	i8080 cpu;
	i8080_init(&cpu);
	cpu.read_byte = readByte;
	cpu.write_byte = writeByte;
	cpu.port_in = portIn;
	cpu.port_out = portOut;

	const int CYCLES = 30000;
    for (;;)
    {
        UpdateTime();
	
		int cycles = 0;
		while (cycles < CYCLES) {
			++cycles;
			i8080_step(&cpu);
		}
		i8080_interrupt(&cpu, 0xCF);
		i8080_interrupt(&cpu, 0xD7);

        Render();
    }
}

