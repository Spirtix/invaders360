#include "stdafx.h"
#include "gameScreenRenderer.h"
#include "shaders.h"

HRESULT gameScreenRenderer::init() {
    HRESULT res;
    if (FAILED(res = initD3D()))
        return res;
    return initScene();
}

gameScreenRenderer::~gameScreenRenderer() {
    if (texture) texture->Release();
    if (VB) VB->Release();
    if (vertexDecl) vertexDecl->Release();
    if (pShader) pShader->Release();
    if (vShader) vShader->Release();
    if (d3dDevice) d3dDevice->Release();
}

void gameScreenRenderer::render(uint8_t* vram) {
    D3DLOCKED_RECT lr;
    if (FAILED(texture->LockRect(0, &lr, NULL, 0 )))
        return;

    for (int x = 0; x < SI_WIDTH; x++)
    {
        for (int yb = 0; yb < 32; yb++)
        {
            BYTE b = vram[x * 32 + yb];
            for( int bit = 0; bit < 8; bit++ )
            {
                int y = 255 - (yb * 8 + bit);
                DWORD* row = (DWORD*)((BYTE*)lr.pBits + y * lr.Pitch);
                row[x] = ((b >> bit) & 1) ? 0xFFFFFFFF : 0xFF000000;
            }
        }
    }

    texture->UnlockRect(0);
    // Clear the backbuffer to a blue color
    d3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, 
                         D3DCOLOR_XRGB(0,0,0), 1.0f, 0L );

    // Draw the triangles in the vertex buffer. This is broken into a few steps:
    
    // We are passing the vertices down a "stream", so first we need
    // to specify the source of that stream, which is our vertex buffer. 
    // Then we need to let D3D know what vertex and pixel shaders to use. 
    d3dDevice->SetVertexDeclaration(vertexDecl);
    d3dDevice->SetStreamSource( 0, VB, 0, sizeof(TEXVERTEX));
    d3dDevice->SetVertexShader(vShader);
    d3dDevice->SetPixelShader(pShader);
   
    // Build the world-view-projection matrix and pass it into the vertex shader
    XMMATRIX matWVP = matWorld * matView * matProj;
    d3dDevice->SetVertexShaderConstantF( 0, (FLOAT*)&matWVP, 4);

	d3dDevice->SetTexture(0, texture);
	d3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	d3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	d3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU,  D3DTADDRESS_CLAMP);
	d3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV,  D3DTADDRESS_CLAMP);

    // Draw the vertices in the vertex buffer
    d3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);

    // Present the backbuffer contents to the display
    d3dDevice->Present(NULL, NULL, NULL, NULL);
}

HRESULT gameScreenRenderer::initD3D()
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
    widescreen = VideoMode.fIsWideScreen;
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
                                    &d3dpp, &d3dDevice ) ) )
        return E_FAIL;

    return S_OK;
}

HRESULT gameScreenRenderer::initScene()
{
    // Compile vertex shader.
    ID3DXBuffer* pVertexShaderCode;
    ID3DXBuffer* pVertexErrorMsg;
    HRESULT hr = D3DXCompileShader(srcVertexShader, 
                                    (UINT)strlen(srcVertexShader),
                                    NULL, 
                                    NULL, 
                                    "main", 
                                    "vs_2_0", 
                                    0, 
                                    &pVertexShaderCode, 
                                    &pVertexErrorMsg, 
                                    NULL);
    if( FAILED(hr) )
    {
        if( pVertexErrorMsg )
            OutputDebugString( (char*)pVertexErrorMsg->GetBufferPointer() );
        return E_FAIL;
    }    

    // Create vertex shader.
    d3dDevice->CreateVertexShader( (DWORD*)pVertexShaderCode->GetBufferPointer(), 
                                      &vShader );

    // Compile pixel shader.
    ID3DXBuffer* pPixelShaderCode;
    ID3DXBuffer* pPixelErrorMsg;
    hr = D3DXCompileShader(srcPixelShader,
                            (UINT)strlen(srcPixelShader),
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
    d3dDevice->CreatePixelShader( (DWORD*)pPixelShaderCode->GetBufferPointer(), 
                                     &pShader );
    
    // Define the vertex elements and
    // Create a vertex declaration from the element descriptions.
    D3DVERTEXELEMENT9 VertexElements[3] =
    {
		{ 0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
    };
    d3dDevice->CreateVertexDeclaration(VertexElements, &vertexDecl);

    // Create the vertex buffer. Here we are allocating enough memory
    // (from the default pool) to hold all our 3 custom vertices. 
    if(FAILED(d3dDevice->CreateVertexBuffer(6*sizeof(TEXVERTEX),
                                                  D3DUSAGE_WRITEONLY, 
                                                  NULL,
                                                  D3DPOOL_MANAGED, 
                                                  &VB, 
                                                  NULL)))
        return E_FAIL;

    // Now we fill the vertex buffer. To do this, we need to Lock() the VB to
    // gain access to the vertices. This mechanism is required because the
    // vertex buffer may still be in use by the GPU. This can happen if the
    // CPU gets ahead of the GPU. The GPU could still be rendering the previous
    // frame.
    TEXVERTEX vertices[] =
    {
		{ -1.0f, -1.0f, 0.0f,  0.0f, 1.0f },
		{ -1.0f,  1.0f, 0.0f,  0.0f, 0.0f },
		{  1.0f,  1.0f, 0.0f,  1.0f, 0.0f },
		{  1.0f,  1.0f, 0.0f,  1.0f, 0.0f },
		{  1.0f, -1.0f, 0.0f,  1.0f, 1.0f },
		{ -1.0f, -1.0f, 0.0f,  0.0f, 1.0f },
    };

    TEXVERTEX* vbVertices;
    if(FAILED(VB->Lock( 0, 0, (void**)&vbVertices, 0 )))
        return E_FAIL;
    memcpy(vbVertices, vertices, 6*sizeof(TEXVERTEX));
    VB->Unlock();

    // Initialize the projection matrix
	FLOAT fAspect = widescreen ? (16.0f / 9.0f) : (4.0f / 3.0f);
	matProj = XMMatrixOrthographicLH(2.0f * fAspect, 2.0f, 0.0f, 1.0f);
	matView = XMMatrixIdentity();

	if (FAILED(d3dDevice->CreateTexture(SI_WIDTH, SI_HEIGHT, 1, 0,
                                         D3DFMT_LIN_A8R8G8B8, 0,
                                         &texture, NULL)))
		return E_FAIL;

	matWorld = XMMatrixScaling((float)SI_WIDTH / SI_HEIGHT, 1.0f, 1.0f);

    return S_OK;
}