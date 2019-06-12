#include <stdio.h>
#include <iostream>
#include <string>
#include <Windows.h>
#include <cstdlib>
#include <fstream>
#include <map>
#define maxn 300
using namespace std;

struct node {
	int w;
	int l, r, p;
	node(int _w = 0, int _l = -1, int _r = -1, int _p = -1) :w(_w), l(_l), r(_r), p(_p) {}
};

node tree[maxn*2];
int v[maxn*2];
string cd[maxn*2];
int d[maxn*2];

struct BitMapFileHeader {
	unsigned int bfSize; //文件大小
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned int bfOffBits;  // 文件头到位图数据的偏移量，单位是unsigned char
	inline void reset() {
		bfSize = 0;
		bfReserved1 = 0;
		bfReserved2 = 0;
		bfOffBits = 0;
	}
};

struct BitMapInfoHeader {
	unsigned int biSize; // InfoHeader需要的字数
	unsigned int biWidth; // 图像的宽度
	unsigned int biHeight; // 图像的高度
	unsigned short biPlanes; 
	unsigned short biBitCount;
	unsigned int biCompression;
	unsigned int biSizeImage; 
	unsigned int biXPelsPexMeter;
	unsigned int biYPelsPexMeter;
	unsigned int biClrUsed;
	unsigned int biClrImportant;
	inline void reset() {
		biSize = 0;
		biWidth = 0;
		biHeight = 0;
		biPlanes = 0;
		biBitCount = 0;
		biCompression = 0;
		biSizeImage = 0;
		biXPelsPexMeter = 0;
		biYPelsPexMeter = 0;
		biClrUsed = 0;
		biClrImportant = 0;
	}
};

struct RGBQuad {
	unsigned char rgbBlue;
	unsigned char rgbGreen;
	unsigned char rgbRed;
	unsigned char rgbReserved;
};

unsigned short bfType;
BitMapFileHeader bitMapFileHeader;
BitMapInfoHeader bitMapInfoHeader;
RGBQuad * rgbQuads;
unsigned char * pData;
int imageSize;
int rgbSize;

///更新数据
inline void releaseRgbQuads() {
	if (rgbQuads) {
		delete rgbQuads;
	}
}
inline void releaseImageData() {
	if (pData) {
		delete pData;
	}
}
void resetBitMapFileHeader() {
	bitMapFileHeader.reset();
	bfType = 0;
}
void resetBitMapInfoHeader() {
	bitMapInfoHeader.reset();
}
void releaseData() {
	imageSize = 0;
	rgbSize = 0;
	resetBitMapFileHeader();
	resetBitMapInfoHeader();
	releaseImageData();
	releaseRgbQuads();
    memset(v,0,sizeof(v));
    memset(d,0,sizeof(d));
    for (int i=0;i<maxn;i++) cd[i]= "";
}


void chooseMin(int n,int& a,int& b){
    int Min = 0x3f3f3f3f;
    for (int i=0;i<n;i++){
        if (tree[i].w<Min&&tree[i].p==-1){
            Min = v[i];
            a = i;
        }
    }
    Min = 0x3f3f3f3f;
    for (int i=0;i<n;i++){
        if (tree[i].w<Min&&i!=a&&tree[i].p==-1){
            Min = v[i];
            b = i;
        }
    }
}

void HuffmanTree(int n){
    for (int i=0;i<n;i++){
        tree[i].w = v[i];
    }
    for (int i=n;i<2*n-1;i++){
        int m1,m2;
        chooseMin(i,m1,m2);
        tree[i].l = m1;
        tree[i].r = m2;
        tree[m1].p = i;
        tree[m2].p = i;
        tree[i].w = tree[m1].w + tree[m2].w;
    }
}

void getCode(bool op, int son,int par) {
    if (par==-1){
        if (op) cd[son] = "1";
        else cd[son] = "0";
        getCode(true, tree[son].l, son);
	    getCode(false, tree[son].r, son);
    }else {
		if (son == -1) return;
		d[son] = d[tree[son].p] + 1;
		if (op) cd[son] = cd[par] + "1";
		else cd[son] = cd[par] + "0";
		getCode(true, tree[son].l, son);
		getCode(false, tree[son].r, son);
	}
}


