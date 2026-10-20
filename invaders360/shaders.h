#pragma once

const char* srcVertexShader = 
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

const char* srcPixelShader = 
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
