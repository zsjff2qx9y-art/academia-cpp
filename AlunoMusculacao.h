#pragma once

#include "Aluno.h"
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>

class AlunoMusculacao : public Aluno {
private:
    std::string objetivo;
    std::string instrutor;

public:
    AlunoMusculacao(const std::string& nome, const std::string& telefone,
                    long long mensalidadeCentavos, const Data& dataMatricula,
                    const std::string& objetivo, const std::string& instrutor)
        : Aluno(nome, telefone, mensalidadeCentavos, dataMatricula),
          objetivo(util::validarTexto(objetivo)),
          instrutor(util::validarTexto(instrutor)) {}

    const std::string& getObjetivo() const { return objetivo; }
    const std::string& getInstrutor() const { return instrutor; }
    std::string getTipo() const override { return "Musculacao"; }

    std::unique_ptr<Aluno> clonar() const override {
        return std::make_unique<AlunoMusculacao>(*this);
    }

    void exibir(std::ostream& saida) const override {
        Aluno::exibir(saida);
        saida << "  Objetivo: " << objetivo << '\n'
              << "  Instrutor: " << instrutor << '\n';
    }

    void editar(std::istream& entrada, std::ostream& saida) override {
        Aluno::editar(entrada, saida);
        const auto novoObjetivo = util::lerTextoOpcional(
            "  Objetivo", objetivo, entrada, saida);
        const auto novoInstrutor = util::lerTextoOpcional(
            "  Instrutor", instrutor, entrada, saida);
        objetivo = novoObjetivo;
        instrutor = novoInstrutor;
    }

    std::string serializar() const override {
        std::ostringstream registro;
        registro << Aluno::serializar() << ' ' << std::quoted(objetivo)
                 << ' ' << std::quoted(instrutor);
        return registro.str();
    }
};
