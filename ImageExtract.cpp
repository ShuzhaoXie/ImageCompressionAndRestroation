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
// string cd[maxn*2];
// int d[maxn*2];

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

int extract(const string& fileName,const string& outputPath){
    releaseData();
    std::ifstream bmpfile;
	bmpfile.open(fileName.c_str(), std::ios::binary);
	if (!bmpfile) {
		cout << "Compress: 找不到该文件！" << endl;
		return -1;
	}
	bmpfile.read((char*)& bfType, sizeof(bfType));
	if (bfType != 0x4D42) {
		std::cout << "extract: 该文件不是bmp文件！" << std::endl;
		return -1;
	}
	bmpfile.read((char*)& bitMapFileHeader, sizeof(bitMapFileHeader));

	bmpfile.read((char*)& bitMapInfoHeader, sizeof(bitMapInfoHeader));

	imageSize = bitMapFileHeader.bfSize - bitMapFileHeader.bfOffBits;

	rgbSize = bitMapFileHeader.bfOffBits - sizeof(bitMapFileHeader) - sizeof(bitMapInfoHeader) - 2;
	rgbQuads = new RGBQuad[rgbSize / sizeof(RGBQuad)];
	bmpfile.read((char*)rgbQuads, rgbSize);
	pData = new unsigned char[imageSize];
	for (int i=0;i<256;i++){
        unsigned int weightRead;
        bmpfile.read((char*)& weightRead,sizeof(unsigned int));
        v[i] = weightRead;
    }
    HuffmanTree(256);
    unsigned int hufSize;
    int T=0;
    int nv = 256*2-2;
    int cnt = 0;
    int wid = bitMapInfoHeader.biWidth, hei = bitMapInfoHeader.biHeight;
    int md = wid%4;
    int base = (md==0)?wid:4-md+wid;
    bmpfile.read((char*)&hufSize,sizeof(unsigned int));
    if (hufSize%32==0){
        T = hufSize/32;
    }else {
        T = hufSize/32 + 1;
    }
    while(T--){
        unsigned int buf = 0;
        bmpfile.read((char*)&buf,sizeof(unsigned int));
        string cdc = "",cd="";
        while(buf>0){
            if (buf%2==1) cdc = cdc + "1";
            else cdc = cdc + "0";
            buf/=2;
        }
        if (cdc.length()!=32) {
            int chajia = 32 - cdc.length();
            for (int i=0;i<chajia;i++) cdc = cdc + "0";
        }
        // if (cdc.length()!=32) cout<<"fucdfsdfsk"<<endl;
        for (int i=31;i>=0;i--){
            cd = cd + cdc[i];
        }
        if (T==0){
            int leng = cd.length();
            for (int i=0;i<hufSize;i++){
                if (cd[i]=='1') nv = tree[nv].l;
                else nv = tree[nv].r;
                if (nv<256){
                    pData[cnt] = nv;
                    if (base != wid){
                        // cnt从0开始
                        if ((cnt+1)%base == wid){
                            cnt = cnt-wid+base+1;
                        }else cnt++;
                    }else cnt++;
                    nv = 256*2-2;
                }
            }
        }else {
            hufSize-=32;
            int leng = cd.length();
            for (int i=0;i<leng;i++){
                if (cd[i]=='1') nv = tree[nv].l;
                else nv = tree[nv].r;
                if (nv<256){
                    pData[cnt] = nv;
                    if (base != wid){
                        if ((cnt+1)%base == wid){
                            cnt = cnt-wid+base+1;
                        }else cnt++;
                    }else cnt++;
                    nv = 256*2-2;
                }
            }
        }
    }
    bmpfile.clear();
    bmpfile.close();
    ofstream fp(outputPath, ios::out | ios::binary);
	if (!fp) {
		cout << "文件无法创建或打开！" << endl;
		return -1;
	}
	fp.write((char*)&bfType, sizeof(unsigned short));
	fp.write((char*)&bitMapFileHeader, sizeof(BitMapFileHeader));
	fp.write((char*)&bitMapInfoHeader, sizeof(BitMapInfoHeader));
	fp.write((char*)rgbQuads, rgbSize);
	fp.write((char*)pData, imageSize);
	fp.clear();
	fp.close();
    return 0;
}


int main(){
    extract("lena1.bmp.hfm","lena2.bmp");
    return 0;
}
