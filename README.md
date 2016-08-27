# ikconv
tune/module converter for "International Karaoke +" ( https://csdb.dk/release/?id=150113 )

================================================================================

create timing file using ultrastar creator
------------------------------------------

- get USC from: https://sourceforge.net/projects/usc/
- load mp3 and fill in song- and artist name.
- use 300 BPM
- tap away to create the timing
- save as ultrastar text file

create IK+ .mod from ultrastar creator .txt
-------------------------------------------

```
$ ./ikconv --readsid tune.sid --readusc ultrastartextfile.txt --writeikmod mymod.mod
```

add custom .mod to IK+ image
----------------------------

```
$ cp IKplus_contrib_beta1_player_only.bin IKplus_test.bin
$ cat mymodule1.mod >> IKplus_test.bin
$ cat mymodule2.mod >> IKplus_test.bin
$ cat mymodule3.mod >> IKplus_test.bin
$ cartconv -p -t easy -i IKplus_test.bin -o IKplus_test.crt
```

================================================================================

TODO:
-----

- fix end-of-file mark (extra 0)
- find out how to insert blank lines (looks like USC can not handle them?)
- make an option to link the IK+ binary in one go so cartconv is not needed
- most other things than the above shown procedure crash for one reason or
  another.
