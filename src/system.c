#include <stdio.h>
#include "system.h"

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}//清空输入缓存区函数
