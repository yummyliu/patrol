# patrol
                                          
                 ,--.                ,--. 
 ,---.  ,--,--.,-'  '-.,--.--. ,---. |  | 
| .-. |' ,-.  |'-.  .-'|  .--'| .-. ||  | 
| '-' '\ '-'  |  |  |  |  |   ' '-' '|  | 
|  |-'  `--`--'  `--'  `--'    `---' `--' 
`--'                                      


## patrol

> go

两种状态:
1. apart: 只做自动故障处理
2. rejoin: commander连接上后，就是归队了，向commander汇报所负责db实例的状态；同时可以通过commander，向patrol发送一些命令：修改参数或者远程执行一些db或os命令；

## commander

> c++

commander是一个PostgreSQL的background worker，收集信息，发送命令，比如：

1. add_patrol
2. remove_patrol
3. get_report
4. exec_command
等等，目前想到这些。



