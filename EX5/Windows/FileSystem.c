#include<stdio.h>
#include<malloc.h>
#include<string.h>
#include<time.h>

#define BLOCKSIZE 1024  //���̿��С
#define SIZE 1024000  //������̿ռ��С
#define END 65535 //FAT�е��ļ�����״̬
#define FREE 0  //FAT���̿���б�־
#define ROOTBLOCKNUM 2  //��Ŀ¼����ռ�̿�����
#define MAXOPENFILE 10  //���ͬʱ���ļ�����
#define MAXFILESIZE 10000   //���ͬʱ���ļ���С

/* �ļ����ƿ�FCB */
typedef struct FCB
{
    char filename[8];   //�ļ���
    char exname[3];     //�ļ���չ��
    unsigned char attribute;    //�ļ������ֶ�
    unsigned short time;    //�ļ�����ʱ��
    unsigned short date;    //�ļ���������
    unsigned short first;   //�ļ���ʼ�̿��
    unsigned long length;   //�ļ�����
    char free;      //��ʾĿ¼���Ƿ�Ϊ��
}fcb;

/* �ļ������FAT */
typedef struct FAT
{
    unsigned short id;
}fat;

/* �û����ļ���USEROPEN */
typedef struct USEROPEN
{
    char filename[8];               //�ļ���
    char exname[3];                 //�ļ���չ��
    unsigned char attribute;        //�ļ�����
    unsigned short time;            //�ļ�����ʱ��
    unsigned short date;            //�ļ���������
    unsigned short first;           //�ļ���ʼ�̿��
    unsigned long length;           //�ļ�����
    char free;      //��ʾĿ¼���Ƿ�Ϊ��
    int father;    //��ʾ��Ŀ¼���ļ�������
    int dirno;      //��Ӧ���ļ���Ŀ¼���ڸ�Ŀ¼�ļ��е��̿��
    int diroff;     //��Ӧ���ļ���Ŀ¼���ڸ�Ŀ¼�ļ���dirno�̿��е�Ŀ¼�����
    char dir[MAXOPENFILE][80];      //��Ӧ���ļ����ڵ�Ŀ¼����������ټ��ָ���ļ��Ƿ��Ѿ���
    int count;      //��дָ�����ļ��е�λ��
    char fcbstate;  //�Ƿ��޸����ļ���FCB����
    char topenfile; //��ʾ���û��򿪱����Ƿ�Ϊ��
}useropen;

/* ������BLOCK0 */
typedef struct BLOCK0
{
    //�洢һЩ������Ϣ������̿��С�����̿������������ļ�����
    unsigned short fbnum;   //�ļ�ħ�����ļ�����

    char information[200];
    unsigned short root;    //��Ŀ¼�ļ�����ʼ�̿��
    unsigned char *startblock;  //�����̿�����������ʼλ��

}block0;

/* ȫ�ֱ������� */
unsigned char *myvhard;     //ָ��������̵���ʼ��ַ
useropen openfilelist[MAXOPENFILE];     //�û����ļ�������
useropen *ptrcurdir;        //ָ���û����ļ����еĵ�ǰĿ¼���ڴ��ļ������λ��
char currentdir[80];        //��¼��ǰĿ¼��Ŀ¼��
unsigned char *startp;      //��¼�����������������ʼλ��
char path[] = "./myfsys";     //����ռ䱣��·��
int curfd;


/* ��Ҫ������� */
void startsys();        //�����ļ�ϵͳ����
void my_format();       //���̸�ʽ������
void my_cd(char *dirname);          //���ĵ�ǰĿ¼����
void my_mkdir(char *dirname);       //������Ŀ¼����
void my_rmdir(char *dirname);       //ɾ����Ŀ¼����
void my_ls(void);       //��ʾĿ¼����
void my_create(char *filename);     //�����ļ�����
void my_rm(char *filename);         //ɾ���ļ�����
int my_open(char *filename);        //���ļ�����
int my_close(int fd);  //�ر��ļ�����
int my_write(int fd);   //д�ļ�����
int do_write(int fd, char *text, int len, char wstyle);    //ʵ��д�ļ�����
int my_read(int fd, int len);       //���ļ�����
int do_read(int fd, int len, char *text);   //ʵ�ʶ��ļ�����
void my_exitsys();      //�˳��ļ�ϵͳ����

unsigned short findFree();      //������һ�����д��̿�
int findFree0();    //Ѱ�ҿ����ļ�����




/* ���庯��ʵ�� */

