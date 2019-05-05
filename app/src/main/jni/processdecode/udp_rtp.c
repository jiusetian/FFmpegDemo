#include <stdio.h>
//#include <winsock2.h>
#include <sys/socket.h>

#pragma comment(lib, "ws2_32.lib")
//#pragma pack是指定数据在内存中的对齐方式
//意思就是 以下代码编译出来的是以1个字节的方式对齐的。这样能节约内存资源，但是会在效率上有所影响
//虽说在效率上有一定的影响，不过，如果编写的是基于协议，如串口通讯的程序，那么必须严格按照一定的规则进行接收数据包。
// 那么，只要#pragma pack(1)，让数据在内存中是连续的，才好处理的。
#pragma pack(1)


int simplest_udp_parser(int port){

}