/*
 * SPDX-FileCopyrightText: Copyright (c) 2022 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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
 * @file VersionDef.h
 *
 * Functions and structures for handling NVCV library version.
 */

#ifndef NVCV_VERSIONDEF_H
#define NVCV_VERSIONDEF_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Declarations of entities to handle NVCV versioning.
 *
 * These utilities allow querying the NVCV header and library versions and
 * properly handle NVCV forward- or backward-compatibility .
 *
 * @defgroup NVCV_CPP_UTIL_VERSION Versioning
 * @{
 */

/** Make a NVCV version identifier with four components.
 * @param[in] major,minor,patch,tweak Version components to be converted to a number.
 * @returns The numeric version representation.
 */
#define NVCV_MAKE_VERSION4(major, minor, patch, tweak) \
    ((uint32_t)((major)*1000000 + (minor)*10000 + (patch)*100 + (tweak)))

/** Make a NVCV version identifier with three components.
 *
 * The tweak version component is considered to be 0.
 *
 * @param[in] major,minor,patch Version components to be converted to a number.
 * @returns The numeric version representation.
 */
#define NVCV_MAKE_VERSION3(major, minor, patch) \
    NVCV_MAKE_VERSION4(major, minor, patch, 0)

/** Make a NVCV version identifier with two components.
 *
 * The patch and tweak version components are considered to be 0.
 *
 * @param[in] major,minor Version components to be converted to a number.
 * @returns The numeric version representation.
 */
#define NVCV_MAKE_VERSION2(major, minor) \
    NVCV_MAKE_VERSION4(major, minor, 0, 0)

/** Make a NVCV version identifier with one component.
 *
 * The minor, patch and tweak version components are considered to be 0.
 *
 * @param[in] major Major version component to be converted to a number.
 * @returns The numeric version representation.
 */
#define NVCV_MAKE_VERSION1(major) \
    NVCV_MAKE_VERSION4(major, 0, 0, 0)

/** Assemble an integer version from its components.
 * This makes it easy to conditionally compile code for different NVCV versions, e.g:
 * \code
 * #if NVCV_VERSION < NVCV_MAKE_VERSION(1,0,0)
 *    // code that runs on versions prior 1.0.0
 * #else
 *    // code that runs on versions after that, including 1.0.0
 * #endif
 * \endcode
 *
 * @param[in] major Major version component, mandatory.
 * @param[in] minor Minor version component. If ommitted, it's considered to be 0.
 * @param[in] patch Patch version component. If ommitted, it's considered to be 0.
 * @param[in] tweak Tweak version component. If ommitted, it's considered to be 0.
 * @returns The numeric version representation.
 */
#if NVCV_DOXYGEN
#   define NVCV_MAKE_VERSION(major,minor,patch,tweak)
#else
#define NVCV_DETAIL_GET_MACRO(_1,_2,_3,_4,NAME,...) NAME
#define NVCV_MAKE_VERSION(...) \
    NVCV_DETAIL_GET_MACRO(__VA_ARGS__, NVCV_MAKE_VERSION4, NVCV_MAKE_VERSION3, NVCV_MAKE_VERSION2, NVCV_MAKE_VERSION1)(__VA_ARGS__)
#endif

/** Major version number component.
 * This is incremented every time there's a incompatible ABI change.
 * In the special case of major version 0, compatibility between minor versions
 * is not guaranteed.
 */
#define NVCV_VERSION_MAJOR 0

/** Minor version number component.
 * This is incremented every time there's a new feature added to NVCV that
 * doesn't break backward compatibility. This number is reset to zero when
 * major version changes.
 */
#define NVCV_VERSION_MINOR 3

/** Patch version number component.
 * This is incremented every time a bug is fixed, but no new functionality is added
 * to the library. This number is reset to zero when minor version changes.
 */
#define NVCV_VERSION_PATCH 0

/** Tweak version number component.
 * Incremented for packaging or documentation updates, etc. The library itself isn't updated.
 * Gets reset to zero when patch version changes.
 */
#define NVCV_VERSION_TWEAK 0

/** Version suffix.
 * String appended to version number to designate special builds.
 */
#define NVCV_VERSION_SUFFIX "beta"

/** NVCV library version.
  * It's an integer value computed from `MAJOR*1000000 + MINOR*10000 + PATCH*100 + TWEAK`.
  * Integer versions can be compared, recent versions are greater than older ones.
  */
#define NVCV_VERSION NVCV_MAKE_VERSION(NVCV_VERSION_MAJOR, NVCV_VERSION_MINOR, NVCV_VERSION_PATCH, NVCV_VERSION_TWEAK)

/** NVCV library version number represented as a string. */
#define NVCV_VERSION_STRING "0.3.0-beta"

/** Selected API version to use.
 * This macro selects which of the supported APIs the code will use.
 *
 * By default this equals to the highest supported API, corresponding to the current major and
 * minor versions of the library.
 *
 * User can override the version by defining this macro before including NVCV headers.
 */
#if NVCV_DOXYGEN
#   define NVCV_VERSION_API
#else
#ifdef NVCV_VERSION_API
#   if NVCV_VERSION_API < NVCV_MAKE_VERSION(NVCV_VERSION_MAJOR) || \
        NVCV_VERSION_API > NVCV_MAKE_VERSION(NVCV_VERSION_MAJOR, NVCV_VERSION_MINOR)
#       error Selected NVCV API version not supported.
#   endif
#else
#   define NVCV_VERSION_API NVCV_MAKE_VERSION(NVCV_VERSION_MAJOR, NVCV_VERSION_MINOR)
#endif
#endif

/** Conditionally enable code when selected API version is exactly given version.
 *
 * @param[in] major,minor API version that will be considered.
 */
#define NVCV_VERSION_API_IS(major,minor) \
    (NVCV_MAKE_VERSION(major,minor) == NVCV_VERSION_API)

/** Conditionally enable code when selected API version is at least given version.
 *
 * @param[in] major,minor Minimum API version that will be considered.
 */
#define NVCV_VERSION_API_AT_LEAST(major,minor) \
    (NVCV_MAKE_VERSION(major,minor) <= NVCV_VERSION_API)

/** Conditionally enable code when selected API version is at most given version.
 *
 * @param[in] major,minor Maximum API version that will be considered.
 */
#define NVCV_VERSION_API_AT_MOST(major,minor) \
    (NVCV_MAKE_VERSION(major,minor) >= NVCV_VERSION_API)

/** Conditionally enable code when selected API version is between two versions.
 *
 * @param[in] min_major,min_minor Minimum API version that will be considered.
 * @param[in] max_major,max_minor Maximum API version that will be considered.
 */
#define NVCV_VERSION_API_IN_RANGE(min_major,min_minor,max_major,max_minor) \
    (NVCV_VERSION_API_AT_LEAST(min_major, min_minor) && NVCV_VERSION_API_AT_MOST(max_major, max_minor))

/** @} */

#ifdef __cplusplus
}
#endif

#endif // NVCV_VERSION_H
