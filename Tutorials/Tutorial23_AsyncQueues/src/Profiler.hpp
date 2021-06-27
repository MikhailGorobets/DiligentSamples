/*
 *  Copyright 2019-2021 Diligent Graphics LLC
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
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

#pragma once

#include <array>
#include <chrono>
#include "SampleBase.hpp"

namespace Diligent
{

class Profiler
{
public:
    enum PASS_TYPE
    {
        FRAME,
        GRAPHICS_1,
        GRAPHICS_2,
        COMPUTE,
        TRANSFER,
    };

    void Initialize(IRenderDevice* pDevice);

    void Begin(IDeviceContext* pContext, PASS_TYPE Pass);
    void End(IDeviceContext* pContext, PASS_TYPE Pass);
    void SetCpuToGpuTransferRate(Uint32 RateInMb);

    void UpdateUI();
    void Update(double ElapsedTime);

private:
    using TimePoint = std::chrono::high_resolution_clock::time_point;
    using SecondsD  = std::chrono::duration<double>;

    RefCntAutoPtr<IRenderDevice> m_Device;

    static constexpr Uint32 NumFramesPOT = 3;

    Uint32 m_FrameId : NumFramesPOT;
    Uint32 m_TempCpuToGpuTransferRateMb     = 0;
    bool   m_SupportsTransferQueueProfiling = false;

    struct PassCounters
    {
        RefCntAutoPtr<IQuery> GpuTimeQueryBegin;
        RefCntAutoPtr<IQuery> GpuTimeQueryEnd;

        // time in seconds
        double GpuTimeBegin = 0.0;
        double GpuTimeEnd   = 0.0;

        TimePoint CpuTImeBegin = {};
        TimePoint CpuTImeEnd   = {};

        bool QuerySupported = false;
        bool Queried        = false;
    };

    struct Frame
    {
        PassCounters Frame;
        PassCounters Graphics1;
        PassCounters Graphics2;
        PassCounters Compute;
        PassCounters Transfer;
    };
    std::array<Frame, (1 << NumFramesPOT)> m_FrameHistory = {};

    struct Graph
    {
        float Gfx1X   = 0.f;
        float Gfx1W   = 0.f;
        float Gfx2X   = 0.f;
        float Gfx2W   = 0.f;
        float CompX   = 0.f;
        float CompW   = 0.f;
        float TransfX = 0.f;
        float TransfW = 0.f;
    };
    Graph  m_Graph1;
    Graph  m_Graph2;
    String m_GpuCountersStr;
    String m_CpuCountersStr;
    double m_AccumTime = 0.0;
};

} // namespace Diligent
