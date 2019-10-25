#pragma once

#include "config.h"

// 对图片目录进行压缩.
bool Compress(const char *dir, int step);

bool UnCompress(const char *dir, int step);
