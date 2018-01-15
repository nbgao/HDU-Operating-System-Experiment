#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define SIZE 512

int main()
{
    int fd;
    char *input = "Test char device.";
    char output[SIZE] = {0};

    fd = open("../mycdev", O_RDWR);
    if(fd < 0)
    {
        printf("打开设备文件失败!\n");
    }else{
        printf("打开设备文件成功!\n");
    }

    write(fd, input, strlen(input));
    read(fd, output, SIZE);

    printf("字符设备中的数据为：%s\n", output);

    ioctl(fd, 2017);
    ioctl(fd, 2018);
    ioctl(fd, 2019);


    close(fd);

    return 0;
}
