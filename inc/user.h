#ifndef USER_H
#define USER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

//函数声明
// 初始化与前后端交互模块
void initUserSystem();                  // 初始化系统（与前端建立连接，同步套餐数据）
void receiveDataFromFrontEnd();         // 接收前端数据
void sendDataToFrontEnd(const char* data); // 向前端发送数据

// 用户核心功能模块
void userLogin();                       // 用户登录
Package inputDemandByForm();            // 调查表填写
void calcUserStar();                    // 计算用户星级
void matchPackagesByDemand(Package demand); // 按需求匹配套餐
void showMatchedPackages();             // 展示推荐套餐并确认选择
void queryUserPackage();                // 查询本人套餐信息
void searchPackage();                   // 搜索窗口
void applyPackageChange();              // 变更套餐需求
void exitUserSystem();                  // 退出系统

#endif // USER_H