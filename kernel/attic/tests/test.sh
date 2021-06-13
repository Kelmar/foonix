nm -ng ../string/string.ro | awk '/^ *U / { next }; /^[0-9a-fA-F]+ C/ { next }; / main$$/ { print "main _crunched_ls_stub"; next }; { print $$3 " " $$3 "$$$$from$$$$ls" }' > string.cro.syms
