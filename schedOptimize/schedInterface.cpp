//
// Created by lynne on 3/24/20.
//

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

//动态链接库路径 /home/lynne/CLionProjects/ResEneAwareOpt/cmake-build-debug/ResEneAwareOpt/schedOptimize
#define LIB_CACULATE_PATH "/home/lynne/CLionProjects/ResEneAwareOpt/schedOptimize/libcaculate.so"
//#define LIB_CACULATE_PATH "./libmhz.so"

//函数指针
typedef int (*CAC_FUNC)(int, int);
typedef void (*PRINT_FUNC)();
typedef int (*RUN_MHZ)();

int main()
{

    void *handle;
    char *error;
    CAC_FUNC cac_func = NULL;

    //打开动态链接库
    handle = dlopen(LIB_CACULATE_PATH, RTLD_LAZY);
    if (!handle) {
    fprintf(stderr, "%s\n", dlerror());
    exit(EXIT_FAILURE);
    }

    //清除之前存在的错误
    dlerror();

    //获取一个函数
    *(void **) (&cac_func) = dlsym(handle, "add");
    if ((error = dlerror()) != NULL)  {
    fprintf(stderr, "%s\n", error);
    exit(EXIT_FAILURE);
    }
    printf("add: %d\n", (*cac_func)(2,7));

    cac_func = (CAC_FUNC)dlsym(handle, "sub");
    printf("sub: %d\n", cac_func(9,2));

    cac_func = (CAC_FUNC)dlsym(handle, "mul");
    printf("mul: %d\n", cac_func(3,2));

    cac_func = (CAC_FUNC)dlsym(handle, "div");
    printf("div: %d\n", cac_func(8,2));

    //PRINT_FUNC print_func = NULL;
    //*(void **) (&print_func) = dlsym(handle, "print");
    //(*print_func)();

/**
    void *handle;
    char *error;

    //打开动态链接库
    handle = dlopen(LIB_CACULATE_PATH, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    //清除之前存在的错误
    dlerror();
    RUN_MHZ run_mhz = dlsym(handle, "run");
    run_mhz();

    //关闭动态链接库
    dlclose(handle);
    exit(EXIT_SUCCESS);
 **/
}
