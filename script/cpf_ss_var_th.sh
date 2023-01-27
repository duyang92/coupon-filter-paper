#!/bin/bash

max_thread=20
eval_target="var_filter_th"
raw_eval_target="raw_ss"
mea_method="ss"
k_val=1024

param_arr_str_case1=(
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

    res_path="../res/cpf_"${mea_method}"_top_""${k_val}""/filter_th/""${tar_filter_th}""/"
    mkdir -p ${res_path}

    ./cpf_ss -eval_target ${eval_target} -k_val ${k_val} -tar_filter_th ${tar_filter_th} \
        -cpf_m ${cpf_m} -cpf_c ${cpf_c} -cpf_p ${cpf_p} -filter_th ${filter_th}
  } &

  if [ ${count_thread} -eq ${max_thread} ] ; then
    count_thread=0
    echo "Waiting"
    wait
  fi

done

res_path="../res/cpf_"${mea_method}"_top_""${k_val}""/raw_ss/"
mkdir -p ${res_path}

./cpf_ss -eval_target ${raw_eval_target} -k_val ${k_val}



