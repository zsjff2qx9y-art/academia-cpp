#include "GerenciadorAcademia.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;

namespace {

void exigir(bool condicao, const std::string& mensagem) {
    if (!condicao) throw std::runtime_error(mensagem);
}

template <typename Excecao, typename Acao>
void exigirExcecao(Acao acao, const std::string& mensagem) {
    bool ocorreu = false;
    try {
        acao();
    } catch (const Excecao&) {
        ocorreu = true;
    }
    exigir(ocorreu, mensagem);
}

void gravar(const fs::path& arquivo, const std::string& texto) {
    fs::create_directories(arquivo.parent_path());
    std::ofstream saida(arquivo, std::ios::binary);
    saida << texto;
    if (!saida) throw std::runtime_error("Falha ao preparar arquivo de teste.");
}

std::string ler(const fs::path& arquivo) {
    std::ifstream entrada(arquivo, std::ios::binary);
    if (!entrada) throw std::runtime_error("Falha ao ler arquivo de teste.");
    return {std::istreambuf_iterator<char>(entrada), std::istreambuf_iterator<char>()};
}

const std::string MUSCULACAO =
    "Musculacao \"Ana Maria\" \"(11) 99999-1234\" 10000 \"2025-03-12\" "
    "\"Condicionamento\" \"Joao Silva\"\n";
const std::string FUNCIONAL =
    "Funcional \"Bruno Lima\" \"11988881234\" 15000 \"2025-03-13\" 3 \"Iniciante\"\n";

} // namespace

