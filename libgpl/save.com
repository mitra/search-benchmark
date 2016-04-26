rm ../dist/csoft.zip
zip -lor ../dist/csoft.zip \
	LICENSE.txt README.txt devkitPro.env palm.env save* \
	idl include libgpl libxdr anise finc tools tsion \
	csoft.* finc.* tsion.* Makefile \
	-x '*.bmp' '*.ico' '*.ncb' '*.opt' '*.plg' '*.tar' '*.tgz' '*.*~' RCS
zip -go ../dist/csoft.zip \
	anise/images/*.bmp \
	anise/images/*.ico \
	tools/icons/*.bmp \
	tools/icons/*.ico

rm ../dist/csoft.tgz
tar czvf ../dist/csoft.tgz \
	--exclude '*.ncb' \
	--exclude '*.opt' \
	--exclude '*.plg' \
	--exclude '*.tar' \
	--exclude '*.tgz' \
	--exclude '*.zip' \
	--exclude '*.*~' \
	--exclude 'RCS' \
	LICENSE.txt README.txt devkitPro.env palm.env save* \
	idl include libgpl libxdr anise finc tools tsion \
	csoft.* finc.* tsion.* Makefile

rm ../dist/esoft.zip
zip -lor ../dist/esoft.zip \
	LICENSE.txt README.txt devkitPro.env palm.env save* \
	idl include libgpl libxdr anise finc tools tsion epoch3 \
	csoft.* esoft.* finc.* tsion.* Makefile \
	-x '*.bmp' '*.ico' '*.ncb' '*.opt' '*.plg' '*.tar' '*.tgz' '*.*~' RCS
zip -go ../dist/esoft.zip \
	anise/images/*.bmp \
	anise/images/*.ico \
	tools/icons/*.bmp \
	tools/icons/*.ico \
	epoch3/tools/icons/*.bmp \
	epoch3/tools/icons/*.ico

rm ../dist/esoft.tgz
tar czvf ../dist/esoft.tgz \
	--exclude '*.ncb' \
	--exclude '*.opt' \
	--exclude '*.plg' \
	--exclude '*.tar' \
	--exclude '*.tgz' \
	--exclude '*.zip' \
	--exclude '*.*~' \
	--exclude 'RCS' \
	LICENSE.txt README.txt devkitPro.env palm.env save* \
	idl include libgpl libxdr anise finc tools tsion epoch3 \
	csoft.* esoft.* finc.* tsion.* Makefile
