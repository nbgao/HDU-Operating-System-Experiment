#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define SIZE 512

int main()
{
    int fd;
    char *input1 = "Test char device.";
    char *input2 = "Gao Pengbing";
    char output[512] = {0};
    fd = open("../mycdev", O_RDWR);
    if(fd < 0)
    {
        printf("打开设备文件失败!\n");
    }else{
        printf("打开设备文件成功!\n");
    }

    write(fd, input1, strlen(input1));
    write(fd, input2, strlen(input2));

    read(fd, &output, SIZE);
    printf("字符设备中的数据为：%s\n", output);

    close(fd);

    return 0;
}
