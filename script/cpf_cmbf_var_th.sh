#!/bin/bash

max_thread=15
eval_target="var_filter_th"
raw_eval_target="raw_cmbf"
mea_method="cmbf"

# params for CpF_with_CM 300KB mea_th 100
total_mem_case1=100
mea_th_case1=320
param_arr_str_case1=(
   "param=(160 75497 2 0.004200 161.268991 2)"
  # ...
)

# params for CpF_with_CM 500KB mea_th 100
total_mem_case2=300
mea_th_case2=320
param_arr_str_case2=(
  "param=(40 176160 2 0.018600 40.038027 2)"
  # ...
)

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
    onoff_num_hash_func=${param[5]}
    
    res_path="../res/cpf_"${mea_method}"_""${total_mem_case1}""KB/raw_cmbf/""${mea_th_case1}""/"
    mkdir -p ${res_path}
    
    res_path="../res/cpf_"${mea_method}"_""${total_mem_case1}""KB/filter_th/""${mea_th_case1}""/""${tar_filter_th}""/"
    mkdir -p ${res_path}

    ./cpf_cmbf -eval_target ${eval_target} -total_mem ${total_mem_case1} -mea_th ${mea_th_case1} -tar_filter_th ${tar_filter_th} \
        -cpf_m ${cpf_m} -cpf_c ${cpf_c} -cpf_p ${cpf_p} -filter_th ${filter_th} \
         -cmbf_num_hash ${onoff_num_hash_func}
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
    onoff_num_hash_func=${param[5]}
    
    res_path="../res/cpf_"${mea_method}"_""${total_mem_case2}""KB/raw_cmbf/""${mea_th_case2}""/"
    mkdir -p ${res_path}
    
    res_path="../res/cpf_"${mea_method}"_""${total_mem_case2}""KB/filter_th/""${mea_th_case2}""/""${tar_filter_th}""/"
    mkdir -p ${res_path}

    ./cpf_cmbf -eval_target ${eval_target} -total_mem ${total_mem_case2} -mea_th ${mea_th_case2} -tar_filter_th ${tar_filter_th} \
        -cpf_m ${cpf_m} -cpf_c ${cpf_c} -cpf_p ${cpf_p} -filter_th ${filter_th} \
         -cmbf_num_hash ${onoff_num_hash_func}
  } &

  if [ ${count_thread} -eq ${max_thread} ] ; then
    count_thread=0
    echo "Waiting"
    wait
  fi

done

./cpf_cmbf -eval_target ${raw_eval_target} -total_mem ${total_mem_case1} -mea_th ${mea_th_case1} \
         -cmbf_num_hash 2
         
./cpf_cmbf -eval_target ${raw_eval_target} -total_mem ${total_mem_case2} -mea_th ${mea_th_case2} \
         -cmbf_num_hash 2