/* �����ļ�ϵͳ���� */
void startsys()
{
    FILE *fp;
    int i;
    unsigned char buffer[SIZE];
    myvhard = (unsigned char *)malloc(SIZE);
    memset(myvhard, 0, SIZE);
    if(fp = fopen(path, "r"))   //���ļ��ɹ�
    {
        fread(buffer, SIZE, 1, fp);
        fclose(fp);
        if(buffer[0]!=0XAA || buffer[1]!=0XAA)  //�����ļ�ϵͳħ��10101010
        {
            printf("myfsys�ļ�ϵͳ������, ���ڿ�ʼ�����ļ�ϵͳ......\n");
            my_format();    //��ʽ��
        }else{      //���ļ�ϵͳ
            printf("myfsys�ļ�ϵͳ�򿪳ɹ���\n");
            for(i=0;i<SIZE;i++)
                myvhard[i] = buffer[i];     //����������е����ݱ��浽�ļ�ϵͳ��
        }
    }else{      //���ļ�ʧ��
        printf("myfsys�ļ�ϵͳ������, ���ڿ�ʼ�����ļ�ϵͳ......\n");
        my_format();    //��ʽ��
    }

    //��ʼ�����ļ�����0����Ŀ¼
    strcpy(openfilelist[0].filename, "root");   //���ø�Ŀ¼
    strcpy(openfilelist[0].exname, "di");       //�����ļ���չ��
    openfilelist[0].attribute=0X2D;     //�����ļ����ԣ��浵����ꡢϵͳ�ļ���ֻ��
    openfilelist[0].time = ((fcb *)(myvhard+5*BLOCKSIZE))->time;    //����ʱ��
    openfilelist[0].date = ((fcb *)(myvhard+5*BLOCKSIZE))->date;    //��������
    openfilelist[0].first = ((fcb *)(myvhard+5*BLOCKSIZE))->first;      //��ʼ�̿��
    openfilelist[0].length = ((fcb *)(myvhard+5*BLOCKSIZE))->length;    //�ļ�����
    openfilelist[0].free = 1;       //�ѷ���
    openfilelist[0].dirno = 5;      //��Ŀ¼������ʼ���
    openfilelist[0].diroff = 0;     //����ƫ����
    openfilelist[0].count = 0;      //��дָ�����ļ��е�λ��
    openfilelist[0].fcbstate = 0;   //�ļ��޸�λ
    openfilelist[0].topenfile = 0;  //���ļ�����Ϊ��
    openfilelist[0].father = 0;     //��Ŀ¼

    memset(currentdir, 0, sizeof((currentdir)));    //��յ�ǰ·��
    strcpy(currentdir, "\\root\\");        //��ǰ·��Ϊ��Ŀ¼
    strcpy(openfilelist->dir[0], currentdir);       //�洢��ǰ����
    startp = ((block0 *)myvhard)->startblock;       //���̿ռ����������׵�ַ
    ptrcurdir = &openfilelist[0];       //ָ�ļ���������û��򿪵�
    curfd = 0;      //��ǰ�ļ���ʶ��

}

 /* ��ʽ������ */
void my_format()
{
    FILE *fp;
    fat *fat1, *fat2;
    block0 *b0;
    time_t *now;
    struct tm *nowtime;
    unsigned char *p;
    fcb *root;
    int i;

    p = myvhard;
    b0 = (block0 *)p;
    fat1 = (fat *)(p+BLOCKSIZE);
    fat2 = (fat *)(p+BLOCKSIZE*3);

    b0->fbnum = 0XAAAA; //�ļ�ϵͳħ��
    b0->root = 5;       //��Ŀ¼��ʼ�̿��
    strcpy(b0->information, "GPB File System\nBlocksize = 1KB  Whole size = 1000KB  BlockNum = 1000  RootBlockNum = 2\n");

    //FAT1 FAT2 ǰ5�����̿��Ѿ����䣬���ΪEND
    for(i=1;i<=7;i++)
    {
        if(i==6)    //�ӵ�6�����������
        {
            fat1->id = 6;
            fat2->id = 6;
        }else{
            fat1->id = END;
            fat2->id = END;
        }
        fat1++;
        fat2++;
    }

    //���������ı�Ǳ�Ϊ����״̬
    for(i=7;i<SIZE/BLOCKSIZE;i++)
    {
        (*fat1).id = FREE;
        (*fat2).id = FREE;
        fat1++;
        fat2++;
    }

    /*  ������Ŀ¼�ļ�root�����������ĵ�һ��������Ŀ¼��
        �ڸ������ϴ������������Ŀ¼�".",".."��
        �����ļ���֮�⣬��������ͬ */
    p += BLOCKSIZE*5;
    root = (fcb *)p;

    //��ǰĿ¼
    strcpy(root->filename, ".");
    strcpy(root->exname, "di");
    root->attribute = 0X28;
    now = (time_t *)malloc(sizeof(time_t));
    time(now);
    nowtime = localtime(now);
    root->time = nowtime->tm_hour*2048 + nowtime->tm_min*32 + nowtime->tm_sec/2;
    root->date = (nowtime->tm_year-80)*512 + (nowtime->tm_mon+1)*32 + nowtime->tm_mday;
    root->first = 5;
    root->length = 2*sizeof(fcb);
    root++;

    //��һ��Ŀ¼
    strcpy(root->filename, "..");
    strcpy(root->exname, "di");
    root->attribute = 0X28;
    time(now);
    nowtime = localtime(now);
    root->time = nowtime->tm_hour*2048 + nowtime->tm_min*32 + nowtime->tm_sec/2;
    root->date = (nowtime->tm_year-80)*512 + (nowtime->tm_mon+1)*32 + nowtime->tm_mday;
    root->first = 5;
    root->length = 2*sizeof(fcb);
    root++;

    for(i=2;i<BLOCKSIZE*2/sizeof(fcb);i++,root++)
    {
        root->filename[0] = '\0';
    }
    fp = fopen(path, "w");
    b0->startblock = p + BLOCKSIZE*4;
    fwrite(myvhard, SIZE, 1, fp);
    free(now);
    fclose(fp);

}