int main() {
    const fs::path pasta = fs::temp_directory_path() /
        ("academia-testes-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
    fs::create_directories(pasta);
    int aprovados = 0;
    int reprovados = 0;
    auto registrar = [&](const std::string& nome, const std::function<void()>& teste) {
        try {
            teste();
            ++aprovados;
            std::cout << "[OK] " << nome << '\n';
        } catch (const std::exception& erro) {
            ++reprovados;
            std::cerr << "[FALHOU] " << nome << ": " << erro.what() << '\n';
        }
    };

    registrar("Datas validam calendario e anos bissextos", [&] {
        exigir(Data(29, 2, 2000).toISO() == "2000-02-29", "Ano 2000 deveria ser bissexto.");
        exigir(Data::deISO("2024-02-29").toString() == "29/02/2024", "Conversao de data incorreta.");
        exigir(Data(1, 1, 1).toISO() == "0001-01-01", "Ano minimo incorreto.");
        exigirExcecao<std::invalid_argument>([] { Data data(29, 2, 1900); }, "1900 nao e bissexto.");
        exigirExcecao<std::invalid_argument>([] { Data data(31, 4, 2026); }, "Abril nao tem 31 dias.");
        exigirExcecao<std::invalid_argument>([] { Data data(1, 1, 0); }, "Ano zero deve ser rejeitado.");
        exigirExcecao<std::invalid_argument>([] { Data::deISO("2026-1-01"); }, "Formato deve ser estrito.");
    });

    registrar("Modelos rejeitam dados invalidos", [&] {
        const Data data(1, 1, 2026);
        exigirExcecao<std::invalid_argument>([&] {
            AlunoMusculacao aluno(" ", "11999991234", 10000, data, "Saude", "Ana");
        }, "Nome vazio deveria ser rejeitado.");
        exigirExcecao<std::invalid_argument>([&] {
            AlunoMusculacao aluno("Ana", "123", 10000, data, "Saude", "Ana");
        }, "Telefone curto deveria ser rejeitado.");
        exigirExcecao<std::invalid_argument>([&] {
            AlunoMusculacao aluno("Ana", "11999991234", 0, data, "Saude", "Ana");
        }, "Mensalidade zero deveria ser rejeitada.");
        exigirExcecao<std::invalid_argument>([&] {
            AlunoFuncional aluno("Ana", "11999991234", 10000, data, 8, "Iniciante");
        }, "Oito aulas semanais deveriam ser rejeitadas.");
    });

    registrar("Primeira execucao inicia vazia e salva arquivo unico", [&] {
        const auto arquivo = pasta / "vazio" / "alunos.txt";
        std::istringstream entrada;
        std::ostringstream saida;
        GerenciadorAcademia cadastro(arquivo.string(), entrada, saida);
        exigir(cadastro.getAlunos().empty(), "Cadastro inicial deveria estar vazio.");
        cadastro.listarTodos();
        cadastro.relatorio();
        cadastro.salvar();
        exigir(ler(arquivo) == "ACADEMIA_V1\n", "Arquivo vazio deveria conter apenas o cabecalho.");
        exigir(saida.str().find("R$ 0,00") != std::string::npos, "Relatorio vazio incorreto.");
    });

    registrar("Cadastro dos dois tipos altera vetor antes do arquivo", [&] {
        const auto arquivo = pasta / "cadastro.txt";
        gravar(arquivo, "ACADEMIA_V1\n");
        std::istringstream entrada(
            "1\nAna Maria\n(11) 99999-1234\n100,50\nHipertrofia\nJoao Silva\n"
            "2\nBruno Lima\n11988881234\n150.25\n3\nIniciante\n");
        std::ostringstream saida;
        GerenciadorAcademia cadastro(arquivo.string(), entrada, saida);
        cadastro.inserir();
        cadastro.inserir();
        exigir(cadastro.getAlunos().size() == 2, "Dois cadastros deveriam existir no vetor.");
        exigir(ler(arquivo) == "ACADEMIA_V1\n", "Inserir nao deve gravar diretamente no arquivo.");
        exigir(cadastro.getAlunos()[0]->getMensalidadeCentavos() == 10050, "Preco com virgula incorreto.");
        exigir(cadastro.getAlunos()[1]->getMensalidadeCentavos() == 15025, "Preco com ponto incorreto.");
        cadastro.salvar();
        std::istringstream novaEntrada;
        std::ostringstream novaSaida;
        GerenciadorAcademia reaberto(arquivo.string(), novaEntrada, novaSaida);
        exigir(reaberto.getAlunos().size() == 2, "Reabertura perdeu registros.");
        const auto* musculacao = dynamic_cast<const AlunoMusculacao*>(reaberto.getAlunos()[0].get());
        const auto* funcional = dynamic_cast<const AlunoFuncional*>(reaberto.getAlunos()[1].get());
        exigir(musculacao && musculacao->getInstrutor() == "Joao Silva", "Subclasse de musculacao nao foi reconstruida.");
        exigir(funcional && funcional->getAulasSemanais() == 3, "Subclasse funcional nao foi reconstruida.");
    });

    registrar("Pesquisa aceita nome completo prefixo espacos e ASCII", [&] {
        const auto arquivo = pasta / "pesquisa.txt";
        gravar(arquivo, "ACADEMIA_V1\n" + MUSCULACAO + FUNCIONAL);
        std::istringstream entrada("ana ma\n");
        std::ostringstream saida;
        GerenciadorAcademia cadastro(arquivo.string(), entrada, saida);
        exigir(cadastro.pesquisar("Ana Maria") == 0, "Nome completo nao foi encontrado.");
        exigir(cadastro.pesquisar("  aNA ma  ") == 0, "Prefixo com espacos/maiusculas nao foi encontrado.");
        exigir(cadastro.pesquisar("inexistente") == -1, "Nome inexistente deveria retornar -1.");
        exigir(cadastro.pesquisar("   ") == -1, "Pesquisa vazia nao deve listar todos.");
        cadastro.exibirUm();
        exigir(saida.str().find("Instrutor: Joao Silva") != std::string::npos, "Exibir nao despachou a subclasse.");
    });

    registrar("Homonimos exigem indice pertencente aos resultados", [&] {
        const auto arquivo = pasta / "homonimos.txt";
        gravar(arquivo, "ACADEMIA_V1\n" + MUSCULACAO + FUNCIONAL + MUSCULACAO);
        std::istringstream entrada("1\n2\n");
        std::ostringstream saida;
        GerenciadorAcademia cadastro(arquivo.string(), entrada, saida);
        exigir(cadastro.pesquisar("Ana Maria") == 2, "Indice fora dos resultados foi aceito.");
        exigir(saida.str().find("[0]") != std::string::npos, "Primeiro indice nao foi mostrado.");
        exigir(saida.str().find("[2]") != std::string::npos, "Segundo indice nao foi mostrado.");
        exigir(saida.str().find("[1]") == std::string::npos, "Aluno fora da busca foi apresentado como candidato.");
    });

    registrar("Selecao de homonimos pode ser cancelada", [&] {
        const auto arquivo = pasta / "cancelar-selecao.txt";
        gravar(arquivo, "ACADEMIA_V1\n" + MUSCULACAO + MUSCULACAO);
        std::istringstream entrada("-1\n");
        std::ostringstream saida;
        GerenciadorAcademia cadastro(arquivo.string(), entrada, saida);
        exigir(cadastro.pesquisar("Ana") == -1, "Cancelamento deveria retornar -1.");
        exigir(cadastro.getAlunos().size() == 2, "Cancelamento alterou o vetor.");
    });

    registrar("Alteracao preserva matricula e Enter mantem campos", [&] {
        const auto arquivo = pasta / "alterar.txt";
        const auto original = "ACADEMIA_V1\n" + MUSCULACAO;
        gravar(arquivo, original);
        std::istringstream entrada("Ana\nAna Souza\n\n125,75\n\nCarla Reis\n");
        std::ostringstream saida;
        GerenciadorAcademia cadastro(arquivo.string(), entrada, saida);
        cadastro.alterar();
        const auto& aluno = cadastro.getAlunos().front();
        exigir(aluno->getNome() == "Ana Souza", "Nome nao foi alterado.");
        exigir(aluno->getTelefone() == "(11) 99999-1234", "Enter deveria manter telefone.");
        exigir(aluno->getMensalidadeCentavos() == 12575, "Mensalidade nao foi alterada.");
        exigir(aluno->getDataMatricula().toISO() == "2025-03-12", "Matricula nao deveria mudar.");
        const auto* musculacao = dynamic_cast<const AlunoMusculacao*>(aluno.get());
        exigir(musculacao && musculacao->getObjetivo() == "Condicionamento" &&
            musculacao->getInstrutor() == "Carla Reis", "Campos da subclasse nao foram editados corretamente.");
        exigir(ler(arquivo) == original, "Alteracao deve ocorrer no vetor antes de salvar.");
        cadastro.salvar();
        exigir(ler(arquivo).find("Ana Souza") != std::string::npos, "Alteracao nao foi persistida.");
    });

    registrar("Alteracao funcional modifica campos especificos", [&] {
        const auto arquivo = pasta / "alterar-funcional.txt";
        gravar(arquivo, "ACADEMIA_V1\n" + FUNCIONAL);
        std::istringstream entrada("Bruno\n\n\n\n5\nAvancado\n");
        std::ostringstream saida;
        GerenciadorAcademia cadastro(arquivo.string(), entrada, saida);
        cadastro.alterar();
        const auto* aluno = dynamic_cast<const AlunoFuncional*>(cadastro.getAlunos().front().get());
        exigir(aluno && aluno->getAulasSemanais() == 5 && aluno->getNivel() == "Avancado", "Campos funcionais nao foram alterados.");
    });

    registrar("Remocao confirma e cancelamento preserva cadastro", [&] {
        const auto arquivo = pasta / "remover.txt";
        const auto original = "ACADEMIA_V1\n" + MUSCULACAO + FUNCIONAL;
        gravar(arquivo, original);
        std::istringstream entrada("Ana\nn\nAna\ns\n");
        std::ostringstream saida;
        GerenciadorAcademia cadastro(arquivo.string(), entrada, saida);
        cadastro.remover();
        exigir(cadastro.getAlunos().size() == 2, "Cancelar remocao deveria preservar os dois alunos.");
        cadastro.remover();
        exigir(cadastro.getAlunos().size() == 1 && cadastro.getAlunos()[0]->getNome() == "Bruno Lima", "Remocao retirou aluno errado.");
        exigir(ler(arquivo) == original, "Remover nao deve editar diretamente o arquivo.");
        cadastro.salvar();
        exigir(ler(arquivo).find("Ana Maria") == std::string::npos, "Remocao nao foi persistida.");
    });

    registrar("EOF em insercao nao adiciona aluno incompleto", [&] {
        const auto arquivo = pasta / "eof-inserir.txt";
        std::istringstream entrada("1\nAna Maria\n11999991234\n100\nSaude\n");
        std::ostringstream saida;
        GerenciadorAcademia cadastro(arquivo.string(), entrada, saida);
        exigirExcecao<std::ios_base::failure>([&] { cadastro.inserir(); }, "EOF deveria interromper insercao.");
        exigir(cadastro.getAlunos().empty(), "Aluno incompleto entrou no cadastro.");
        cadastro.salvar();
        exigir(ler(arquivo) == "ACADEMIA_V1\n", "Insercao incompleta foi persistida.");
    });

    registrar("EOF em edicao preserva todos os dados originais", [&] {
        const auto arquivo = pasta / "eof-alterar.txt";
        const auto original = "ACADEMIA_V1\n" + MUSCULACAO;
        gravar(arquivo, original);
        std::istringstream entrada("Ana\nNome Modificado\n\n250\nNovo Objetivo\n");
        std::ostringstream saida;
        GerenciadorAcademia cadastro(arquivo.string(), entrada, saida);
        exigirExcecao<std::ios_base::failure>([&] { cadastro.alterar(); }, "EOF deveria interromper edicao.");
        exigir(cadastro.getAlunos()[0]->getNome() == "Ana Maria", "Edicao incompleta alterou dados comuns.");
        cadastro.salvar();
        exigir(ler(arquivo) == original, "Edicao incompleta alterou o arquivo.");
    });

    registrar("Textos com aspas barras pipes e acentos sobrevivem a reabertura", [&] {
        const auto arquivo = pasta / "textos.txt";
        const std::string nome = "João \"Jota\" | Souza";
        const std::string objetivo = "Força \\ mobilidade | saúde";
        std::istringstream entrada("1\n" + nome + "\n11999991234\n100\n" + objetivo + "\nLuísa Reis\n");
        std::ostringstream saida;
        GerenciadorAcademia cadastro(arquivo.string(), entrada, saida);
        cadastro.inserir();
        cadastro.salvar();
        std::istringstream novaEntrada;
        std::ostringstream novaSaida;
        GerenciadorAcademia reaberto(arquivo.string(), novaEntrada, novaSaida);
        const auto* aluno = dynamic_cast<const AlunoMusculacao*>(reaberto.getAlunos()[0].get());
        exigir(aluno && aluno->getNome() == nome && aluno->getObjetivo() == objetivo,
               "Textos foram modificados pela serializacao.");
    });

    registrar("Relatorio calcula media geral e totais por modalidade", [&] {
        const auto arquivo = pasta / "relatorio.txt";
        gravar(arquivo, "ACADEMIA_V1\n" + MUSCULACAO + FUNCIONAL);
        std::istringstream entrada;
        std::ostringstream saida;
        GerenciadorAcademia cadastro(arquivo.string(), entrada, saida);
        cadastro.relatorio();
        const auto texto = saida.str();
        exigir(texto.find("Alunos cadastrados: 2") != std::string::npos, "Contagem incorreta.");
        exigir(texto.find("Mensalidades previstas por mes: R$ 250,00") != std::string::npos, "Soma monetaria incorreta.");
        exigir(texto.find("Media das mensalidades: R$ 125,00") != std::string::npos, "Media incorreta.");
        exigir(texto.find("Musculacao: 1 aluno(s), R$ 100,00") != std::string::npos, "Total da musculacao incorreto.");
        exigir(texto.find("Funcional: 1 aluno(s), R$ 150,00") != std::string::npos, "Total do funcional incorreto.");
        exigir(texto.find("Nome:") == std::string::npos, "Relatorio nao deve ser simples listagem.");
    });

    registrar("Entradas invalidas sao rejeitadas antes de cadastrar", [&] {
        const auto arquivo = pasta / "validacao.txt";
        std::istringstream entrada(
            "0\nabc\n2\n\nAna Maria\n123\n11999991234\n"
            "nan\n-1\n0\n10,999\n1000000,01\n99,90\n0\n8\n3\n\nIniciante\n");
        std::ostringstream saida;
        GerenciadorAcademia cadastro(arquivo.string(), entrada, saida);
        cadastro.inserir();
        const auto* aluno = dynamic_cast<const AlunoFuncional*>(cadastro.getAlunos()[0].get());
        exigir(aluno && aluno->getNome() == "Ana Maria" && aluno->getMensalidadeCentavos() == 9990 &&
               aluno->getAulasSemanais() == 3 && aluno->getNivel() == "Iniciante", "Entrada invalida contaminou o cadastro.");
    });

    registrar("Arquivos corrompidos sao recusados sem modificar bytes", [&] {
        const std::string casos[] = {
            "", "OUTRA_VERSAO\n", "ACADEMIA_V1\nDesconhecido \"Ana\"\n",
            "ACADEMIA_V1\nMusculacao \"Ana\" \"11999991234\" -10 \"2026-01-01\" \"Saude\" \"Luisa\"\n",
            "ACADEMIA_V1\nFuncional \"Ana\" \"11999991234\" 10000 \"2026-02-30\" 3 \"Iniciante\"\n",
            "ACADEMIA_V1\nFuncional \"Ana\" \"11999991234\" 10000 \"2026-01-01\" 9 \"Iniciante\"\n",
            "ACADEMIA_V1\n" + MUSCULACAO + "registro incompleto\n",
            "ACADEMIA_V1\nFuncional \"Ana\" \"11999991234\" 10000 \"2026-01-01\" 3 \"Iniciante\" lixo\n"
        };
        const auto arquivo = pasta / "corrompido.txt";
        for (const auto& conteudo : casos) {
            gravar(arquivo, conteudo);
            std::istringstream entrada;
            std::ostringstream saida;
            exigirExcecao<std::runtime_error>([&] {
                GerenciadorAcademia cadastro(arquivo.string(), entrada, saida);
            }, "Arquivo corrompido foi aceito.");
            exigir(ler(arquivo) == conteudo, "Arquivo corrompido foi sobrescrito.");
        }
    });

    registrar("Falha de gravacao preserva destino e remove temporario", [&] {
        const auto arquivo = pasta / "falha" / "alunos.txt";
        std::istringstream entrada;
        std::ostringstream saida;
        GerenciadorAcademia cadastro(arquivo.string(), entrada, saida);
        // Um diretorio nao vazio no destino provoca uma falha real no rename,
        // inclusive quando os testes rodam com permissao de administrador.
        fs::create_directories(arquivo);
        gravar(arquivo / "original.txt", "conteudo que deve ser preservado");
        exigirExcecao<fs::filesystem_error>([&] { cadastro.salvar(); }, "Falha ao substituir destino deveria ser informada.");
        exigir(ler(arquivo / "original.txt") == "conteudo que deve ser preservado", "Destino anterior foi modificado.");
        for (const auto& item : fs::directory_iterator(arquivo.parent_path())) {
            exigir(item.path() == arquivo, "Arquivo temporario sobrou apos falha.");
        }
    });

    std::error_code erroLimpeza;
    fs::remove_all(pasta, erroLimpeza);
    if (erroLimpeza) {
        std::cerr << "Falha ao limpar arquivos temporarios: " << erroLimpeza.message() << '\n';
        return 1;
    }
    std::cout << '\n' << aprovados << " teste(s) aprovado(s), " << reprovados << " falha(s).\n";
    return reprovados == 0 ? 0 : 1;
}
