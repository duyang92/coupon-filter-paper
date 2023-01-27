#!/bin/bash

max_thread=25
eval_target="var_filter_th"
raw_eval_target="raw_nds"
mea_method="nds"

# params for CpF_with_NDS 100KB mea_th 100
total_mem_case1=100
mea_th_case1=100
param_arr_str_case1=(
  "param=(55 16777 4 0.033300 54.319635 0.030093)"
  # ...
)

# total_mem_case1=100
# mea_th_case1=200
# param_arr_str_case1=(
  # ...
# )

# params for CpF_with_NDS 300KB mea_th 100
total_mem_case2=300
mea_th_case2=100
param_arr_str_case2=(
  "param=(35 16777 4 0.059400 34.010531 0.275224)"
  # ...
)

# total_mem_case2=300
# mea_th_case2=200
# param_arr_str_case2=(
  # ...
# )

count_thread=0
for param_str in "${param_arr_str_case1[@]}"
do
  count_thread=$(( $count_thread + 1 ))
  {
    eval ${param_str}

    tar_filter_th=${param[0]}
    cpf_m=${param[1]}
    cpf_c=${param[2]}
    cpf_p=${param[3]}
    filter_th=${param[4]}
    nds_p=${param[5]}
    
    res_path="../res/cpf_"${mea_method}"_""${total_mem_case1}""KB/filter_th/""${mea_th_case1}""/""${tar_filter_th}""/"
    echo $res_path
    mkdir -p ${res_path}

    ./cpf_nds -eval_target ${eval_target} -total_mem ${total_mem_case1} -mea_th ${mea_th_case1} -tar_filter_th ${tar_filter_th} -cpf_m ${cpf_m} \
       -cpf_c ${cpf_c} -cpf_p ${cpf_p} -filter_th ${filter_th} -nds_p ${nds_p}    
  } &

  if [ ${count_thread} -eq ${max_thread} ] ; then
    count_thread=0
    echo "Waiting"
    wait
  fi
done

for param_str in "${param_arr_str_case2[@]}"
do
  count_thread=$(( $count_thread + 1 ))
  {
    eval ${param_str}

    tar_filter_th=${param[0]}
    cpf_m=${param[1]}
    cpf_c=${param[2]}
    cpf_p=${param[3]}
    filter_th=${param[4]}
    nds_p=${param[5]}
    
    res_path="../res/cpf_"${mea_method}"_""${total_mem_case2}""KB/filter_th/""${mea_th_case2}""/""${tar_filter_th}""/"
    # echo $res_path
    mkdir -p ${res_path}

    ./cpf_nds -eval_target ${eval_target} -total_mem ${total_mem_case2} -mea_th ${mea_th_case2} -tar_filter_th ${tar_filter_th} -cpf_m ${cpf_m} \
       -cpf_c ${cpf_c} -cpf_p ${cpf_p} -filter_th ${filter_th} -nds_p ${nds_p} 
  } &

  if [ ${count_thread} -eq ${max_thread} ] ; then
    count_thread=0
    echo "Waiting"
    wait
  fi
done

 res_path="../res/cpf_"${mea_method}"_""${total_mem_case1}""KB/raw_nds/""${mea_th_case1}""/"
 echo ${res_path}
 mkdir -p ${res_path}
 ./cpf_nds -eval_target ${raw_eval_target} -total_mem ${total_mem_case1} -mea_th ${mea_th_case1}

 res_path="../res/cpf_"${mea_method}"_""${total_mem_case2}""KB/raw_nds/""${mea_th_case2}""/"
 echo ${res_path}
 mkdir -p ${res_path}
 ./cpf_nds -eval_target ${raw_eval_target} -total_mem ${total_mem_case2} -mea_th ${mea_th_case2}



