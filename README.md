# UnixShell

####目前实现功能:   
1.正常执行命令,如touch out.txt   
2.管道: ls|wc|wc   
3.输入输出重定向到文件:ls>out.txt, wc<out.txt   
4.后台执行程序: gedit out.txt &   
5.作业控制:jobs, fg, bg   
6.ctrl+z中止前台进程, ctrl+c杀死前台进程   
7.内建cd命令改变工作目录,如cd /usr   
8.如果"cd"命令后没加参数,则进入图形化的"文件导航系统",用于改变工作目录   
9.自动补全:Tab键自动补全所有命令+目录下的文件/文件夹,上下键历史命令,各种快捷键   
10.命令"history"查看历史命令,以文件形式存在用户家目录.test_history文件里    
![filenavigation](/filenavigation.png)
