# 复制本工程后有报错

如果复制本工程以后有12个error。且都是 关于配置问题的，请按照下面的进行修改。

`注意：`我的ti两个包在E盘，和我的代码工程（通常我都放在D盘）分开了，所以我工程都是用的绝对路径。复制工程再修改，出现的这些报错，多半是因为我是绝对路径的原因。一级标题这里仅作为使用绝对路径的参考。不过使用相对路径的，大多也是这三步没改对，可以按照这三条去改你的路径。

## 改 SysConfig 的 Before Build 命令

    打开 Project -> Options for Target 'main' -> User

  把 Run #1 保持勾选，把命令改成这一整行：（这里写的是绝对路径）

这个地址也就是你配置环境时，下载的mspm0_sdk_2_09_00_01包里面的syscfg.bat文件，大多是前半部分存在的盘和sdk包的版本号不一样（E:\ti\mspm0_sdk_2_09_00_01）

```bash
cmd.exe /C "E:\ti\mspm0_sdk_2_09_00_01\tools\keil\syscfg.bat '$P' empty.syscfg"
```

```bash
cmd.exe /C "E:\ti\sysconfig_1.27.0\sysconfig_cli.bat -o .. -s E:\ti\mspm0_sdk_2_09_00_01\.metadata\product.json
  --compiler keil ..\empty.syscfg"
```



长短命令的区别

```c
 短命令
  它调用的是 TI 提供的包装脚本 /E:/ti/mspm0_sdk_2_09_00_01/tools/keil/syscfg.bat。
  这个脚本会自动帮你做 3 件事：

  1. 从 '$P' 开始一路往上找 .metadata\product.json
  2. 一路往上找 .syscfg 文件所在目录
  3. 最后自己再去调用真正的：

  sysconfig_cli.bat -o ... -s ... --compiler keil ...

  也就是说，短命令是“自动推断”。

  长命令
  我给你的长命令是把这些路径全部写死：

  cmd.exe /C "E:\ti\sysconfig_1.27.0\sysconfig_cli.bat -o .. -s E:\ti\mspm0_sdk_2_09_00_01\.metadata\product.json
  --compiler keil ..\empty.syscfg"

  它不猜路径，直接指定：

  - SysConfig 工具在哪
  - product.json 在哪
  - 输出目录在哪
  - empty.syscfg 在哪

  也就是“显式指定”。

  为什么你现在短命令也能用
  说明你当前这个工程，已经满足了 syscfg.bat 的自动查找条件。
  常见原因只有两个：

  1. 工程目录结构刚好让它能找到 .metadata\product.json
  2. 你现在打开的工程其实已经回到了 SDK 示例的标准布局附近

  所以短命令不是“更正确”，只是“当前碰巧能推断成功”。

  两者怎么选

  - 工程长期放在 TI SDK 示例目录结构里：短命令可以用，省事。
  - 工程经常复制、改名、换盘符、脱离 SDK 单独放：长命令更稳。

  你下次该注意的核心
  syscfg.bat 适合“原厂示例工程”
  sysconfig_cli.bat + 明确绝对路径 适合“你自己的独立工程”

  你这次之前出问题，本质就是：工程已经脱离原来的目录结构了，但还在用依赖自动推断的那套方式。
```



  ## 改编译器 Include Paths

    打开 Options for Target -> C/C++

  把 Include Paths 改成：

```bash
    E:\ti\mspm0_sdk_2_09_00_01\source\third_party\CMSIS\Core\Include;
    E:\ti\mspm0_sdk_2_09_00_01\source;
    ..\..\ti_6612
    ..\user;
    ..\driver;
    ..\logic
```

  重点有两个：
  2_04_00_06 全部换成 2_09_00_01
  把 .\..\ti_6612 加进去，不然 ti_msp_dl_config.h 会找不到,也就是包含了你这个工程的prj编译文件和scr等等代码的杂七杂八的文件夹

## 改链接库路径

```bash
打开 Options for Target -> Linker
```

这个也是，不一样的就是前半部分存在的盘和sdk包的版本号不一样（E:\ti\mspm0_sdk_2_09_00_01）

```bash

E:\ti\mspm0_sdk_2_09_00_01\source\ti\driverlib\lib\keil\m0p\mspm0g1x0x_g3x0x\driverlib.a
```

##   你下次记住这样就够了

  复制 MSPM0 + Keil 工程后，第一时间检查这三类路径：

```bash
  SysConfig 命令
  Include Paths
  driverlib.a 路径
```

  它们只要还带着旧目录、旧 SDK 版本、或者相对路径依赖，工程就会炸。

#  Keil 打开图形界面时加载 SysConfig 扩展失败

`报错：`

无法加载以下来源的扩展程序: E:\ti\sysconfig_1.27.0. 清单文件缺失或不可读取

`原因：`

从 SysConfig 1.26+ 开始，package.json 被移到了 ...\app 子目录，Keil 如果还把源目录指到 E:
  \ti\sysconfig_1.27.0 根目录，就会报这个错。TI 的说明和修复见这里：

```tex
https://e2e.ti.com/support/microcontrollers/msp-low-power-microcontrollers-group/msp430/f/msp-low-power-microcontroller-forum/1628616/mspm0c1103-bug-report-mspm0-sdk-v2-09-sysconfig-import-path-issue-with-mdk-arm-keil-fix-attached
```

  Keil 的官方集成步骤见 TI 文档：

```tex
https://software-dl.ti.com/msp430/esd/MSPM0-SDK/2_05_01_00/docs/english/tools/sysconfig_guide/doc_guide/doc_guide-srcs/sysconfig_guide.html
```

`修改`（默认你已经完成基本的修改sysconfig位置）

1. 在之前配置好的MSPM0_SDK_syscfg_menu_import.cfg文件中,Command这一栏后面加上\app

在"E:\ti\mspm0_sdk_2_09_00_01\tools\keil\MSPM0_SDK_syscfg_menu_import.cfg"下的MSPM0_SDK_syscfg_menu_import.cfg文件，除了基本的改sysconfig的位置以外，需要在Command这一栏后面加上\app，



下面是示例

```bash
[Sysconfig v1.27.0 - MSPM0 SDK v2_09_00_01]
Command=E:\ti\sysconfig_1.27.0\nw\nw.exe E:\ti\sysconfig_1.27.0\app
Initial Folder=E:\ti\mspm0_sdk_2_09_00_01
Arguments=--compiler keil -s ".metadata\product.json" "#E"
Prompt for Arguments=0
Run Minimized=0
Run Independent=-1

```

2. 删掉之前导入的，重新导入用于打开 SysConfig GUI 的“工具菜单项”

```bash
最上方，打开 Tools -> Customize Tools Menu... -> 把以前的删掉 -> 重新选到你的 E:/ti/mspm0_sdk_2_09_00_01/tools/keil/MSPM0_SDK_syscfg_menu_import.cfg（要双击MSPM0_SDK_syscfg_menu_import.cfg打开）-> import
```

