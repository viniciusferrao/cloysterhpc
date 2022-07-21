/*
 * Copyright 2021 Vinícius Ferrão <vinicius@ferrao.net.br>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CLOYSTERHPC_CONST_H_
#define CLOYSTERHPC_CONST_H_

// We strive for constexpr correctness, but we sometimes still need a macro
namespace cloyster {

#define PRODUCT_NAME "CloysterHPC"
constexpr const char* productName = PRODUCT_NAME;

#define PRODUCT_URL "http://github.com/viniciusferrao/cloysterhpc"
constexpr const char* productUrl = PRODUCT_URL;

#define INSTALL_PATH "/opt/cloysterhpc"
constexpr const char* installPath = INSTALL_PATH;

#if defined(_DUMMY_) || defined(__APPLE__)
#define CHROOT "chroot"
[[maybe_unused]] constexpr const char* chroot = CHROOT;
#else
#define CHROOT
#endif

} // namespace cloyster

using cloyster::installPath;
using cloyster::productName;
using cloyster::productUrl;

#endif // CLOYSTERHPC_CONST_H_
