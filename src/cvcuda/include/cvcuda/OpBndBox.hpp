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
 * @file OpBndBox.hpp
 *
 * @brief Defines the public C++ Class for the BndBox operation.
 * @defgroup NVCV_CPP_ALGORITHM__BND_BOX BndBox
 * @{
 */

#ifndef CVCUDA__BND_BOX_HPP
#define CVCUDA__BND_BOX_HPP

#include "IOperator.hpp"
#include "OpBndBox.h"

#include <cuda_runtime.h>
#include <nvcv/ITensor.hpp>
#include <nvcv/ImageFormat.hpp>
#include <nvcv/alloc/Requirements.hpp>

namespace cvcuda {

class BndBox final : public IOperator
{
public:
    explicit BndBox();

    ~BndBox();

    void operator()(cudaStream_t stream, nvcv::ITensor &in, nvcv::ITensor &out, const NVCVBndBoxesI bboxes);

    virtual NVCVOperatorHandle handle() const noexcept override;

private:
    NVCVOperatorHandle m_handle;
};

inline BndBox::BndBox()
{
    nvcv::detail::CheckThrow(cvcudaBndBoxCreate(&m_handle));
    assert(m_handle);
}

inline BndBox::~BndBox()
{
    nvcvOperatorDestroy(m_handle);
    m_handle = nullptr;
}

inline void BndBox::operator()(cudaStream_t stream, nvcv::ITensor &in, nvcv::ITensor &out, const NVCVBndBoxesI bboxes)
{
    nvcv::detail::CheckThrow(cvcudaBndBoxSubmit(m_handle, stream, in.handle(), out.handle(), bboxes));
}

inline NVCVOperatorHandle BndBox::handle() const noexcept
{
    return m_handle;
}

} // namespace cvcuda

#endif // CVCUDA__BND_BOX_HPP
