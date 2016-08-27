
CARTBIN=./examples/IKplus_contrib_beta1_player_only.bin
#VERBOSE=--verbose
#VERBOSE=--verbose --verbose
#TESTING=-DTESTING

all: ikconv

ikconv: Makefile main.c lrc.c ultrastar.c ikmod.c sid.c karaoke.h  Makefile
	$(CC) $(TESTING) -O3 -W -Wall -Wextra -Wno-unused-result -o ikconv main.c

.PHONY: test

test: testikmod testlrc testusc

testikmod: ikconv
	mkdir -p ./test
#	./ikconv $(VERBOSE) --readikmod ./examples/01-aces_high.mod --writeikmod ./test/aces_high-1.mod
	./ikconv $(VERBOSE) --readikmod ./examples/01-aces_high.mod --writesid ./test/aces_high.sid --writeikasm ./test/aces_high.asm
	acme --initmem 255  -o ./test/aces_high-asm.mod ./test/aces_high.asm

testlrc: ikconv
	mkdir -p ./test
	./ikconv $(VERBOSE) --readlrc ./examples/Break_Free.lrc --writelrc ./test/Break_Free.lrc
	./ikconv $(VERBOSE) --readlrc ./examples/Break_Free.lrc --writeusc ./test/Break_Free-2.usc
	./ikconv $(VERBOSE) --readsid ./examples/Break_Free.sid --readlrc ./examples/Break_Free.lrc --writeikmod ./test/Break_Free-lrc.mod

#	./ikconv $(VERBOSE) --readikmod ./test/01-aces_high.mod --writelrc ./test/aces_high.lrc --writesid ./test/aces_high.sid
#	./ikconv $(VERBOSE) --readsid ./test/aces_high.sid --readlrc ./test/aces_high.lrc --writeikmod ./test/aces_high-2.mod

testusc: ikconv
	mkdir -p ./test
	./ikconv $(VERBOSE) --readusc "./examples/Queen - I want to break free.txt" --writeusc ./test/Break_Free.usc
	./ikconv $(VERBOSE) --readsid ./examples/Break_Free.sid --readusc "./examples/Queen - I want to break free.txt" --writeikmod ./test/Break_Free-usc.mod --writeikasm ./test/Break_Free-usc.asm
	acme --initmem 255 -o ./test/Break_Free-usc-asm.mod ./test/Break_Free-usc.asm

#	./ikconv $(VERBOSE) --readikmod ./test/01-aces_high.mod --writeusc ./test/aces_high.usc --writesid ./test/aces_high-1.sid
#	./ikconv $(VERBOSE) --readikmod ./test/Break_Free-usc.mod --writeusc ./test/Break_Free-1.usc --writesid ./test/Break_Free-1.sid
	./ikconv $(VERBOSE) --readikmod ./test/Break_Free-usc-asm.mod --writesid ./test/Break_Free-2.sid --writeikasm ./test/Break_Free-2.asm

testcartik: testikmod
	mkdir -p ./test
	cp $(CARTBIN) ./test/IKplus_v3_test.bin
	cat ./test/aces_high-asm.mod >> ./test/IKplus_v3_test.bin
	dd if=/dev/null of=./test/IKplus_v3_test.bin bs=1 count=0 seek=1048576
	x64sc -carteasy ./test/IKplus_v3_test.bin

testcartusc: ikconv
	mkdir -p ./test
	./ikconv $(VERBOSE) --readsid ./examples/Break_Free.sid --readusc "./examples/Queen - I want to break free.txt" --writeikmod ./test/Break_Free-usc.mod
	./ikconv $(VERBOSE) --readsid ./examples/Break_Free.sid --readusc "./examples/Queen - I want to break free.txt" --writeikasm ./test/Break_Free-usc.asm
	acme --initmem 255  -o ./test/Break_Free-usc-asm.mod ./test/Break_Free-usc.asm
	cp $(CARTBIN) ./test/IKplus_v3_test.bin
	diff ./test/Break_Free-usc.mod ./test/Break_Free-usc-asm.mod
#	cat ./test/Break_Free-usc.mod >> ./test/IKplus_v3_test.bin
	cat ./test/Break_Free-usc-asm.mod >> ./test/IKplus_v3_test.bin
	dd if=/dev/null of=./test/IKplus_v3_test.bin bs=1 count=0 seek=1048576
	x64sc -carteasy ./test/IKplus_v3_test.bin

testcartlrc: testlrc
	mkdir -p ./test
	cp $(CARTBIN) ./test/IKplus_v3_test.bin
	cat ./test/Break_Free-lrc.mod >> ./test/IKplus_v3_test.bin
	dd if=/dev/null of=./test/IKplus_v3_test.bin bs=1 count=0 seek=1048576
	x64sc -carteasy ./test/IKplus_v3_test.bin

clean:
	rm -f ikconv
	rm -f ./test/*