/* ���ĵ�ǰĿ¼���� */
void my_cd(char *dirname)
{
    char *dir, text[MAXFILESIZE];
    int fd, i;
    dir = strtok(dirname, "\\");
    if(strcmp(dir, ".") == 0)
        return;
    if(strcmp(dir, "..") == 0)
    {
        fd = openfilelist[curfd].father;
        my_close(curfd);
        curfd = fd;
        ptrcurdir = &openfilelist[curfd];
        return;
    }
    if(strcmp(dir, "root") == 0)
    {
        for(i=1;i<MAXOPENFILE;i++)
        {
            if(openfilelist[i].free)
                my_close(i);
        }
        ptrcurdir = &openfilelist[0];
        curfd = 0;
        dir = strtok(NULL, "\\");
    }

    while(dir)
    {
        if((fd = my_open(dir)) > 0)
        {
            ptrcurdir = &openfilelist[fd];
            curfd = fd;
        }else
            return ;
        dir = strtok(NULL, "\\");
    }

}


/* ������Ŀ¼���� */
void my_mkdir(char *dirname)
{
    fcb *dir_fcb, *pcb_tmp;
    int size, fd, i;
    unsigned short block_num;
    char text[MAXFILESIZE], *p;
    time_t *now;
    struct tm *nowtime;

    //����ǰ���ļ���Ϣ����text�У�size��ʵ�ʶ�ȡ���ֽ���
    openfilelist[curfd].count = 0;
    size = do_read(curfd, openfilelist[curfd].length, text);
    dir_fcb = (fcb *)text;

    //����Ƿ�����ͬ��Ŀ¼��
    for(i=0;i<size/sizeof(fcb);i++)
    {
        if(strcmp(dirname, dir_fcb->filename) == 0)
        {
            printf("����: ��Ŀ¼���Ѵ���!\n");
            return ;
        }
        dir_fcb++;
    }

    //����һ�����еĴ��ļ�����
    dir_fcb = (fcb *)text;
    for(i=0;i<size/sizeof(fcb);i++)
    {
        if(strcmp(dir_fcb->filename, "") == 0)
            break;
        dir_fcb++;
    }
    openfilelist[curfd].count = i*sizeof(fcb);

    if((fd = findFree0()) < 0)
        return ;

    //Ѱ�ҿ����̿�
    block_num = findFree();
    if(block_num == END)
        return ;

    pcb_tmp = (fcb *)malloc(sizeof(fcb));
    now = (time_t *)malloc(sizeof(time_t));

    //�ڵ�ǰĿ¼���½�Ŀ¼��
    pcb_tmp->attribute = 0X30;      //���ԣ��浵����Ŀ¼
    time(now);
    nowtime = localtime(now);
    pcb_tmp->time = nowtime->tm_hour*2048 + nowtime->tm_min*32 + nowtime->tm_sec/2;
    pcb_tmp->date = (nowtime->tm_year-80)*512 + (nowtime->tm_mon+1)*32 + nowtime->tm_mday;
    strcpy(pcb_tmp->filename, dirname);
    strcpy(pcb_tmp->exname, "di");
    pcb_tmp->first = block_num;
    pcb_tmp->length = 2*sizeof(fcb);

    openfilelist[fd].attribute = pcb_tmp->attribute;
    openfilelist[fd].count = 0;
    openfilelist[fd].date = pcb_tmp->date;
    strcpy(openfilelist[fd].dir[fd], openfilelist[curfd].dir[curfd]);

    p = openfilelist[fd].dir[fd];
    while(*p != '\0')
        p++;
    strcpy(p, dirname);
    while(*p != '\0')
        p++;
    *p = '\\';
    p++;
    *p = '\0';

    openfilelist[fd].dirno = openfilelist[curfd].first;
    openfilelist[fd].diroff = i;
    strcpy(openfilelist[fd].filename, pcb_tmp->filename);
    strcpy(openfilelist[fd].exname, pcb_tmp->exname);
    openfilelist[fd].fcbstate = 1;      //�޸�λ��Ϊ1
    openfilelist[fd].first = pcb_tmp->first;
    openfilelist[fd].length = pcb_tmp->length;
    openfilelist[fd].free = 1;
    openfilelist[fd].time = pcb_tmp->time;
    openfilelist[fd].topenfile = 1;     //�򿪱���Ϊ1

    //����.����Ŀ¼��
    do_write(curfd, (char *)pcb_tmp, sizeof(fcb), 2);
    pcb_tmp->attribute = 0X28;      //���ԣ��浵�����
    time(now);
    nowtime = localtime(now);
    pcb_tmp->time = nowtime->tm_hour*2048 + nowtime->tm_min*32 + nowtime->tm_sec/2;
    pcb_tmp->date = (nowtime->tm_year-80)*512 + (nowtime->tm_mon+1)*32 + nowtime->tm_mday;
    strcpy(pcb_tmp->filename, ".");
    strcpy(pcb_tmp->exname, "di");
    pcb_tmp->first = block_num;
    pcb_tmp->length = 2*sizeof(fcb);

    //����..����Ŀ¼��
    do_write(fd, (char *)pcb_tmp, sizeof(fcb), 2);
    pcb_tmp->attribute = 0X28;
    time(now);
    nowtime = localtime(now);
    pcb_tmp->time = nowtime->tm_hour*2048 + nowtime->tm_min*32 + nowtime->tm_sec/2;
    pcb_tmp->date = (nowtime->tm_year-80)*512 + (nowtime->tm_mon+1)*32 + nowtime->tm_mday;
    strcpy(pcb_tmp->filename, "..");
    strcpy(pcb_tmp->exname, "di");
    pcb_tmp->first = openfilelist[curfd].first;
    pcb_tmp->length = openfilelist[curfd].length;

    //����д����do_writeд��FCB����
    do_write(fd, (char *)pcb_tmp, sizeof(fcb), 2);

    openfilelist[curfd].count = 0;
    do_read(curfd, openfilelist[curfd].length, text);

    pcb_tmp = (fcb *)text;
    pcb_tmp->length = openfilelist[curfd].length;
    my_close(fd);

    openfilelist[curfd].count = 0;
    do_write(curfd, text, pcb_tmp->length, 2);

}

