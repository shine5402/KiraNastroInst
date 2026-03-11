# Test Data Notes

This directory contains sample files used by the unit tests.

## Jazz-100-A.wav / Jazz-100-A.txt

**Source:** "One Note Jazz" BGM set by ちえP (Chie)
**Original site:** http://chie.cc/

From the readme included with the files:
> 楽に関わる権利は全て放棄しますので自由にご利用ください。再配布・改変なども自由です。
> (All rights to this music have been relinquished. Feel free to use it freely.
>  Redistribution and modification are also allowed.)

## test_stereo.{flac,ogg,mp3,opus,aiff} / test_mono.flac

2-second stereo (and 1-channel mono) excerpts derived from Jazz-100-A.wav above,
transcoded with ffmpeg for use in multi-format loading tests.  These files share
the same license as Jazz-100-A.wav (all rights relinquished by ちえP).

Generated with:
```
ffmpeg -i Jazz-100-A.wav -t 2 -map_metadata -1 test_stereo.flac
ffmpeg -i Jazz-100-A.wav -t 2 -map_metadata -1 -c:a vorbis -strict experimental test_stereo.ogg
ffmpeg -i Jazz-100-A.wav -t 2 -map_metadata -1 -c:a libmp3lame -q:a 4 test_stereo.mp3
ffmpeg -i Jazz-100-A.wav -t 2 -map_metadata -1 -c:a libopus -b:a 128k test_stereo.opus
ffmpeg -i Jazz-100-A.wav -t 2 -map_metadata -1 test_stereo.aiff
ffmpeg -i Jazz-100-A.wav -t 2 -ac 1 -map_metadata -1 test_mono.flac
ffmpeg -i Jazz-100-A.wav -t 2 -map_metadata -1 -c:a aac_at -b:a 128k test_stereo_aac.m4a
ffmpeg -i Jazz-100-A.wav -t 2 -map_metadata -1 -c:a alac test_stereo_alac.m4a
```

`test_stereo_aac.m4a` and `test_stereo_alac.m4a` require platform codecs (CoreAudio
on macOS, WindowsMediaAudio on Windows) and are silently skipped on Linux.

## 8mora.txt / 8mora-comment.txt

**Source:** VCV reclist by 辰 (Tatsumi); romaji comment version by Haru.jpg
**Original list:** https://ch.nicovideo.jp/tatsu3/blomaga/ar426004

From the readme:
> This romaji version (and the reupload) follow the original regulations.
> Haru.jpg do NOT claim any rights to this modified version.
> All rights are the property of the original author: 辰 (Tatsumi).

The original creator's terms permit free use of the reclist for voicebank recording.
These files are included here solely for unit testing of the parser logic.
