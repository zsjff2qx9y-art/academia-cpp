#pragma once

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include "AlunoMusculacao.h"
#include "AlunoFuncional.h"
#include "Utils.h"

// O vetor guarda os objetos durante o uso. O arquivo so e acessado na
// abertura e no encerramento; nenhum CRUD altera o arquivo diretamente.
class GerenciadorAcademia {
private:
    std::vector<std::unique_ptr<Aluno>> alunos;
    std::string arquivo;
    std::istream& entrada;
    std::ostream& saida;

    static constexpr std::size_t LIMITE_ALUNOS = 100000;

    static std::unique_ptr<Aluno> criarDeLinha(const std::string& linha) {
        std::istringstream registro(linha);
        std::string tipo, nome, telefone, dataISO;
        long long mensalidade = 0;
        if (!(registro >> tipo >> std::quoted(nome) >> std::quoted(telefone)
              >> mensalidade >> std::quoted(dataISO))) {
            throw std::invalid_argument("campos comuns ausentes ou invalidos");
        }

        const Data data = Data::deISO(dataISO);
        std::unique_ptr<Aluno> aluno;
        if (tipo == "Musculacao") {
            std::string objetivo, instrutor;
            if (!(registro >> std::quoted(objetivo) >> std::quoted(instrutor))) {
                throw std::invalid_argument("dados de musculacao invalidos");
            }
            aluno = std::make_unique<AlunoMusculacao>(
                nome, telefone, mensalidade, data, objetivo, instrutor);
        } else if (tipo == "Funcional") {
            int aulasSemanais = 0;
            std::string nivel;
            if (!(registro >> aulasSemanais >> std::quoted(nivel))) {
                throw std::invalid_argument("dados de treinamento funcional invalidos");
            }
            aluno = std::make_unique<AlunoFuncional>(
                nome, telefone, mensalidade, data, aulasSemanais, nivel);
        } else {
            throw std::invalid_argument("modalidade desconhecida");
        }

        registro >> std::ws;
        if (!registro.eof()) {
            throw std::invalid_argument("conteudo extra no registro");
        }
        return aluno;
    }

    void carregar() {
        std::error_code erro;
        const bool existe = std::filesystem::exists(arquivo, erro);
        if (erro) {
            throw std::runtime_error("Nao foi possivel consultar o arquivo: " + erro.message());
        }
        if (!existe) return; // Primeira execucao: o cadastro comeca vazio.

        std::ifstream origem(arquivo);
        if (!origem) {
            throw std::runtime_error("Nao foi possivel abrir o arquivo para leitura.");
        }
        std::string linha;
        if (!std::getline(origem, linha) || util::trim(linha) != "ACADEMIA_V1") {
            throw std::runtime_error("Arquivo de alunos invalido na linha 1: cabecalho/versao.");
        }

        // So substitui o vetor quando TODOS os registros forem validos.
        // Um arquivo danificado interrompe a abertura e nao e sobrescrito.
        std::vector<std::unique_ptr<Aluno>> carregados;
        std::size_t numeroLinha = 1;
        while (std::getline(origem, linha)) {
            ++numeroLinha;
            if (util::trim(linha).empty()) continue;
            try {
                if (carregados.size() >= LIMITE_ALUNOS) {
                    throw std::invalid_argument("limite de alunos excedido");
                }
                carregados.push_back(criarDeLinha(linha));
            } catch (const std::exception& causa) {
                throw std::runtime_error("Arquivo de alunos invalido na linha " +
                    std::to_string(numeroLinha) + ": " + causa.what());
            }
        }
        if (origem.bad()) {
            throw std::runtime_error("Falha durante a leitura do arquivo de alunos.");
        }
        alunos.swap(carregados);
        saida << alunos.size() << " aluno(s) carregado(s).\n";
    }

