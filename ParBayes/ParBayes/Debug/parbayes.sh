#!/bin/sh
/data/wangy/ParBayes/ParBayes/Debug/ParBayes -a strLearn_HC -pl /data/wangy/test/data/sc_p_15w_new_dis.shp -c 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14 -C 14 -os /data/wangy/test/result/stucture.xml

/data/wangy/ParBayes/ParBayes/Debug/ParBayes -a parLearn_EM  -pl /data/wangy/test/data/sc_p_15w_new_dis.shp -c 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14 -C 14 -is /data/wangy/test/result/stucture.xml -ops /data/wangy/test/result/bnet.xml

/data/wangy/ParBayes/ParBayes/Debug/ParBayes -a to2Infer_Naive -ips /data/wangy/test/result/bnet.xml -c 0,1,2,3,4,5,6,7,8,9,10,11,12,13,15 -C 15 -pi /data/wangy/test/data/sc_p_15w_new_dis.shp