/* ɾ����Ŀ¼���� */
void my_rmdir(char *dirname)
{
    int size, fd;
    char text[MAXFILESIZE];
    fcb *fcb_ptr, *fcb_tmp1, *fcb_tmp2;
    unsigned short block_num;
    int i, j;
    fat *fat1, *fat_ptr;
    if(strcmp(dirname, ".") == 0 || strcmp(dirname, "..") == 0)
    {
        printf("����: �޷�ɾ����Ŀ¼!\n");
        return ;
    }
    fat1 = (fat *)(myvhard + BLOCKSIZE);
    openfilelist[curfd].count = 0;
    size = do_read(curfd, openfilelist[curfd].length, text);
    fcb_ptr = (fcb *)text;
    for(i=0;i<size/sizeof(fcb);i++)
    {
        if(strcmp(dirname, fcb_ptr->filename) == 0)
            break;
        fcb_ptr++;
    }
    if(i >= size/sizeof(fcb))
    {
        printf("����: ��Ŀ¼������!\n");
        return ;
    }

    block_num = fcb_ptr->first;
    fcb_tmp2 = fcb_tmp1 = (fcb *)(myvhard+block_num*BLOCKSIZE);
    for(j=0;j<fcb_tmp1->length/sizeof(fcb);j++)
    {
        if(strcmp(fcb_tmp2->filename, ".") && strcmp(fcb_tmp2->filename, "..") && fcb_tmp2->filename[0]!='\0')
        {
            printf("����: ��Ŀ¼��Ϊ��!\n");
            return ;
        }
        fcb_tmp2++;
    }

    while(block_num != END)
    {
        fat_ptr = fat1 + block_num;
        block_num = fat_ptr->id;
        fat_ptr->id = FREE;
    }

    //���
    strcpy(fcb_ptr->filename, "");
    strcpy(fcb_ptr->exname, "");
    fcb_ptr->first = END;
    openfilelist[curfd].count = 0;
    do_write(curfd, text, openfilelist[curfd].length, 2);

}

/* ��ʾĿ¼���� */
void my_ls(void)
{
    fcb *fcb_ptr;
    int i;
    char text[MAXFILESIZE];
    unsigned short block_num;
    openfilelist[curfd].count = 0;
    do_read(curfd, openfilelist[curfd].length, text);
    fcb_ptr = (fcb *)text;
    for(i=0;i<(int)(openfilelist[curfd].length/sizeof(fcb));i++)
    {
        if(fcb_ptr->filename[0] != '\0')
        {
            if(fcb_ptr->attribute & 0X20)   //Ŀ¼��
            {
                if(strlen(fcb_ptr->filename) >= 7)
                    printf("%s\\\t<DIR>\t\t%d/%d/%d\t%02d:%02d:%02d\n", fcb_ptr->filename, ((fcb_ptr->date)>>9)+1980, ((fcb_ptr->date)>>5) & 0X000F, (fcb_ptr->date) & 0X001F, fcb_ptr->time>>11, (fcb_ptr->time>>5) & 0X003F, fcb_ptr->time & 0X001F*2);
                else
                    printf("%s\\\t\t<DIR>\t\t%d/%d/%d\t%02d:%02d:%02d\n", fcb_ptr->filename, ((fcb_ptr->date)>>9)+1980, ((fcb_ptr->date)>>5) & 0X000F, (fcb_ptr->date) & 0X001F, fcb_ptr->time>>11, (fcb_ptr->time>>5) & 0X003F, fcb_ptr->time & 0X001F*2);
            }else{
                if(strlen(fcb_ptr->filename) + strlen(fcb_ptr->exname) >= 7)
                    printf("%s.%s\t%ld B\t\t%d/%d/%d\t%02d:%02d:%02d\t\n", fcb_ptr->filename, fcb_ptr->exname, fcb_ptr->length, ((fcb_ptr->date)>>9)+1980, (fcb_ptr->date)>>5 & 0X000F, fcb_ptr->date & 0X001F, fcb_ptr->time>>11, (fcb_ptr->time>>5) & 0X003F, fcb_ptr->time & 0X001F*2);
                else
                    printf("%s.%s\t\t%ld B\t\t%d/%d/%d\t%02d:%02d:%02d\t\n", fcb_ptr->filename, fcb_ptr->exname, fcb_ptr->length, ((fcb_ptr->date)>>9)+1980, (fcb_ptr->date)>>5 & 0X000F, fcb_ptr->date & 0X001F, fcb_ptr->time>>11, (fcb_ptr->time>>5) & 0X003F, fcb_ptr->time & 0X001F*2);
            }
        }
        fcb_ptr++;
    }
    openfilelist[curfd].count = 0;
}