    bool estaVazio() const {
        if (!alunos.empty()) return false;
        saida << "Nenhum aluno cadastrado.\n";
        return true;
    }

public:
    explicit GerenciadorAcademia(const std::string& arquivo = "alunos.txt",
                                std::istream& entrada = std::cin,
                                std::ostream& saida = std::cout)
        : arquivo(arquivo), entrada(entrada), saida(saida) {
        if (util::trim(arquivo).empty()) {
            throw std::invalid_argument("Informe o caminho do arquivo de alunos.");
        }
        carregar();
    }

    const std::vector<std::unique_ptr<Aluno>>& getAlunos() const {
        return alunos;
    }

    void salvar() const {
        const std::filesystem::path destino(arquivo);
        if (!destino.parent_path().empty()) {
            std::filesystem::create_directories(destino.parent_path());
        }

        // Escreve ao lado do destino antes de substitui-lo. Uma falha de
        // escrita mantem o arquivo anterior e e informada ao chamador.
        std::filesystem::path temporario = destino;
        temporario += ".tmp-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count());
        try {
            std::ofstream gravacao(temporario, std::ios::trunc);
            if (!gravacao) {
                throw std::runtime_error("Nao foi possivel criar o arquivo temporario.");
            }
            gravacao << "ACADEMIA_V1\n";
            for (const auto& aluno : alunos) gravacao << aluno->serializar() << '\n';
            gravacao.flush();
            if (!gravacao) {
                throw std::runtime_error("Falha ao gravar os alunos.");
            }
            gravacao.close();
            if (!gravacao) {
                throw std::runtime_error("Falha ao finalizar a gravacao dos alunos.");
            }
            std::filesystem::rename(temporario, destino);
        } catch (...) {
            std::error_code ignorado;
            std::filesystem::remove(temporario, ignorado);
            throw;
        }
        saida << alunos.size() << " aluno(s) salvo(s) em " << arquivo << ".\n";
    }

    // Nome completo ou prefixo, incluindo espacos. O retorno e o indice
    // atual do vetor (comeca em zero); -1 significa ausencia/cancelamento.
    int pesquisar(const std::string& nome) {
        const std::string alvo = util::minuscula(util::trim(nome));
        std::vector<int> encontrados;
        if (!alvo.empty()) {
            for (std::size_t i = 0; i < alunos.size(); ++i) {
                const auto candidato = util::minuscula(alunos[i]->getNome());
                if (candidato.compare(0, alvo.size(), alvo) == 0) {
                    encontrados.push_back(static_cast<int>(i));
                }
            }
        }
        if (encontrados.empty()) {
            saida << "Nenhum aluno encontrado.\n";
            return -1;
        }
        if (encontrados.size() == 1) return encontrados.front();

        saida << "Foram encontrados " << encontrados.size() << " alunos:\n";
        for (const int indice : encontrados) {
            saida << "\n[" << indice << "]\n";
            alunos[static_cast<std::size_t>(indice)]->exibir(saida);
        }
        while (true) {
            const int indice = util::lerInteiro(
                "Indice desejado (-1 cancela): ", -1,
                static_cast<int>(alunos.size()) - 1, entrada, saida);
            if (indice == -1) return -1;
            if (std::find(encontrados.begin(), encontrados.end(), indice) != encontrados.end()) {
                return indice;
            }
            saida << "Esse indice nao esta entre os resultados exibidos.\n";
        }
    }

    // 1. Inserir
    void inserir() {
        if (alunos.size() >= LIMITE_ALUNOS) {
            throw std::runtime_error("Limite de 100000 alunos atingido.");
        }
        saida << "\n-- Inserir aluno --\n1. Musculacao\n2. Treinamento funcional\n";
        const int tipo = util::lerInteiro("Modalidade: ", 1, 2, entrada, saida);
        const auto nome = util::lerTexto("Nome completo: ", entrada, saida);
        const auto telefone = util::lerTelefone("Telefone: ", entrada, saida);
        const auto mensalidade = util::lerCentavos("Mensalidade (R$): ", entrada, saida);
        const Data matricula = Data::hoje();

        std::unique_ptr<Aluno> aluno;
        if (tipo == 1) {
            const auto objetivo = util::lerTexto("Objetivo: ", entrada, saida);
            const auto instrutor = util::lerTexto("Instrutor: ", entrada, saida);
            aluno = std::make_unique<AlunoMusculacao>(
                nome, telefone, mensalidade, matricula, objetivo, instrutor);
        } else {
            const int aulas = util::lerInteiro("Aulas por semana (1 a 7): ", 1, 7, entrada, saida);
            const auto nivel = util::lerTexto("Nivel (ex.: iniciante): ", entrada, saida);
            aluno = std::make_unique<AlunoFuncional>(
                nome, telefone, mensalidade, matricula, aulas, nivel);
        }
        // Nenhum objeto incompleto entra no cadastro se ocorrer EOF.
        alunos.push_back(std::move(aluno));
        saida << "Aluno cadastrado!\n";
    }

