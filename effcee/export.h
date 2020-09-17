// Copyright 2020 The Effcee Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef EFFCEE_EXPORT_H
#define EFFCEE_EXPORT_H

#ifdef _WIN32

#if EFFCEE_BUILDING_DLL
#define EFFCEE_EXPORT __declspec(dllexport)
#else
#define EFFCEE_EXPORT __declspec(dllimport)
#endif

#else  // #ifdef _WIN32

#if __GNUC__ >= 4
#define EFFCEE_EXPORT __attribute__((visibility("default")))
#define EFFCEE_NO_EXPORT __attribute__((visibility("hidden")))
#endif

#endif  // #ifdef _WIN32

#ifndef EFFCEE_EXPORT
#define EFFCEE_EXPORT
#endif

#ifndef EFFCEE_NO_EXPORT
#define EFFCEE_NO_EXPORT
#endif

#endif  // EFFCEE_EXPORT_H