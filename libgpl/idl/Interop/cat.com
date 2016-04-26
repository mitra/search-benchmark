#/bin/csh

foreach file (*.idl ../Misc/Messaging.idl)
echo "# 1 "'"'$file'"'
cat $file
end
