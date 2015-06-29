# UnixShell

####目前实现功能:   
1.正常执行命令,如ls -l   
2.管道: ls|wc|wc   
3.输入输出重定向到文件:ls>out.txt, wc<out.txt   
4.后台执行程序: gedit out.txt &   
5.作业控制:jobs, fg, bg, ctrl+z, ctrl+c   
6.历史命令:history   
7.查看和增加环境变量:echopath, addpath    
8.内建cd命令改变工作目录,如cd /usr   
9.如果"cd"命令后没加参数,则进入图形化的"文件导航系统"   
10.自动补全：命令名、文件名Tab自动补全，上下键历史命令，各种快捷键:ctrl+b, ctrl+f, ctrl+u, ctrl+k...    
![filenavigation](/filenavigation.png)
