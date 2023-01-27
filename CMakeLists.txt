cmake_minimum_required(VERSION 3.10)
project(coupon-filter-paper)

set(CMAKE_CXX_STANDARD 11)

add_executable(find_param find_opt_param.cpp)
add_executable(cpf_cm CpF_with_CM.cpp)
add_executable(cpf_ss CpF_with_SS.cpp)
add_executable(cpf_nds CpF_with_NDS.cpp)
add_executable(cpf_vhll CpF_with_vHLL.cpp)
add_executable(cpf_onoff CpF_with_OnOff.cpp)
add_executable(cpf_cmbf CpF_with_CM_BF.cpp)