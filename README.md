```
                 ,--.                ,--.
 ,---.  ,--,--.,-'  '-.,--.--. ,---. |  |
| .-. |' ,-.  |'-.  .-'|  .--'| .-. ||  |
| '-' '\ '-'  |  |  |  |  |   ' '-' '|  |
|  |-'  `--`--'  `--'  `--'    `---' `--'
`--'
```

# 构成

## patrol

> go

主要任务做做自动故障处理，同时是一个小web服务，commander请求相应的http接口，获得相应信息

## commander

> c++

commander是一个PostgreSQL的background worker，收集信息，发送命令，比如：

#### 操作函数

1. add_patrol udf
2. remove_patrol udf
3. exec_command udf
4. get_db_report udtf

> 等等，目前想到这些。

# 目标

目标是对生成系统的影响为0，即不在生产系统上安装任何插件

# roadmap

1. commander启动，创建元数据表

``` sql

db_info
rel_info
func_info


db_superversion
rel_superversion
func_superversion

```

2. 设计patrol的http接口，从而实现commander中的操作函数(操作函数中，向patrol服务请求相应http接口实现)
3. 完成patrol模块的的相应连接模块代码
4. 与patrol建立连接
5. 按照巡检对象元信息表内容，定时上报信息
6. commander前加一个web服务(单加或者整合到)，curl的方式获得信息

# 关于commander与patrol的通信方式，考虑了两种方案，最后决定使用http

1. tcp长连接 rpc
2. http调用 : 不用写代码可以curl的方式用，方便,并且也没那么高频

# install with docker

build:
    docker build -t postgres-commander --rm=true .

debug:
    docker run -i -t --entrypoint=sh postgres-commander
    docker run -i -t --entrypoint=sh postgres-commander

run:
    docker run -i -P postgres-commander
