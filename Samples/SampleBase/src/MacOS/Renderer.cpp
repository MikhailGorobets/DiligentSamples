/*     Copyright 2015-2018 Egor Yusov
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF ANY PROPRIETARY RIGHTS.
 *
 *  In no event and under no legal theory, whether in tort (including negligence), 
 *  contract, or otherwise, unless required by applicable law (such as deliberate 
 *  and grossly negligent acts) or agreed to in writing, shall any Contributor be
 *  liable for any damages, including any direct, indirect, special, incidental, 
 *  or consequential damages of any character arising as a result of this License or 
 *  out of the use or inability to use the software (including but not limited to damages 
 *  for loss of goodwill, work stoppage, computer failure or malfunction, or any and 
 *  all other commercial damages or losses), even if such Contributor has been advised 
 *  of the possibility of such damages.
 */

#include "Renderer.h"
#include "AntTweakBar.h"
#include "Errors.h"
#include "RenderDeviceFactoryOpenGL.h"

using namespace Diligent;

Renderer::Renderer()
{
    pSample.reset( CreateSample() );
}

Renderer::~Renderer()
{
    pSample.reset();
    TwTerminate();
    pSwapChain.Release();
    pDeviceContext.Release();
    pRenderDevice.Release();
}

void Renderer::Init()
{
    SwapChainDesc SCDesc;
    EngineCreationAttribs EngineCreationAttribs;
    Uint32 NumDeferredContexts = 0;
    pSample->GetEngineInitializationAttribs(DeviceType::OpenGL, EngineCreationAttribs, NumDeferredContexts);
    if(NumDeferredContexts != 0)
    {
        LOG_ERROR_MESSAGE("Deferred contexts are not supported by OpenGL implementation");
        NumDeferredContexts = 0;
    }
    
    GetEngineFactoryOpenGL()->CreateDeviceAndSwapChainGL(EngineCreationAttribs, &pRenderDevice, &pDeviceContext, SCDesc, nullptr, &pSwapChain );
    
    // Initialize AntTweakBar
    // TW_OPENGL and TW_OPENGL_CORE were designed to select rendering with
    // very old GL specification. Using these modes results in applying some
    // odd offsets which distorts everything
    // Latest OpenGL works very much like Direct3D11, and
    // Tweak Bar will never know if D3D or OpenGL is actually used
    if (!TwInit(TW_DIRECT3D11, pRenderDevice.RawPtr(), pDeviceContext.RawPtr(), pSwapChain->GetDesc().ColorBufferFormat))
    {
        LOG_ERROR_MESSAGE("AntTweakBar initialization failed");
    }
    TwDefine(" TW_HELP visible=false ");
    
    int width = 1024, height = 768;
    //glfwGetWindowSize(window, &width, &height);
    IDeviceContext *ppContexts[] = {pDeviceContext};
    pSample->Initialize(pRenderDevice, ppContexts, NumDeferredContexts, pSwapChain);
    pSample->WindowResize( pSwapChain->GetDesc().Width, pSwapChain->GetDesc().Height );
    std::string Title = pSample->GetSampleName();
    TwWindowSize(width, height);
    
    PrevTime = timer.GetElapsedTime();
    FilteredFrameTime = 0.0;
}

void Renderer::WindowResize(int width, int height)
{
    pSwapChain->Resize(width, height);
    pSample->WindowResize( width, height );
    TwWindowSize(width, height);
}

void Renderer::Render()
{
    // Render the scene
    auto CurrTime = timer.GetElapsedTime();
    auto ElapsedTime = CurrTime - PrevTime;
    PrevTime = CurrTime;
    
    pDeviceContext->SetRenderTargets(0, nullptr, nullptr);
    
    pSample->Update(CurrTime, ElapsedTime);
    pSample->Render();
    
    // Draw tweak bars
    // Restore default render target in case the sample has changed it
    pDeviceContext->SetRenderTargets(0, nullptr, nullptr);
    TwDraw();
    
    //pSwapChain->Present();
    
/*    double filterScale = 0.2;
    filteredFrameTime = filteredFrameTime * (1.0 - filterScale) + filterScale * ElapsedTime;
    std::stringstream fpsCounterSS;
    fpsCounterSS << " - " << std::fixed << std::setprecision(1) << filteredFrameTime * 1000;
    fpsCounterSS << " ms (" << 1.0 / filteredFrameTime << " fps)";
    glfwSetWindowTitle(window, (Title + fpsCounterSS.str()).c_str());
*/
}

