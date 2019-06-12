# ImageCompressionAndRestroation

## 利用霍夫曼树实现8位256色灰度图像的压缩与复原

## 一、实验语言与环境

- Windows
- C++ 
- Visual Studio Code或code::blocks
- MinGW-gcc.exe

## 二、代码解析

### 图像压缩

详见ImageCompress.cpp

1. 获取256种颜色的权值

   ```c++
   int wid = bitMapInfoHeader.biWidth, hei = bitMapInfoHeader.biHeight;
   int md = wid%4;
   int base = (md==0)? wid:4-md+wid;
   for (int j=0;j<hei;j++){
       for (int i=0;i<wid;i++){
           v[pData[j*base+i]] +=1; //颜色 i 的权值 v[i]
       }
   }
   ```

2. 建立霍夫曼树并获取每种颜色的霍夫曼编码（具体代码实现详见：ImageCompress.cpp）

   ```c++
   HuffmanTree(256);
   getCode(true, tree[2 * 256 - 2].l,-1);
   getCode(false, tree[2 * 256 - 2].r,-1);
   ```

3. 将图像的文件头、信息头、调色板写入后，将每个颜色的权值写入，权值用unsigned int型写入二进制文件，存入权值目的是方便在解压时重建霍夫曼树。这里不用写入颜色序号，因为这本就是颜色`i`对应`v[i]`，按照顺序排列好了，无需再写入颜色`i`，只需写入`v[i]`即可。

   ```c++
   //写入每种颜色的权重
       for (int i=0;i<256;i++){
           unsigned int weight_to_write = v[i];
           ot.write((char*)&weight_to_write,sizeof(unsigned int));
       }
   ```

   

4. 写入霍夫曼编码后的位图数据

   - 这里我将所有位图数据的霍夫曼编码长度加了起来，计算出总长度`hufSize`,并将其以`unsigned int`型写入文件，这样方便我在解压时读入位图数据

     ```c++
     unsigned int hufSize = 0;
         for (int j=0;j<hei;j++){
             for (int i=0;i<wid;i++){
                 int color = pData[j*base+i];
                 // cd[color]：该颜色对应的霍夫曼编码，string类型
                 int leng = cd[color].length(); 
                 hufSize+=leng;
             }
         }
     //写入huffman编码
     ot.write((char*)&hufSize,sizeof(unsigned int));
     ```

   - 利用一个32位，即`unsigned int`型的缓冲区`buf`来存储即将要写入文件的霍夫曼编码（之前是用`string`保存，需要转换成`unsigned int`），利用变量`buflen`记录长度，每当`buflen`达到32时，就将`buf`写入文件，将`buf`和`buflen`归零，循环往复，直到将所有的位图数据霍夫曼编码写入，注意，最后一个位图数据的霍夫曼编码可能无法用完`buf`的`32bits`的空间，所以需要将`buf`右移相应的位数，保证写入的位图数据是相连的。更多细节请参考代码。

     ```c++
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
     //最后一次写入，不足则右移
         if (buflen>0){
             buf = (buf<<(32-buflen));
             ot.write((char*)&buf,sizeof(unsigned int));
         }
     ```

### 压缩算法的复杂度分析

主要影响复杂度的是图像的大小，即width和height，以下用w和h表示

1. 建立霍夫曼树的时间复杂度：

$$
获取权值：w*h
$$

$$
建树：(n+2n-2)*(n-1)/2,n = 256
$$

$$
(n+2n-2)*(n-1)/2=97655
$$



2. 计算`hufSize`：

$$
w*h
$$

3. 写入位图数据：

$$
{\sum_{i=0}^{w*h-1}}length_i,length为每个位图数据的霍夫曼编码长度
$$

4. 总结，这里为了方便，用8表示`length`，所以总的复杂度为：

$$
O(max(97655,w*h))
$$

### 图像复原

详见ImageExtract.cpp

1. 读入无关紧要的东西，以及权值，重建霍夫曼树

   ```c++
   for (int i=0;i<256;i++){
           unsigned int weightRead;
           bmpfile.read((char*)& weightRead,sizeof(unsigned int));
           v[i] = weightRead; 
       }
   HuffmanTree(256);
   ```

2. 读入hufSize，根据hufSize确定要读入多少个32位的buf

   ```c++
   bmpfile.read((char*)&hufSize,sizeof(unsigned int));
       if (hufSize%32==0){
           T = hufSize/32;
       }else {
           T = hufSize/32 + 1;
       }
   ```

3. 读入霍夫曼编码位图数据并将其复原，方法是根据霍夫曼编码dfs重建的霍夫曼树即可，需要注意一些细节，详情见代码。

   ```c++
   while(T--){
           unsigned int buf = 0;
           bmpfile.read((char*)&buf,sizeof(unsigned int));
           string cdc = "",cd="";
           while(buf>0){
               if (buf%2==1) cdc = cdc + "1";
               else cdc = cdc + "0";
               buf/=2;
           }
       	//不足32位要补全
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
   ```

### 复原算法的复杂度分析

1. 重新建立霍夫曼树

$$
(n+2n-2)*(n-1)/2=97655
$$

2. 复原位图数据

$$
T*32 = hufSize
$$

$$
hufSize=8*w*h
$$

3. 总的时间复杂度

$$
O(max(97655,w*h))
$$

## 三、样例测试

输入lena1.bmp（8位）

![1](C:/Users/User/Documents/201901~06/%E6%95%B0%E5%AD%97%E5%9B%BE%E5%83%8F%E5%A4%84%E7%90%86/201611210137_%E8%A7%A3%E4%B9%A6%E7%85%A7_%E5%AE%9E%E9%AA%8C%E5%9B%9B_%E5%9B%BE%E5%83%8F%E5%8E%8B%E7%BC%A9%E4%B8%8E%E5%A4%8D%E5%8E%9F/asserts/lena1.bmp)

压缩后结果为，lena1.bmp.hfm，二进制文件，用一个喜欢的名字吧。

复原后结果：

![2](C:/Users/User/Documents/201901~06/%E6%95%B0%E5%AD%97%E5%9B%BE%E5%83%8F%E5%A4%84%E7%90%86/201611210137_%E8%A7%A3%E4%B9%A6%E7%85%A7_%E5%AE%9E%E9%AA%8C%E5%9B%9B_%E5%9B%BE%E5%83%8F%E5%8E%8B%E7%BC%A9%E4%B8%8E%E5%A4%8D%E5%8E%9F/asserts/lena2.bmp)

结果一模一样。

实际压缩率：

![4](C:/Users/User/Documents/201901~06/%E6%95%B0%E5%AD%97%E5%9B%BE%E5%83%8F%E5%A4%84%E7%90%86/201611210137_%E8%A7%A3%E4%B9%A6%E7%85%A7_%E5%AE%9E%E9%AA%8C%E5%9B%9B_%E5%9B%BE%E5%83%8F%E5%8E%8B%E7%BC%A9%E4%B8%8E%E5%A4%8D%E5%8E%9F/asserts/4.png)
$$
239/258 = 92.6\%
$$

## 四、实验总结

- 部分代码可以做算法复杂度优化
- 一开始我的想法是，将颜色和颜色对应的霍夫曼编码写入，再把位图数据对应的霍夫曼编码的长度和霍夫曼编码写入，很明显这是会让图像变大一倍；改正后的算法是写入权值和霍夫曼编码后的位图数据，不写入每个霍夫曼编码的长度，于是图像“扩张”就变回了图像压缩。

