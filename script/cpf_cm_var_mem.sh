#!/bin/bash

max_thread=15
eval_target="var_mem_percent"
mea_method="cm"

# params for CpF_with_CM 300KB mea_th 100
total_mem_case1=300
mea_th_case1=100
param_arr_str_case1=(
   "param=(0.20 125829 4 0.033600 55.047410 4 20)"
  # ...
)

# params for CpF_with_CM 500KB mea_th 100
total_mem_case2=500
mea_th_case2=100
param_arr_str_case2=(
   "param=(0.16 223696 3 0.058700 25.988706 4 20)"
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
    cm_num_hash_func=${param[5]}
    cm_counter_size=${param[6]}

    res_path="../res/cpf_"${mea_method}"_""${total_mem_case1}""KB/mem_percent/""${mea_th_case1}""/""${mem_percent}""/"
    mkdir -p ${res_path}

   ./cpf_cm -eval_target ${eval_target} -total_mem ${total_mem_case1} -mea_th ${mea_th_case1} -cpf_m ${cpf_m} \
      -cpf_c ${cpf_c} -cpf_p ${cpf_p} -filter_th ${filter_th} \
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

    mem_percent=${param[0]}
    cpf_m=${param[1]}
    cpf_c=${param[2]}
    cpf_p=${param[3]}
    filter_th=${param[4]}
    cm_num_hash_func=${param[5]}
    cm_counter_size=${param[6]}

    res_path="../res/cpf_"${mea_method}"_""${total_mem_case2}""KB/mem_percent/""${mea_th_case2}""/""${mem_percent}""/"
    mkdir -p ${res_path}

   ./cpf_cm -eval_target ${eval_target} -total_mem ${total_mem_case2} -mea_th ${mea_th_case2} -cpf_m ${cpf_m} \
      -cpf_c ${cpf_c} -cpf_p ${cpf_p} -filter_th ${filter_th} \
       -cm_num_hash_func ${cm_num_hash_func} -cm_counter_size ${cm_counter_size}
  } &

  if [ ${count_thread} -eq ${max_thread} ] ; then
    count_thread=0
    echo "Waiting"
    wait
  fi

done
wait









