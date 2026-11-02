#pragma once

#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

// A data faz parte de cada aluno por composicao.
class Data {
private:
    int dia;
    int mes;
    int ano;

public:
    Data(int dia = 1, int mes = 1, int ano = 2026)
        : dia(dia), mes(mes), ano(ano) {
        if (ano < 1 || ano > 9999 || mes < 1 || mes > 12) {
            throw std::invalid_argument("Ano ou mes invalido.");
        }
        const bool bissexto = ano % 400 == 0 || (ano % 4 == 0 && ano % 100 != 0);
        const int diasNoMes[] = {31, bissexto ? 29 : 28, 31, 30, 31, 30,
                                 31, 31, 30, 31, 30, 31};
        if (dia < 1 || dia > diasNoMes[mes - 1]) {
            throw std::invalid_argument("Dia inexistente nesse mes.");
        }
    }

    int getDia() const { return dia; }
    int getMes() const { return mes; }
    int getAno() const { return ano; }

    static Data hoje() {
        const std::time_t agora = std::time(nullptr);
        const std::tm* local = std::localtime(&agora);
        if (local == nullptr) {
            throw std::runtime_error("Nao foi possivel consultar a data atual.");
        }
        return Data(local->tm_mday, local->tm_mon + 1, local->tm_year + 1900);
    }

    static Data deISO(const std::string& texto) {
        if (texto.size() != 10 || texto[4] != '-' || texto[7] != '-') {
            throw std::invalid_argument("Data invalida. Use AAAA-MM-DD.");
        }
        for (std::size_t i = 0; i < texto.size(); ++i) {
            if (i != 4 && i != 7 && (texto[i] < '0' || texto[i] > '9')) {
                throw std::invalid_argument("Data invalida. Use AAAA-MM-DD.");
            }
        }
        return Data(std::stoi(texto.substr(8, 2)), std::stoi(texto.substr(5, 2)),
                    std::stoi(texto.substr(0, 4)));
    }

    std::string toISO() const {
        std::ostringstream texto;
        texto << std::setfill('0') << std::setw(4) << ano << '-'
              << std::setw(2) << mes << '-' << std::setw(2) << dia;
        return texto.str();
    }

    std::string toString() const {
        std::ostringstream texto;
        texto << std::setfill('0') << std::setw(2) << dia << '/'
              << std::setw(2) << mes << '/' << std::setw(4) << ano;
        return texto.str();
    }
};
