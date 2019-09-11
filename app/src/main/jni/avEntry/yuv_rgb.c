/**
 * 视音频数据处理入门：RGB、YUV像素数据处理
 * 本文中像素的采样位数一律为8bit。由于1Byte=8bit，所以一个像素的一个分量的采样值占用1Byte
 */
#include <jni.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>

/**
 * 分离YUV420P像素数据中的Y、U、V分量
 * 在YUV420P中，如果视频帧的宽和高分别为w和h，那么一帧YUV420P像素数据一共占用w*h*3/2 Byte的数据，其中前w*h像素存Y，后面w*h/4像素分别存储U和V
 *
 * Split Y, U, V planes in YUV420P file.
 * @param url  Location of Input YUV file. 位置
 * @param w    Width of Input YUV file. YUV图片的宽度
 * @param h    Height of Input YUV file. YUV图片的高度
 * @param num  Number of frames to process. 多少帧要处理
 */
int simplest_yuv420_split(char *url, int w, int h, int num) {

    FILE *fp = fopen(url, "rb+");
    FILE *fp1 = fopen("output_420_y.y", "wb+");
    FILE *fp2 = fopen("output_420_u.y", "wb+");
    FILE *fp3 = fopen("output_420_v.y", "wb+");

    //分配内存空间
    unsigned char *pic = (unsigned char *) malloc(w * h * 3 / 2);

    //分别将yuv三个数据写入到不同的文件中
    for (int i = 0; i < num; i++) {
        //将文件中的像素读取到内存pic中
        //读取一帧YUV420像素数据保存在pic中
        fread(pic, 1, w * h * 3 / 2, fp);
        //y，前面w*h个byte存储了y分量
        /*
        ptr -- 这是指向要被写入的元素数组的指针。
        size -- 这是要被写入的每个元素的大小，以字节为单位。
        nmemb -- 这是元素的个数，每个元素的大小为 size 字节。
        stream -- 这是指向 FILE 对象的指针，该 FILE 对象指定了一个输出流
        */
        fwrite(pic, 1, w * h, fp1);
        //u
        fwrite(pic + w * h, 1, w * h / 4, fp2);
        //v
        fwrite(pic + w * h * 5 / 4, 1, w * h / 4, fp3);
    }

    //释放资源
    free(pic);
    fclose(fp);
    fclose(fp1);
    fclose(fp2);
    fclose(fp3);

    return 0;
}

/**
 * 分离YUV444P像素数据中的Y、U、V分量
 * 如果视频帧的宽和高分别为w和h，那么一帧YUV444P像素数据一共占用w*h*3 Byte的数据。其中前w*h Byte存储Y，接着的w*h Byte存储U，最后w*h Byte存储V
 */
int simplest_yuv444_split(char *url, int w, int h, int num) {
    FILE *fp = fopen(url, "rb+");
    FILE *fp1 = fopen("output_444_y.y", "wb+");
    FILE *fp2 = fopen("output_444_u.y", "wb+");
    FILE *fp3 = fopen("output_444_v.y", "wb+");

    //分配内存空间，其中unsigned char是一个byte大小，所以w*H*3是指byte的大小，然后pic指针指向第一个byte的内存地址
    unsigned char *pic = (unsigned char *) malloc(w * h * 3);

    for (int i = 0; i < num; i++) {
        //首先将文件读取到内存中，然后再分配
        fread(pic, 1, w * h * 3, fp);
        //分别读取
        fread(pic, 1, w * h, fp1);
        fread(pic + w * h, 1, w * h, fp2);
        fread(pic + w * h * 2, 1, w * h, fp3);
    }

    free(pic);
    fclose(fp);
    fclose(fp1);
    fclose(fp2);
    fclose(fp3);
    return 0;
}


/**
 *  将YUV420P像素数据去掉颜色（变成灰度图）
 *  我们需要将U和V分别设置为128即可
 *  这是因为U、V是图像中的经过偏置处理的色度分量。色度分量在偏置处理前的取值范围是-128至127，这时候的无色对应的是0值。经过偏置后色度分量取值变成了0至255
 *  因而此时的无色对应的就是128了。
 */
int simplest_yuv420_gray(char *url, int w, int h, int num) {

    FILE *fp = fopen(url, "rb+");
    FILE *fp1 = fopen("output_gray.yuv", "wb+");
    unsigned char *pic = (unsigned char *) malloc(w * h * 3 / 2);

    for (int i = 0; i < num; ++i) {
        fread(pic, 1, w * h * 3 / 2, fp);
        //通过memset函数，将u和v部分的值设置为128
        memset(pic + w * h, 128, w * h / 2);
        //将改变过的yv值一起写入到fp1文件中
        fwrite(pic, 1, w * h * 3 / 2, fp1);
    }

    free(pic);
    fclose(fp);
    fclose(fp1);
    return 0;
}

