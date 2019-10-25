#include <io.h>
#include <vector>
#include "zlib\zlib.h"
#include "ImageCompress.h"

#define CHECK_CODE(p, c) if ((c) != (p)) { printf("%s\n", __FILE__); return -1; }

#define CHECK_FILE(p) CHECK_CODE(p, 1)

#define CHECK_ZLIB(p) CHECK_CODE(p, 0)

#define CHECK_BOOL(p) CHECK_CODE(p, true)

// 获取指定目录下的图片.
std::vector<std::string> ListImages(const std::string &dir)
{
	std::vector<std::string> ret;
	//文件句柄
	intptr_t hFile = 0;
	//文件信息  
	struct _finddata_t fileinfo;
	std::string p;
	try
	{
		if ((hFile = _findfirst(p.assign(dir).append("\\*.jpg").c_str(), &fileinfo)) != -1)
		{
			do {
				std::string child = dir + "\\" + fileinfo.name;
				ret.push_back(child);
			} while (_findnext(hFile, &fileinfo) == 0);
			_findclose(hFile);
		}
	}
	catch (std::exception e) { if (hFile) _findclose(hFile); }

	return ret;
}

// 保存图片文件.
int save(const std::string &path, const cv::Mat &ref, const cv::Mat &m) {
	cv::Mat diff, sign = m > ref; // sign=sign(m-ref)
	cv::absdiff(m, ref, diff);    // diff= abs(m-ref), m = sign*diff + ref
	std::string new_path = path;
	FILE *f = fopen(new_path.c_str(), "wb");
	if (f)
	{
		int len = diff.rows * diff.step[0], z, w;
		uchar *buffer = new uchar[len];
		// 行、宽
		CHECK_FILE(w = fwrite(&diff.rows, sizeof(int), 1, f));
		CHECK_FILE(w = fwrite(&diff.cols, sizeof(int), 1, f));
		uLongf dst = len;
		CHECK_ZLIB(z=compress(buffer, &dst, sign.data, len));
		// 数据长度、数据
		CHECK_FILE(w = fwrite(&dst, sizeof(uLongf), 1, f));
		CHECK_FILE(w = fwrite(buffer, dst, 1, f));
		dst = len;
		CHECK_ZLIB(z=compress(buffer, &dst, diff.data, len));
		// 数据长度、数据
		CHECK_FILE(w = fwrite(&dst, sizeof(uLongf), 1, f));
		CHECK_FILE(w = fwrite(buffer, dst, 1, f));

		delete[] buffer;
		fclose(f);
	}
	return 1;
}

// 加载图片文件.
int load(const std::string &path, const cv::Mat &ref) {
	std::string new_path = path;
	FILE *f = fopen(new_path.c_str(), "rb");
	if (f)
	{
		int rows, cols, z, r;
		// 行、宽
		CHECK_FILE(r = fread(&rows, sizeof(int), 1, f));
		CHECK_FILE(r = fread(&cols, sizeof(int), 1, f));
		cv::Mat sign(rows, cols, CV_8UC3);
		cv::Mat diff(rows, cols, CV_8UC3);
		uLongf dst, src;
		int len = rows * cols * 4;
		uchar *buffer = new uchar[len];
		// 数据长度、数据
		CHECK_FILE(r = fread(&src, sizeof(uLongf), 1, f));
		CHECK_FILE(r = fread(buffer, src, 1, f));
		dst = len;
		CHECK_ZLIB(z=uncompress(sign.data, &dst, buffer, src));
		// 数据长度、数据
		CHECK_FILE(r = fread(&src, sizeof(uLongf), 1, f));
		CHECK_FILE(r = fread(buffer, src, 1, f));
		dst = len;
		CHECK_ZLIB(z=uncompress(diff.data, &dst, buffer, src));
		fclose(f);
		// 还原图像 m = sign*diff + ref
		cv::bitwise_and(sign, 1, sign);
		sign.convertTo(sign, CV_32FC3);
		diff.convertTo(diff, CV_32FC3);
		cv::multiply(sign - 1, diff, diff);
		cv::Mat m;
		ref.convertTo(m, CV_32FC3);
		m = m + diff;
		m.convertTo(m, CV_8UC3);
		CHECK_BOOL(cv::imwrite(new_path, m));

		delete[] buffer;
		fclose(f);
	}
	return 1;
}

// 对图片目录进行压缩.
bool Compress(const char * dir, int step)
{
	std::vector<std::string> images = ListImages(dir);
	if (images.size() == 0)
	{
		return false;
	}
	cv::Mat ref = cv::imread(images.at(0));
	for (int i=1, c = 0; i < images.size(); ++i)
	{
		cv::Mat m = cv::imread(images.at(i));
		if (m.rows != ref.rows || m.cols != ref.cols)
			continue;
		if (++c <= step){
			save(images.at(i).c_str(), ref, m);
		}else{
			c = 0;
			ref = m;
		}
	}
	
	return true;
}

// 对图片目录进行解压缩.
bool UnCompress(const char * dir, int step)
{
	std::vector<std::string> images = ListImages(dir);
	if (images.size() == 0)
	{
		return false;
	}
	cv::Mat ref = cv::imread(images.at(0));
	for (int i = 1, c = 0; i < images.size(); ++i)
	{
		if (++c <= step) {
			load(images.at(i).c_str(), ref);
		}
		else {
			cv::Mat m = cv::imread(images.at(i));
			c = 0;
			ref = m;
		}
	}

	return true;
}
