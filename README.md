# KiraNastro Inst.

<img width="1151" height="607" alt="3f15c21f7e6e58eabd4ca61e4f8ba72e" src="https://github.com/user-attachments/assets/e850e42a-14b4-4701-8581-a212efde6c5e" />

[日本語](#日本語) | [English](#english) | [中文](#中文)

---

## 日本語

> ⚠️ 工事中のα版です。あまりテストできていないため、使用の際は自己責任でよろしくお願いします。

DAW上で動作するUTAU音源収録支援ツールです。楽器プラグインとして動作し、BGMのループ再生とともにDAWの再生位置に応じて現在の録音項目を画面に表示します。

### 使い方

1. Releasesからプラグインを入手してDAWに読み込む
2. 収録を進める
3. DAWから録音をオーディオファイルとして書き出す
4. **Export KiraWavTar Desc...** を実行して書き出したファイルと同じ場所に保存する
5. 書き出したファイルをKiraWAVTarに読み込ませると、原音設定用のwavファイルを自動で切り出せます

### ライセンス

[GPLv3](LICENSE.txt)

内蔵リソース（録音リスト・ガイドBGM）のライセンスおよび権利表記については、[resource/builtin/](resource/builtin/) を参照してください。

---

## English

> ⚠️ Work in progress (alpha). Not thoroughly tested — use at your own risk.

A UTAU voicebank recording assistant plugin for DAWs. It runs as an instrument plugin, playing back a BGM loop while displaying the current recording prompt on screen in sync with the DAW's playback position.

### Usage

1. Download the plugin from Releases and load it into your DAW
2. Record your voicebank
3. Export the recorded audio as audio files from your DAW
4. Run **Export KiraWavTar Desc...** and save the output to the same folder as the exported files
5. Load the files into KiraWAVTar — it will automatically split them into wav files ready for labeling

### License

[GPLv3](LICENSE.txt)

For license and attribution information regarding built-in resources (reclist and guide BGM), see [resource/builtin/](resource/builtin/).

---

## 中文

> ⚠️ 本工具目前为测试中的Alpha版本，尚未经过充分测试，使用时请自行承担风险。

可以在DAW里进行UTAU音源收录的辅助插件。作为乐器插件在DAW内进行自发的BGM循环播放，切同时根据DAW的播放位置在画面上显示当前的录音条目。

### 使用方法

1. 从Releases下载插件并加载到DAW中
2. 开始录音
3. 在DAW中将录音导出为音频文件
4. 运行 **Export KiraWavTar Desc...**，将输出文件保存至与导出音频相同的文件夹中
5. 将文件加载到KiraWAVTar中，即可自动分割出用于标注的wav文件

### 许可证

[GPLv3](LICENSE.txt)

若要查看内置资源（录音列表、导引BGM）的许可证及权利声明，请参阅 [resource/builtin/](resource/builtin/)。
