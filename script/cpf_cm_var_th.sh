#!/bin/bash

max_thread=20
eval_target="var_filter_th"
raw_eval_target="raw_cm"
mea_method="cm"

# params for CpF_with_CM 300KB mea_th 100
total_mem_case1=300
mea_th_case1=100
param_arr_str_case1=(
   "param=(55 125829 4 0.033700 54.881951 4 20)"
  # ...
)

# params for CpF_with_CM 500KB mea_th 100
total_mem_case2=500
mea_th_case2=100
param_arr_str_case2=(
   "param=(25 146800 4 0.073800 25.172793 4 20)"
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
    cm_num_hash_func=${param[5]}
    cm_counter_size=${param[6]}
    
    res_path="../res/cpf_"${mea_method}"_""${total_mem_case1}""KB/raw_cm/""${mea_th_case1}""/"
    mkdir -p ${res_path}
    
    res_path="../res/cpf_"${mea_method}"_""${total_mem_case1}""KB/filter_th/""${mea_th_case1}""/""${tar_filter_th}""/"
    mkdir -p ${res_path}

    ./cpf_cm -eval_target ${eval_target} -total_mem ${total_mem_case1} -mea_th ${mea_th_case1} -tar_filter_th ${tar_filter_th} \
        -cpf_m ${cpf_m} -cpf_c ${cpf_c} -cpf_p ${cpf_p} -filter_th ${filter_th} \
         -cm_num_hash_func ${cm_num_hash_func} -cm_counter_size ${cm_counter_size}
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
    cm_num_hash_func=${param[5]}
    cm_counter_size=${param[6]}
    
    res_path="../res/cpf_"${mea_method}"_""${total_mem_case2}""KB/raw_cm/""${mea_th_case2}""/"
    mkdir -p ${res_path}
    
    res_path="../res/cpf_"${mea_method}"_""${total_mem_case2}""KB/filter_th/""${mea_th_case2}""/""${tar_filter_th}""/"
    mkdir -p ${res_path}

    ./cpf_cm -eval_target ${eval_target} -total_mem ${total_mem_case2} -mea_th ${mea_th_case2} -tar_filter_th ${tar_filter_th} \
        -cpf_m ${cpf_m} -cpf_c ${cpf_c} -cpf_p ${cpf_p} -filter_th ${filter_th} \
         -cm_num_hash_func ${cm_num_hash_func} -cm_counter_size ${cm_counter_size}
  } &

  if [ ${count_thread} -eq ${max_thread} ] ; then
    count_thread=0
    echo "Waiting"
    wait
  fi

done

./cpf_cm -eval_target ${raw_eval_target} -total_mem ${total_mem_case1} -mea_th ${mea_th_case1} \
         -cm_num_hash_func 4 -cm_counter_size 20 
         
./cpf_cm -eval_target ${raw_eval_target} -total_mem ${total_mem_case2} -mea_th ${mea_th_case2} \
         -cm_num_hash_func 4 -cm_counter_size 20 


