
mac2lineMac.exe: mac2lineMac.cpp ../*.h
	g++ mac2lineMac.cpp -o mac2lineMac.exe

test: sample.linemac

sample.linemac: mac2lineMac.exe ../samples/sample.mac
	./mac2lineMac.exe -i ../samples/sample.mac -o sample.linemac

.PHONY: test