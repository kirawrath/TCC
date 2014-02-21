main.pdf: main.tex
	pdflatex --output-directory output --halt-on-error main.tex &&\
		evince output/main.pdf