int readbmpfile(const string& fileName){
    releaseData();
    std::ifstream bmpfile;
	bmpfile.open(fileName.c_str(), std::ios::binary);
	if (!bmpfile) {
		cout << "Compress: 找不到该文件！" << endl;
		return -1;
	}
	bmpfile.read((char*)& bfType, sizeof(bfType));
	if (bfType != 0x4D42) {
		std::cout << "Compress: 该文件不是bmp文件！" << std::endl;
		return -1;
	}
	bmpfile.read((char*)& bitMapFileHeader, sizeof(bitMapFileHeader));

	bmpfile.read((char*)& bitMapInfoHeader, sizeof(bitMapInfoHeader));

	imageSize = bitMapFileHeader.bfSize - bitMapFileHeader.bfOffBits;

	rgbSize = bitMapFileHeader.bfOffBits - sizeof(bitMapFileHeader) - sizeof(bitMapInfoHeader) - 2;
	rgbQuads = new RGBQuad[rgbSize / sizeof(RGBQuad)];
	bmpfile.read((char*)rgbQuads, rgbSize);
	pData = new unsigned char[imageSize];
	bmpfile.read((char*)pData, imageSize);
    bmpfile.clear();
    bmpfile.close();
    return 0;
}
int compress(const string& fileName){
    int wid = bitMapInfoHeader.biWidth, hei = bitMapInfoHeader.biHeight;
    int md = wid%4;
    int base = (md==0)? wid:4-md+wid;
    for (int j=0;j<hei;j++){
        for (int i=0;i<wid;i++){
            v[pData[j*base+i]] +=1;
        }
    }
    HuffmanTree(256);
    getCode(true, tree[2 * 256 - 2].l,-1);
	getCode(false, tree[2 * 256 - 2].r,-1);
    // for (int i=0;i<256;i++){
    //     cout<<cd[i]<<endl;
    // }
    ofstream ot;
	ot.open(fileName.c_str(), ios::out|ios::binary);
	if (!ot) {
		cout << "Compress: 输出路径无效？？？" << endl;
		return -1;
	}
	ot.write((char*)&bfType, sizeof(unsigned short));
	ot.write((char*)&bitMapFileHeader, sizeof(BitMapFileHeader));
	ot.write((char*)&bitMapInfoHeader, sizeof(BitMapInfoHeader));
	ot.write((char*)rgbQuads, rgbSize);

    //写入每种颜色的权重
    for (int i=0;i<256;i++){
        unsigned int weight_to_write = v[i];
        ot.write((char*)&weight_to_write,sizeof(unsigned int));
    }
    // cout<<"fuck"<<endl;
    // cout<<wid*hei<<endl;
    unsigned int buf = 0;
    int buflen = 0;
    unsigned int hufSize = 0;
    for (int j=0;j<hei;j++){
        for (int i=0;i<wid;i++){
            int color = pData[j*base+i];
            int leng = cd[color].length();
            hufSize+=leng;
        }
    }
    ot.write((char*)&hufSize,sizeof(unsigned int));
    // cout<<"hufSize: "<<hufSize<<endl;
    // int cnt = 0;
    for (int j=0;j<hei;j++){
        for (int i=0;i<wid;i++){
            int color = pData[j*base+i];
            int leng = cd[color].length();
            for (int k=0;k<leng;k++){
                if (cd[color][k]=='0'){
                    buf *= 2;
                }else buf = buf*2+1;
                buflen++;
                if (buflen==32){
                    ot.write((char*)&buf,sizeof(unsigned int));
                    // cnt++;
                    buf = 0;
                    buflen = 0;
                }
            }
        }
    }
    if (buflen>0){
        buf = (buf<<(32-buflen));
        ot.write((char*)&buf,sizeof(unsigned int));
    }
    // cout<<cnt<<endl;
    ot.close();
    releaseData();
}


int main(){
    readbmpfile("lena1.bmp");
    compress("lena1.bmp.hfm");
    return 0;
}
