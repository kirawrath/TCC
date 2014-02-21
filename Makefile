all: main.tex
	-bibtex output/main.aux
	pdflatex --output-directory output --halt-on-error main.tex &&\
		evince output/main.pdf
bib: clean
	pdflatex --output-directory output --halt-on-error main.tex > /dev/null &&\
	bibtex output/main.aux && \
	pdflatex --output-directory output --halt-on-error main.tex > /dev/null &&\
	pdflatex --output-directory output --halt-on-error main.tex > /dev/null
	
clean:
	rm -f main.aux  main.blg  main.lof  main.lot  main.out  main.toc\
		main.bbl  main.las  main.log  main.lsb  main.pdf output/*

