
P4_PATH=/root/p4code/p4src/bgw_switch.p4 
P4_NAME=bgw_switch

cmake $SDE/p4studio/ \
	-DCMAKE_INSTALL_PREFIX=$SDE/install \
	-DCMAKE_MODULE_PATH=$SDE/cmake \
	-DP4_NAME=$P4_NAME \
	-DP4_PATH=$P4_PATH \
	-DP4_LANG=p4-16  \
	-DP4FLAGS="--verbose 2 --create-graphs -g"
