# HookThreadDllIj

> 环境：win10 ltsc

通过windows鼠标钩子注入dll



### 如何使用

1.cmd 运行：

`C:\Users\ab\Desktop>HookThreadDllIj.exe C:\Users\ab\Desktop\HookDll.dll WinX86.exe`

2.Dbgview 查看挂钩情况



### 注意：DLL需要导出并处理 **MouseProc** 

>  [MouseProc](https://learn.microsoft.com/zh-cn/windows/win32/winmsg/mouseproc) 

```c++
LRESULT CALLBACK MouseProc(
  _In_ int    nCode,
  _In_ WPARAM wParam,
  _In_ LPARAM lParam
);
```