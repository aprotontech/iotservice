# 测试

## 测试配置
文件 ```tdefines.php``` 定了测试一些配置，可以根据需要调整，具体如下：
| 配置项 | 可选值 | 意义 |
| - | - | - |
| PROTON_PRINT_CORE_LOGGER | 1/0 | 是否打印扩展自身的日志 |
| PROTON_PRINT_LOGGER_LEVEL | [0,5] | 日志级别 |


## 初始化
运行单测前，需要用composer安全相关依赖
```
composer init
```

## 一键测试
执行下面的命令，运行所有的测试
```bash
bash run_tests.sh
```

## 单独测试
可以参考 ```run_tests.sh``` 文件内容，运行单个测试