/* �����ļ����� */
void my_create(char *filename)
{
    char *fname, *exname, text[MAXFILESIZE];
    int fd, size, i;
    fcb *filefcb, *fcb_tmp;
    time_t *now;
    struct tm *nowtime;
    unsigned short block_num;
    fat *fat1, *fat_ptr;

    fat1 = (fat *)(myvhard+BLOCKSIZE);
    fname = strtok(filename, ".");
    exname = strtok(NULL, ".");
    if(strcmp(fname, "") == 0)
    {
        printf("����: �����ļ�����Ҫ���ļ���!\n");
        return ;
    }
    if(!exname)
    {
        printf("����: �����ļ�����Ҫ���ļ���չ��!\n");
        return ;
    }

    openfilelist[curfd].count = 0;
    size = do_read(curfd, openfilelist[curfd].length, text);
    filefcb = (fcb *)text;
    for(i=0;i<size/sizeof(fcb);i++)
    {
        if(strcmp(fname, filefcb->filename)==0 && strcmp(exname, filefcb->exname)==0)
        {
            printf("����: ���ļ����Ѿ�����!\n");
            return ;
        }
        filefcb++;
    }

    filefcb = (fcb *)text;
    for(i=0;i<size/sizeof(fcb);i++)
    {
        if(strcmp(filefcb->filename, "") == 0)
            break;
        filefcb++;
    }
    openfilelist[curfd].count = i*sizeof(fcb);

    block_num = findFree();
    if(block_num == END)
        return ;

    fcb_tmp = (fcb *)malloc(sizeof(fcb));
    now = (time_t *)malloc(sizeof(time_t));

    fcb_tmp->attribute = 0X00;
    time(now);
    nowtime = localtime(now);
    fcb_tmp->time = nowtime->tm_hour*2048 + nowtime->tm_min*32 + nowtime->tm_sec/2;
    fcb_tmp->date = (nowtime->tm_year-80)*512 + (nowtime->tm_mon+1)*32 + nowtime->tm_mday;
    strcpy(fcb_tmp->filename, fname);
    strcpy(fcb_tmp->exname, exname);
    fcb_tmp->first = block_num;
    fcb_tmp->length = 0;

    do_write(curfd, (char *)fcb_tmp, sizeof(fcb), 2);
    free(fcb_tmp);
    free(now);
    openfilelist[curfd].count = 0;
    do_read(curfd, openfilelist[curfd].length, text);
    fcb_tmp = (fcb *)text;
    fcb_tmp->length = openfilelist[curfd].length;
    openfilelist[curfd].count = 0;
    do_write(curfd, text, openfilelist[curfd].length, 2);
    openfilelist[curfd].fcbstate = 1;
    fat_ptr = (fat *)(fat1 + block_num);
    fat_ptr->id = END;

    printf("�����ļ��ɹ�!\n");
}


/* ɾ���ļ����� */
void my_rm(char *filename)
{
    char *fname, *exname;
    char text[MAXFILESIZE];
    fcb *fcb_ptr;
    int i,size;
    unsigned short block_num;
    fat *fat1, *fat_ptr;

    fat1 = (fat *)(myvhard+BLOCKSIZE);
    fname = strtok(filename, ".");
    exname = strtok(NULL, ".");
    if(!fname || strcmp(fname, "") == 0)
    {
        printf("����: ɾ���ļ�����Ҫ����ȷ���ļ���!\n");
        return ;
    }
    if(!exname)
    {
        printf("����: ɾ���ļ�����Ҫ����ȷ���ļ���չ��!\n");
        return ;
    }

    openfilelist[curfd].count = 0;
    size = do_read(curfd, openfilelist[curfd].length, text);
    fcb_ptr = (fcb *)text;
    for(i=0;i<size/sizeof(fcb);i++)
    {
        if(strcmp(fname, fcb_ptr->filename)==0 && strcmp(exname, fcb_ptr->exname)==0)
            break;
        fcb_ptr++;
    }
    if(i >= size/sizeof(fcb))
    {
        printf("����: ���ļ���������!\n");
        return ;
    }

    block_num = fcb_ptr->first;
    while(block_num != END)
    {
        fat_ptr = fat1 + block_num;
        block_num = fat_ptr->id;
        fat_ptr->id = FREE;
    }

    strcpy(fcb_ptr->filename, "");
    strcpy(fcb_ptr->exname, "");
    fcb_ptr->first = END;
    openfilelist[curfd].count = 0;
    do_write(curfd, text, openfilelist[curfd].length, 2);

}

/* ���ļ����� */
int my_open(char *filename)
{
    int i, fd, size;
    char text[MAXFILESIZE], *p, *fname, *exname;
    fcb *fcb_ptr;
    char exnamed = 0;
    fname = strtok(filename, ".");
    exname = strtok(NULL, ".");
    if(!exname)
    {
        exname = (char *)malloc(3);
        strcpy(exname, "di");
        exnamed = 1;
    }
    for(i=0;i<MAXOPENFILE;i++)
    {
        if(strcmp(openfilelist[i].filename, filename)==0 && strcmp(openfilelist[i].exname, exname)==0 && i!=curfd)
        {
            printf("����: ���ļ��Ѿ�����!\n");
            return -1;
        }
    }
    openfilelist[curfd].count = 0;
    size = do_read(curfd, openfilelist[curfd].length, text);
    fcb_ptr = (fcb *)text;

    for(i=0;i<size/sizeof(fcb);i++)
    {
        if(strcmp(filename, fcb_ptr->filename)==0 && strcmp(fcb_ptr->exname, exname)==0)
            break;
        fcb_ptr++;
    }
    if(i >= size/sizeof(fcb))
    {
        printf("����: ���ļ���������!\n");
        return curfd;
    }
    if(exnamed)
        free(exname);

    if((fd = findFree0()) == -1)
        return -1;

    strcpy(openfilelist[fd].filename, fcb_ptr->filename);
    strcpy(openfilelist[fd].exname, fcb_ptr->exname);
    openfilelist[fd].attribute = fcb_ptr->attribute;
    openfilelist[fd].count = 0;
    openfilelist[fd].date = fcb_ptr->date;
    openfilelist[fd].first = fcb_ptr->first;
    openfilelist[fd].length = fcb_ptr->length;
    openfilelist[fd].time = fcb_ptr->time;
    openfilelist[fd].father = curfd;
    openfilelist[fd].dirno = openfilelist[curfd].first;
    openfilelist[fd].diroff = i;
    openfilelist[fd].fcbstate = 0;
    openfilelist[fd].free = 1;
    openfilelist[fd].topenfile = 1;
    strcpy(openfilelist[fd].dir[fd], openfilelist[curfd].dir[curfd]);
    p = openfilelist[fd].dir[fd];
    while(*p != '\0')
        p++;
    strcpy(p, filename);
    while(*p != '\0')
        p++;

    if(openfilelist[fd].attribute & 0X20)       //���ԣ��浵
    {
        *p = '\\';
        p++;
        *p = '\0';
    }else{
        *p = '.';
        p++;
        strcpy(p, openfilelist[fd].exname);
    }
    return fd;

}


