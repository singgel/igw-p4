# igw-p4

`igw-p4` 是一套面向 Barefoot/Intel Tofino 可编程交换芯片的 IGW（Internet Gateway）P4 数据面工程。仓库包含 P4-16/TNA 管线源码、EIP/NAT/redirect 相关转发逻辑、BF-RT 表项初始化代码、交换机启动与运维脚本，以及基于 PTF 的回归测试框架。

## 功能概览

- 基于 P4-16 + TNA/T2NA 编写 IGW 转发管线。
- 支持 VXLAN 封装/解封装、VNI/隧道处理与外层 MAC 重写。
- 支持 EIP 入/出方向处理、NAT、redirect、meter/drop 统计等互联网网关能力。
- 支持 ECMP/下一跳选择、哈希计算、系统 ACL、镜像、端口流量统计。
- 提供 BF-RT C/Python 表项封装与启动初始化脚本。
- 提供 FRR 配置、SONiC 插件、运维脚本、PTF 测试框架等配套内容。

## 目录结构

```text
.
├── README                 # 原始简要说明
├── README.md              # 仓库说明
├── p4src/                 # IGW P4 管线源码与构建脚本
│   ├── igw_switch.p4      # P4 主入口
│   ├── common/            # 通用 header、metadata、类型、常量定义
│   ├── pipe02/            # 外部方向 pipeline parser/ingress/egress
│   ├── pipe13/            # 内部方向 pipeline parser/ingress/egress
│   └── src/               # ACL、bridge、hash、meter、mirror、nexthop、tunnel、NAT 等模块
├── igw-h/
│   ├── init_setup/        # bf_switchd/hostif/port manager 初始化相关代码
│   └── rpc_client/        # Python BF-RT 表项初始化客户端
├── ptf-test/              # PTF 回归测试框架
└── tools/                 # BF-RT 代码生成、FRR、运维脚本、依赖包等工具
```

## P4 管线入口

主程序入口为：

```text
p4src/igw_switch.p4
```

`igw_switch.p4` 组装了以下模块：

- `common/`：协议头、metadata、平台适配、表规模与类型定义。
- `src/acl.p4`：系统 ACL、CPU mirror/drop 等处理。
- `src/bridge.p4`：桥接与 VXLAN metadata 处理。
- `src/hashes.p4`：内层流量与 NLB/EIP 相关哈希计算。
- `src/meter.p4`：限速、着色与 drop 相关逻辑。
- `src/mirror.p4`：镜像与采样处理。
- `src/nexthop.p4`：ECMP group/action selector 与下一跳选择。
- `src/tunnel.p4`：隧道封装/解封装处理。
- `src/vmipv4.p4`：IPv4 VM/业务转发逻辑。
- `src/vxlan_route.p4`：VXLAN 路由处理。
- `src/traffic_stats.p4`：EIP/端口流量与 drop 统计。
- `src/nat.p4`：NAT/EIP 地址转换相关处理。
- `src/redirect.p4`：流量重定向相关处理。

最终由 `pipe02` 与 `pipe13` 两组 pipeline 组成交换机主程序：

```p4
Pipeline(P02_IngressParser(), P02_Ingress(), P02_IngressDeparser(),
         P02_EgressParser(), P02_Egress(), P02_EgressDeparser()) pipe_ext;

Pipeline(P13_IngressParser(), P13_Ingress(), P13_IngressDeparser(),
         P13_EgressParser(), P13_Egress(), P13_EgressDeparser()) pipe_int;

Switch(pipe_ext, pipe_int) main;
```

## 构建环境

该工程依赖 Barefoot/Intel Tofino SDE，源码中保留了 SDE 9.3.x 与 SDE 9.7.x 的构建示例。

常用环境变量：

```bash
export SDE=/root/sde/bf-sde-9.7.3
export SDE_INSTALL=$SDE/install
```

请根据实际 SDE 安装路径、BSP 包路径和芯片平台调整。

## 构建 P4 程序

进入 P4 源码目录：

