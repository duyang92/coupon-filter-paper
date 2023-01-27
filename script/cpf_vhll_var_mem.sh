#!/bin/bash

max_thread=15
eval_target="var_mem_percent"
mea_method="vhll"

# params for CpF_with_vHLL 100KB mea_th 100
total_mem_case1=100
mea_th_case1=100
param_arr_str_case1=(
   "param=(0.16 44739 3 0.015500 64.161916 128)"
  # ...
)


# params for CpF_with_vHLL 300KB mea_th 100
total_mem_case2=300
mea_th_case2=100
param_arr_str_case2=(
   "param=(0.16 134217 3 0.029600 35.075523 128)"
  # ...
)


# clear if is needed
# res_root="../res/cpf_"${mea_method}"_""${total_mem}""KB/mem_percent/"${mea_th}"
# rm -rf ${res_root}

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
    num_v_reg=${param[5]}

   res_path="../res/cpf_"${mea_method}"_""${total_mem_case1}""KB/mem_percent/""${mea_th_case1}""/""${mem_percent}""/"
   mkdir -p ${res_path}

   ./cpf_vhll -eval_target ${eval_target} -total_mem ${total_mem_case1} -mea_th ${mea_th_case1} -cpf_m ${cpf_m} \
      -cpf_c ${cpf_c} -cpf_p ${cpf_p} -filter_th ${filter_th} -num_v_reg ${num_v_reg}
  } &

  if [ ${count_thread} -eq ${max_thread} ] ; then
    echo "Waiting"
    count_thread=0
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
    num_v_reg=${param[5]}

   res_path="../res/cpf_"${mea_method}"_""${total_mem_case2}""KB/mem_percent/""${mea_th_case2}""/""${mem_percent}""/"
   mkdir -p ${res_path}

   ./cpf_vhll -eval_target ${eval_target} -total_mem ${total_mem_case2} -mea_th ${mea_th_case2} -cpf_m ${cpf_m} \
      -cpf_c ${cpf_c} -cpf_p ${cpf_p} -filter_th ${filter_th} -num_v_reg ${num_v_reg}
  } &

  if [ ${count_thread} -eq ${max_thread} ] ; then
    echo "Waiting"
    count_thread=0
    wait
  fi

done
wait

