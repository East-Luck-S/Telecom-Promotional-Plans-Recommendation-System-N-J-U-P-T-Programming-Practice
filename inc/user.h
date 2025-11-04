#ifndef USER_H
#define USER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// 工具函数
void clearInputBuffer();                  // 清空输入缓冲区
int isStrEmpty(const char* str);          // 判断字符串是否为空
void trimStr(char* str);                  // 去除字符串首尾空格

// 文件操作函数
int loadPackagesFromText();               // 从文本文件加载套餐
int savePackagesToText();                 // 保存套餐到文本文件
int loadUsersFromText();                  // 从文本文件加载用户
int saveUsersToText();                    // 保存用户到文本文件
User* findUser(const char* userId);       // 查找用户

// 核心功能函数
void inputDemandByForm();                 // 填写需求调查
void calcUserStar();                      // 计算用户星级
void matchPackagesByDemand();             // 匹配套餐
void showMatchedPackages();               // 显示推荐套餐
void queryUserPackage();                  // 查询个人套餐
void applyPackageChange();                // 变更套餐
void userFunctionMenu();                  // 用户功能菜单

#endif // USER_H