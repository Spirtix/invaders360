#pragma once
#include <stdint.h>
#include <vector>
#include "rectangle.h"

class gameScreenRenderer {
public:
    gameScreenRenderer();
    gameScreenRenderer(std::vector<rectangle>& overlay);
    ~gameScreenRenderer();
    HRESULT init();
    void render(uint8_t* vram);
private:
    HRESULT initD3D();
    HRESULT initScene();
    D3DDevice* d3dDevice;
    D3DVertexBuffer* VB;
    D3DVertexDeclaration* vertexDecl;
    D3DVertexShader* vShader;
    D3DPixelShader* pShader;
    D3DTexture* texture;
    static const UINT SI_WIDTH  = 224;
    static const UINT SI_HEIGHT = 256;
    uint32_t overlay[SI_WIDTH * SI_HEIGHT];

    XMMATRIX matWorld;
    XMMATRIX matProj;
    XMMATRIX matView;
    BOOL widescreen;
    struct TEXVERTEX
    {
        float Position[3];
        float Tex[2];
    };
};