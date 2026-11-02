#include "GerenciadorAcademia.h"
#include <exception>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc > 2) {
        std::cerr << "Uso: " << argv[0] << " [arquivo-de-alunos]\n";
        return 1;
    }
    try {
        GerenciadorAcademia gerenciador(argc == 2 ? argv[1] : "alunos.txt");
        int opcao = 0;
        do {
            try {
                std::cout << "\n=== ACADEMIA - CADASTRO DE ALUNOS ===\n"
                          << "1. Inserir\n2. Listar todos\n3. Exibir um (pesquisar por nome)\n"
                          << "4. Alterar (pesquisar por nome)\n5. Remover (pesquisar por nome)\n"
                          << "6. Exibir relatorio\n7. Sair\n";
                opcao = util::lerInteiro("Opcao: ", 1, 7, std::cin, std::cout);
                switch (opcao) {
                    case 1: gerenciador.inserir(); break;
                    case 2: gerenciador.listarTodos(); break;
                    case 3: gerenciador.exibirUm(); break;
                    case 4: gerenciador.alterar(); break;
                    case 5: gerenciador.remover(); break;
                    case 6: gerenciador.relatorio(); break;
                    default: break;
                }
            } catch (const std::ios_base::failure&) {
                std::cout << "\nFim da entrada. Encerrando...\n";
                opcao = 7;
            } catch (const std::exception& erro) {
                std::cout << "Operacao nao concluida: " << erro.what() << '\n';
            }
        } while (opcao != 7);
        gerenciador.salvar();
        std::cout << "Dados salvos. Ate logo!\n";
        return 0;
    } catch (const std::exception& erro) {
        std::cerr << "Erro: " << erro.what() << '\n';
        return 1;
    }
}