/* �ر��ļ����� */
int my_close(int fd)
{
    fcb *fa_fcb;
    char text[MAXFILESIZE];
    int fa;

    if(fd > MAXOPENFILE || fd <= 0)
    {
        printf("����: ���ļ���������!\n");
        return -1;
    }

    fa = openfilelist[fd].father;
    if(openfilelist[fd].fcbstate)
    {
        fa = openfilelist[fd].father;
        openfilelist[fa].count = 0;
        do_read(fa, openfilelist[fa].length, text);

        fa_fcb = (fcb *)(text+sizeof(fcb)*openfilelist[fd].diroff);
        fa_fcb->attribute = openfilelist[fd].attribute;
        fa_fcb->date = openfilelist[fd].date;
        fa_fcb->first = openfilelist[fd].first;
        fa_fcb->length = openfilelist[fd].length;
        fa_fcb->time = openfilelist[fd].time;
        strcpy(fa_fcb->filename, openfilelist[fd].filename);
        strcpy(fa_fcb->exname, openfilelist[fd].exname);

        openfilelist[fa].count = 0;
        do_write(fa, text, openfilelist[fa].length, 2);

    }

    openfilelist[fd].attribute = 0;
    openfilelist[fd].count = 0;
    openfilelist[fd].date = 0;
    strcpy(openfilelist[fd].dir[fd], "");
    strcpy(openfilelist[fd].filename, "");
    strcpy(openfilelist[fd].exname, "");
    openfilelist[fd].length = 0;
    openfilelist[fd].time = 0;
    openfilelist[fd].free = 0;
    openfilelist[fd].topenfile = 0;

    return fa;
}


/* д�ļ����� */
int my_write(int fd)
{
    int wstyle=-1, wlen=0;
    fat *fat1, *fat_ptr;
    unsigned short block_num;
    unsigned char *block_ptr;
    char text[MAXFILESIZE];
    fat1 = (fat *)(myvhard + BLOCKSIZE);
    if(fd > MAXOPENFILE)
    {
        printf("���ļ���������!\n");
        return -1;
    }
    while(wstyle<0 || wstyle>3)
    {
        printf("��ѡ��д�ļ��ķ�ʽ:\n(1) �ض�д     (2) ����д    ��3��׷��д    (0) ����\n");
        scanf("%d", &wstyle);
        getchar();
        switch(wstyle)
        {
            case 1:     //�ض�д
            {
                block_num = openfilelist[fd].first;
                fat_ptr = fat1 + block_num;
                while(fat_ptr->id != END)
                {
                    block_num = fat_ptr->id;
                    fat_ptr->id = FREE;
                    fat_ptr = fat1 + block_num;
                }
                fat_ptr->id = FREE;
                block_num = openfilelist[fd].first;
                fat_ptr = fat1 + block_num;
                fat_ptr->id = END;
                openfilelist[fd].length = 0;
                openfilelist[fd].count = 0;
                break;
            }
            case 2:     //����д
            {
                openfilelist[fd].count = 0;
                break;
            }
            case 3:     //׷��д
            {
                openfilelist[fd].count = openfilelist[fd].length;
                break;
            }
            case 0:
            {
                return -1;
            }
            default:
                break;
        }
    }
    printf("������д�������:(##END##��������)\n");
    while(gets(text))
    {
        if(strcmp(text, "##END##") == 0)
            break;
        int len = strlen(text);
        text[len++] = '\n';
        text[len] = '\0';
        if(do_write(fd, text, len, wstyle) > 0)
            wlen += len;
        else
            return -1;
    }
    if(openfilelist[fd].count > openfilelist[fd].length)
        openfilelist[fd].length = openfilelist[fd].count;

    openfilelist[fd].fcbstate = 1;
    return wlen;
}

