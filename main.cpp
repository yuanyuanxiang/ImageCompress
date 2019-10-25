
#include "ImageCompress.h"

// 对指定文件夹下的图片序列进行压缩/解压缩.
int main(int argc, const char **argv) {

	const char *dir = argc > 1 ? argv[1] : "./face";

	bool b = Compress(dir, 25);

	return 0;
}
