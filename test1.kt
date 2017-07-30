#!./hoge

/** widget define ******************************************
val =   [min, max, def, shift]    #scale widget, sc is optional
val = sp[min, max, def, shift]    #spin widget
val = c["str0","str1","str2,def]  #combo widget, def is optional
val = r["str0","str1","str2,def]  #radio widget, def is optional
*****************************************************/

str0 = "tete"

$page "page1"

$data32 hoge0 = {
  .addr = 0x0001
  .data = val_0 | val_1 | val_2 << 8 | val_3 << 12
  .misk = 1
}
  
#[min,max,def,shift]

val_0 = [0, 12,  10, 0]
val_1 = sp[0, 15,  4, 4]
val_2 = c["str0","str1","str2",2]
val_3 = r["str00","str11","str22",1]