/* ʵ��д�ļ����� */
int do_write(int fd, char *text, int len, char wstyle)
{
    unsigned char *buf;
    unsigned short block_num;
    int offset, len_tmp1=0, len_tmp2=0, rwptr, i;
    unsigned char *block_ptr, *p;
    char *text_tmp, flag;
    fat *fat1, *fat_ptr;
    fat1 = (fat *)(myvhard + BLOCKSIZE);
    text_tmp = text;

    //������ʱ������
    buf = (unsigned char *)malloc(BLOCKSIZE);
    if(buf == NULL)
    {
        printf("�����ڴ�ռ�ʧ��!\n");
        return -1;
    }

    rwptr = offset = openfilelist[fd].count;
    block_num = openfilelist[fd].first;
    fat_ptr = fat1 + block_num;
    while(offset >= BLOCKSIZE)
    {
        offset = offset - BLOCKSIZE;
        block_num = fat_ptr->id;
        if(block_num == END)
        {
            block_num = fat_ptr->id = findFree();
            if(block_num == END)
                return -1;
            fat_ptr = fat1 + block_num;
            fat_ptr->id = END;
        }
        fat_ptr = fat1 + block_num;
    }

    fat_ptr->id = END;
    block_ptr = (unsigned char *)(myvhard + block_num*BLOCKSIZE);
    while(len_tmp1 < len)
    {
        for(i=0;i<BLOCKSIZE;i++)
            buf[i] = 0;
        for(i=0;i<BLOCKSIZE;i++)
        {
            buf[i] = block_ptr[i];
            len_tmp2++;
            if(len_tmp2 == openfilelist[curfd].length)
                break;
        }

        for(p=buf+offset;p<buf+BLOCKSIZE;p++)
        {
            *p = *text_tmp;
            len_tmp1++;
            text_tmp++;
            offset++;
            if(len_tmp1 == len)
                break;
        }
        for(i=0;i<BLOCKSIZE;i++)
            block_ptr[i] = buf[i];
        openfilelist[fd].count = rwptr + len_tmp1;
        if(offset >= BLOCKSIZE)
        {
            offset = offset - BLOCKSIZE;
            block_num = fat_ptr->id;
            if(block_num == END)
            {
                block_num = fat_ptr->id = findFree();
                if(block_num == END)
                    return -1;
                fat_ptr = fat1 + block_num;
                fat_ptr->id = END;
            }
            fat_ptr = fat1 + block_num;
            block_ptr = (unsigned char *)(myvhard+block_num*BLOCKSIZE);
        }
    }
    free(buf);
    if(openfilelist[fd].count > openfilelist[fd].length)
        openfilelist[fd].length  = openfilelist[fd].count;
    openfilelist[fd].fcbstate = 1;

    return len_tmp1;
}


/* Ѱ����һ�������̿�� */
unsigned short findFree()
{
    unsigned short i;
    fat *fat1, *fat_ptr;

    fat1 = (fat *)(myvhard + BLOCKSIZE);
    for(i=6;i<END;i++)
    {
        fat_ptr = fat1 + i;
        if(fat_ptr->id == FREE)
            return i;
    }
    printf("����: �޷��ҵ������̿�!\n");
    return END;
}

/* Ѱ�ҿ����ļ����� */
int findFree0()
{
    int i;
    for(i=0;i<MAXOPENFILE;i++)
    {
        if(openfilelist[i].free == 0)
            return i;
    }
    printf("����: �޷��ҵ������ļ�����!\n");
    return -1;
}


/* ���ļ����� */
int my_read(int fd, int len)
{
    char text[MAXFILESIZE];
    if(fd > MAXOPENFILE)
    {
        printf("���ļ���������!\n");
        return -1;
    }
    openfilelist[curfd].count = 0;
    if(do_read(fd, len, text)>0)
    {
        text[len] = '\0';
        printf("%s\n", text);
    }else if(len == 0){

    }else{
        printf("������!\n");
        return -1;
    }
    return len;
}

/* ʵ�ʶ��ļ����� */
int do_read(int fd, int len, char *text)
{
    unsigned char *buf;
    unsigned short block_num;
    int offset, len_tmp, i;
    unsigned char *block_ptr, *p;
    char *text_tmp;
    fat *fat1, *fat_ptr;
    fat1 = (fat *)(myvhard + BLOCKSIZE);
    len_tmp = len;
    text_tmp = text;

    //���뻺����
    buf = (unsigned char *)malloc(1024);
    if(buf == NULL)
    {
        printf("�����ڴ�ռ�ʧ��!\n");
        return -1;
    }

    offset = openfilelist[fd].count;
    block_num = openfilelist[fd].first;
    fat_ptr = fat1 + block_num;
    while(offset >= BLOCKSIZE)
    {
        offset = offset - BLOCKSIZE;
        block_num = fat_ptr->id;
        fat_ptr = fat1 + block_num;
        if(block_num == END)
        {
            printf("����: �ô��̿��Ѿ�����!\n");
            return -1;
        }
    }

    block_ptr = (unsigned char *)(myvhard + block_num*BLOCKSIZE);
    for(i=0;i<BLOCKSIZE;i++)
        buf[i] = block_ptr[i];

    while(len > 0)
    {
        if(BLOCKSIZE - offset > len)
        {   //����offset��ʼ��buf�е�len���ȶ���text��
            for(p=buf+offset;len>0;p++,text_tmp++)
            {
                *text_tmp = *p;
                len--;
                offset++;
                openfilelist[fd].count++;
            }
        }else{  //����offset��ʼ��ʣ�����ݶ���text��
            for(p=buf+offset;p<buf+BLOCKSIZE;p++,text_tmp++)
            {
                *text_tmp = *p;
                len--;
                openfilelist[fd].count++;
            }
            offset = 0;
            block_num = fat_ptr->id;
            fat_ptr = fat1 + block_num;
            block_ptr = (unsigned char *)(myvhard + block_num*BLOCKSIZE);
            for(i=0;i<BLOCKSIZE;i++)
                buf[i] = block_ptr[i];
        }
    }
    free(buf);
    return len_tmp - len;
}

