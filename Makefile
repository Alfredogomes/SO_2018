help:
	@echo "Instalar o programa:         "make install""
	@echo "Editar o notebook:           "make edit""
	@echo "Voltar ao estado inicial:    "make reset""
	@echo "Executar o programa:         "make run""
	@echo "Apagar o notebook:           "make clean""

install:
	@echo gcc processa.c -o notebook	
	@gcc processa.c -o notebook
	@make edit
	@cp notebook.nb{,.kbp}
	@make pasta

pasta:
	@mkdir teste

edit:
	@vim "notebook.nb"

view:
	@more notebook.nb

reset:
	@rm -f notebook.nb
	@cp notebook.nb.kbp notebook.nb

run:
	@./notebook notebook.nb
	@make clean

clean:
	@rm -f teste/*.txt

cleanAll:
	@rm -f notebook.nb
	@ rm -f notebook.nb.kbp
	@make clean
