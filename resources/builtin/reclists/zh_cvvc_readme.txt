Notes: The usage of this reclist to be included in this product has been approved by its original author.
说明：本录音表的原作者已经批准了对在本软件中嵌入这份录音表的使用。

---

感谢下载Hr.J式CVVC中文录音表。

本录音表沿用Risku式CVVC中文录音方案所使用的音标，经由程序自动生成。
经测可用，但不保证100%的音素完整度。

由于本录音表为机械生成，因此相对目前既存的录音表来说较为难读，
初学者建议使用3字或3+6字录音表。8字表仅推荐录音熟练者及刁钻人群使用。

本录音表也可用于DeepVocal声库的录音。
仅打算用于DeepVocal声库时，建议使用lite录音表。

---

录音量一览表：

字数	包含音素		重复音素		录音文件数	原音设定行数	对应BPM
3+3字	- CV/CV/CV_L/VC	不保留		796		2175		120
3+5字	- CV/CV/CV_L/VC	不保留		608		2175		120/140
3+6字	- CV/CV/CV_L/VC	不保留		565		2175		120/140
3+8字	- CV/CV/CV_L/VC	不保留		518		2175		120/140
3字	- CV/CV/VC	CV3个/VC不保留	705		2274		120
5字	- CV/CV/VC	CV3个/VC不保留	403		2255		120/140
6字	- CV/CV/VC	CV3个/VC不保留	333		2217		120/140
8字	- CV/CV/VC	CV3个/VC不保留	252		2207		120/140
3字lite	CV/VC		CV3个/VC不保留	623		2105		120
5字lite	CV/VC		CV3个/VC不保留	317		1992		120
6字lite	CV/VC		CV3个/VC不保留	252		1965		120
8字lite	CV/VC		CV3个/VC不保留	179		1912		120

·参考用
Risku式3字	- CV/CV/VC	不保留	988		1972（大致）	-
松尾式8字		CV/VC		不保留	211		1545（大致）	120
lite8字理论最低值	CV/VC		不保留	182		1932		-

Others录音表包含以下音素：
长母音（_a_L.wav/a L）、喉塞音（_a-a.wav/・ a/a R・）
语尾息（_a_R.wav/a 息R）、语尾吸气（_a_B.wav/a 吸R）
呼吸音（@br1～@br5）

omake-Vowels表包含本录音表涉及的全部元音，可根据自行需要定制语尾息等追加音素。
实际录音时，推荐您录制任一基础录音表+Others录音表。
在同一音阶内，基础录音和Others录音可合并至同一文件夹/同一oto.ini内。

---

关于3+3字、3+5字、3+6、3+8字录音表：

请使用3~8字的BGM录音，且将第3字长读，直至读满BGM所需拍数。
第3字的音素将以「ka_L」的形式赋予别名，可作为长采样随时调用。
当然，您选择的BGM越长（字数越多），获得的长音素也就随之越长。

当您读完所有3字部分之后，
录音表将回到多字形式（3字情况下则是回到非“CV_CV_CV”的形式），在此部分补足VC音素。

文件夹中有附带录音范例（BGM：OneNoteJazz/ちえP/140bpm），
录制时可用于参考。

---

关于原音设定：

本录音表附带的oto.ini均按照CV→VC的顺序排列。
若您通过UTAU读取音源并覆盖oto.ini，将会打乱这个顺序。
因此，推荐您使用Setparam统一设定完之后，再将音源放入UTAU使用。

本录音表附带的oto.ini并未考虑由不同BGM产生的开头空白长度差。
您可以在SetParam中打开，记录第一个标记的左blank设置前后的差值，
然后使用ctrl+m统一增加或减少左blank的值，即可让标记移动至相对大致正确的位置。

在统一改动oto.ini中的数值时，请注意备份。

---

补充：

音源完成后，请将文件夹中附带的presamp.ini一同放入音源文件夹中。
（和音源头像、character.txt等处于同一文件夹）
这样就可以使用presamp或autoCVVC了。

---

生成工具：
https://bowlroll.net/file/213514 pass: cvvc
※beta版，无GUI，中～上级者向。

---

制作者一览

录音表生成软件制作：科林 / sder.colin@gmail.com / @sder_colin
配布 / 3+5、3+8字录音表设计：Haru.jpg / limiaomiao3304@aliyun.com / @Haru_jpg

特别鸣谢
中文CVVC音标设计者：Risku / http://riskucvvc.lofter.com
