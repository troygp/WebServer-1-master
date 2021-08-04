#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

int main(){
    char* srcDir_;
    struct stat mmFileStat_;
    srcDir_ = getcwd(nullptr, 256);
    strncat(srcDir_, "/404.html", 16);
    printf("%s\n", srcDir_);
    int ret = stat((srcDir_), &mmFileStat_);
    if(ret < 0){
        printf("失败\n");
    }

    return 0;
}