## Coupon Filter

### Introduction

This repository includes code for following paper:
Xiaocan Wu, He Huang, Yang Du, Yu-E Sun, Shigang Chen. "Coupon Filter: A Universal and Lightweight Filter Framework for More Accurate Data Stream Processing".

Data stream processing plays a critical role in providing fundamental statistics for various applications, such as anomaly detection. Still, the unbalanced distribution of data streams severely affects the performance of related algorithms, which motivates the recent studies on filter structure design to enhance the existing algorithms for a more precise estimation result. However, these filters are mainly designed for frequency-based filtration, while none of them can conduct universal filtration; apparently, frequency is not the only targeted metric in practical processing tasks, metrics like cardinality and persistence are of equal importance. To cope with the issue, we propose a novel filter framework to implement universal, lightweight, and accurate filtration. The filter framework is called Coupon Filter due to the interpretation of its flow-level filtration as a coupon collection process. We prove the filtration efficiency of our filter design and formally analyze its recording process. We deploy our filter on the three typical metrics (frequency, cardinality, and persistence) to illustrate its advantages. The experimental results on real Internet traces demonstrate the effectiveness of our filter in enhancing existing stream processing approaches in terms of accuracy and throughput. 

### About this repo

The core filter structure **Coupon Filter** is implemented in **/CouponFilter/CouponFilter.h**.

Other baseline methods and stream processing methods are implemented based on following papers:

- **Cold Filter ([Source Code](https://github.com/zhouyangpkuer/ColdFilter))**: Yang Zhou, Tong Yang, Jie Jiang, Bin Cui, Minlan Yu, Xiaoming Li, Steve Uhlig. "Cold Filter: A Meta-Framework for Faster and More Accurate Stream Processing". in Proc. of ACM SIGMOD, 2018, pp. 741-756. 
- **LogLog Filter ([Source Code](https://github.com/ICDE2021/LogLogFilter))**:	Peng Jia, Pinghui Wang, Junzhou Zhao, Ye Yuan, Jing Tao, Xiaohong Guan. "LogLog Filter: Filtering Cold Items within a Large Range over High Speed Data Streams". in Proc. of IEEE ICDE, 2021, pp. 804-815. 
- **CountMin**: Graham Cormode, S. Muthukrishnan. "An improved data stream summary: the count-min sketch and its applications". Journal of Algorithms, 55(1): 58-75 (2005). 
- **Space-Saving**: Ahmed Metwally, Divyakant Agrawal, Amr El Abbadi. "Efficient Computation of Frequent and Top-k Elements in Data Streams". in Proc. of ICDT, 2005, pp. 398-412. 
- **NDS**: He Huang, Yu-E Sun, Chaoyi Ma, Shigang Chen, Yang Du, Haibo Wang, Qingjun Xiao. "Spread Estimation With Non-Duplicate Sampling in High-Speed Networks". IEEE/ACM Trans. Netw., 29(5): 2073-2086 (2021). 
- **vHLL**: You Zhou, Yian Zhou, Min Chen, Shigang Chen. "Persistent Spread Measurement for Big Network Data Based on Register Intersection". Proc. ACM Meas. Anal. Comput. Syst., 1(1): 15:1-15:29 (2017). 
- **CM-BF**: Haipeng Dai, Muhammad Shahzad, Alex X. Liu, Yuankun Zhong. "Finding Persistent Items in Data Streams". Proc. VLDB Endow., 10(4): 289-300 (2016).
- **On-Off ([Source Code](https://github.com/Sketch-Data-Stream/On-Off-Sketch))**: Yinda Zhang, Jinyang Li, Yutian Lei, Tong Yang, Zhetao Li, Gong Zhang, Bin Cui. "On-Off Sketch: A Fast and Accurate Sketch on Persistence". Proc. VLDB Endow., 14(2): 128-140 (2020). 

### Requirements
- cmake >= 3.10
- g++

### How to build

You can use the following commands to build and run.

```
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
bash ../script/xxx.sh
```