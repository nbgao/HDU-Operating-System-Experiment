#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define SIZE 512

int main()
{
    int fd;
    char input[SIZE];
    char output[SIZE] = {0};
    fd = open("../mycdev", O_RDWR);
    if(fd < 0)
    {
        printf("打开设备文件失败!\n");
    }else{
        printf("打开设备文件成功!\n");


        printf("请输入数据: (exit:退出程序 clean:清空设备数据)\n");
        scanf("%s[^\n]", input);
        int count = 0;
        while(strcmp(input, "exit")!=0 && strcmp(input, "clean")!=0 && count <= 10)
        {
            write(fd, input, strlen(input));
            scanf("%s[^\n]", input);
            count++;
        }
        if(strcmp(input, "clean") == 0)
        {
            printf("清空设备数据\n");
            ioctl(fd, 2019);
        }else{
            read(fd, &output, SIZE);
            printf("字符设备中的数据为：%s\n", output);

            ioctl(fd, 2017);
            ioctl(fd, 2018);
        }
    }
    close(fd);

    return 0;
}
