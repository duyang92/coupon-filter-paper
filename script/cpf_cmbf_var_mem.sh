#!/bin/bash

max_thread=10
eval_target="var_mem_percent"
mea_method="cmbf"

# params for CpF_with_CM 300KB mea_th 100
total_mem_case1=100
mea_th_case1=320
param_arr_str_case1=(
   "param=(0.12 50331 2 0.003400 202.540539 2)"
  # ...
)

# params for CpF_with_CM 500KB mea_th 100
total_mem_case2=300
mea_th_case2=320
param_arr_str_case2=(
  "param=(0.12 150994 2 0.011400 61.882162 2)"
  # ...
)

count_thread=0
for param_str in "${param_arr_str_case1[@]}"
do
  count_thread=$(( $count_thread + 1 ))
  {
    eval ${param_str}

    mem_percent=${param[0]}
    cpf_m=${param[1]}
    cpf_c=${param[2]}
    cpf_p=${param[3]}
    filter_th=${param[4]}
    onoff_num_hash_func=${param[5]}

    res_path="../res/cpf_"${mea_method}"_""${total_mem_case1}""KB/mem_percent/""${mea_th_case1}""/""${mem_percent}""/"
    mkdir -p ${res_path}

    ./cpf_cmbf -eval_target ${eval_target} -total_mem ${total_mem_case1} -mea_th ${mea_th_case1} \
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

    mem_percent=${param[0]}
    cpf_m=${param[1]}
    cpf_c=${param[2]}
    cpf_p=${param[3]}
    filter_th=${param[4]}
    onoff_num_hash_func=${param[5]}

    res_path="../res/cpf_"${mea_method}"_""${total_mem_case2}""KB/mem_percent/""${mea_th_case2}""/""${mem_percent}""/"
    mkdir -p ${res_path}

    ./cpf_cmbf -eval_target ${eval_target} -total_mem ${total_mem_case2} -mea_th ${mea_th_case2} \
        -cpf_m ${cpf_m} -cpf_c ${cpf_c} -cpf_p ${cpf_p} -filter_th ${filter_th} \
         -cmbf_num_hash ${onoff_num_hash_func}

  } &

  if [ ${count_thread} -eq ${max_thread} ] ; then
    count_thread=0
    echo "Waiting"
    wait
  fi

done
wait









