#pragma once

#include "Data.h"
#include "Utils.h"
#include <iomanip>
#include <istream>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>

// Superclasse abstrata: os dois tipos de aluno compartilham estes dados.
class Aluno {
private:
    std::string nome;
    std::string telefone;
    long long mensalidadeCentavos;
    Data dataMatricula;

public:
    Aluno(const std::string& nome, const std::string& telefone,
          long long mensalidadeCentavos, const Data& dataMatricula)
        : nome(util::validarTexto(nome)), telefone(util::validarTelefone(telefone)),
          mensalidadeCentavos(mensalidadeCentavos), dataMatricula(dataMatricula) {
        util::validarMensalidade(mensalidadeCentavos);
    }

    virtual ~Aluno() = default;

    const std::string& getNome() const { return nome; }
    const std::string& getTelefone() const { return telefone; }
    long long getMensalidadeCentavos() const { return mensalidadeCentavos; }
    const Data& getDataMatricula() const { return dataMatricula; }

    virtual std::string getTipo() const = 0;
    virtual std::unique_ptr<Aluno> clonar() const = 0;

    virtual void exibir(std::ostream& saida) const {
        saida << "  Tipo: " << getTipo() << '\n'
              << "  Nome: " << nome << '\n'
              << "  Telefone: " << telefone << '\n'
              << "  Mensalidade: " << util::dinheiro(mensalidadeCentavos) << '\n'
              << "  Matricula: " << dataMatricula.toString() << '\n';
    }

    virtual void editar(std::istream& entrada, std::ostream& saida) {
        // Os valores so sao aplicados depois que a leitura comum terminou.
        const auto novoNome = util::lerTextoOpcional("  Nome", nome, entrada, saida);
        const auto novoTelefone = util::lerTelefoneOpcional(
            "  Telefone", telefone, entrada, saida);
        const auto novaMensalidade = util::lerCentavosOpcional(
            "  Mensalidade", mensalidadeCentavos, entrada, saida);
        nome = novoNome;
        telefone = novoTelefone;
        mensalidadeCentavos = novaMensalidade;
    }

    // std::quoted preserva espacos, aspas, barras e outros textos do cadastro.
    virtual std::string serializar() const {
        std::ostringstream registro;
        registro << getTipo() << ' ' << std::quoted(nome) << ' '
                 << std::quoted(telefone) << ' ' << mensalidadeCentavos << ' '
                 << std::quoted(dataMatricula.toISO());
        return registro.str();
    }
};
