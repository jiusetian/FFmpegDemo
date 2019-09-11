
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * 负责找到输入流中的frame部分，保存到data中
 * @param buffer 音频流的指针位置
 * @param buf_size 音频流中指针位置往后还剩下多大的流
 * @param data 用来保存找到的frame
 * @param data_size 用来保存frame的大小
 * @return
 */
int getADTSframe(unsigned char *buffer, int buf_size, unsigned char *data, int *data_size) {
    int size = 0;

    if (!buffer || !data || !data_size) {
        return -1;
    }

    while (1) {
        //buf_size代表buffer剩下的大小
        if (buf_size < 7) {
            return -1;
        }
        //Sync words
        //如果指针定位了到了分隔符synvword
        if ((buffer[0] == 0xff) && ((buffer[1] & 0xf0) == 0xf0)) {
            size |= ((buffer[3] & 0x03) << 11);     //high 2 bit
            size |= buffer[4] << 3;                //middle 8 bit
            size |= ((buffer[5] & 0xe0) >> 5);        //low 3bit
            break;
        }
        --buf_size;
        ++buffer; //不断移动buffer的指针
    }

    //说明此时找到了syncword，但是后面的frame不完整了
    if (buf_size < size) {
        return 1;
    }
    //找到了完整的frame，将对应的frame复制到data中
    memcpy(data, buffer, size);
    *data_size = size; //data_size 代表当前frame的大小

    return 0;
}

int simplest_aac_parser(char *url) {
    int data_size = 0;
    int size = 0; //读取到的frame的大小
    int cnt = 0;
    int offset = 0;

    //FILE *myout=fopen("output_log.txt","wb+");
    FILE *myout = stdout;

    unsigned char *aacframe = (unsigned char *) malloc(1024 * 5); //5KB
    unsigned char *aacbuffer = (unsigned char *) malloc(1024 * 1024); //1M

    //打开音频流文件
    FILE *ifile = fopen(url, "rb");
    if (!ifile) {
        printf("Open file error");
        return -1;
    }

    printf("-----+- ADTS Frame Table -+------+\n");
    printf(" NUM | Profile | Frequency| Size |\n");
    printf("-----+---------+----------+------+\n");

    while (!feof(ifile)) {
        //读取一定大小为1M的数据
        data_size = fread(aacbuffer + offset, 1, 1024 * 1024 - offset, ifile);
        unsigned char *input_data = aacbuffer;

        while (1) {
            int ret = getADTSframe(input_data, data_size, aacframe, &size);
            if (ret == -1) {
                break;
            } else if (ret == 1) { //代表读取的frame不完整
                //将不完整的frame复制到aacbuffer尾部，等待下一次解析
                memcpy(aacbuffer, input_data, data_size);
                offset = data_size;
                break;
            }

            char profile_str[10] = {0};
            char frequence_str[10] = {0};

            unsigned char profile = aacframe[2] & 0xC0;
            profile = profile >> 6;
            switch (profile) {
                case 0:
                    sprintf(profile_str, "Main");
                    break;
                case 1:
                    sprintf(profile_str, "LC");
                    break;
                case 2:
                    sprintf(profile_str, "SSR");
                    break;
                default:
                    sprintf(profile_str, "unknown");
                    break;
            }

            unsigned char sampling_frequency_index = aacframe[2] & 0x3C;
            sampling_frequency_index = sampling_frequency_index >> 2;
            switch (sampling_frequency_index) {
                case 0:
                    sprintf(frequence_str, "96000Hz");
                    break;
                case 1:
                    sprintf(frequence_str, "88200Hz");
                    break;
                case 2:
                    sprintf(frequence_str, "64000Hz");
                    break;
                case 3:
                    sprintf(frequence_str, "48000Hz");
                    break;
                case 4:
                    sprintf(frequence_str, "44100Hz");
                    break;
                case 5:
                    sprintf(frequence_str, "32000Hz");
                    break;
                case 6:
                    sprintf(frequence_str, "24000Hz");
                    break;
                case 7:
                    sprintf(frequence_str, "22050Hz");
                    break;
                case 8:
                    sprintf(frequence_str, "16000Hz");
                    break;
                case 9:
                    sprintf(frequence_str, "12000Hz");
                    break;
                case 10:
                    sprintf(frequence_str, "11025Hz");
                    break;
                case 11:
                    sprintf(frequence_str, "8000Hz");
                    break;
                default:
                    sprintf(frequence_str, "unknown");
                    break;
            }


            fprintf(myout, "%5d| %8s|  %8s| %5d|\n", cnt, profile_str, frequence_str, size);
            data_size -= size; //代表buf剩下的大小
            input_data += size; //往后移动buf的指针
            cnt++;
        }

    }
    fclose(ifile);
    free(aacbuffer);
    free(aacframe);

    return 0;
}