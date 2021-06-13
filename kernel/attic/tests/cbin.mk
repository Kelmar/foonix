ls_make: ls .PHONY
	( cd ls; printf '.PATH: ${ls_SRCDIR}\n.CURDIR:= ${ls_SRCDIR}\n.include "$${.CURDIR}/Makefile"\n'\
	| ${MAKE} -f- CRUNCHEDPROG=1 DBG="${DBG}" depend )
	( cd ls; printf '.PATH: ${ls_SRCDIR}\n.CURDIR:= ${ls_SRCDIR}\n.include "$${.CURDIR}/Makefile"\n'\
	| ${MAKE} -f- CRUNCHEDPROG=1 DBG="${DBG}" ${ls_OBJS} ) 

ls.cro: ls .WAIT ${ls_OBJPATHS}
	${LD} -r -o ls/ls.ro $(ls_OBJPATHS)
	${NM} -ng ls/ls.ro | awk '/^ *U / { next }; /^[0-9a-fA-F]+ C/ { next }; / main$$/ { print "main _crunched_ls_stub"; next }; { print $$3 " " $$3 "$$$$from$$$$ls" }' > ls.cro.syms
	${OBJCOPY} --redefine-syms ls.cro.syms ls/ls.ro ls.cro

