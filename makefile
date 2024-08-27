httpServer:Server.cc
	g++ -o $@ $^ -std=c++14
.PHNOY:
clean:
	rm -rf httpServer