```bash
cd p4src
```

### SDE 9.7.x / p4studio

```bash
P4_PATH=/root/p4code/p4src/igw_switch.p4
P4_NAME=igw_switch

cmake $SDE/p4studio/ \
  -DCMAKE_INSTALL_PREFIX=$SDE/install \
  -DCMAKE_MODULE_PATH=$SDE/cmake \
  -DP4_NAME=$P4_NAME \
  -DP4_PATH=$P4_PATH \
  -DP4_LANG=p4-16 \
  -DP4FLAGS="--verbose 2 --create-graphs -g"
```

也可以参考脚本：

```bash
p4src/igw_sde_9.7_build.sh
```

### SDE 9.3.x / p4-build

```bash
$SDE/pkgsrc/p4-build/configure \
  P4_PATH=/root/p4code/p4src/igw_switch.p4 \
  P4_NAME=igw_switch \
  P4_PREFIX=igw_switch \
  P4_VERSION=p4-16 \
  P4_ARCHITECTURE=tna \
  P4FLAGS="--verbose 2 --create-graphs -g" \
  --with-tofino \
  --prefix=$SDE_INSTALL
```

也可以参考脚本：

```bash
p4src/igw_sde_9.3_build.sh
```

## 初始化与运行

### init_setup

`igw-h/init_setup/` 包含设备初始化相关 C 代码与 switch API/BF-RT 封装，主要用于：

- 启动/配合 `bf_switchd`；
- 创建 hostif；
- 初始化端口管理；
- 加载 `hostif.json` 与 FRR 配置；
- 初始化 EIP、ECMP、meter、mirror、ACL 等 BF-RT 表项。

使用前需要按当前 SDE 版本与部署路径调整 Makefile 和配置文件路径。

### rpc_client

`igw-h/rpc_client/` 是 Python 初始化客户端，主入口为：

```bash
igw-h/rpc_client/igw_start_up.py
```

该脚本默认连接本机 BF-RT gRPC server：

```python
P4_NAME = "igw_switch"
GRPC_SERVER = "127.0.0.1"
```

启动逻辑包括 mirror、ECMP group、drop ACL 等表项初始化。运行前请确认：

1. P4 程序已编译并加载；
2. `bf_switchd` 已启动；
3. Python `sys.path` 中的 SDE/BF-RT 包路径与实际环境一致；
4. `PIPELINE_NUM` 与设备 pipeline 数一致；
5. EIP/NAT/redirect/meter 等业务表项与当前部署环境匹配。

## 测试

PTF 测试位于：

```text
ptf-test/
```

常用运行方式：

```bash
cd ptf-test
sudo ptf --test-dir tests --platform-dir platforms --relax -P remote
```

测试前需要根据实际测试床修改：

- `platforms/remote.py` 中的端口映射；
- `mytests/topo.py` 中的交换机/服务器 MAC、IP 等拓扑信息。

## 工具目录

`tools/` 包含部署与运维相关工具，例如：

- `bfrt-apigen-tool/`：根据 BF-RT JSON 生成接口代码；
- `frr/`：FRR/BGP 配置示例；
- `ops/`：上线、恢复、监控、隔离、EIP/网络配置等运维脚本；
- `python2.7lib/`、`python3.5lib/`：运行环境依赖包；
- `sde9.7-profiles/`、`sde-9.7-dep/`：SDE 9.7 构建 profile 与依赖示例；
- `sonic-plugins/`：SONiC 平台插件示例。

## 注意事项

- 本仓库依赖专有 Tofino SDE，不能只用开源工具链完整编译运行。
- 构建脚本中的 `/root/p4code`、`/root/sde` 等路径为示例路径，需要按实际环境修改。
- `hostif.json`、FRR 配置、端口映射、pipeline 数、EIP/NAT/redirect 表项均与部署环境强相关。
- 仓库中的运维文档和脚本可能包含环境假设，公开或复用前建议先做敏感信息审查。

## License

仓库未声明开源许可证。使用、分发或改造前请先确认授权范围。
