/*
 * SPDX-FileCopyrightText: Copyright (c) 2022-2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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

#include "Operators.hpp"

#include <common/PyUtil.hpp>
#include <common/String.hpp>
#include <cvcuda/OpPillowResize.hpp>
#include <cvcuda/Types.h>
#include <nvcv/TensorDataAccess.hpp>
#include <nvcv/cuda/TypeTraits.hpp>
#include <nvcv/python/Image.hpp>
#include <nvcv/python/ImageBatchVarShape.hpp>
#include <nvcv/python/ImageFormat.hpp>
#include <nvcv/python/ResourceGuard.hpp>
#include <nvcv/python/Stream.hpp>
#include <nvcv/python/Tensor.hpp>
#include <pybind11/stl.h>

namespace cvcudapy {

namespace {
Tensor PillowResizeInto(Tensor &output, Tensor &input, nvcv::ImageFormat format, NVCVInterpolationType interp,
                        std::optional<Stream> pstream)
{
    if (!pstream)
    {
        pstream = Stream::Current();
    }
    auto in_access  = nvcv::TensorDataAccessStridedImagePlanar::Create(input.exportData());
    auto out_access = nvcv::TensorDataAccessStridedImagePlanar::Create(output.exportData());
    if (!in_access || !out_access)
    {
        throw std::runtime_error("Incompatible input/output tensor layout");
    }
    int          w              = std::max(in_access->numCols(), out_access->numCols());
    int          h              = std::max(in_access->numRows(), out_access->numRows());
    int          max_batch_size = in_access->numSamples();
    nvcv::Size2D size{w, h};
    auto         pillowResize = CreateOperator<cvcuda::PillowResize>(size, max_batch_size, format);

    ResourceGuard guard(*pstream);
    guard.add(LockMode::LOCK_READ, {input});
    guard.add(LockMode::LOCK_WRITE, {output});
    guard.add(LockMode::LOCK_WRITE, {*pillowResize});

    pillowResize->submit(pstream->cudaHandle(), input, output, interp);

    return output;
}

Tensor PillowResize(Tensor &input, const Shape &out_shape, nvcv::ImageFormat format, NVCVInterpolationType interp,
                    std::optional<Stream> pstream)
{
    Tensor output = Tensor::Create(out_shape, input.dtype(), input.layout());

    return PillowResizeInto(output, input, format, interp, pstream);
}

ImageBatchVarShape VarShapePillowResizeInto(ImageBatchVarShape &output, ImageBatchVarShape &input,
                                            NVCVInterpolationType interpolation, std::optional<Stream> pstream)
{
    if (!pstream)
    {
        pstream = Stream::Current();
    }

    nvcv::Size2D maxSrcSize = input.maxSize();
    nvcv::Size2D maxDstSize = output.maxSize();

    int          w              = std::max(maxSrcSize.w, maxDstSize.w);
    int          h              = std::max(maxSrcSize.h, maxDstSize.h);
    int          max_batch_size = input.capacity();
    nvcv::Size2D size{w, h};
    auto         pillowResize = CreateOperator<cvcuda::PillowResize>(size, max_batch_size, input.uniqueFormat());

    ResourceGuard guard(*pstream);
    guard.add(LockMode::LOCK_READ, {input});
    guard.add(LockMode::LOCK_WRITE, {output});
    guard.add(LockMode::LOCK_WRITE, {*pillowResize});

    pillowResize->submit(pstream->cudaHandle(), input, output, interpolation);

    return output;
}

ImageBatchVarShape VarShapePillowResize(ImageBatchVarShape &input, const std::vector<std::tuple<int, int>> &outSizes,
                                        NVCVInterpolationType interpolation, std::optional<Stream> pstream)
{
    ImageBatchVarShape output = ImageBatchVarShape::Create(input.capacity());

    if (static_cast<int32_t>(outSizes.size()) != input.numImages())
    {
        throw std::runtime_error("Invalid outSizes passed");
    }

    for (int i = 0; i < input.numImages(); ++i)
    {
        nvcv::ImageFormat format = input[i].format();
        auto              size   = outSizes[i];
        auto              image  = Image::Create({std::get<0>(size), std::get<1>(size)}, format);
        output.pushBack(image);
    }

    return VarShapePillowResizeInto(output, input, interpolation, pstream);
}

} // namespace

void ExportOpPillowResize(py::module &m)
{
    using namespace pybind11::literals;

    m.def("pillowresize", &PillowResize, "src"_a, "shape"_a, "format"_a, "interp"_a = NVCV_INTERP_LINEAR, py::kw_only(),
          "stream"_a = nullptr, R"pbdoc(

        Executes the Pillow Resize operation on the given cuda stream.

        See also:
            Refer to the CV-CUDA C API reference for the Pillow Resize operator
            for more details and usage examples.

        Args:
            src (Tensor): Input tensor containing one or more images.
            shape (Shape): Shape of the output image.
            format (ImageFormat): Format of the input and output images.
            interp(Interp): Interpolation type used for transform.
            stream (Stream, optional): CUDA Stream on which to perform the operation.

        Returns:
            cvcuda.Tensor: The output tensor.

        Caution:
            Restrictions to several arguments may apply. Check the C
            API references of the CV-CUDA operator.
    )pbdoc");

    m.def("pillowresize_into", &PillowResizeInto, "dst"_a, "src"_a, "format"_a, "interp"_a = NVCV_INTERP_LINEAR,
          py::kw_only(), "stream"_a = nullptr, R"pbdoc(

        Executes the Pillow Resize operation on the given cuda stream.

        See also:
            Refer to the CV-CUDA C API reference for the Pillow Resize operator
            for more details and usage examples.

        Args:
            dst (Tensor): Output tensor to store the result of the operation.
            src (Tensor): Input tensor containing one or more images.
            shape (Shape): Shape of the output image.
            format (ImageFormat): Format of the input and output images.
            interp(Interp): Interpolation type used for transform.
            stream (Stream, optional): CUDA Stream on which to perform the operation.

        Returns:
            None

        Caution:
            Restrictions to several arguments may apply. Check the C
            API references of the CV-CUDA operator.
    )pbdoc");

    m.def("pillowresize", &VarShapePillowResize, "src"_a, "sizes"_a, "interp"_a = NVCV_INTERP_LINEAR, py::kw_only(),
          "stream"_a = nullptr, R"pbdoc(

        Executes the Pillow Resize operation on the given cuda stream.

        See also:
            Refer to the CV-CUDA C API reference for the Pillow Resize operator
            for more details and usage examples.

        Args:
            src (ImageBatchVarShape): Input image batch containing one or more images.
            sizes (Tuple vector): Shapes of output images.
            interp(Interp): Interpolation type used for transform.
            stream (Stream, optional): CUDA Stream on which to perform the operation.

        Returns:
            cvcuda.ImageBatchVarShape: The output image batch.

        Caution:
            Restrictions to several arguments may apply. Check the C
            API references of the CV-CUDA operator.
    )pbdoc");

    m.def("pillowresize_into", &VarShapePillowResizeInto, "dst"_a, "src"_a, "interp"_a = NVCV_INTERP_LINEAR,
          py::kw_only(), "stream"_a = nullptr, R"pbdoc(

        Executes the Pillow Resize operation on the given cuda stream.

        See also:
            Refer to the CV-CUDA C API reference for the Pillow Resize operator
            for more details and usage examples.

        Args:
            src (ImageBatchVarShape): Input image batch containing one or more images.
            dst (ImageBatchVarShape): Output image batch containing the result of the operation.
            sizes (Tuple vector): Shapes of output images.
            interp(Interp): Interpolation type used for transform.
            stream (Stream, optional): CUDA Stream on which to perform the operation.

        Returns:
            None

        Caution:
            Restrictions to several arguments may apply. Check the C
            API references of the CV-CUDA operator.
    )pbdoc");
}

} // namespace cvcudapy
