GCC = gcc
LIBS = -lcurl 

web_status-csv: web_status-csv.o
	$(GCC) -g -o $@ $< $(LIBS)
	./web_status-csv

web_status.o: web_status-csv.cpp
	$(GCC) -g -c $< 

clean:
	rm -f web_status-csv.o web_status-csv
