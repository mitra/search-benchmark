rm ../dist/csoftv.zip
zip -lor ../dist/csoftv.zip \
	LICENSE.txt README.txt saveVMS.com \
	include libgpl libxdr anise tools epoch3 \
	-x '*.ico' '*.ncb' '*.plg' '*.tar' '*.*~' 'RCS' \
	'*.awk' '*Makefile.*' '*.dep' '*.dsp' '*.mak' '*.def' '*.unix' '*.vx'
