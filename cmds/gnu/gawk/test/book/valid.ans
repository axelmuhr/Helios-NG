BEGIN { FS = "\t" }
/^=/  { print substr($0, 2); next }
{ printf("%s {\n\tprintf(\"line %%d, %s: %%s\\n\",NR,$0) }\n",
      $1, $2)
}
