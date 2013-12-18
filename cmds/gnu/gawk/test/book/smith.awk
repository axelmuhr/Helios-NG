BEGIN         { RS = ""; FS = "\n" }
$1 ~ /Smith$/ { print $1, $4 }   # name, phone
