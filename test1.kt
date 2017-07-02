#!./hoge

"hoge0"

/*
a1 = 10 + 2
a1 = 30
b1 = a1+10*2
b1=40
*/


#aa = 0x1aaa00CB



$page test_page1


$data32 hoge0
hoge0.addr = 0x5
#hoge0.data = val_0 | val_1<<4 | val_2<<8 
hoge0.data = val_0 | val_3
hoge0.misk = 1


#[min,max,def,shift]

val_0 = spin[0, (3+2)*2,  1, 4]
#val_1 = [0, 15, 10, 0]
#val_2 = [0, 1, 1, 0]

val_3 = r["hoge00","hoge11","hoge22",2]





$data64 hoge1
hoge1.addr = 10*3
hoge1.data = val1_1 | val1_0 | val1_2

#[min,max,def,shift]
val1_0 = [0, 7,  1, 0]
val1_1 = [0, 15, 4, 4]
val1_2 = [0, 15, 2, 60]




$page test_page2

$data32 hoge2
hoge2.addr = 10*2
hoge2.data = val2_0 | val2_1<<4 | val2_2<<8 | val2_3 << 12


#[min,max,def,shift]
val2_0 = [0, (3+2)*2,  1, 0]
val2_1 = [0, 15, 10, 0]
val2_2 = [0, 1, 0, 0]
val2_3 = r["hoge000","hoge111","hoge222",1]





