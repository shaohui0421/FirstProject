function num_strunit(){
  local list="$*"
  local num=0
  for m in $list
  do
    num=$(expr $num + 1 )
  done
  echo $num
}

function find_strunit(){
  local str="$1"
  local list="$2"
  for m in $list
  do
    if [ "$m" = "$str" ]; then
      return 1
    fi
  done
  return 0
}

function add_strunit(){
  local str="$1"
  local list="$2"
  local ret=0
  find_strunit "$str" "$list"
  if [ $? = 0 ]; then
    ret=1
    echo $list $str
  else
    echo $list
  fi
  return $ret
}

function del_strunit(){
  local str="$1"
  local list="$2"
  local listtmp=
  local m=0
  for m in $list
  do
    if [ ! "$m" = "$str" ]; then
      listtmp="$m $listtmp"
    else
      ret=1
    fi
  done
  echo $listtmp
  return $ret
}