/* �˳��ļ�ϵͳ���� */
void my_exitsys()
{
    FILE *fp;
    fcb *rootfcb;
    char text[MAXFILESIZE];
    while(curfd)
        curfd = my_close(curfd);

    if(openfilelist[curfd].fcbstate)
    {   //����������̿ռ�
        openfilelist[curfd].count = 0;
        do_read(curfd, openfilelist[curfd].length, text);
        rootfcb = (fcb *)text;
        rootfcb->length = openfilelist[curfd].length;
        openfilelist[curfd].count = 0;
        do_write(curfd, text, openfilelist[curfd].length, 2);
    }
    fp = fopen(path, "w");
    fwrite(myvhard, SIZE, 1, fp);
    free(myvhard);      //�ͷ�����ռ�
    fclose(fp);
}

/* ��ʼҳ�� */
void StartPage()
{
    printf("*****************************File System*****************************\n\n");
	printf("������\t\t�������\t\t����˵��\n\n");
    printf("ls\t\t��\t\t\t��ʾ��ǰĿ¼�µ�Ŀ¼���ļ�\n");
    printf("cd\t\tĿ¼��\t\t\t�л���ǰĿ¼��ָ��Ŀ¼\n");
	printf("mkdir\t\tĿ¼��\t\t\t�ڵ�ǰĿ¼������Ŀ¼\n");
	printf("rmdir\t\tĿ¼��\t\t\t�ڵ�ǰĿ¼ɾ��ָ��Ŀ¼\n");
	printf("create\t\t�ļ���\t\t\t�ڵ�ǰĿ¼�´���ָ���ļ�\n");
	printf("rm\t\t�ļ���\t\t\t�ڵ�ǰĿ¼��ɾ��ָ���ļ�\n");
	printf("open\t\t�ļ���\t\t\t�ڵ�ǰĿ¼�´�ָ���ļ�\n");
    printf("close\t\t��\t\t\t�ڴ��ļ�״̬�£��رո��ļ�\n");
    printf("read\t\t��\t\t\t�ڴ��ļ�״̬�£���ȡ���ļ�\n");
    printf("write\t\t��\t\t\t�ڴ��ļ�״̬�£�д���ļ�\n");
	printf("exit\t\t��\t\t\t�˳�ϵͳ\n\n");
	printf("*********************************************************************\n\n");

}

/* ������ */
void main()
{
    char cmd[12][10] = {"ls", "cd", "mkdir", "rmdir", "create", "rm", "open", "close", "read", "write", "exit"};
    char buf[30], *str;
    int cmd_num, i;

    startsys();     //�����ļ�ϵͳ
    StartPage();    //��ʼҳ��

    while(1)
    {
        printf("%s > ", openfilelist[curfd].dir[curfd]);
        gets(buf);
        cmd_num = -1;
        if(strcmp(buf, ""))
        {
            str = strtok(buf, " ");
            for(i=0;i<12;i++)
            {
                if(strcmp(str, cmd[i]) == 0)
                {
                    cmd_num = i;
                    break;
                }
            }
            switch(cmd_num)
            {
                case 0:{    //ls
                    if(openfilelist[curfd].attribute & 0X20)
                    {
                        my_ls();
                        putchar('\n');
                    }else
                        printf("������Ϸ�������!\n");
                    break;
                }
                case 1:{    //cd
                    str = strtok(NULL, " ");
                    if(str && openfilelist[curfd].attribute & 0X20)
                        my_cd(str);
                    else
                        printf("������Ϸ�������!\n");
                    break;
                }
                case 2:{    //mkdir
                    str = strtok(NULL, " ");
                    if(str && openfilelist[curfd].attribute & 0X20)
                        my_mkdir(str);
                    else
                        printf("������Ϸ�������!\n");
                    break;
                }
                case 3:{    //rmdir
                    str = strtok(NULL, " ");
                    if(str && openfilelist[curfd].attribute & 0X20)
                        my_rmdir(str);
                    else
                        printf("������Ϸ�������!\n");
                    break;
                }
                case 4:{    //create
                    str = strtok(NULL, " ");
                    if(str && openfilelist[curfd].attribute & 0X20)
                        my_create(str);
                    else
                        printf("������Ϸ�������!\n");
                    break;
                }
                case 5:{    //rm
                    str = strtok(NULL, " ");
                    if(str && openfilelist[curfd].attribute & 0X20)
                        my_rm(str);
                    else
                        printf("������Ϸ�������!\n");
                    break;
                }
                case 6:{    //open
                    str = strtok(NULL, " ");
                    if(str && openfilelist[curfd].attribute & 0X20)
                    {
                        if(strchr(str, '.'))
                            curfd = my_open(str);
                        else
                            printf("���ļ���Ҫ���ļ���չ��!\n");
                    }else
                        printf("������Ϸ�������!\n");
                    break;
                }
                case 7:{    //close
                    if(!(openfilelist[curfd].attribute & 0X20))
                        curfd = my_close(curfd);
                    else
                        printf("���ļ���û�б���!\n");
                    break;
                }
                case 8:{    //read
                    if(!(openfilelist[curfd].attribute & 0X20))
                        my_read(curfd, openfilelist[curfd].length);
                    else
                        printf("���ļ���û�б���!\n");
                    break;
                }
                case 9:{    //write
                    if(!(openfilelist[curfd].attribute & 0X20))
                        my_write(curfd);
                    else
                        printf("���ļ���û�б���!\n");
                    break;
                }
                case 10:{   //exit
                    if(openfilelist[curfd].attribute & 0X20)
                    {
                        my_exitsys();
                        return ;
                    }else
                        printf("������Ϸ�������!\n");
                    break;
                }
                default:{
                    printf("������Ϸ�������!\n");
                    break;
                }
            }
        }
    }

}
