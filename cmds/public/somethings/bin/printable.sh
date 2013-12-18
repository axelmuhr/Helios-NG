file $* | sed -n -e  "/:.*text/s/\(.*\):.*/\1/p" -e "/:.*script/s/\(.*\):.*/\1/p"
exit 0
