#!/bin/bash

#################################### funkcije

function printaj #funkcija ki jo uporablja drevo za printanje globine
{
   i=0
   while [ "$i" -lt "$1" ]
   do
     echo -n "----" 
    i=$(($i+1))
   done

}

function pomoc
{
 echo "Uporaba: $1 akcija parametri"
}

function status 
{
   prvo=$1
   drugo=$2
   gcd=1
   i=1
 while [[ $i -le $prvo && $i -le $drugo ]]
do

      if (( $prvo%$i==0 && $drugo%$i==0 ))
      then 
             gcd=$i
      fi
      i=$(($i+1))
done
 exit $gcd;   
}

function leto
{
   for i in $@
   do
    (( $i%4 == 0 && ($i%100 != 0 || $i%400 == 0) )) && echo "Leto $i je prestopno." || echo "Leto $i ni prestopno."   
   done
}

function fib
{
     p=0
     d=1
     max=${@:$#}
     i=0
     current=$1
     while [ $i -le $max ]
       do 
         if [ $i -eq $current ]
          then
           echo "$i: $p"
           shift
           current=$1
         fi   
          t=$p
          p=$d
          d=$(( $t + $d ))             
          i=$(($i+1))
        done
}

function userinfo 
{
    for i in $@
    do
     izpis="$i: "
     podatki=$(cat /etc/passwd | sed -n '/^'$i'.*/p' | awk -F: '{print $1,$3,$4}') #iz datoteke extrakta želeno vrstico (sed) ter z awk vrne 1,3,4 mesto splitano po :
     read -a data <<< "$podatki" #prebere vrnjene podatke v array
     if [ ${#data[@]} != 0 ] #ce smo nasli pregledujemo naprej
      then 
         if [ ${data[1]} = ${data[2]} ]
          then
           izpis=$izpis"enaka "
         fi
     obstajaVhome=$(dir /home | sed -n '/'$i'/p') #preveri ce obstaja v /home
    if [ -d "/home/uni" ]
    then 
      obstajaVuni=$(dir /home/uni | sed -n '/'$i'/p') # -||- v /home/uni/
    else
      obstajaVuni=""
    fi
         if [ ${#obstajaVhome} != 0 ] || [ ${#obstajaVuni} != 0 ] 
          then
           izpis=$izpis"obstaja "
         fi
          read -a group <<< $(id -G $i) # v array prebere vse indexe skupin v katerih je i,dolzina arraya je nas odgovor
          izpis=$izpis${#group[@]}
     else # ne obstaja
      izpis=$izpis"err" 
     fi
     echo $izpis
     izpis=""
    done   
}

function tocke
{
 st_studentov=0
 povprecje=0
 RANDOM=42 #nastavi seme
 while read -ra Vhod
  do
    if ( [ ${#Vhod[0]} = 1 ] && [ "${Vhod[0]}" != "#" ] ) || [ "${Vhod[0]:0:1}" != "#" ] #v primeru da je komentar, moznost da # sledi presledek in komentar ali komentar brez presledka
     then
      tocke=$((${Vhod[1]}+${Vhod[2]}+${Vhod[3]}))
       if [ ${#Vhod[@]} = 5 ] #preveri ce podan parameter p
        then
          if [ "${Vhod[4]}" = 'p' ] || [ "${Vhod[4]}" = 'P' ]
            then
             tocke=$(($tocke / 2))
          fi
       else
          if [ ${Vhod[0]:2:1} = 1 ] && [ ${Vhod[0]:3:1} = 4 ]
           then
            rand=$RANDOM #potrebno da se RANDOM ne ponastavi v subshell
            tocke=$(($tocke + $rand%5 +1))
          fi 
       fi
       if [ $tocke -gt 50 ]
        then
         tocke=50
       fi    
      echo "${Vhod[0]}: $tocke"
      st_studentov=$(($st_studentov + 1))
      povprecje=$(($povprecje + $tocke))  
    fi
  done
  echo "St. studentov: $st_studentov"
  povprecje=$(($povprecje / $st_studentov))
  echo "Povprecne tocke: $povprecje"      
}

function drevo
{
 MaxGlobina=$2
 trenutnaGlobina=$3
 escapeBackslashes=$( echo "$1" | sed -e 's/\//\\\//g' ) #escapa backslashe da sed ne joka
if [ $4 = 1 ]
then
 echo "DIR   $1"
fi

for vsebina in "$1"/*
do
  ime=$( echo "$vsebina" | sed -e 's/'$escapeBackslashes'\///' ) #iz celotne poti extrakta ime 
if [ -f "$vsebina" ]
then
    printaj $trenutnaGlobina
    echo "FILE  $ime"

elif [ -d "$vsebina" ]
then  
      printaj $trenutnaGlobina
      echo "DIR   $ime"
      globina=$(( $trenutnaGlobina+1 ))
     if (( $globina <= $MaxGlobina )) # ce smo ze dosegli max globino ne pregledujemo vec naprej
     then 
      (drevo "$vsebina" $MaxGlobina $globina 0) #run in subshell      
     fi

elif [ -L "$vsebina" ]
then
    printaj $trenutnaGlobina
    echo "LINK  $ime"

elif [ -c "$vsebina" ]
then
    printaj $trenutnaGlobina
    echo "CHAR  $ime"

elif [ -b "$vsebina" ]
then
    printaj $trenutnaGlobina
    echo "BLOCK $ime"

elif [ -p "$vsebina" ]
then
    printaj $trenutnaGlobina
    echo "PIPE  $ime"

elif [ -S "$vsebina" ]
then
    printaj $trenutnaGlobina
    echo "SOCK  $ime"
fi

done

} 

function unknown
{
echo "Napacna uporaba skripte!"
     pomoc $1
}

######################################################### klici funkcij glede na podane akcije

 if [ $# -eq 0 ]
 then
   unknown "$0"
   exit 42
 fi

   ime_skripte="$0"
   akcija=$1
   shift
   parametri="$@"

if [ "$akcija" = "pomoc" ]
then
     pomoc $ime_skripte
fi

if [ "$akcija" = "status" ]  
then 
     status $parametri 
fi

if [ "$akcija" = "leto" ]
then
     leto $parametri  
fi

if [ "$akcija" = "fib" ] 
then
      fib $parametri
fi

if [ "$akcija" = "userinfo" ]
then
     userinfo $parametri
fi

if [ "$akcija" = "tocke" ]
then
     tocke
fi

if [ "$akcija" = "drevo" ]
then 
     repo=${1:-$(pwd)} #privzeta vrednost
     globina=${2:-3} # same here
     drevo "$repo" "$globina" 1 1 # prva 1 predstavlja trenutno globino za izpis "----" druga 1 pa je znak da se funkcija klice prvic torej se mora izpisati celotni repo in ne zgolj ime
fi


exit 0
