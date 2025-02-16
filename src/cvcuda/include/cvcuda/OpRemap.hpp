/*
 * SPDX-FileCopyrightText: Copyright (c) 2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file OpRemap.hpp
 *
 * @brief Defines the public C++ Class for the Remap operation.
 * @defgroup NVCV_CPP_ALGORITHM__REMAP Remap
 * @{
 */

#ifndef CVCUDA__REMAP_HPP
#define CVCUDA__REMAP_HPP

#include "IOperator.hpp"
#include "OpRemap.h"

#include <cuda_runtime.h>
#include <nvcv/IImageBatch.hpp>
#include <nvcv/ITensor.hpp>
#include <nvcv/ImageFormat.hpp>
#include <nvcv/alloc/Requirements.hpp>

namespace cvcuda {

class Remap final : public IOperator
{
public:
    explicit Remap();

    ~Remap();

    void operator()(cudaStream_t stream, nvcv::ITensor &in, nvcv::ITensor &out, nvcv::ITensor &map,
                    NVCVInterpolationType inInterp, NVCVInterpolationType mapInterp, NVCVRemapMapValueType mapValueType,
                    bool alignCorners, NVCVBorderType border, float4 borderValue);

    void operator()(cudaStream_t stream, nvcv::IImageBatch &in, nvcv::IImageBatch &out, nvcv::ITensor &map,
                    NVCVInterpolationType inInterp, NVCVInterpolationType mapInterp, NVCVRemapMapValueType mapValueType,
                    bool alignCorners, NVCVBorderType border, float4 borderValue);

    virtual NVCVOperatorHandle handle() const noexcept override;

private:
    NVCVOperatorHandle m_handle;
};

inline Remap::Remap()
{
    nvcv::detail::CheckThrow(cvcudaRemapCreate(&m_handle));
    assert(m_handle);
}

inline Remap::~Remap()
{
    nvcvOperatorDestroy(m_handle);
}

inline void Remap::operator()(cudaStream_t stream, nvcv::ITensor &in, nvcv::ITensor &out, nvcv::ITensor &map,
                              NVCVInterpolationType inInterp, NVCVInterpolationType mapInterp,
                              NVCVRemapMapValueType mapValueType, bool alignCorners, NVCVBorderType border,
                              float4 borderValue)
{
    nvcv::detail::CheckThrow(cvcudaRemapSubmit(m_handle, stream, in.handle(), out.handle(), map.handle(), inInterp,
                                               mapInterp, mapValueType, static_cast<int8_t>(alignCorners), border,
                                               borderValue));
}

inline void Remap::operator()(cudaStream_t stream, nvcv::IImageBatch &in, nvcv::IImageBatch &out, nvcv::ITensor &map,
                              NVCVInterpolationType inInterp, NVCVInterpolationType mapInterp,
                              NVCVRemapMapValueType mapValueType, bool alignCorners, NVCVBorderType border,
                              float4 borderValue)
{
    nvcv::detail::CheckThrow(cvcudaRemapVarShapeSubmit(m_handle, stream, in.handle(), out.handle(), map.handle(),
                                                       inInterp, mapInterp, mapValueType,
                                                       static_cast<int8_t>(alignCorners), border, borderValue));
}

inline NVCVOperatorHandle Remap::handle() const noexcept
{
    return m_handle;
}

} // namespace cvcuda

#endif // CVCUDA__REMAP_HPP