/**
 * 将YUV420P像素数据的亮度减半
 * 打算将亮度减半，只要将图像的每个像素的Y值取出来，除以2就可以了
 */
int simplest_yuv420_halfy(char *url, int w, int h, int num) {
    FILE *fp = fopen(url, "rb+");
    FILE *fp1 = fopen("output_half.yuv", "wb+");

    unsigned char *pic = (unsigned char *) malloc(w * h * 3 / 2);

    for (int i = 0; i < num; ++i) {
        fread(pic, 1, w * h * 3 / 2, fp);
        //遍历Y像素，将所有Y的值减半
        for (int j = 0; j < w * h; j++) {
            unsigned temp = pic[j] / 2;
            pic[j] = temp;
        }
        //将改变过的pic写入到文件中
        fwrite(pic, 1, w * h * 3 / 2, fp1);
    }
    //释放资源
    free(pic);
    fclose(fp);
    fclose(fp1);
    return 0;
}

/**
 * 将YUV420P像素数据的周围加上边框
 * Add border for YUV420P file
 * @param url     Location of Input YUV file.
 * @param w       Width of Input YUV file.
 * @param h       Height of Input YUV file.
 * @param border  Width of Border.
 * @param num     Number of frames to process.
 *
 */
int simplest_yuv420_border(char *url, int w, int h, int border, int num) {
    FILE *fp = fopen(url, "rb+");
    FILE *fp1 = fopen("output_border.yuv", "wb+");

    unsigned char *pic = (unsigned char *) malloc(w * h * 3 / 2);

    for (int i = 0; i < num; i++) {
        fread(pic, 1, w * h * 3 / 2, fp);
        //Y 遍历所有的Y变量
        for (int j = 0; j < h; j++) {
            for (int k = 0; k < w; k++) {
                //k<border是指左边框范围，k>(w-border)是右边框范围，j<border是上边框范围，j>(h-border)是下边框范围
                if (k < border || k > (w - border) || j < border || j > (h - border)) {
                    //改变当前位置的Y的值为255
                    pic[j * w + k] = 255;
                    //pic[j*w+k]=0;
                }
            }
        }

        fwrite(pic, 1, w * h * 3 / 2, fp1);
    }

    free(pic);
    fclose(fp);
    fclose(fp1);

    return 0;
}


/**
 *  分离RGB24像素数据中的R、G、B分量
 *  GB24格式规定首先存储第一个像素的R、G、B，然后存储第二个像素的R、G、B…以此类推。
 *  类似于YUV420P的存储方式称为Planar方式，而类似于RGB24的存储方式称为Packed方式
 */
int simplest_rgb24_split(char *url, int w, int h, int num) {

    FILE *fp = fopen(url, "rb+");
    FILE *fp1 = fopen("output_r.y", "wb+");
    FILE *fp2 = fopen("output_g.y", "wb+");
    FILE *fp3 = fopen("output_b.y", "wb+");

    unsigned char *pic = (unsigned char *) malloc(w * h * 3);

    for (int i = 0; i < num; ++i) {
        fread(pic, 1, w * h * 3, fp);
        //因为rgb24格式的每个像素的三个分量的连续存储的
        for (int j = 0; j < w * h * 3; j = j + 3) {
            //第一个参数，代表开始写的位置；第二个参数代表每次写入的大小，第三个参数代表写入的个数
            fwrite(pic + j, 1, 1, fp1);
            fwrite(pic + j + 1, 1, 1, fp2);
            fwrite(pic + j + 2, 1, 1, fp3);
        }
    }
    return 0;
}

/**
 * 将RGB24格式像素数据封装为BMP图像
 * 通过代码可以看出，改程序完成了主要完成了两个工作：
 * 1)将RGB数据前面加上文件头。
 * 2)将RGB数据中每个像素的“B”和“R”的位置互换。
 * BMP文件是由BITMAPFILEHEADER、BITMAPINFOHEADER、RGB像素数据共3个部分构成，它的结构如下图所示。
 */
