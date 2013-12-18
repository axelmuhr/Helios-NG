BEGIN		{ printf("/*\n") ;}
/RCS file:.*/	{ file[++i]=$6 ; ignoredate=0; }
/head:/		{ printf(" *\t%-30s\t%s\n",file[i],$2) ; revision[i]=$2; }
/date:/		{ if (ignoredate==0) {
			date[i]=$2;
			time[i]=$3;
			time[i]=substr($3,0,length($3)-1);
			author[i]=substr($5,0,length($5)-1);
			state[i]=substr($7,0,length($7)-1);
			ignoredate=1;
		} }
END		{ printf(" */\n\nextern char rcs_ident[] /*/ = \"@(#) $");
		  printf("Header$\\n\" /*/ ;\n");
		  imax=i;
		  for(i=1;i<=imax;i++) {
			name="";
			for(j=1;substr(file[i],j,1)!="";j++) {
				this=substr(file[i],j,1);
				if (this == ".") {
					name=sprintf("%s%s",name,"_") ;
				} else if (this == ",") {
					name=sprintf("%s%s",name,"_") ;
				} else {
					name=sprintf("%s%s",name,this) ;
				}
			}
			printf("extern\tchar\tV_%s[] /*/ = \"@(#) ", name);
			printf("Header: %s,v %s %s %s %s %s\\n\" /*/ ;\n", \
				file[i],revision[i],date[i],time[i], \
				author[i],state[i]) ;
		  }
		  }
