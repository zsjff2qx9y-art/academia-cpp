#pragma once

#include "Aluno.h"
#include <iomanip>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

class AlunoFuncional : public Aluno {
private:
    int aulasSemanais;
    std::string nivel;

public:
    AlunoFuncional(const std::string& nome, const std::string& telefone,
                   long long mensalidadeCentavos, const Data& dataMatricula,
                   int aulasSemanais, const std::string& nivel)
        : Aluno(nome, telefone, mensalidadeCentavos, dataMatricula),
          aulasSemanais(aulasSemanais), nivel(util::validarTexto(nivel)) {
        if (aulasSemanais < 1 || aulasSemanais > 7) {
            throw std::invalid_argument("Aulas semanais devem estar entre 1 e 7.");
        }
    }

    int getAulasSemanais() const { return aulasSemanais; }
    const std::string& getNivel() const { return nivel; }
    std::string getTipo() const override { return "Funcional"; }

    std::unique_ptr<Aluno> clonar() const override {
        return std::make_unique<AlunoFuncional>(*this);
    }

    void exibir(std::ostream& saida) const override {
        Aluno::exibir(saida);
        saida << "  Aulas semanais: " << aulasSemanais << '\n'
              << "  Nivel: " << nivel << '\n';
    }

    void editar(std::istream& entrada, std::ostream& saida) override {
        Aluno::editar(entrada, saida);
        const int novasAulas = util::lerInteiroOpcional(
            "  Aulas semanais", aulasSemanais, 1, 7, entrada, saida);
        const auto novoNivel = util::lerTextoOpcional(
            "  Nivel", nivel, entrada, saida);
        aulasSemanais = novasAulas;
        nivel = novoNivel;
    }

    std::string serializar() const override {
        std::ostringstream registro;
        registro << Aluno::serializar() << ' ' << aulasSemanais
                 << ' ' << std::quoted(nivel);
        return registro.str();
    }
};