int simplest_rgb24_to_bmp(const char *rgb24path, int width, int height, const char *bmppath) {

    typedef struct {
        long imageSize;
        long blank;
        long startPosition;
    } BmpHeader;

    typedef struct {
        long Length;
        long width;
        long height;
        unsigned short colorPlane;
        unsigned short bitColor;
        long zipFormat;
        long realSize;
        long xPels;
        long yPels;
        long colorUse;
        long colorImportant;
    } InfoHeader;

    int i = 0, j = 0;
    BmpHeader m_BMPHeader = {0};
    InfoHeader m_BMPInfoHeader = {0};
    char bfType[] = {'B', 'M'};
    int header_size = sizeof(bfType) + sizeof(m_BMPHeader) + sizeof(m_BMPInfoHeader);
    //缓冲流
    unsigned char *rgb24_buffer = NULL;
    //读取rgb数据的文件和存储bmp数据的文件
    FILE *fp_rgb24 = NULL, *fp_bmp = NULL;

    if (fp_rgb24 = fopen(rgb24path, "rb") == NULL) {
        printf("不能打开rgb文件\n");
        return -1;
    }

    if (fp_bmp = fopen(bmppath, "wb") == NULL) {
        printf("bmp文件不能打开\n");
        return -1;
    }

    //为rgb缓冲控件分配内存
    rgb24_buffer = (unsigned char *) malloc(width * height * 3);
    //将文件中rgb数据写入缓存中
    fread(rgb24_buffer, 1, width * height * 3, fp_rgb24);

    //给header赋值
    m_BMPHeader.imageSize = height * width * 3;
    m_BMPHeader.startPosition = header_size;

    m_BMPInfoHeader.Length = sizeof(InfoHeader);
    m_BMPInfoHeader.height = height;
    m_BMPInfoHeader.width = width;

    m_BMPInfoHeader.height = -height;
    m_BMPInfoHeader.colorPlane = 1;
    m_BMPInfoHeader.bitColor = 24;
    m_BMPInfoHeader.realSize = 3 * width * height;

    //将header写入bmp文件
    fwrite(bfType, 1, sizeof(bfType), fp_bmp);
    fwrite(&m_BMPHeader, 1, sizeof(m_BMPHeader), fp_bmp);
    fwrite(&m_BMPInfoHeader, 1, sizeof(m_BMPInfoHeader), fp_bmp);

    //BMP save R1|G1|B1,R2|G2|B2 as B1|G1|R1,B2|G2|R2
    //所以我们要讲R和B的位置调换
    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            char temp = rgb24_buffer[(j * width + i) * 3 + 2];
            rgb24_buffer[(j * width + i) * 3 + 2] = rgb24_buffer[(j * width + i) * 3 + 0];
            rgb24_buffer[(j * width + i) * 3 + 0] = temp;
        }
    }

    //将缓存区调还过R和B位置的rgb数据写入bmp文件
    fwrite(rgb24_buffer, 1, width * height * 3, fp_bmp);
    //释放资源
    fclose(fp_rgb24);
    fclose(fp_bmp);
    free(rgb24_buffer);
    return 0;

}

/**
 * Convert RGB24 file to YUV420P file
 * @param url_in  Location of Input RGB file.
 * @param w       Width of Input RGB file.
 * @param h       Height of Input RGB file.
 * @param num     Number of frames to process.
 * @param url_out Location of Output YUV file.
 */
int simplest_rgb24_to_yuv420(char *url_in, int w, int h, int num, char *url_out) {
    FILE *fp = fopen(url_in, "rb+");
    FILE *fp1 = fopen(url_out, "wb+");

    unsigned char *pic_rgb24 = (unsigned char *) malloc(w * h * 3);
    unsigned char *pic_yuv420 = (unsigned char *) malloc(w * h * 3 / 2);

    for (int i = 0; i < num; i++) {
        fread(pic_rgb24, 1, w * h * 3, fp);
        RGB24_TO_YUV420(pic_rgb24, w, h, pic_yuv420);
        fwrite(pic_yuv420, 1, w * h * 3 / 2, fp1);
    }

    free(pic_rgb24);
    free(pic_yuv420);
    fclose(fp);
    fclose(fp1);

    return 0;
}


unsigned char clip_value(unsigned char x, unsigned char min_val, unsigned char max_val) {
    if (x > max_val) {
        return max_val;
    } else if (x < min_val) {
        return min_val;
    } else {
        return x;
    }
}

//RGB to YUV420
/**
 *
 * @param RgbBuf 保存rgb数据的指针
 * @param w
 * @param h
 * @param yuvBuf
 * @return
 */
int RGB24_TO_YUV420(unsigned char *RgbBuf, int w, int h, unsigned char *yuvBuf) {
    unsigned char *ptrY, *ptrU, *ptrV, *ptrRGB;
    memset(yuvBuf, 0, w * h * 3 / 2);
    ptrY = yuvBuf;
    ptrU = yuvBuf + w * h;
    ptrV = ptrU + (w * h * 1 / 4);
    unsigned char y, u, v, r, g, b;
    for (int j = 0; j < h; j++) {
        ptrRGB = RgbBuf + w * j * 3;
        for (int i = 0; i < w; i++) {

            r = *(ptrRGB++);
            g = *(ptrRGB++);
            b = *(ptrRGB++);
            y = (unsigned char) ((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
            u = (unsigned char) ((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
            v = (unsigned char) ((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
            *(ptrY++) = clip_value(y, 0, 255);
            if (j % 2 == 0 && i % 2 == 0) {
                *(ptrU++) = clip_value(u, 0, 255);
            } else {
                if (i % 2 == 0) {
                    *(ptrV++) = clip_value(v, 0, 255);
                }
            }
        }
    }
    return 1;
}
