    // 2. Listar todos
    void listarTodos() const {
        saida << "\n-- Todos os alunos --\n";
        if (estaVazio()) return;
        for (std::size_t i = 0; i < alunos.size(); ++i) {
            saida << "\n[" << i << "]\n";
            alunos[i]->exibir(saida); // Despacho virtual conforme a modalidade.
        }
    }

    // 3. Exibir um: reutiliza a mesma selecao de alterar e remover.
    void exibirUm() {
        if (estaVazio()) return;
        const int indice = pesquisar(util::lerTexto("Nome (ou inicio do nome): ", entrada, saida));
        if (indice >= 0) alunos[static_cast<std::size_t>(indice)]->exibir(saida);
    }

    // 4. Alterar
    void alterar() {
        if (estaVazio()) return;
        const int indice = pesquisar(util::lerTexto("Nome (ou inicio do nome): ", entrada, saida));
        if (indice < 0) return;
        const auto posicao = static_cast<std::size_t>(indice);
        saida << "Editando (Enter mantem o valor atual):\n";
        auto alterado = alunos[posicao]->clonar();
        alterado->editar(entrada, saida); // Sobrescrita na subclasse.
        // A copia evita uma edicao parcial quando a entrada termina.
        alunos[posicao] = std::move(alterado);
        saida << "Aluno alterado!\n";
    }

    // 5. Remover
    void remover() {
        if (estaVazio()) return;
        const int indice = pesquisar(util::lerTexto("Nome (ou inicio do nome): ", entrada, saida));
        if (indice < 0) return;
        alunos[static_cast<std::size_t>(indice)]->exibir(saida);
        const auto resposta = util::lerTexto("Confirmar remocao? (s/n): ", entrada, saida);
        if (util::minuscula(resposta) == "s") {
            alunos.erase(alunos.begin() + indice);
            saida << "Aluno removido!\n";
        } else {
            saida << "Remocao cancelada.\n";
        }
    }

    // 6. Relatorio: indicadores agregados, diferentes da listagem.
    void relatorio() const {
        long long total = 0;
        std::map<std::string, std::pair<std::size_t, long long>> porModalidade;
        porModalidade["Musculacao"] = {0, 0};
        porModalidade["Funcional"] = {0, 0};
        for (const auto& aluno : alunos) {
            total += aluno->getMensalidadeCentavos();
            auto& resumo = porModalidade[aluno->getTipo()];
            ++resumo.first;
            resumo.second += aluno->getMensalidadeCentavos();
        }
        const auto quantidade = static_cast<long long>(alunos.size());
        const long long media = quantidade == 0 ? 0 : (total + quantidade / 2) / quantidade;
        saida << "\n===== RELATORIO DA ACADEMIA =====\n"
              << "Alunos cadastrados: " << alunos.size() << '\n'
              << "Mensalidades previstas por mes: " << util::dinheiro(total) << '\n'
              << "Media das mensalidades: " << util::dinheiro(media) << '\n'
              << "Por modalidade:\n";
        for (const auto& modalidade : porModalidade) {
            saida << "  " << modalidade.first << ": " << modalidade.second.first
                  << " aluno(s), " << util::dinheiro(modalidade.second.second) << '\n';
        }
        saida << "Valores previstos com base nos cadastros; nao representam pagamentos recebidos.\n";
    }
};
